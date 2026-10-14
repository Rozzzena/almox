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
    printf("  ===============================================================\n");
    printf(COR_RESET);
    printf("  Comandos principais: " COR_VERDE "adicionar" COR_RESET ", " COR_VERMELHO "remover" COR_RESET ", " COR_CIANO "listar" COR_RESET ", " COR_AMARELO "consultar" COR_RESET ", " COR_MAGENTA "historico" COR_RESET "\n");
    printf("  Digite " COR_BRANCO "'ajuda'" COR_RESET " para instruções ou " COR_BRANCO "'sair'" COR_RESET " para encerrar.\n\n");
}

static void exibir_ajuda(void) {
    printf("\n" COR_CIANO "----------------------- GUIA DE COMANDOS -----------------------" COR_RESET "\n");
    printf(COR_VERDE   "  adicionar <item> <quantidade>\n" COR_RESET);
    printf("      Soma a quantidade ao item. Se o item não existir, ele é criado.\n");
    printf("      Exemplos:\n");
    printf("        adicionar Parafuso 100\n");
    printf("        adicionar \"Cabo de Rede Cat6\" 50\n\n");

    printf(COR_VERMELHO "  remover <item> <quantidade>\n" COR_RESET);
    printf("      Subtrai a quantidade do estoque do item.\n");
    printf("      Exemplos:\n");
    printf("        remover Parafuso 25\n");
    printf("        remover \"Cabo de Rede Cat6\" 10\n\n");

    printf(COR_CIANO "  listar\n" COR_RESET);
    printf("      Exibe a tabela com todos os itens, quantidades e status.\n\n");

    printf(COR_AMARELO "  consultar <termo>\n" COR_RESET);
    printf("      Busca itens pelo nome ou parte do nome.\n");
    printf("      Exemplo: consultar cabo\n\n");

    printf(COR_MAGENTA "  historico [limite]\n" COR_RESET);
    printf("      Exibe o histórico de entradas e saídas (padrão: últimas 20).\n");
    printf("      Exemplo: historico 10\n\n");

    printf(COR_BRANCO "  limpar\n" COR_RESET);
    printf("      Limpa a tela do terminal.\n\n");

    printf(COR_BRANCO "  sair\n" COR_RESET);
    printf("      Salva e fecha a aplicação.\n");
    printf(COR_CIANO "----------------------------------------------------------------" COR_RESET "\n\n");
}

/* Quebra a linha de comando em argumentos respeitando aspas */
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
            if (*p == '\"') {
                *p = '\0';
                p++;
            }
        } else {
            argv_out[argc++] = p;
            while (*p && !isspace((unsigned char)*p)) p++;
            if (*p) {
                *p = '\0';
                p++;
            }
        }
    }
    return argc;
}

