#include "almoxarifado.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* Cores ANSI para o terminal */
#define COR_RESET    "\033[0m"
#define COR_VERMELHO "\033[1;31m"
#define COR_VERDE    "\033[1;32m"
#define COR_AMARELO  "\033[1;33m"
#define COR_AZUL     "\033[1;34m"
#define COR_MAGENTA  "\033[1;35m"
#define COR_CIANO    "\033[1;36m"
#define COR_BRANCO   "\033[1;37m"
#define COR_CINZA    "\033[0;90m"

/* ======================================================================
 * Utilitários
 * ====================================================================== */

void almox_aparar_espacos(char *str) {
    if (!str) return;
    /* Remove BOM UTF-8 se presente */
    unsigned char *u = (unsigned char *)str;
    if (u[0] == 0xEF && u[1] == 0xBB && u[2] == 0xBF) {
        memmove(str, str + 3, strlen(str) - 2);
    }
    char *inicio = str;
    while (*inicio && isspace((unsigned char)*inicio)) inicio++;
    char *fim = inicio + strlen(inicio);
    while (fim > inicio && isspace((unsigned char)*(fim - 1))) fim--;
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
        s1++; s2++;
    }
    return (unsigned char)tolower((unsigned char)*s1) -
           (unsigned char)tolower((unsigned char)*s2);
}

void almox_obter_data_hora_atual(char *buffer, size_t tamanho) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    if (tm_info) strftime(buffer, tamanho, "%Y-%m-%d %H:%M:%S", tm_info);
    else         snprintf(buffer, tamanho, "0000-00-00 00:00:00");
}

void almox_obter_data_atual(char *buffer, size_t tamanho) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    if (tm_info) strftime(buffer, tamanho, "%Y-%m-%d", tm_info);
    else         snprintf(buffer, tamanho, "0000-00-00");
}

/* ======================================================================
 * Inicialização
 * ====================================================================== */

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

/* ======================================================================
 * Busca por índice (inclui inativos — necessário para reativação)
 * ====================================================================== */

int almox_buscar_indice(const Almoxarifado *almox, const char *nome) {
    if (!almox || !nome) return -1;
    for (int i = 0; i < almox->total_itens; i++) {
        if (almox_strcasecmp(almox->itens[i].nome, nome) == 0) return i;
    }
    return -1;
}

/* ======================================================================
 * Helpers de parsing JSON (sem dependências externas)
 * ====================================================================== */

static void extrair_string_json(const char *bloco, const char *chave,
                                char *destino, size_t dest_tam) {
    destino[0] = '\0';
    char busca[64];
    snprintf(busca, sizeof(busca), "\"%s\"", chave);
    const char *pos = strstr(bloco, busca);
    if (!pos) return;
    pos += strlen(busca);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t' ||
                    *pos == '\r' || *pos == '\n')) pos++;
    if (*pos == '"') {
        pos++;
        size_t idx = 0;
        while (*pos && *pos != '"' && idx + 1 < dest_tam) {
            if (*pos == '\\' && *(pos + 1)) pos++;
            destino[idx++] = *pos++;
        }
        destino[idx] = '\0';
    }
}

static int extrair_int_json(const char *bloco, const char *chave, int *resultado) {
    char busca[64];
    snprintf(busca, sizeof(busca), "\"%s\"", chave);
    const char *pos = strstr(bloco, busca);
    if (!pos) return 0;
    pos += strlen(busca);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t' ||
                    *pos == '\r' || *pos == '\n')) pos++;
    if (*pos == '-' || isdigit((unsigned char)*pos)) {
        *resultado = atoi(pos);
        return 1;
    }
    return 0;
}

static int extrair_float_json(const char *bloco, const char *chave, double *resultado) {
    char busca[64];
    snprintf(busca, sizeof(busca), "\"%s\"", chave);
    const char *pos = strstr(bloco, busca);
    if (!pos) return 0;
    pos += strlen(busca);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t' ||
                    *pos == '\r' || *pos == '\n')) pos++;
    if (*pos == '-' || isdigit((unsigned char)*pos)) {
        *resultado = atof(pos);
        return 1;
    }
    return 0;
}

