#include "almoxarifado.h"

#define COR_RESET    "\033[0m"
#define COR_VERMELHO "\033[1;31m"
#define COR_VERDE    "\033[1;32m"
#define COR_AMARELO  "\033[1;33m"
#define COR_AZUL     "\033[1;34m"
#define COR_MAGENTA  "\033[1;35m"
#define COR_CIANO    "\033[1;36m"
#define COR_BRANCO   "\033[1;37m"
#define COR_CINZA    "\033[0;90m"

static void exibir_banner(void) {
    printf(COR_CIANO);
    printf("  ===============================================================\n");
    printf("  *             SISTEMA DE ALMOXARIFADO (CLI)                   *\n");
    printf("  *        Schema MongoDB  |  Persistência JSON  |  C99         *\n");
    printf("  ===============================================================\n");
    printf(COR_RESET);
    printf("  Comandos: " COR_VERDE "adicionar" COR_RESET ", " COR_VERMELHO "remover" COR_RESET
           ", " COR_AMARELO "desativar" COR_RESET ", " COR_CIANO "listar" COR_RESET
           ", " COR_MAGENTA "relatorio" COR_RESET ", " COR_AZUL "consultar" COR_RESET
           ", " COR_BRANCO "historico" COR_RESET "\n");
    printf("  Digite " COR_BRANCO "'ajuda'" COR_RESET " para instruções ou "
           COR_BRANCO "'sair'" COR_RESET " para encerrar.\n\n");
}

static void exibir_ajuda(void) {
    printf("\n" COR_CIANO "------------------------- GUIA DE COMANDOS -------------------------" COR_RESET "\n");

    printf(COR_VERDE "  adicionar <item> <quantidade> [valor_unitario]\n" COR_RESET);
    printf("      Cadastra um novo item ou incrementa o saldo de um existente.\n");
    printf("      O valor unitário é opcional (padrão: 0.00).\n");
    printf("      Exemplos:\n");
    printf("        adicionar Parafuso 100\n");
    printf("        adicionar \"Luva de Seguranca\" 40 12.90\n\n");

    printf(COR_VERMELHO "  remover <item> <quantidade>\n" COR_RESET);
    printf("      Dá baixa no estoque. Bloqueado se saldo insuficiente.\n");
    printf("      Exemplos:\n");
    printf("        remover Parafuso 25\n\n");

    printf(COR_AMARELO "  desativar <item>\n" COR_RESET);
    printf("      Exclusão lógica — ativo=false. Item some das listagens normais.\n");
    printf("      Equivalente a: db.produtos.updateOne({$set: {ativo: false}})\n");
    printf("      Use 'adicionar' no mesmo item para reativá-lo.\n\n");

    printf(COR_CIANO "  listar\n" COR_RESET);
    printf("      Exibe apenas itens ativos (ativo=true). Equivale a find({ativo:true}).\n\n");

    printf(COR_CIANO "  listar tudo\n" COR_RESET);
    printf("      Exibe todos os itens, incluindo inativos.\n\n");

    printf(COR_MAGENTA "  relatorio\n" COR_RESET);
    printf("      Calcula valor total em estoque, total de itens e unidades.\n");
    printf("      Equivalente ao $aggregate do almoxarifadopim.mongodb.js (Etapa 7).\n\n");

    printf(COR_AZUL "  consultar <termo>\n" COR_RESET);
    printf("      Busca itens ativos pelo nome (parcial, sem distinção de maiúsculas).\n");
    printf("      Exemplo: consultar luva\n\n");

    printf(COR_BRANCO "  historico [limite]\n" COR_RESET);
    printf("      Exibe o histórico de movimentações (padrão: últimas 20).\n");
    printf("      Exemplo: historico 10\n\n");

    printf(COR_BRANCO "  limpar\n" COR_RESET);
    printf("      Limpa a tela do terminal.\n\n");

    printf(COR_BRANCO "  sair\n" COR_RESET);
    printf("      Salva e fecha a aplicação.\n");
    printf(COR_CIANO "---------------------------------------------------------------------" COR_RESET "\n\n");
}