/* Executa um comando a partir de uma lista de argumentos */
static int processar_comando(Almoxarifado *almox, int argc, char *argv[]) {
    if (argc == 0) return 1;

    char *cmd = argv[0];

    /* ADICIONAR */
    if (almox_strcasecmp(cmd, "adicionar") == 0 || almox_strcasecmp(cmd, "add") == 0) {
        if (argc < 3) {
            printf(COR_AMARELO "Uso: adicionar <nome_do_item> <quantidade>\n" COR_RESET);
            printf("Exemplo: adicionar \"Parafuso M8\" 100\n");
            return 1;
        }

        /* Suporta nomes com espaços mesmo sem aspas:
           Se argc > 3, os tokens do meio formam o nome e o último é a quantidade */
        char nome_completo[MAX_NOME] = "";
        int qtd = atoi(argv[argc - 1]);

        if (qtd <= 0) {
            printf(COR_VERMELHO "Erro: A quantidade deve ser um número inteiro positivo (recebido '%s').\n" COR_RESET, argv[argc - 1]);
            return 1;
        }

        for (int i = 1; i < argc - 1; i++) {
            if (i > 1) strncat(nome_completo, " ", sizeof(nome_completo) - strlen(nome_completo) - 1);
            strncat(nome_completo, argv[i], sizeof(nome_completo) - strlen(nome_completo) - 1);
        }

        almox_adicionar(almox, nome_completo, qtd);
        return 1;
    }

    /* REMOVER */
    if (almox_strcasecmp(cmd, "remover") == 0 || almox_strcasecmp(cmd, "rm") == 0 || almox_strcasecmp(cmd, "del") == 0) {
        if (argc < 3) {
            printf(COR_AMARELO "Uso: remover <nome_do_item> <quantidade>\n" COR_RESET);
            printf("Exemplo: remover \"Parafuso M8\" 20\n");
            return 1;
        }

        char nome_completo[MAX_NOME] = "";
        int qtd = atoi(argv[argc - 1]);

        if (qtd <= 0) {
            printf(COR_VERMELHO "Erro: A quantidade deve ser um número inteiro positivo (recebido '%s').\n" COR_RESET, argv[argc - 1]);
            return 1;
        }

        for (int i = 1; i < argc - 1; i++) {
            if (i > 1) strncat(nome_completo, " ", sizeof(nome_completo) - strlen(nome_completo) - 1);
            strncat(nome_completo, argv[i], sizeof(nome_completo) - strlen(nome_completo) - 1);
        }

        almox_remover(almox, nome_completo, qtd);
        return 1;
    }

    /* LISTAR */
    if (almox_strcasecmp(cmd, "listar") == 0 || almox_strcasecmp(cmd, "ls") == 0 || almox_strcasecmp(cmd, "estoque") == 0) {
        almox_listar(almox);
        return 1;
    }

    /* CONSULTAR */
    if (almox_strcasecmp(cmd, "consultar") == 0 || almox_strcasecmp(cmd, "buscar") == 0 || almox_strcasecmp(cmd, "busca") == 0) {
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

    /* HISTORICO */
    if (almox_strcasecmp(cmd, "historico") == 0 || almox_strcasecmp(cmd, "hist") == 0 || almox_strcasecmp(cmd, "log") == 0) {
        int limite = 20;
        if (argc >= 2) {
            int arg_limite = atoi(argv[1]);
            if (arg_limite > 0) limite = arg_limite;
        }
        almox_exibir_historico(almox, limite);
        return 1;
    }

    /* AJUDA */
    if (almox_strcasecmp(cmd, "ajuda") == 0 || almox_strcasecmp(cmd, "help") == 0 || almox_strcasecmp(cmd, "?") == 0) {
        exibir_ajuda();
        return 1;
    }

    /* LIMPAR */
    if (almox_strcasecmp(cmd, "limpar") == 0 || almox_strcasecmp(cmd, "cls") == 0 || almox_strcasecmp(cmd, "clear") == 0) {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        exibir_banner();
        return 1;
    }

    /* SAIR */
    if (almox_strcasecmp(cmd, "sair") == 0 || almox_strcasecmp(cmd, "exit") == 0 || almox_strcasecmp(cmd, "quit") == 0 || almox_strcasecmp(cmd, "q") == 0) {
        printf(COR_VERDE "Encerrando aplicação. Todos os dados foram salvos em '%s'. Até logo!\n" COR_RESET, almox->caminho_arquivo);
        return 0; /* Sinaliza término */
    }

    /* COMANDO DESCONHECIDO */
    printf(COR_AMARELO "Comando desconhecido: '%s'. Digite 'ajuda' para ver a lista de comandos.\n" COR_RESET, cmd);
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
    printf("Dados carregados de: " COR_CIANO "%s" COR_RESET " (%d itens em estoque)\n\n",
           almox.caminho_arquivo, almox.total_itens);

    char linha[512];
    while (1) {
        printf(COR_CIANO "almox> " COR_RESET);
        fflush(stdout);

        if (!fgets(linha, sizeof(linha), stdin)) {
            break; /* EOF (Ctrl+C / Ctrl+Z) */
        }

        /* Remove quebra de linha */
        linha[strcspn(linha, "\r\n")] = '\0';
        almox_aparar_espacos(linha);

        if (strlen(linha) == 0) {
            continue;
        }

        char *args[32];
        int num_args = dividir_argumentos(linha, args, 32);

        if (!processar_comando(&almox, num_args, args)) {
            break;
        }
    }

    return 0;
}
