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
    printf("  *               SISTEMA DE VENDAS E ESTOQUE                   *\n");
    printf("  *        Gestão de Compras, Vendas e Produtos | C99           *\n");
    printf("  ===============================================================\n");
    printf(COR_RESET);
    printf("  Comandos: " COR_VERDE "cadastrar" COR_RESET ", " COR_AZUL "comprar" COR_RESET
           ", " COR_VERMELHO "vender" COR_RESET ", " COR_AMARELO "editar" COR_RESET
           ", " COR_AMARELO "desativar" COR_RESET ", " COR_CIANO "listar" COR_RESET
           ", " COR_MAGENTA "relatorio" COR_RESET ", " COR_VERDE "exportar" COR_RESET
           ", " COR_BRANCO "consultar" COR_RESET ", " COR_CINZA "historico" COR_RESET "\n");
    printf("  Digite " COR_BRANCO "'ajuda'" COR_RESET " para instruções ou "
           COR_BRANCO "'sair'" COR_RESET " para encerrar.\n\n");
}

static void exibir_ajuda(void) {
    printf("\n" COR_CIANO "------------------------- GUIA DE COMANDOS -------------------------" COR_RESET "\n");

    printf(COR_VERDE "  cadastrar\n" COR_RESET);
    printf("      Cadastra um novo produto solicitando campo a campo (nome, qtd, custo, venda, est. mín).\n");
    printf("      (Ou: cadastrar <produto> <quantidade> [valor_venda])\n\n");

    printf(COR_AZUL "  comprar [produto] [quantidade] [valor_unitario]\n" COR_RESET);
    printf("      Registra uma compra / entrada de estoque de produto.\n");
    printf("      Exemplos:\n");
    printf("        comprar                (modo interativo)\n");
    printf("        comprar Parafuso 100\n");
    printf("        comprar \"Luva de Seguranca\" 40 12.90\n\n");

    printf(COR_VERMELHO "  vender [produto] [quantidade]\n" COR_RESET);
    printf("      Realiza uma venda e dá baixa no estoque (com trava contra saldo negativo).\n");
    printf("      Exemplos:\n");
    printf("        vender                 (modo interativo)\n");
    printf("        vender Parafuso 25\n\n");

    printf(COR_AMARELO "  editar <codigo>\n" COR_RESET);
    printf("      Edita interativamente os dados de um produto (nome, qtd, custo, venda, estoque mín).\n");
    printf("      Pressione Enter em qualquer campo para manter o valor atual.\n\n");

    printf(COR_AMARELO "  desativar <produto>\n" COR_RESET);
    printf("      Exclusão lógica — desativa o produto sem apagar seu histórico.\n");
    printf("      Use 'comprar' ou 'cadastrar' no mesmo item para reativá-lo.\n\n");

    printf(COR_CIANO "  listar [tudo]\n" COR_RESET);
    printf("      Exibe produtos em estoque, valores totais e status (OK, BAIXO, ZERADO).\n\n");

    printf(COR_MAGENTA "  relatorio\n" COR_RESET);
    printf("      Calcula custo total, valor financeiro de venda, projeção de lucro e alertas.\n\n");

    printf(COR_VERDE "  exportar [arquivo.csv]\n" COR_RESET);
    printf("      Exporta o estoque completo para planilha CSV compatível com Excel.\n");
    printf("      Exemplo: exportar estoque.csv\n\n");

    printf(COR_BRANCO "  consultar <termo>\n" COR_RESET);
    printf("      Busca produtos ativos pelo nome (busca parcial).\n\n");

    printf(COR_CINZA "  historico [limite]\n" COR_RESET);
    printf("      Exibe o histórico de movimentações (compras, vendas, cadastros e edições).\n\n");

    printf(COR_BRANCO "  limpar  |  sair\n" COR_RESET);
    printf("      Limpa a tela ou encerra a aplicação.\n");
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

/* Realiza o cadastro interativo de produto solicitando campo a campo */
static void cadastrar_produto_interativo(Almoxarifado *almox) {
    char nome[MAX_NOME];
    char buffer[128];
    int quantidade = 0;
    double precoCusto = 0.0;
    double precoVenda = 0.0;
    int estoqueMin = 0;

    printf("\n" COR_CIANO "=== CADASTRO DE PRODUTO ===" COR_RESET "\n");
    printf(COR_CINZA "(Dica: pressione Enter após cada campo ou digite 'cancelar' para voltar)\n\n" COR_RESET);

    /* Campo 1: Nome do Produto */
    while (1) {
        printf(COR_BRANCO "Nome do produto: " COR_RESET);
        fflush(stdout);
        if (!fgets(nome, sizeof(nome), stdin)) return;
        nome[strcspn(nome, "\r\n")] = '\0';
        almox_aparar_espacos(nome);

        if (almox_strcasecmp(nome, "cancelar") == 0 || almox_strcasecmp(nome, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Cadastro cancelado.\n\n" COR_RESET);
            return;
        }

        if (strlen(nome) == 0) {
            printf(COR_AMARELO "  O nome do produto não pode ficar vazio. Tente novamente.\n" COR_RESET);
            continue;
        }
        break;
    }

    /* Campo 2: Quantidade */
    while (1) {
        printf(COR_BRANCO "Quantidade inicial: " COR_RESET);
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);

        if (almox_strcasecmp(buffer, "cancelar") == 0 || almox_strcasecmp(buffer, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Cadastro cancelado.\n\n" COR_RESET);
            return;
        }

        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\0' || val <= 0) {
            printf(COR_AMARELO "  Quantidade inválida! Digite um número inteiro maior que 0 (ex: 50).\n" COR_RESET);
            continue;
        }
        quantidade = (int)val;
        break;
    }

    /* Campo 3: Preço de Custo */
    while (1) {
        printf(COR_BRANCO "Preço de CUSTO unitário em R$ (ou Enter para 0.00): " COR_RESET);
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);

        if (almox_strcasecmp(buffer, "cancelar") == 0 || almox_strcasecmp(buffer, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Cadastro cancelado.\n\n" COR_RESET);
            return;
        }

        if (strlen(buffer) == 0) {
            precoCusto = 0.0;
            break;
        }

        for (int i = 0; buffer[i]; i++) {
            if (buffer[i] == ',') buffer[i] = '.';
        }

        char *endptr;
        double v = strtod(buffer, &endptr);
        if (endptr == buffer || *endptr != '\0' || v < 0.0) {
            printf(COR_AMARELO "  Valor inválido! Digite um número positivo.\n" COR_RESET);
            continue;
        }
        precoCusto = v;
        break;
    }

    /* Campo 4: Preço de Venda */
    while (1) {
        printf(COR_BRANCO "Preço de VENDA unitário em R$ (ou Enter para 0.00): " COR_RESET);
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);

        if (almox_strcasecmp(buffer, "cancelar") == 0 || almox_strcasecmp(buffer, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Cadastro cancelado.\n\n" COR_RESET);
            return;
        }

        if (strlen(buffer) == 0) {
            precoVenda = 0.0;
            break;
        }

        for (int i = 0; buffer[i]; i++) {
            if (buffer[i] == ',') buffer[i] = '.';
        }

        char *endptr;
        double v = strtod(buffer, &endptr);
        if (endptr == buffer || *endptr != '\0' || v < 0.0) {
            printf(COR_AMARELO "  Valor inválido! Digite um preço numérico positivo (ex: 15.90 ou 15,90).\n" COR_RESET);
            continue;
        }
        precoVenda = v;
        break;
    }

    /* Campo 5: Estoque Mínimo */
    while (1) {
        printf(COR_BRANCO "Estoque mínimo de segurança (ou Enter para 5): " COR_RESET);
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);

        if (almox_strcasecmp(buffer, "cancelar") == 0 || almox_strcasecmp(buffer, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Cadastro cancelado.\n\n" COR_RESET);
            return;
        }

        if (strlen(buffer) == 0) {
            estoqueMin = 5;
            break;
        }

        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\0' || val < 0) {
            printf(COR_AMARELO "  Estoque mínimo inválido! Digite um inteiro maior ou igual a 0.\n" COR_RESET);
            continue;
        }
        estoqueMin = (int)val;
        break;
    }

    printf("\n");
    almox_adicionar_completo(almox, nome, quantidade, precoCusto, precoVenda, estoqueMin);
    printf("\n");
}

