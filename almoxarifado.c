#include "almoxarifado.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* Cores ANSI para o terminal */
#define COR_RESET   "\033[0m"
#define COR_VERMELHO "\033[1;31m"
#define COR_VERDE    "\033[1;32m"
#define COR_AMARELO  "\033[1;33m"
#define COR_AZUL     "\033[1;34m"
#define COR_MAGENTA  "\033[1;35m"
#define COR_CIANO    "\033[1;36m"
#define COR_BRANCO   "\033[1;37m"
#define COR_CINZA    "\033[0;90m"

void almox_aparar_espacos(char *str) {
    if (!str) return;
    /* Remove marca de ordem de byte UTF-8 (BOM) se presente */
    unsigned char *u = (unsigned char *)str;
    if (u[0] == 0xEF && u[1] == 0xBB && u[2] == 0xBF) {
        memmove(str, str + 3, strlen(str) - 2);
    }
    char *inicio = str;
    while (*inicio && isspace((unsigned char)*inicio)) {
        inicio++;
    }
    char *fim = inicio + strlen(inicio);
    while (fim > inicio && isspace((unsigned char)*(fim - 1))) {
        fim--;
    }
    size_t tam = (size_t)(fim - inicio);
    memmove(str, inicio, tam);
    str[tam] = '\0';
}

int almox_strcasecmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);
    while (*s1 && *s2) {
        unsigned char c1 = (unsigned char)tolower((unsigned char)*s1);
        unsigned char c2 = (unsigned char)tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return (unsigned char)tolower((unsigned char)*s1) - (unsigned char)tolower((unsigned char)*s2);
}

void almox_obter_data_hora_atual(char *buffer, size_t tamanho) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    if (tm_info) {
        strftime(buffer, tamanho, "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        snprintf(buffer, tamanho, "0000-00-00 00:00:00");
    }
}

void almox_inicializar(Almoxarifado *almox, const char *caminho) {
    memset(almox, 0, sizeof(Almoxarifado));
    if (caminho && strlen(caminho) > 0) {
        strncpy(almox->caminho_arquivo, caminho, sizeof(almox->caminho_arquivo) - 1);
    } else {
        strncpy(almox->caminho_arquivo, ARQUIVO_PADRAO, sizeof(almox->caminho_arquivo) - 1);
    }

#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | 0x0004); /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
        }
    }
    SetConsoleOutputCP(65001); /* UTF-8 */
    SetConsoleCP(65001);
#endif

    almox_carregar_json(almox);
}

int almox_buscar_indice(const Almoxarifado *almox, const char *nome) {
    if (!almox || !nome) return -1;
    for (int i = 0; i < almox->total_itens; i++) {
        if (almox_strcasecmp(almox->itens[i].nome, nome) == 0) {
            return i;
        }
    }
    return -1;
}

static void extrair_valor_string_json(const char *bloco, const char *chave, char *destino, size_t dest_tam) {
    destino[0] = '\0';
    char busca[64];
    snprintf(busca, sizeof(busca), "\"%s\"", chave);
    const char *pos = strstr(bloco, busca);
    if (!pos) return;
    
    pos += strlen(busca);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t' || *pos == '\r' || *pos == '\n')) {
        pos++;
    }
    if (*pos == '\"') {
        pos++;
        size_t idx = 0;
        while (*pos && *pos != '\"' && idx + 1 < dest_tam) {
            if (*pos == '\\' && *(pos + 1)) {
                pos++;
            }
            destino[idx++] = *pos++;
        }
        destino[idx] = '\0';
    }
}

static int extrair_valor_int_json(const char *bloco, const char *chave, int *resultado) {
    char busca[64];
    snprintf(busca, sizeof(busca), "\"%s\"", chave);
    const char *pos = strstr(bloco, busca);
    if (!pos) return 0;

    pos += strlen(busca);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t' || *pos == '\r' || *pos == '\n')) {
        pos++;
    }
    if (*pos == '-' || isdigit((unsigned char)*pos)) {
        *resultado = atoi(pos);
        return 1;
    }
    return 0;
}