/* Suporta "true"/"false" (JSON) e 1/0 (compatibilidade) */
static int extrair_bool_json(const char *bloco, const char *chave, int *resultado) {
    char busca[64];
    snprintf(busca, sizeof(busca), "\"%s\"", chave);
    const char *pos = strstr(bloco, busca);
    if (!pos) return 0;
    pos += strlen(busca);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t' ||
                    *pos == '\r' || *pos == '\n')) pos++;
    if (strncmp(pos, "true",  4) == 0) { *resultado = 1; return 1; }
    if (strncmp(pos, "false", 5) == 0) { *resultado = 0; return 1; }
    if (*pos == '1')                   { *resultado = 1; return 1; }
    if (*pos == '0')                   { *resultado = 0; return 1; }
    return 0;
}

/* ======================================================================
 * Carregar JSON
 * Compatível com arquivos antigos (só nome+quantidade) e novos (schema completo).
 * ====================================================================== */

int almox_carregar_json(Almoxarifado *almox) {
    FILE *f = fopen(almox->caminho_arquivo, "rb");
    if (!f) return 0; /* Arquivo ainda não existe — primeira execução */

    fseek(f, 0, SEEK_END);
    long tamanho = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (tamanho <= 0 || tamanho > 10 * 1024 * 1024) {
        fclose(f);
        return 0;
    }

    char *conteudo = (char *)malloc((size_t)tamanho + 1);
    if (!conteudo) { fclose(f); return 0; }

    size_t lidos = fread(conteudo, 1, (size_t)tamanho, f);
    conteudo[lidos] = '\0';
    fclose(f);

    almox->total_itens     = 0;
    almox->total_historico = 0;
    almox->proximo_codigo  = 1;

    const char *sec_itens = strstr(conteudo, "\"itens\"");
    const char *sec_hist  = strstr(conteudo, "\"historico\"");

    /* --- 1. Parse dos itens ------------------------------------------ */
    if (sec_itens) {
        const char *p = sec_itens;
        const char *fim_itens = sec_hist ? sec_hist : (conteudo + tamanho);
        while (p < fim_itens) {
            const char *obj_ini = strchr(p, '{');
            if (!obj_ini || obj_ini >= fim_itens) break;
            const char *obj_fim = strchr(obj_ini, '}');
            if (!obj_fim || obj_fim >= fim_itens) break;

            size_t bloco_tam = (size_t)(obj_fim - obj_ini + 1);
            char bloco[600];
            if (bloco_tam < sizeof(bloco)) {
                memcpy(bloco, obj_ini, bloco_tam);
                bloco[bloco_tam] = '\0';

                char   nome[MAX_NOME]  = "";
                char   data[25]        = "";
                int    qtd             = 0;
                int    codigo          = 0;
                double valorUnit       = 0.0;
                int    ativo           = 1; /* default: ativo */

                extrair_string_json(bloco, "nome",         nome,     sizeof(nome));
                extrair_string_json(bloco, "dataEntrada",  data,     sizeof(data));
                extrair_int_json   (bloco, "quantidade",  &qtd);
                extrair_int_json   (bloco, "codigo",      &codigo);
                extrair_float_json (bloco, "valorUnitario",&valorUnit);
                extrair_bool_json  (bloco, "ativo",       &ativo);

                if (strlen(nome) > 0 && almox->total_itens < MAX_ITENS) {
                    ItemEstoque *it = &almox->itens[almox->total_itens++];
                    snprintf(it->nome, sizeof(it->nome), "%s", nome);
                    snprintf(it->dataEntrada, sizeof(it->dataEntrada), "%s", data);
                    it->quantidade    = qtd;
                    it->valorUnitario = valorUnit;
                    it->ativo         = ativo;

                    /* Atribui código: usa o gravado ou gera um novo */
                    if (codigo > 0) {
                        it->codigo = codigo;
                        if (codigo >= almox->proximo_codigo)
                            almox->proximo_codigo = codigo + 1;
                    } else {
                        it->codigo = almox->proximo_codigo++;
                    }
                }
            }
            p = obj_fim + 1;
        }
    }

    /* --- 2. Parse do histórico --------------------------------------- */
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
                extrair_string_json(bloco, "data_hora",       reg.data_hora,  sizeof(reg.data_hora));
                extrair_string_json(bloco, "operacao",        reg.operacao,   sizeof(reg.operacao));
                extrair_string_json(bloco, "nome",            reg.nome,       sizeof(reg.nome));
                extrair_int_json   (bloco, "quantidade",     &reg.quantidade);
                extrair_int_json   (bloco, "saldo_anterior", &reg.saldo_anterior);
                extrair_int_json   (bloco, "saldo_novo",     &reg.saldo_novo);

                if (strlen(reg.nome) > 0 && strlen(reg.data_hora) > 0 &&
                    almox->total_historico < MAX_HISTORICO) {
                    almox->historico[almox->total_historico++] = reg;
                }
            }
            p = obj_fim + 1;
        }
    }

    free(conteudo);
    return 1;
}

