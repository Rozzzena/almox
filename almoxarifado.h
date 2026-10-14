#ifndef ALMOXARIFADO_H
#define ALMOXARIFADO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_ITENS 1000
#define MAX_HISTORICO 2000
#define MAX_NOME 100
#define ARQUIVO_PADRAO "almoxarifado.json"

/* Estrutura para um item do estoque */
typedef struct {
    char nome[MAX_NOME];
    int quantidade;
} ItemEstoque;

/* Estrutura para o registro de movimentação */
typedef struct {
    char data_hora[25];  /* Formato: "AAAA-MM-DD HH:MM:SS" */
    char operacao[16];   /* "ADICIONAR" ou "REMOVER" */
    char nome[MAX_NOME];
    int quantidade;
    int saldo_anterior;
    int saldo_novo;
} RegistroHistorico;

/* Estrutura principal do almoxarifado */
typedef struct {
    ItemEstoque itens[MAX_ITENS];
    int total_itens;
    RegistroHistorico historico[MAX_HISTORICO];
    int total_historico;
    char caminho_arquivo[260];
} Almoxarifado;

/* Protótipos das funções principais */
void almox_inicializar(Almoxarifado *almox, const char *caminho);
int  almox_carregar_json(Almoxarifado *almox);
int  almox_salvar_json(const Almoxarifado *almox);

int  almox_adicionar(Almoxarifado *almox, const char *nome, int quantidade);
int  almox_remover(Almoxarifado *almox, const char *nome, int quantidade);
void almox_listar(const Almoxarifado *almox);
void almox_consultar(const Almoxarifado *almox, const char *termo);
void almox_exibir_historico(const Almoxarifado *almox, int limite);

/* Funções utilitárias */
int  almox_strcasecmp(const char *s1, const char *s2);
int  almox_buscar_indice(const Almoxarifado *almox, const char *nome);
void almox_obter_data_hora_atual(char *buffer, size_t tamanho);
void almox_aparar_espacos(char *str);

#endif /* ALMOXARIFADO_H */