int almox_carregar_json(Almoxarifado *almox) {
    FILE *f = fopen(almox->caminho_arquivo, "rb");
    if (!f) {
        return 0; /* Arquivo não existe ainda */
    }

    fseek(f, 0, SEEK_END);
    long tamanho = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (tamanho <= 0 || tamanho > 10 * 1024 * 1024) {
        fclose(f);
        return 0;
    }

    char *conteudo = (char *)malloc((size_t)tamanho + 1);
    if (!conteudo) {
        fclose(f);
        return 0;
    }

    size_t lidos = fread(conteudo, 1, (size_t)tamanho, f);
    conteudo[lidos] = '\0';
    fclose(f);

    almox->total_itens = 0;
    almox->total_historico = 0;

    /* 1. Parse dos itens */
    const char *sec_itens = strstr(conteudo, "\"itens\"");
    const char *sec_hist = strstr(conteudo, "\"historico\"");

    if (sec_itens) {
        const char *p = sec_itens;
        const char *fim_itens = sec_hist ? sec_hist : (conteudo + tamanho);
        while (p < fim_itens) {
            const char *obj_ini = strchr(p, '{');
            if (!obj_ini || obj_ini >= fim_itens) break;
            const char *obj_fim = strchr(obj_ini, '}');
            if (!obj_fim || obj_fim >= fim_itens) break;

            size_t bloco_tam = (size_t)(obj_fim - obj_ini + 1);
            char bloco[512];
            if (bloco_tam < sizeof(bloco)) {
                memcpy(bloco, obj_ini, bloco_tam);
                bloco[bloco_tam] = '\0';

                char nome[MAX_NOME];
                int qtd = 0;
                extrair_valor_string_json(bloco, "nome", nome, sizeof(nome));
                if (extrair_valor_int_json(bloco, "quantidade", &qtd) && strlen(nome) > 0) {
                    if (almox->total_itens < MAX_ITENS) {
                        snprintf(almox->itens[almox->total_itens].nome, sizeof(almox->itens[almox->total_itens].nome), "%s", nome);
                        almox->itens[almox->total_itens].quantidade = qtd;
                        almox->total_itens++;
                    }
                }
            }
            p = obj_fim + 1;
        }
    }

    /* 2. Parse do histórico */
    if (sec_hist) {
        const char *p = sec_hist;
        while (*p) {
            const char *obj_ini = strchr(p, '{');
            if (!obj_ini) break;
            const char *obj_fim = strchr(obj_ini, '}');
            if (!obj_fim) break;

            size_t bloco_tam = (size_t)(obj_fim - obj_ini + 1);
            char bloco[1024];
            if (bloco_tam < sizeof(bloco)) {
                memcpy(bloco, obj_ini, bloco_tam);
                bloco[bloco_tam] = '\0';

                RegistroHistorico reg;
                memset(&reg, 0, sizeof(reg));
                extrair_valor_string_json(bloco, "data_hora", reg.data_hora, sizeof(reg.data_hora));
                extrair_valor_string_json(bloco, "operacao", reg.operacao, sizeof(reg.operacao));
                extrair_valor_string_json(bloco, "nome", reg.nome, sizeof(reg.nome));
                extrair_valor_int_json(bloco, "quantidade", &reg.quantidade);
                extrair_valor_int_json(bloco, "saldo_anterior", &reg.saldo_anterior);
                extrair_valor_int_json(bloco, "saldo_novo", &reg.saldo_novo);

                if (strlen(reg.nome) > 0 && strlen(reg.data_hora) > 0) {
                    if (almox->total_historico < MAX_HISTORICO) {
                        almox->historico[almox->total_historico++] = reg;
                    }
                }
            }
            p = obj_fim + 1;
        }
    }

    free(conteudo);
    return 1;
}

