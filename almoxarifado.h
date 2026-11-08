#ifndef ALMOXARIFADO_H
#define ALMOXARIFADO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_ITENS      1000
#define MAX_HISTORICO  2000
#define MAX_NOME       100
#define ARQUIVO_PADRAO "vendas.json"

/*
 * ItemEstoque — schema de produtos para vendas e estoque
 */
typedef struct {
    int    codigo;           /* Código identificador único do produto */
    char   nome[MAX_NOME];
    char   dataEntrada[25];  /* Formato ISO: "AAAA-MM-DD"             */
    double precoCusto;       /* Preço de custo unitário               */
    double valorUnitario;    /* Preço de venda unitário               */
    int    quantidade;       /* Quantidade em estoque                 */
    int    estoqueMinimo;    /* Quantidade mínima de segurança        */
    int    ativo;            /* 1 = ativo, 0 = inativo                */
} ItemEstoque;

/* Registro de movimentação — compras, vendas e exclusão */
typedef struct {
    char data_hora[25];  /* "AAAA-MM-DD HH:MM:SS" */
    char operacao[16];   /* "ADICIONAR", "REMOVER" ou "DESATIVAR" */
    char nome[MAX_NOME];
    int  quantidade;
    int  saldo_anterior;
    int  saldo_novo;
} RegistroHistorico;

/* Estrutura principal do sistema de vendas e estoque */
typedef struct {
    ItemEstoque       itens[MAX_ITENS];
    int               total_itens;
    RegistroHistorico historico[MAX_HISTORICO];
    int               total_historico;
    char              caminho_arquivo[260];
    int               proximo_codigo;   /* Contador auto-incremento */
} SistemaVendas;

typedef SistemaVendas Almoxarifado; /* Alias para compatibilidade */

/* ---- Ciclo de vida ----------------------------------------------------- */
void almox_inicializar    (Almoxarifado *almox, const char *caminho);
int  almox_carregar_json  (Almoxarifado *almox);
int  almox_salvar_json    (const Almoxarifado *almox);

/* ---- CRUD / Compras e Vendas ------------------------------------------- */
int  almox_adicionar      (Almoxarifado *almox, const char *nome, int quantidade, double valorUnitario);
int  almox_adicionar_completo(Almoxarifado *almox, const char *nome, int quantidade,
                              double precoCusto, double valorUnitario, int estoqueMinimo);
int  almox_editar_produto (Almoxarifado *almox, int codigo, const char *novo_nome,
                           int nova_qtd, double novo_custo, double novo_preco,
                           int novo_estoque_min, int alterar_campos);
int  almox_remover        (Almoxarifado *almox, const char *nome, int quantidade);
int  almox_desativar      (Almoxarifado *almox, const char *nome); /* Exclusão lógica */

/* Nomenclatura oficial de Vendas e Compras */
#define vendas_cadastrar almox_adicionar
#define vendas_comprar   almox_adicionar
#define vendas_vender    almox_remover
#define vendas_listar    almox_listar
#define vendas_relatorio almox_relatorio
#define vendas_consultar almox_consultar

/* ---- Leitura / Relatórios / Exportação -------------------------------- */
void almox_listar         (const Almoxarifado *almox, int mostrar_inativos);
void almox_consultar      (const Almoxarifado *almox, const char *termo);
void almox_exibir_historico(const Almoxarifado *almox, int limite);
void almox_relatorio      (const Almoxarifado *almox); /* Equivale ao $aggregate do Mongo */
int  almox_exportar_csv   (const Almoxarifado *almox, const char *caminho_csv);

/* ---- Utilitários ------------------------------------------------------- */
int  almox_strcasecmp           (const char *s1, const char *s2);
int  almox_buscar_indice        (const Almoxarifado *almox, const char *nome);
int  almox_buscar_indice_codigo (const Almoxarifado *almox, int codigo);
void almox_obter_data_hora_atual(char *buffer, size_t tamanho);
void almox_obter_data_atual     (char *buffer, size_t tamanho);
void almox_aparar_espacos       (char *str);

#endif /* ALMOXARIFADO_H */
