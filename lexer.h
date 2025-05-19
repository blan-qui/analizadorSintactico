
#ifndef LEXER_H
#define LEXER_H

typedef enum {
    L_LLAVE, R_LLAVE, L_CORCHETE, R_CORCHETE, COMA, DOS_PUNTOS,
    STRING, NUMBER, PR_TRUE, PR_FALSE, PR_NULL, EOF_TOKEN, ERROR_TOKEN
} Token;

extern int numLinea;

void initLexer(const char *filename);
Token getToken();
const char* tokenToString(Token token);

#endif