/* Edita um produto existente de forma interativa */
static void editar_produto_interativo(Almoxarifado *almox, int codigo) {
    int idx = almox_buscar_indice_codigo(almox, codigo);
    if (idx < 0) {
        printf(COR_VERMELHO "Erro: Produto com código %03d não encontrado.\n" COR_RESET, codigo);
        return;
    }

    ItemEstoque *it = &almox->itens[idx];
    printf("\n" COR_CIANO "=== EDITAR PRODUTO [Cód. %03d] ===" COR_RESET "\n", it->codigo);
    printf(COR_CINZA "(Dica: pressione Enter sem digitar nada para manter o valor atual)\n\n" COR_RESET);

    char buffer[128];
    char novo_nome[MAX_NOME];
    snprintf(novo_nome, sizeof(novo_nome), "%s", it->nome);
    int nova_qtd = it->quantidade;
    double novo_custo = it->precoCusto;
    double novo_preco = it->valorUnitario;
    int novo_est_min = it->estoqueMinimo;
    int alterar_campos = 0;

    /* 1. Nome */
    printf(COR_BRANCO "Nome [%s]: " COR_RESET, it->nome);
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);
        if (almox_strcasecmp(buffer, "cancelar") == 0) {
            printf(COR_AMARELO "[INFO] Edição cancelada.\n\n" COR_RESET);
            return;
        }
        if (strlen(buffer) > 0) {
            strncpy(novo_nome, buffer, sizeof(novo_nome) - 1);
            novo_nome[sizeof(novo_nome) - 1] = '\0';
            alterar_campos |= 1;
        }
    }

    /* 2. Quantidade */
    printf(COR_BRANCO "Quantidade em estoque [%d]: " COR_RESET, it->quantidade);
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);
        if (almox_strcasecmp(buffer, "cancelar") == 0) {
            printf(COR_AMARELO "[INFO] Edição cancelada.\n\n" COR_RESET);
            return;
        }
        if (strlen(buffer) > 0) {
            char *endptr;
            long val = strtol(buffer, &endptr, 10);
            if (endptr != buffer && *endptr == '\0' && val >= 0) {
                nova_qtd = (int)val;
                alterar_campos |= 2;
            }
        }
    }

    /* 3. Preço de custo */
    printf(COR_BRANCO "Preço de custo [R$ %.2f]: " COR_RESET, it->precoCusto);
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);
        if (almox_strcasecmp(buffer, "cancelar") == 0) {
            printf(COR_AMARELO "[INFO] Edição cancelada.\n\n" COR_RESET);
            return;
        }
        if (strlen(buffer) > 0) {
            for (int i = 0; buffer[i]; i++) if (buffer[i] == ',') buffer[i] = '.';
            char *endptr;
            double v = strtod(buffer, &endptr);
            if (endptr != buffer && *endptr == '\0' && v >= 0.0) {
                novo_custo = v;
                alterar_campos |= 4;
            }
        }
    }

    /* 4. Preço de venda */
    printf(COR_BRANCO "Preço de venda [R$ %.2f]: " COR_RESET, it->valorUnitario);
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);
        if (almox_strcasecmp(buffer, "cancelar") == 0) {
            printf(COR_AMARELO "[INFO] Edição cancelada.\n\n" COR_RESET);
            return;
        }
        if (strlen(buffer) > 0) {
            for (int i = 0; buffer[i]; i++) if (buffer[i] == ',') buffer[i] = '.';
            char *endptr;
            double v = strtod(buffer, &endptr);
            if (endptr != buffer && *endptr == '\0' && v >= 0.0) {
                novo_preco = v;
                alterar_campos |= 8;
            }
        }
    }

    /* 5. Estoque mínimo */
    printf(COR_BRANCO "Estoque mínimo [%d]: " COR_RESET, it->estoqueMinimo);
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);
        if (almox_strcasecmp(buffer, "cancelar") == 0) {
            printf(COR_AMARELO "[INFO] Edição cancelada.\n\n" COR_RESET);
            return;
        }
        if (strlen(buffer) > 0) {
            char *endptr;
            long val = strtol(buffer, &endptr, 10);
            if (endptr != buffer && *endptr == '\0' && val >= 0) {
                novo_est_min = (int)val;
                alterar_campos |= 16;
            }
        }
    }

    printf("\n");
    if (alterar_campos == 0) {
        printf(COR_AMARELO "[INFO] Nenhuma alteração foi realizada.\n\n" COR_RESET);
        return;
    }

    almox_editar_produto(almox, codigo, novo_nome, nova_qtd, novo_custo, novo_preco, novo_est_min, alterar_campos);
    printf("\n");
}

