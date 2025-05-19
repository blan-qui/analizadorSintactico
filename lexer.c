
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

#define TAMLEX 256

FILE *archivo_fuente;
char lexema[TAMLEX];
int numLinea = 1;

void initLexer(const char *filename) {
    archivo_fuente = fopen(filename, "r");
    if (!archivo_fuente) {
        fprintf(stderr, "Error al abrir el archivo fuente.\n");
        exit(1);
    }
}

const char* tokenToString(Token token) {
    switch (token) {
        case L_LLAVE: return "L_LLAVE";
        case R_LLAVE: return "R_LLAVE";
        case L_CORCHETE: return "L_CORCHETE";
        case R_CORCHETE: return "R_CORCHETE";
        case COMA: return "COMA";
        case DOS_PUNTOS: return "DOS_PUNTOS";
        case STRING: return "STRING";
        case NUMBER: return "NUMBER";
        case PR_TRUE: return "PR_TRUE";
        case PR_FALSE: return "PR_FALSE";
        case PR_NULL: return "PR_NULL";
        case EOF_TOKEN: return "EOF";
        case ERROR_TOKEN: return "ERROR";
        default: return "UNKNOWN";
    }
}

Token procesar_literal_cadena() {
    int c, i = 0;
    while ((c = fgetc(archivo_fuente)) != EOF && c != '"') {
        if (i < TAMLEX - 1) lexema[i++] = c;
    }
    lexema[i] = '\0';
    if (c == '"') return STRING;
    return ERROR_TOKEN;
}

Token procesar_numero(int c) {
    int i = 0, tiene_punto = 0, tiene_exp = 0;
    lexema[i++] = c;
    while ((c = fgetc(archivo_fuente)) != EOF) {
        if (isdigit(c)) lexema[i++] = c;
        else if (c == '.' && !tiene_punto) {
            lexema[i++] = c; tiene_punto = 1;
        } else if ((c == 'e' || c == 'E') && !tiene_exp) {
            lexema[i++] = c; tiene_exp = 1;
            c = fgetc(archivo_fuente);
            if (c == '+' || c == '-' || isdigit(c)) {
                lexema[i++] = c;
            } else {
                return ERROR_TOKEN;
            }
        } else {
            ungetc(c, archivo_fuente); break;
        }
    }
    lexema[i] = '\0';
    return NUMBER;
}

Token procesar_palabra(char *palabra) {
    if (strcmp(palabra, "true") == 0) return PR_TRUE;
    if (strcmp(palabra, "false") == 0) return PR_FALSE;
    if (strcmp(palabra, "null") == 0) return PR_NULL;
    return ERROR_TOKEN;
}

Token getToken() {
    int c;
    while ((c = fgetc(archivo_fuente)) != EOF) {
        if (isspace(c)) {
            if (c == '\n') numLinea++;
            continue;
        }
        if (c == '{') { printf("TOKEN: L_LLAVE\n"); return L_LLAVE; }
        if (c == '}') { printf("TOKEN: R_LLAVE\n"); return R_LLAVE; }
        if (c == '[') { printf("TOKEN: L_CORCHETE\n"); return L_CORCHETE; }
        if (c == ']') { printf("TOKEN: R_CORCHETE\n"); return R_CORCHETE; }
        if (c == ',') { printf("TOKEN: COMA\n"); return COMA; }
        if (c == ':') { printf("TOKEN: DOS_PUNTOS\n"); return DOS_PUNTOS; }
        if (c == '"') {
            Token t = procesar_literal_cadena();
            printf("TOKEN: STRING\n");
            return t;
        }
        if (isdigit(c)) {
            Token t = procesar_numero(c);
            printf("TOKEN: NUMBER\n");
            return t;
        }
        if (isalpha(c)) {
            int i = 0;
            lexema[i++] = c;
            while ((c = fgetc(archivo_fuente)) != EOF && (isalnum(c) || c == '_')) {
                if (i < TAMLEX - 1) lexema[i++] = c;
            }
            lexema[i] = '\0';
            if (c != EOF) ungetc(c, archivo_fuente);
            Token t = procesar_palabra(lexema);
            printf("TOKEN: %s\n", tokenToString(t));
            return t;
        }
        printf("TOKEN: ERROR\n");
        return ERROR_TOKEN;
    }
    printf("TOKEN: EOF\n");
    return EOF_TOKEN;
}

