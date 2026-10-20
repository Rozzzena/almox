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
#define ARQUIVO_PADRAO "almoxarifado.json"

/*
 * ItemEstoque — schema espelhado da collection 'produtos' do MongoDB
 * (referência: teste/almoxarifadopim.mongodb.js)
 *
 *   db.produtos.insertOne({
 *     codigo: 101, nome: 'Parafuso M6', dataEntrada: new Date('2026-09-10'),
 *     valorUnitario: 0.50, quantidade: 500, ativo: true
 *   });
 */
typedef struct {
    int    codigo;           /* PK auto-incremento (equivalente ao _id no Mongo) */
    char   nome[MAX_NOME];
    char   dataEntrada[25];  /* Formato ISO: "AAAA-MM-DD"                         */
    double valorUnitario;
    int    quantidade;
    int    ativo;            /* 1 = true (ativo), 0 = false (exclusão lógica)     */
} ItemEstoque;

/* Registro de movimentação — array "historico" do JSON */
typedef struct {
    char data_hora[25];  /* "AAAA-MM-DD HH:MM:SS" */
    char operacao[16];   /* "ADICIONAR", "REMOVER" ou "DESATIVAR" */
    char nome[MAX_NOME];
    int  quantidade;
    int  saldo_anterior;
    int  saldo_novo;
} RegistroHistorico;

/* Estrutura principal do almoxarifado */
typedef struct {
    ItemEstoque       itens[MAX_ITENS];
    int               total_itens;
    RegistroHistorico historico[MAX_HISTORICO];
    int               total_historico;
    char              caminho_arquivo[260];
    int               proximo_codigo;   /* Contador auto-incremento */
} Almoxarifado;

/* ---- Ciclo de vida ----------------------------------------------------- */
void almox_inicializar    (Almoxarifado *almox, const char *caminho);
int  almox_carregar_json  (Almoxarifado *almox);
int  almox_salvar_json    (const Almoxarifado *almox);

/* ---- CRUD -------------------------------------------------------------- */
int  almox_adicionar      (Almoxarifado *almox, const char *nome, int quantidade, double valorUnitario);
int  almox_remover        (Almoxarifado *almox, const char *nome, int quantidade);
int  almox_desativar      (Almoxarifado *almox, const char *nome); /* Exclusão lógica */

/* ---- Leitura / Relatórios --------------------------------------------- */
void almox_listar         (const Almoxarifado *almox, int mostrar_inativos);
void almox_consultar      (const Almoxarifado *almox, const char *termo);
void almox_exibir_historico(const Almoxarifado *almox, int limite);
void almox_relatorio      (const Almoxarifado *almox); /* Equivale ao $aggregate do Mongo */

/* ---- Utilitários ------------------------------------------------------- */
int  almox_strcasecmp           (const char *s1, const char *s2);
int  almox_buscar_indice        (const Almoxarifado *almox, const char *nome);
void almox_obter_data_hora_atual(char *buffer, size_t tamanho);
void almox_obter_data_atual     (char *buffer, size_t tamanho);
void almox_aparar_espacos       (char *str);

#endif /* ALMOXARIFADO_H */