/* Realiza a venda de produto interativamente solicitando campo a campo */
static void vender_produto_interativo(Almoxarifado *almox) {
    char nome[MAX_NOME];
    char buffer[128];
    int quantidade = 0;

    printf("\n" COR_CIANO "=== REALIZAR VENDA DE PRODUTO ===" COR_RESET "\n");
    printf(COR_CINZA "(Dica: digite 'cancelar' para voltar)\n\n" COR_RESET);

    while (1) {
        printf(COR_BRANCO "Nome do produto a vender: " COR_RESET);
        fflush(stdout);
        if (!fgets(nome, sizeof(nome), stdin)) return;
        nome[strcspn(nome, "\r\n")] = '\0';
        almox_aparar_espacos(nome);

        if (almox_strcasecmp(nome, "cancelar") == 0 || almox_strcasecmp(nome, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Venda cancelada.\n\n" COR_RESET);
            return;
        }

        if (strlen(nome) == 0) {
            printf(COR_AMARELO "  O nome do produto não pode ficar vazio. Tente novamente.\n" COR_RESET);
            continue;
        }
        int idx = almox_buscar_indice(almox, nome);
        if (idx < 0) {
            printf(COR_VERMELHO "  Produto '%s' não encontrado no estoque! Tente novamente.\n" COR_RESET, nome);
            continue;
        }
        if (!almox->itens[idx].ativo) {
            printf(COR_AMARELO "  Produto '%s' está inativo no sistema.\n" COR_RESET, almox->itens[idx].nome);
            return;
        }
        printf(COR_CINZA "  -> Saldo em estoque: %d unidade(s) | Preço unitário: R$ %.2f\n" COR_RESET,
               almox->itens[idx].quantidade, almox->itens[idx].valorUnitario);
        break;
    }

    while (1) {
        printf(COR_BRANCO "Quantidade a vender: " COR_RESET);
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);

        if (almox_strcasecmp(buffer, "cancelar") == 0 || almox_strcasecmp(buffer, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Venda cancelada.\n\n" COR_RESET);
            return;
        }

        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\0' || val <= 0) {
            printf(COR_AMARELO "  Quantidade inválida! Digite um número inteiro maior que 0.\n" COR_RESET);
            continue;
        }
        quantidade = (int)val;
        break;
    }

    printf("\n");
    almox_remover(almox, nome, quantidade);
    printf("\n");
}