int almox_salvar_json(const Almoxarifado *almox) {
    FILE *f = fopen(almox->caminho_arquivo, "wb");
    if (!f) {
        fprintf(stderr, COR_VERMELHO "Erro ao abrir '%s' para salvar o estoque.\n" COR_RESET, almox->caminho_arquivo);
        return 0;
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"itens\": [\n");
    for (int i = 0; i < almox->total_itens; i++) {
        fprintf(f, "    {\n");
        fprintf(f, "      \"nome\": \"%s\",\n", almox->itens[i].nome);
        fprintf(f, "      \"quantidade\": %d\n", almox->itens[i].quantidade);
        fprintf(f, "    }%s\n", (i == almox->total_itens - 1) ? "" : ",");
    }
    fprintf(f, "  ],\n");

    fprintf(f, "  \"historico\": [\n");
    for (int i = 0; i < almox->total_historico; i++) {
        const RegistroHistorico *h = &almox->historico[i];
        fprintf(f, "    {\n");
        fprintf(f, "      \"data_hora\": \"%s\",\n", h->data_hora);
        fprintf(f, "      \"operacao\": \"%s\",\n", h->operacao);
        fprintf(f, "      \"nome\": \"%s\",\n", h->nome);
        fprintf(f, "      \"quantidade\": %d,\n", h->quantidade);
        fprintf(f, "      \"saldo_anterior\": %d,\n", h->saldo_anterior);
        fprintf(f, "      \"saldo_novo\": %d\n", h->saldo_novo);
        fprintf(f, "    }%s\n", (i == almox->total_historico - 1) ? "" : ",");
    }
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");

    fclose(f);
    return 1;
}

static void registrar_movimentacao(Almoxarifado *almox, const char *op, const char *nome, int qtd, int anterior, int novo) {
    if (almox->total_historico >= MAX_HISTORICO) {
        /* Desloca 1 posição para a esquerda para manter os mais recentes */
        memmove(&almox->historico[0], &almox->historico[1], sizeof(RegistroHistorico) * (MAX_HISTORICO - 1));
        almox->total_historico = MAX_HISTORICO - 1;
    }
    RegistroHistorico *r = &almox->historico[almox->total_historico++];
    almox_obter_data_hora_atual(r->data_hora, sizeof(r->data_hora));
    strncpy(r->operacao, op, sizeof(r->operacao) - 1);
    strncpy(r->nome, nome, sizeof(r->nome) - 1);
    r->quantidade = qtd;
    r->saldo_anterior = anterior;
    r->saldo_novo = novo;
}

int almox_adicionar(Almoxarifado *almox, const char *nome_bruto, int quantidade) {
    char nome[MAX_NOME];
    strncpy(nome, nome_bruto, sizeof(nome) - 1);
    nome[sizeof(nome) - 1] = '\0';
    almox_aparar_espacos(nome);

    if (strlen(nome) == 0) {
        printf(COR_VERMELHO "Erro: Nome do item não pode ser vazio.\n" COR_RESET);
        return 0;
    }
    if (quantidade <= 0) {
        printf(COR_VERMELHO "Erro: Quantidade para adicionar deve ser maior que zero (recebido: %d).\n" COR_RESET, quantidade);
        return 0;
    }

    int idx = almox_buscar_indice(almox, nome);
    int saldo_ant = 0;
    int saldo_novo = 0;

    if (idx >= 0) {
        /* Item já existe -> soma as quantidades */
        saldo_ant = almox->itens[idx].quantidade;
        almox->itens[idx].quantidade += quantidade;
        saldo_novo = almox->itens[idx].quantidade;
        printf(COR_VERDE "[SUCESSO] Item atualizado: '%s'\n" COR_RESET, almox->itens[idx].nome);
        printf("          Quantidade adicionada: " COR_VERDE "+%d" COR_RESET "\n", quantidade);
        printf("          Saldo anterior: %d | " COR_CIANO "Novo saldo: %d" COR_RESET "\n", saldo_ant, saldo_novo);
    } else {
        /* Novo item */
        if (almox->total_itens >= MAX_ITENS) {
            printf(COR_VERMELHO "Erro: Capacidade máxima de itens atingida (%d).\n" COR_RESET, MAX_ITENS);
            return 0;
        }
        idx = almox->total_itens++;
        snprintf(almox->itens[idx].nome, sizeof(almox->itens[idx].nome), "%s", nome);
        almox->itens[idx].quantidade = quantidade;
        saldo_ant = 0;
        saldo_novo = quantidade;
        printf(COR_VERDE "[SUCESSO] Novo item cadastrado: '%s'\n" COR_RESET, almox->itens[idx].nome);
        printf("          Quantidade inicial: " COR_CIANO "%d" COR_RESET "\n", saldo_novo);
    }

    registrar_movimentacao(almox, "ADICIONAR", almox->itens[idx].nome, quantidade, saldo_ant, saldo_novo);
    almox_salvar_json(almox);
    return 1;
}