/* ======================================================================
 * Salvar JSON
 * Formato espelhado do schema MongoDB do arquivo almoxarifadopim.mongodb.js
 * ====================================================================== */

int almox_salvar_json(const Almoxarifado *almox) {
    FILE *f = fopen(almox->caminho_arquivo, "wb");
    if (!f) {
        fprintf(stderr, COR_VERMELHO "Erro ao abrir '%s' para salvar.\n" COR_RESET,
                almox->caminho_arquivo);
        return 0;
    }

    /* --- itens -------------------------------------------------------- */
    fprintf(f, "{\n");
    fprintf(f, "  \"itens\": [\n");
    for (int i = 0; i < almox->total_itens; i++) {
        const ItemEstoque *it = &almox->itens[i];
        fprintf(f, "    {\n");
        fprintf(f, "      \"codigo\": %d,\n",           it->codigo);
        fprintf(f, "      \"nome\": \"%s\",\n",         it->nome);
        fprintf(f, "      \"dataEntrada\": \"%s\",\n",  it->dataEntrada);
        fprintf(f, "      \"valorUnitario\": %.2f,\n",  it->valorUnitario);
        fprintf(f, "      \"quantidade\": %d,\n",       it->quantidade);
        fprintf(f, "      \"ativo\": %s\n",             it->ativo ? "true" : "false");
        fprintf(f, "    }%s\n", (i == almox->total_itens - 1) ? "" : ",");
    }
    fprintf(f, "  ],\n");

    /* --- historico ---------------------------------------------------- */
    fprintf(f, "  \"historico\": [\n");
    for (int i = 0; i < almox->total_historico; i++) {
        const RegistroHistorico *h = &almox->historico[i];
        fprintf(f, "    {\n");
        fprintf(f, "      \"data_hora\": \"%s\",\n",      h->data_hora);
        fprintf(f, "      \"operacao\": \"%s\",\n",       h->operacao);
        fprintf(f, "      \"nome\": \"%s\",\n",           h->nome);
        fprintf(f, "      \"quantidade\": %d,\n",         h->quantidade);
        fprintf(f, "      \"saldo_anterior\": %d,\n",     h->saldo_anterior);
        fprintf(f, "      \"saldo_novo\": %d\n",          h->saldo_novo);
        fprintf(f, "    }%s\n", (i == almox->total_historico - 1) ? "" : ",");
    }
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");

    fclose(f);
    return 1;
}

/* ======================================================================
 * Registrar movimentação no histórico
 * ====================================================================== */

static void registrar_movimentacao(Almoxarifado *almox, const char *op,
                                   const char *nome, int qtd,
                                   int anterior, int novo) {
    if (almox->total_historico >= MAX_HISTORICO) {
        /* Descarta o mais antigo para manter os mais recentes */
        memmove(&almox->historico[0], &almox->historico[1],
                sizeof(RegistroHistorico) * (MAX_HISTORICO - 1));
        almox->total_historico = MAX_HISTORICO - 1;
    }
    RegistroHistorico *r = &almox->historico[almox->total_historico++];
    almox_obter_data_hora_atual(r->data_hora, sizeof(r->data_hora));
    strncpy(r->operacao, op,   sizeof(r->operacao) - 1);
    strncpy(r->nome,     nome, sizeof(r->nome)     - 1);
    r->quantidade     = qtd;
    r->saldo_anterior = anterior;
    r->saldo_novo     = novo;
}

/* ======================================================================
 * ADICIONAR — cria item novo ou atualiza quantidade de item existente.
 * Equivalente ao insertOne / updateOne($inc) do MongoDB.
 * Se o item existia mas estava inativo, ele é reativado automaticamente.
 * ====================================================================== */