/* Realiza a compra/reposição de produto interativamente solicitando campo a campo */
static void comprar_produto_interativo(Almoxarifado *almox) {
    char nome[MAX_NOME];
    char buffer[128];
    int quantidade = 0;
    double valor = 0.0;

    printf("\n" COR_CIANO "=== COMPRA / ENTRADA DE ESTOQUE ===" COR_RESET "\n");
    printf(COR_CINZA "(Dica: digite 'cancelar' para voltar)\n\n" COR_RESET);

    while (1) {
        printf(COR_BRANCO "Nome do produto comprado: " COR_RESET);
        fflush(stdout);
        if (!fgets(nome, sizeof(nome), stdin)) return;
        nome[strcspn(nome, "\r\n")] = '\0';
        almox_aparar_espacos(nome);

        if (almox_strcasecmp(nome, "cancelar") == 0 || almox_strcasecmp(nome, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Compra cancelada.\n\n" COR_RESET);
            return;
        }

        if (strlen(nome) == 0) {
            printf(COR_AMARELO "  O nome do produto não pode ficar vazio. Tente novamente.\n" COR_RESET);
            continue;
        }
        int idx = almox_buscar_indice(almox, nome);
        if (idx >= 0) {
            printf(COR_CINZA "  -> Produto existente. Saldo atual: %d un. | Preço atual: R$ %.2f\n" COR_RESET,
                   almox->itens[idx].quantidade, almox->itens[idx].valorUnitario);
        } else {
            printf(COR_CINZA "  -> Novo produto será cadastrado automaticamente.\n" COR_RESET);
        }
        break;
    }

    while (1) {
        printf(COR_BRANCO "Quantidade comprada: " COR_RESET);
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);

        if (almox_strcasecmp(buffer, "cancelar") == 0 || almox_strcasecmp(buffer, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Compra cancelada.\n\n" COR_RESET);
            return;
        }

        char *endptr;
        long val = strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\0' || val <= 0) {
            printf(COR_AMARELO "  Quantidade inválida! Digite um número inteiro maior que 0.\n" COR_RESET);
            continue;
        }
        quantidade = (int)val;
        break;
    }

    while (1) {
        printf(COR_BRANCO "Preço unitário em R$ (ou Enter para manter atual): " COR_RESET);
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        buffer[strcspn(buffer, "\r\n")] = '\0';
        almox_aparar_espacos(buffer);

        if (almox_strcasecmp(buffer, "cancelar") == 0 || almox_strcasecmp(buffer, "sair") == 0) {
            printf(COR_AMARELO "[INFO] Compra cancelada.\n\n" COR_RESET);
            return;
        }

        if (strlen(buffer) == 0) {
            valor = 0.0;
            break;
        }

        for (int i = 0; buffer[i]; i++) {
            if (buffer[i] == ',') buffer[i] = '.';
        }

        char *endptr;
        double v = strtod(buffer, &endptr);
        if (endptr == buffer || *endptr != '\0' || v < 0.0) {
            printf(COR_AMARELO "  Preço inválido! Digite um número positivo.\n" COR_RESET);
            continue;
        }
        valor = v;
        break;
    }

    printf("\n");
    almox_adicionar(almox, nome, quantidade, valor);
    printf("\n");
}