int almox_remover(Almoxarifado *almox, const char *nome_bruto, int quantidade) {
    char nome[MAX_NOME];
    strncpy(nome, nome_bruto, sizeof(nome) - 1);
    nome[sizeof(nome) - 1] = '\0';
    almox_aparar_espacos(nome);

    if (strlen(nome) == 0) {
        printf(COR_VERMELHO "Erro: Nome do item não pode ser vazio.\n" COR_RESET);
        return 0;
    }
    if (quantidade <= 0) {
        printf(COR_VERMELHO "Erro: Quantidade para remover deve ser maior que zero (recebido: %d).\n" COR_RESET, quantidade);
        return 0;
    }

    int idx = almox_buscar_indice(almox, nome);
    if (idx < 0) {
        printf(COR_VERMELHO "Erro: Item '%s' não encontrado no almoxarifado.\n" COR_RESET, nome);
        return 0;
    }

    int saldo_ant = almox->itens[idx].quantidade;
    if (saldo_ant < quantidade) {
        printf(COR_AMARELO "[AVISO] Saldo insuficiente para remover %d unidade(s) de '%s'.\n" COR_RESET, quantidade, almox->itens[idx].nome);
        printf("        Saldo atual disponível: " COR_CIANO "%d" COR_RESET "\n", saldo_ant);
        return 0;
    }

    almox->itens[idx].quantidade -= quantidade;
    int saldo_novo = almox->itens[idx].quantidade;

    printf(COR_VERDE "[SUCESSO] Baixa realizada no item: '%s'\n" COR_RESET, almox->itens[idx].nome);
    printf("          Quantidade removida: " COR_VERMELHO "-%d" COR_RESET "\n", quantidade);
    printf("          Saldo anterior: %d | " COR_CIANO "Novo saldo: %d" COR_RESET "\n", saldo_ant, saldo_novo);

    registrar_movimentacao(almox, "REMOVER", almox->itens[idx].nome, quantidade, saldo_ant, saldo_novo);
    almox_salvar_json(almox);
    return 1;
}

void almox_listar(const Almoxarifado *almox) {
    printf("\n" COR_CIANO "========================================================================\n");
    printf("                     ESTOQUE DO ALMOXARIFADO                           \n");
    printf("========================================================================" COR_RESET "\n");

    if (almox->total_itens == 0) {
        printf(COR_CINZA "  Nenhum item cadastrado no momento.\n" COR_RESET);
        printf("  Dica: Use " COR_VERDE "adicionar <item> <quantidade>" COR_RESET " para registrar novos itens.\n");
        printf("------------------------------------------------------------------------\n\n");
        return;
    }

    printf(COR_BRANCO "%-5s | %-40s | %-12s | %-10s\n" COR_RESET, "No.", "DESCRIÇÃO DO ITEM", "QUANTIDADE", "STATUS");
    printf("------------------------------------------------------------------------\n");

    int soma_total_unidades = 0;
    for (int i = 0; i < almox->total_itens; i++) {
        const ItemEstoque *item = &almox->itens[i];
        soma_total_unidades += item->quantidade;

        const char *status_str = "OK";
        const char *cor_status = COR_VERDE;
        if (item->quantidade == 0) {
            status_str = "ZERADO";
            cor_status = COR_VERMELHO;
        } else if (item->quantidade <= 5) {
            status_str = "BAIXO";
            cor_status = COR_AMARELO;
        }

        printf("%-5d | %-40s | %10d   | %s%-10s" COR_RESET "\n",
               i + 1, item->nome, item->quantidade, cor_status, status_str);
    }

    printf("------------------------------------------------------------------------\n");
    printf("Total de itens cadastrados: " COR_CIANO "%d" COR_RESET " | Total de unidades: " COR_CIANO "%d" COR_RESET "\n\n",
           almox->total_itens, soma_total_unidades);
}