/* Quebra a linha de comando em argumentos respeitando aspas duplas */
static int dividir_argumentos(char *linha, char *argv_out[], int max_args) {
    int argc = 0;
    char *p = linha;

    while (*p && argc < max_args) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        if (*p == '\"') {
            p++;
            argv_out[argc++] = p;
            while (*p && *p != '\"') p++;
            if (*p == '\"') { *p = '\0'; p++; }
        } else {
            argv_out[argc++] = p;
            while (*p && !isspace((unsigned char)*p)) p++;
            if (*p) { *p = '\0'; p++; }
        }
    }
    return argc;
}

/* Executa um comando a partir de uma lista de argumentos */
static int processar_comando(Almoxarifado *almox, int argc, char *argv[]) {
    if (argc == 0) return 1;

    char *cmd = argv[0];

    /* ------------------------------------------------------------------ */
    /* ADICIONAR                                                            */
    /* Sintaxe: adicionar <nome> <quantidade> [valor_unitario]             */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "adicionar") == 0 || almox_strcasecmp(cmd, "add") == 0) {
        if (argc < 3) {
            printf(COR_AMARELO "Uso: adicionar <nome> <quantidade> [valor_unitario]\n" COR_RESET);
            printf("Exemplo: adicionar \"Luva de Seguranca\" 40 12.90\n");
            return 1;
        }

        /*
         * Detecta valor_unitario opcional:
         * Se o último argumento contiver ponto decimal e argc >= 4,
         * ele é tratado como valor_unitario e o penúltimo como quantidade.
         * Caso contrário, o último é quantidade e valor_unitario = 0.0.
         */
        double valor = 0.0;
        int    qtd;
        int    nome_ate; /* índice exclusivo do último token do nome */

        if (argc >= 4 && strchr(argv[argc - 1], '.') != NULL) {
            valor    = atof(argv[argc - 1]);
            qtd      = atoi(argv[argc - 2]);
            nome_ate = argc - 2;
        } else {
            qtd      = atoi(argv[argc - 1]);
            nome_ate = argc - 1;
        }

        if (qtd <= 0) {
            printf(COR_VERMELHO "Erro: A quantidade deve ser um inteiro positivo (recebido '%s').\n" COR_RESET,
                   argv[nome_ate]);
            return 1;
        }

        char nome_completo[MAX_NOME] = "";
        for (int i = 1; i < nome_ate; i++) {
            if (i > 1) strncat(nome_completo, " ", sizeof(nome_completo) - strlen(nome_completo) - 1);
            strncat(nome_completo, argv[i], sizeof(nome_completo) - strlen(nome_completo) - 1);
        }

        almox_adicionar(almox, nome_completo, qtd, valor);
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* REMOVER                                                              */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "remover") == 0 || almox_strcasecmp(cmd, "rm") == 0 ||
        almox_strcasecmp(cmd, "del") == 0) {
        if (argc < 3) {
            printf(COR_AMARELO "Uso: remover <nome> <quantidade>\n" COR_RESET);
            return 1;
        }

        char nome_completo[MAX_NOME] = "";
        int qtd = atoi(argv[argc - 1]);

        if (qtd <= 0) {
            printf(COR_VERMELHO "Erro: A quantidade deve ser um inteiro positivo (recebido '%s').\n" COR_RESET,
                   argv[argc - 1]);
            return 1;
        }

        for (int i = 1; i < argc - 1; i++) {
            if (i > 1) strncat(nome_completo, " ", sizeof(nome_completo) - strlen(nome_completo) - 1);
            strncat(nome_completo, argv[i], sizeof(nome_completo) - strlen(nome_completo) - 1);
        }

        almox_remover(almox, nome_completo, qtd);
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* DESATIVAR — exclusão lógica (ativo = false)                         */
    /* Equivale a: db.produtos.updateOne({$set: {ativo: false}})           */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "desativar") == 0 || almox_strcasecmp(cmd, "inativar") == 0) {
        if (argc < 2) {
            printf(COR_AMARELO "Uso: desativar <nome>\n" COR_RESET);
            return 1;
        }

        char nome_completo[MAX_NOME] = "";
        for (int i = 1; i < argc; i++) {
            if (i > 1) strncat(nome_completo, " ", sizeof(nome_completo) - strlen(nome_completo) - 1);
            strncat(nome_completo, argv[i], sizeof(nome_completo) - strlen(nome_completo) - 1);
        }

        almox_desativar(almox, nome_completo);
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* LISTAR                                                               */
    /* listar      -> somente ativos  (find({ativo: true}))                */
    /* listar tudo -> todos           (find({}))                           */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "listar") == 0 || almox_strcasecmp(cmd, "ls") == 0 ||
        almox_strcasecmp(cmd, "estoque") == 0) {
        int mostrar_inativos = 0;
        if (argc >= 2 && almox_strcasecmp(argv[1], "tudo") == 0)
            mostrar_inativos = 1;
        almox_listar(almox, mostrar_inativos);
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* RELATORIO — equivale ao $aggregate do almoxarifadopim.mongodb.js    */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "relatorio") == 0 || almox_strcasecmp(cmd, "rel") == 0 ||
        almox_strcasecmp(cmd, "aggregate") == 0) {
        almox_relatorio(almox);
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* CONSULTAR                                                            */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "consultar") == 0 || almox_strcasecmp(cmd, "buscar") == 0 ||
        almox_strcasecmp(cmd, "busca") == 0) {
        if (argc < 2) {
            printf(COR_AMARELO "Uso: consultar <termo>\n" COR_RESET);
            return 1;
        }
        char termo[MAX_NOME] = "";
        for (int i = 1; i < argc; i++) {
            if (i > 1) strncat(termo, " ", sizeof(termo) - strlen(termo) - 1);
            strncat(termo, argv[i], sizeof(termo) - strlen(termo) - 1);
        }
        almox_consultar(almox, termo);
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* HISTORICO                                                            */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "historico") == 0 || almox_strcasecmp(cmd, "hist") == 0 ||
        almox_strcasecmp(cmd, "log") == 0) {
        int limite = 20;
        if (argc >= 2) {
            int arg = atoi(argv[1]);
            if (arg > 0) limite = arg;
        }
        almox_exibir_historico(almox, limite);
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* AJUDA                                                                */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "ajuda") == 0 || almox_strcasecmp(cmd, "help") == 0 ||
        almox_strcasecmp(cmd, "?") == 0) {
        exibir_ajuda();
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* LIMPAR                                                               */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "limpar") == 0 || almox_strcasecmp(cmd, "cls") == 0 ||
        almox_strcasecmp(cmd, "clear") == 0) {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        exibir_banner();
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* SAIR                                                                 */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "sair") == 0 || almox_strcasecmp(cmd, "exit") == 0 ||
        almox_strcasecmp(cmd, "quit") == 0 || almox_strcasecmp(cmd, "q") == 0) {
        printf(COR_VERDE "Encerrando. Dados salvos em '%s'. Até logo!\n" COR_RESET,
               almox->caminho_arquivo);
        return 0;
    }

    /* Comando desconhecido */
    printf(COR_AMARELO "Comando desconhecido: '%s'. Digite 'ajuda' para ver os comandos.\n" COR_RESET, cmd);
    return 1;
}

int main(int argc, char *argv[]) {
    Almoxarifado almox;
    almox_inicializar(&almox, ARQUIVO_PADRAO);

    /* MODO 1: Argumentos diretos na linha de comando */
    if (argc > 1) {
        processar_comando(&almox, argc - 1, &argv[1]);
        return 0;
    }

    /* MODO 2: Shell / Prompt Interativo */
    exibir_banner();
    printf("Dados carregados de: " COR_CIANO "%s" COR_RESET " (%d item(ns) ativo(s))\n\n",
           almox.caminho_arquivo, almox.total_itens);

    char linha[512];
    while (1) {
        printf(COR_CIANO "almox> " COR_RESET);
        fflush(stdout);

        if (!fgets(linha, sizeof(linha), stdin)) break; /* EOF */

        linha[strcspn(linha, "\r\n")] = '\0';
        almox_aparar_espacos(linha);

        if (strlen(linha) == 0) continue;

        char *args[32];
        int num_args = dividir_argumentos(linha, args, 32);

        if (!processar_comando(&almox, num_args, args)) break;
    }

    return 0;
}