/* Executa um comando a partir de uma lista de argumentos */
static int processar_comando(Almoxarifado *almox, int argc, char *argv[]) {
    if (argc == 0) return 1;

    char *cmd = argv[0];

    /* ------------------------------------------------------------------ */
    /* CADASTRAR / ADICIONAR                                              */
    /* Sintaxe interativa: cadastrar                                      */
    /* Sintaxe CLI rápida: cadastrar <nome> <quantidade> [valor_unitario] */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "cadastrar") == 0 || almox_strcasecmp(cmd, "novo") == 0) {
        if (argc == 1) {
            cadastrar_produto_interativo(almox);
            return 1;
        }
        if (argc < 3) {
            printf(COR_AMARELO "Dica: Digite apenas 'cadastrar' para preencher campo a campo.\n" COR_RESET);
            printf(COR_AMARELO "Ou em linha única: cadastrar <nome> <quantidade> [valor_unitario]\n" COR_RESET);
            return 1;
        }

        double valor = 0.0;
        int    qtd;
        int    nome_ate;

        if (argc >= 4 && (strchr(argv[argc - 1], '.') != NULL || strchr(argv[argc - 1], ',') != NULL)) {
            for (char *c = argv[argc - 1]; *c; c++) if (*c == ',') *c = '.';
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
    /* COMPRAR (ou adicionar estoque)                                     */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "comprar") == 0 || almox_strcasecmp(cmd, "compra") == 0 ||
        almox_strcasecmp(cmd, "adicionar") == 0 || almox_strcasecmp(cmd, "add") == 0) {

        if (argc == 1) {
            comprar_produto_interativo(almox);
            return 1;
        }

        if (argc < 3) {
            printf(COR_AMARELO "Uso: comprar <produto> <quantidade> [valor_unitario]\n" COR_RESET);
            printf("Ou digite apenas 'comprar' para preenchimento guiado.\n");
            return 1;
        }

        double valor = 0.0;
        int    qtd;
        int    nome_ate;

        if (argc >= 4 && (strchr(argv[argc - 1], '.') != NULL || strchr(argv[argc - 1], ',') != NULL)) {
            for (char *c = argv[argc - 1]; *c; c++) if (*c == ',') *c = '.';
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
    /* VENDER (ou remover estoque)                                        */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "vender") == 0 || almox_strcasecmp(cmd, "venda") == 0 ||
        almox_strcasecmp(cmd, "remover") == 0 || almox_strcasecmp(cmd, "rm") == 0 ||
        almox_strcasecmp(cmd, "del") == 0) {

        if (argc == 1) {
            vender_produto_interativo(almox);
            return 1;
        }

        if (argc < 3) {
            printf(COR_AMARELO "Uso: vender <produto> <quantidade>\n" COR_RESET);
            printf("Ou digite apenas 'vender' para preenchimento guiado.\n");
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
    /* EDITAR — Atualização interativa de produto pelo código               */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "editar") == 0 || almox_strcasecmp(cmd, "edit") == 0 ||
        almox_strcasecmp(cmd, "atualizar") == 0) {
        if (argc < 2) {
            printf(COR_AMARELO "Uso: editar <codigo>\n" COR_RESET);
            printf("Exemplo: editar 1\n");
            return 1;
        }
        int codigo = atoi(argv[1]);
        if (codigo <= 0) {
            printf(COR_VERMELHO "Erro: Código inválido '%s'. Digite um número positivo.\n" COR_RESET, argv[1]);
            return 1;
        }
        editar_produto_interativo(almox, codigo);
        return 1;
    }

    /* ------------------------------------------------------------------ */
    /* EXPORTAR — Gera arquivo CSV compatível com Excel                    */
    /* ------------------------------------------------------------------ */
    if (almox_strcasecmp(cmd, "exportar") == 0 || almox_strcasecmp(cmd, "csv") == 0) {
        const char *arquivo = (argc >= 2) ? argv[1] : "estoque.csv";
        almox_exportar_csv(almox, arquivo);
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
        printf(COR_CIANO "vendas> " COR_RESET);
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