void almox_consultar(const Almoxarifado *almox, const char *termo_bruto) {
    char termo[MAX_NOME];
    strncpy(termo, termo_bruto, sizeof(termo) - 1);
    termo[sizeof(termo) - 1] = '\0';
    almox_aparar_espacos(termo);

    if (strlen(termo) == 0) {
        printf(COR_VERMELHO "Erro: Digite o nome do item a consultar.\n" COR_RESET);
        return;
    }

    printf("\n" COR_CIANO "Buscando por '%s':\n" COR_RESET, termo);
    int encontrados = 0;

    for (int i = 0; i < almox->total_itens; i++) {
        /* Busca parcial insensível a maiúsculas */
        char nome_min[MAX_NOME];
        char termo_min[MAX_NOME];
        for (int k = 0; almox->itens[i].nome[k] && k < MAX_NOME - 1; k++) {
            nome_min[k] = (char)tolower((unsigned char)almox->itens[i].nome[k]);
            nome_min[k + 1] = '\0';
        }
        for (int k = 0; termo[k] && k < MAX_NOME - 1; k++) {
            termo_min[k] = (char)tolower((unsigned char)termo[k]);
            termo_min[k + 1] = '\0';
        }

        if (strstr(nome_min, termo_min) != NULL) {
            printf("  -> [%d] " COR_BRANCO "%-35s" COR_RESET " | Saldo: " COR_CIANO "%d" COR_RESET " unidades\n",
                   i + 1, almox->itens[i].nome, almox->itens[i].quantidade);
            encontrados++;
        }
    }

    if (encontrados == 0) {
        printf(COR_AMARELO "  Nenhum item correspondente a '%s' foi encontrado.\n" COR_RESET, termo);
    }
    printf("\n");
}

void almox_exibir_historico(const Almoxarifado *almox, int limite) {
    printf("\n" COR_CIANO "========================================================================================\n");
    printf("                           HISTÓRICO DE MOVIMENTAÇÕES                                   \n");
    printf("========================================================================================" COR_RESET "\n");

    if (almox->total_historico == 0) {
        printf(COR_CINZA "  Nenhuma movimentação registrada até o momento.\n" COR_RESET);
        printf("----------------------------------------------------------------------------------------\n\n");
        return;
    }

    if (limite <= 0 || limite > almox->total_historico) {
        limite = almox->total_historico;
    }

    printf(COR_BRANCO "%-19s | %-10s | %-30s | %-6s | %-7s | %-7s\n" COR_RESET,
           "DATA / HORA", "TIPO", "ITEM", "QTD", "ANT.", "NOVO");
    printf("----------------------------------------------------------------------------------------\n");

    int inicio = almox->total_historico - limite;
    for (int i = almox->total_historico - 1; i >= inicio; i--) {
        const RegistroHistorico *h = &almox->historico[i];
        const char *cor_tipo = strcmp(h->operacao, "ADICIONAR") == 0 ? COR_VERDE : COR_VERMELHO;

        printf("%-19s | %s%-10s" COR_RESET " | %-30s | %6d | %7d | %s%7d" COR_RESET "\n",
               h->data_hora, cor_tipo, h->operacao, h->nome, h->quantidade, h->saldo_anterior,
               COR_CIANO, h->saldo_novo);
    }
    printf("----------------------------------------------------------------------------------------\n");
    printf("Exibindo últimas %d movimentações de um total de %d.\n\n", limite, almox->total_historico);
}