int almox_adicionar(Almoxarifado *almox, const char *nome_bruto,
                    int quantidade, double valorUnitario) {
    char nome[MAX_NOME];
    strncpy(nome, nome_bruto, sizeof(nome) - 1);
    nome[sizeof(nome) - 1] = '\0';
    almox_aparar_espacos(nome);

    if (strlen(nome) == 0) {
        printf(COR_VERMELHO "Erro: Nome do item não pode ser vazio.\n" COR_RESET);
        return 0;
    }
    if (quantidade <= 0) {
        printf(COR_VERMELHO "Erro: Quantidade deve ser maior que zero (recebido: %d).\n" COR_RESET, quantidade);
        return 0;
    }
    if (valorUnitario < 0.0) {
        printf(COR_VERMELHO "Erro: Valor unitário não pode ser negativo.\n" COR_RESET);
        return 0;
    }

    int idx = almox_buscar_indice(almox, nome);
    int saldo_ant = 0, saldo_novo = 0;

    if (idx >= 0) {
        /* Item já existe: soma quantidade */
        saldo_ant = almox->itens[idx].quantidade;
        almox->itens[idx].quantidade += quantidade;
        saldo_novo = almox->itens[idx].quantidade;

        /* Reativa se estava inativo (equivale a {$set: {ativo: true}}) */
        if (!almox->itens[idx].ativo) {
            almox->itens[idx].ativo = 1;
            printf(COR_AMARELO "[INFO] Item reativado: '%s'\n" COR_RESET, almox->itens[idx].nome);
        }

        /* Atualiza valor unitário se um novo foi informado */
        if (valorUnitario > 0.0)
            almox->itens[idx].valorUnitario = valorUnitario;

        printf(COR_VERDE "[SUCESSO] Item atualizado: '%s' (Cód. %03d)\n" COR_RESET,
               almox->itens[idx].nome, almox->itens[idx].codigo);
        printf("          Adicionado       : " COR_VERDE "+%d unidade(s)" COR_RESET "\n", quantidade);
        printf("          Saldo anterior   : %d  >>  " COR_CIANO "Novo saldo: %d" COR_RESET "\n",
               saldo_ant, saldo_novo);
        printf("          Valor unitário   : " COR_CIANO "R$ %.2f" COR_RESET "\n",
               almox->itens[idx].valorUnitario);
    } else {
        /* Novo item (equivale ao insertOne do MongoDB) */
        if (almox->total_itens >= MAX_ITENS) {
            printf(COR_VERMELHO "Erro: Capacidade máxima atingida (%d itens).\n" COR_RESET, MAX_ITENS);
            return 0;
        }
        idx = almox->total_itens++;
        ItemEstoque *it = &almox->itens[idx];
        it->codigo        = almox->proximo_codigo++;
        snprintf(it->nome, sizeof(it->nome), "%s", nome);
        almox_obter_data_atual(it->dataEntrada, sizeof(it->dataEntrada));
        it->valorUnitario = valorUnitario;
        it->quantidade    = quantidade;
        it->ativo         = 1;
        saldo_ant         = 0;
        saldo_novo        = quantidade;

        printf(COR_VERDE "[SUCESSO] Novo item cadastrado: '%s' (Cód. %03d)\n" COR_RESET,
               it->nome, it->codigo);
        printf("          Quantidade inicial: " COR_CIANO "%d unidade(s)" COR_RESET "\n", saldo_novo);
        printf("          Valor unitário    : " COR_CIANO "R$ %.2f" COR_RESET "\n", it->valorUnitario);
        printf("          Data de entrada   : " COR_CIANO "%s" COR_RESET "\n", it->dataEntrada);
    }

    registrar_movimentacao(almox, "ADICIONAR", almox->itens[idx].nome,
                           quantidade, saldo_ant, saldo_novo);
    almox_salvar_json(almox);
    return 1;
}

/* ======================================================================
 * REMOVER — baixa de estoque com proteção de saldo mínimo.
 * Equivalente ao updateOne({quantidade: {$gte: N}}, {$inc: {quantidade: -N}})
 * ====================================================================== */

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
        printf(COR_VERMELHO "Erro: Quantidade deve ser maior que zero (recebido: %d).\n" COR_RESET, quantidade);
        return 0;
    }

    int idx = almox_buscar_indice(almox, nome);
    if (idx < 0) {
        printf(COR_VERMELHO "Erro: Item '%s' não encontrado.\n" COR_RESET, nome);
        return 0;
    }
    if (!almox->itens[idx].ativo) {
        printf(COR_AMARELO "[AVISO] Item '%s' está inativo. Use 'adicionar' para reativá-lo.\n" COR_RESET,
               almox->itens[idx].nome);
        return 0;
    }

    int saldo_ant = almox->itens[idx].quantidade;
    /* Proteção: não permite retirada maior que o saldo (matchedCount 0 no Mongo) */
    if (saldo_ant < quantidade) {
        printf(COR_AMARELO "[AVISO] Saldo insuficiente: '%s' tem apenas %d unidade(s) disponível(is).\n" COR_RESET,
               almox->itens[idx].nome, saldo_ant);
        return 0;
    }

    almox->itens[idx].quantidade -= quantidade;
    int saldo_novo = almox->itens[idx].quantidade;

    printf(COR_VERDE "[SUCESSO] Baixa realizada: '%s' (Cód. %03d)\n" COR_RESET,
           almox->itens[idx].nome, almox->itens[idx].codigo);
    printf("          Removido         : " COR_VERMELHO "-%d unidade(s)" COR_RESET "\n", quantidade);
    printf("          Saldo anterior   : %d  >>  " COR_CIANO "Novo saldo: %d" COR_RESET "\n",
           saldo_ant, saldo_novo);

    registrar_movimentacao(almox, "REMOVER", almox->itens[idx].nome,
                           quantidade, saldo_ant, saldo_novo);
    almox_salvar_json(almox);
    return 1;
}

/* ======================================================================
 * DESATIVAR — exclusão lógica (ativo = false).
 * Equivalente ao updateOne({$set: {ativo: false}}) do MongoDB.
 * O item permanece no JSON mas não aparece nas listagens normais.
 * ====================================================================== */

int almox_desativar(Almoxarifado *almox, const char *nome_bruto) {
    char nome[MAX_NOME];
    strncpy(nome, nome_bruto, sizeof(nome) - 1);
    nome[sizeof(nome) - 1] = '\0';
    almox_aparar_espacos(nome);

    int idx = almox_buscar_indice(almox, nome);
    if (idx < 0) {
        printf(COR_VERMELHO "Erro: Item '%s' não encontrado.\n" COR_RESET, nome);
        return 0;
    }
    if (!almox->itens[idx].ativo) {
        printf(COR_AMARELO "[AVISO] Item '%s' já está inativo.\n" COR_RESET, almox->itens[idx].nome);
        return 0;
    }

    int saldo_atual = almox->itens[idx].quantidade;
    almox->itens[idx].ativo = 0;

    printf(COR_AMARELO "[DESATIVADO] '%s' (Cód. %03d) marcado como inativo.\n" COR_RESET,
           almox->itens[idx].nome, almox->itens[idx].codigo);
    printf("             Saldo no momento da desativação: %d unidade(s)\n", saldo_atual);
    printf("             Use 'adicionar' com o mesmo nome para reativar o item.\n");

    registrar_movimentacao(almox, "DESATIVAR", almox->itens[idx].nome,
                           saldo_atual, saldo_atual, 0);
    almox_salvar_json(almox);
    return 1;
}

/* ======================================================================
 * LISTAR — exibe tabela do estoque.
 * mostrar_inativos=0 : equivale a find({ativo: true})
 * mostrar_inativos=1 : equivale a find({})  (todos)
 * ====================================================================== */

void almox_listar(const Almoxarifado *almox, int mostrar_inativos) {
    printf("\n" COR_CIANO "=======================================================================================\n");
    printf("                           ESTOQUE DO ALMOXARIFADO                                    \n");
    printf("=======================================================================================" COR_RESET "\n");

    /* Conta quantos serão exibidos */
    int exibidos = 0;
    for (int i = 0; i < almox->total_itens; i++) {
        if (!mostrar_inativos && !almox->itens[i].ativo) continue;
        exibidos++;
    }

    if (exibidos == 0) {
        printf(COR_CINZA "  Nenhum item %sencontrado.\n" COR_RESET,
               mostrar_inativos ? "" : "ativo ");
        printf("  Dica: " COR_VERDE "adicionar <item> <qtd> [valor_unitario]" COR_RESET "\n");
        printf("---------------------------------------------------------------------------------------\n\n");
        return;
    }

    printf(COR_BRANCO
           "%-4s | %-5s | %-28s | %-8s | %-11s | %-10s | %-8s\n"
           COR_RESET,
           "N.", "Cód.", "DESCRIÇÃO", "QTD", "VL.UNIT(R$)", "TOTAL(R$)", "STATUS");
    printf("---------------------------------------------------------------------------------------\n");

    int    n               = 0;
    int    total_unidades  = 0;
    double valor_total_est = 0.0;

    for (int i = 0; i < almox->total_itens; i++) {
        const ItemEstoque *it = &almox->itens[i];
        if (!mostrar_inativos && !it->ativo) continue;
        n++;

        double subtotal = it->valorUnitario * it->quantidade;
        if (it->ativo) {
            total_unidades  += it->quantidade;
            valor_total_est += subtotal;
        }

        const char *status_str;
        const char *cor_status;
        if (!it->ativo) {
            status_str = "INATIVO"; cor_status = COR_CINZA;
        } else if (it->quantidade == 0) {
            status_str = "ZERADO";  cor_status = COR_VERMELHO;
        } else if (it->quantidade <= 5) {
            status_str = "BAIXO";   cor_status = COR_AMARELO;
        } else {
            status_str = "OK";      cor_status = COR_VERDE;
        }

        printf("%-4d | %-5d | %-28s | %8d | %11.2f | %10.2f | %s%-8s" COR_RESET "\n",
               n, it->codigo, it->nome, it->quantidade,
               it->valorUnitario, subtotal, cor_status, status_str);
    }

    printf("---------------------------------------------------------------------------------------\n");
    printf("Itens exibidos: " COR_CIANO "%d" COR_RESET
           "  |  Unidades totais (ativos): " COR_CIANO "%d" COR_RESET
           "  |  Valor em estoque: " COR_CIANO "R$ %.2f" COR_RESET "\n\n",
           n, total_unidades, valor_total_est);
}

/* ======================================================================
 * CONSULTAR — busca parcial, case-insensitive, somente ativos.
 * Equivalente a find({ativo:true, nome: /termo/i}) do MongoDB.
 * ====================================================================== */

void almox_consultar(const Almoxarifado *almox, const char *termo_bruto) {
    char termo[MAX_NOME];
    strncpy(termo, termo_bruto, sizeof(termo) - 1);
    termo[sizeof(termo) - 1] = '\0';
    almox_aparar_espacos(termo);

    if (strlen(termo) == 0) {
        printf(COR_VERMELHO "Erro: Digite o nome do item a consultar.\n" COR_RESET);
        return;
    }

    printf("\n" COR_CIANO "Buscando por '%s' (itens ativos):\n" COR_RESET, termo);
    int encontrados = 0;

    for (int i = 0; i < almox->total_itens; i++) {
        const ItemEstoque *it = &almox->itens[i];
        if (!it->ativo) continue;

        /* Converte ambos para minúsculo antes de comparar */
        char nome_min[MAX_NOME], termo_min[MAX_NOME];
        int k;
        for (k = 0; it->nome[k] && k < MAX_NOME - 1; k++) {
            nome_min[k] = (char)tolower((unsigned char)it->nome[k]);
            nome_min[k + 1] = '\0';
        }
        for (k = 0; termo[k] && k < MAX_NOME - 1; k++) {
            termo_min[k] = (char)tolower((unsigned char)termo[k]);
            termo_min[k + 1] = '\0';
        }

        if (strstr(nome_min, termo_min) != NULL) {
            const char *cor_saldo = (it->quantidade <= 5) ? COR_AMARELO : COR_CIANO;
            printf("  -> [Cód. %03d] " COR_BRANCO "%-30s" COR_RESET
                   " | Saldo: %s%d" COR_RESET " un."
                   " | R$ %.2f/un."
                   " | Entrada: %s\n",
                   it->codigo, it->nome,
                   cor_saldo, it->quantidade,
                   it->valorUnitario, it->dataEntrada);
            encontrados++;
        }
    }

    if (encontrados == 0)
        printf(COR_AMARELO "  Nenhum item ativo com '%s' no nome.\n" COR_RESET, termo);
    printf("\n");
}

/* ======================================================================
 * HISTÓRICO de movimentações
 * ====================================================================== */

void almox_exibir_historico(const Almoxarifado *almox, int limite) {
    printf("\n" COR_CIANO "========================================================================================\n");
    printf("                           HISTÓRICO DE MOVIMENTAÇÕES                                   \n");
    printf("========================================================================================" COR_RESET "\n");

    if (almox->total_historico == 0) {
        printf(COR_CINZA "  Nenhuma movimentação registrada.\n" COR_RESET);
        printf("----------------------------------------------------------------------------------------\n\n");
        return;
    }

    if (limite <= 0 || limite > almox->total_historico)
        limite = almox->total_historico;

    printf(COR_BRANCO "%-19s | %-10s | %-30s | %-6s | %-7s | %-7s\n" COR_RESET,
           "DATA / HORA", "TIPO", "ITEM", "QTD", "ANT.", "NOVO");
    printf("----------------------------------------------------------------------------------------\n");

    int inicio = almox->total_historico - limite;
    for (int i = almox->total_historico - 1; i >= inicio; i--) {
        const RegistroHistorico *h = &almox->historico[i];
        const char *cor_tipo;
        if      (strcmp(h->operacao, "ADICIONAR") == 0) cor_tipo = COR_VERDE;
        else if (strcmp(h->operacao, "DESATIVAR") == 0) cor_tipo = COR_CINZA;
        else                                             cor_tipo = COR_VERMELHO;

        printf("%-19s | %s%-10s" COR_RESET " | %-30s | %6d | %7d | %s%7d" COR_RESET "\n",
               h->data_hora, cor_tipo, h->operacao,
               h->nome, h->quantidade, h->saldo_anterior,
               COR_CIANO, h->saldo_novo);
    }
    printf("----------------------------------------------------------------------------------------\n");
    printf("Exibindo últimas %d movimentações de um total de %d.\n\n", limite, almox->total_historico);
}

/* ======================================================================
 * RELATÓRIO — valor total em estoque.
 * Espelha o $aggregate do almoxarifadopim.mongodb.js (Etapa 7):
 *
 *   db.produtos.aggregate([
 *     { $match: { ativo: true } },
 *     { $group: {
 *         _id: null,
 *         valorTotal:      { $sum: { $multiply: ['$valorUnitario', '$quantidade'] } },
 *         totalDeItens:    { $sum: 1 },
 *         totalDeUnidades: { $sum: '$quantidade' }
 *     }}
 *   ]);
 * ====================================================================== */

void almox_relatorio(const Almoxarifado *almox) {
    printf("\n" COR_CIANO "========================================================================\n");
    printf("              RELATÓRIO — VALOR TOTAL EM ESTOQUE                        \n");
    printf("         (equivalente ao $aggregate do almoxarifadopim.mongodb.js)      \n");
    printf("========================================================================" COR_RESET "\n");

    double valor_total    = 0.0;
    int    total_itens    = 0;
    int    total_unidades = 0;
    int    itens_zerados  = 0;
    int    itens_baixo    = 0;
    int    itens_inativos = 0;

    for (int i = 0; i < almox->total_itens; i++) {
        const ItemEstoque *it = &almox->itens[i];
        if (!it->ativo) { itens_inativos++; continue; }

        valor_total    += it->valorUnitario * it->quantidade;
        total_itens++;
        total_unidades += it->quantidade;
        if      (it->quantidade == 0) itens_zerados++;
        else if (it->quantidade <= 5) itens_baixo++;
    }

    printf("  " COR_BRANCO "Itens ativos             :" COR_RESET " " COR_CIANO  "%d"      COR_RESET "\n", total_itens);
    printf("  " COR_BRANCO "Total de unidades         :" COR_RESET " " COR_CIANO  "%d"      COR_RESET "\n", total_unidades);
    printf("  " COR_BRANCO "Itens inativos            :" COR_RESET " " COR_CINZA  "%d"      COR_RESET "\n", itens_inativos);
    printf("  " COR_BRANCO "Itens com estoque zerado  :" COR_RESET " " COR_VERMELHO "%d"    COR_RESET "\n", itens_zerados);
    printf("  " COR_BRANCO "Itens com estoque baixo   :" COR_RESET " " COR_AMARELO "%d (<=5 un.)" COR_RESET "\n", itens_baixo);
    printf("  -----------------------------------------------------------------\n");
    printf("  " COR_BRANCO "VALOR TOTAL EM ESTOQUE    :" COR_RESET " " COR_VERDE  "R$ %.2f" COR_RESET "\n", valor_total);
    printf("========================================================================\n\n");
}

