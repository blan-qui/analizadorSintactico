
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

#define STACK_SIZE 1000

typedef enum {
    // No terminales
    NT_JSON, NT_ELEMENT, NT_OBJECT, NT_ATTRIBUTES_LIST_OPT, NT_ATTRIBUTES_LIST_PRIME,
    NT_ATTRIBUTE, NT_ATTRIBUTE_NAME, NT_ATTRIBUTE_VALUE, NT_ARRAY,
    NT_ELEMENT_LIST_OPT, NT_ELEMENT_LIST_PRIME,
    EPSILON, DOLLAR,

    // Terminales (offset desde 100 para no chocar con no terminales)
    SYM_L_LLAVE = 100, SYM_R_LLAVE, SYM_L_CORCHETE, SYM_R_CORCHETE, SYM_COMA, SYM_DOS_PUNTOS,
    SYM_STRING, SYM_NUMBER, SYM_PR_TRUE, SYM_PR_FALSE, SYM_PR_NULL, SYM_EOF
} Symbol;

Token currentToken;
Symbol stack[STACK_SIZE];
int top = -1;
//apila un simbolo en la pila
void push(Symbol sym) {
    if (top < STACK_SIZE - 1) stack[++top] = sym;
}
//desapila y devuelve el simbolo en el tope de la pial
Symbol pop() {
    return (top >= 0) ? stack[top--] : DOLLAR;
}
//devuelve el simbolo en el tope de la pila sin desapilarlo
Symbol peek() {
    return (top >= 0) ? stack[top] : DOLLAR;
}
//convierte un simbolo (terminal o no terminal) a su representcion como texto
const char* symbolToString(Symbol sym) {
    switch (sym) {
        case NT_JSON: return "json";
        case NT_ELEMENT: return "element";
        case NT_OBJECT: return "object";
        case NT_ATTRIBUTES_LIST_OPT: return "attributes-list-opt";
        case NT_ATTRIBUTES_LIST_PRIME: return "attributes-list'";
        case NT_ATTRIBUTE: return "attribute";
        case NT_ATTRIBUTE_NAME: return "attribute-name";
        case NT_ATTRIBUTE_VALUE: return "attribute-value";
        case NT_ARRAY: return "array";
        case NT_ELEMENT_LIST_OPT: return "element-list-opt";
        case NT_ELEMENT_LIST_PRIME: return "element-list'";
        case EPSILON: return "e";
        case DOLLAR: return "$";
        case SYM_L_LLAVE: return "L_LLAVE";
        case SYM_R_LLAVE: return "R_LLAVE";
        case SYM_L_CORCHETE: return "L_CORCHETE";
        case SYM_R_CORCHETE: return "R_CORCHETE";
        case SYM_COMA: return "COMA";
        case SYM_DOS_PUNTOS: return "DOS_PUNTOS";
        case SYM_STRING: return "STRING";
        case SYM_NUMBER: return "NUMBER";
        case SYM_PR_TRUE: return "PR_TRUE";
        case SYM_PR_FALSE: return "PR_FALSE";
        case SYM_PR_NULL: return "PR_NULL";
        case SYM_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}
//determina si el token actual esta en el conjunto follow de un simbolo dado
int inFollow(Symbol A, Token t) {
    switch (A) {
        case NT_ATTRIBUTES_LIST_OPT:
        case NT_ATTRIBUTES_LIST_PRIME:
        case NT_ATTRIBUTE:
            return (t == R_LLAVE);
        case NT_ELEMENT_LIST_OPT:
        case NT_ELEMENT_LIST_PRIME:
            return (t == R_CORCHETE);
        default:
            return (t == EOF_TOKEN);
    }
}

void parse_error(Symbol sym, Token tok) {
    printf("[Noo, la polizia noo. ERROR] Linea %d: se esperaba '%s', pero se encontro '%s'",
           numLinea, symbolToString(sym), tokenToString(tok));
}

void match(Token esperado) {
    if (currentToken == esperado) {
        currentToken = getToken();
    } else {
        parse_error((Symbol)(esperado + 100), currentToken);
        currentToken = getToken();  // Panic Mode SCAN
    }
}
//aplica una regla gramatical a partir del simbolo no terminal en el tope de la pila
void applyRule(Symbol sym) {
    switch (sym) {
        case NT_JSON:
            push(SYM_EOF);
            push(NT_ELEMENT);
            break;
        case NT_ELEMENT:
            if (currentToken == L_LLAVE) push(NT_OBJECT);
            else if (currentToken == L_CORCHETE) push(NT_ARRAY);
            else parse_error(sym, currentToken);
            break;
        case NT_OBJECT:
            match(L_LLAVE);
            if (currentToken == R_LLAVE) match(R_LLAVE);
            else {
                push(SYM_R_LLAVE);
                push(NT_ATTRIBUTES_LIST_OPT);
            }
            break;
        case NT_ATTRIBUTES_LIST_OPT:
            if (currentToken == STRING) {
                push(NT_ATTRIBUTES_LIST_PRIME);
                push(NT_ATTRIBUTE);
            } else if (currentToken == R_LLAVE) {
                push(EPSILON);
            } else {
                parse_error(sym, currentToken);
            }
            break;
        case NT_ATTRIBUTES_LIST_PRIME:
            if (currentToken == COMA) {
                match(COMA);
                push(NT_ATTRIBUTES_LIST_PRIME);
                push(NT_ATTRIBUTE);
            } else if (currentToken == R_LLAVE) {
                push(EPSILON);
            } else {
                parse_error(sym, currentToken);
            }
            break;
        case NT_ATTRIBUTE:
            push(NT_ATTRIBUTE_VALUE);
            push(SYM_DOS_PUNTOS);
            push(NT_ATTRIBUTE_NAME);
            break;
        case NT_ATTRIBUTE_NAME:
            match(STRING);
            break;
        case NT_ATTRIBUTE_VALUE:
            if (currentToken == STRING || currentToken == NUMBER ||
                currentToken == PR_TRUE || currentToken == PR_FALSE || currentToken == PR_NULL) {
                match(currentToken);
            } else if (currentToken == L_LLAVE || currentToken == L_CORCHETE) {
                push(NT_ELEMENT);
            } else {
                parse_error(sym, currentToken);
            }
            break;
        case NT_ARRAY:
            match(L_CORCHETE);
            if (currentToken == R_CORCHETE) match(R_CORCHETE);
            else {
                push(SYM_R_CORCHETE);
                push(NT_ELEMENT_LIST_OPT);
            }
            break;
        case NT_ELEMENT_LIST_OPT:
            if (currentToken == L_LLAVE || currentToken == L_CORCHETE) {
                push(NT_ELEMENT_LIST_PRIME);
                push(NT_ELEMENT);
            } else if (currentToken == R_CORCHETE) {
                push(EPSILON);
            } else {
                parse_error(sym, currentToken);
            }
            break;
        case NT_ELEMENT_LIST_PRIME:
            if (currentToken == COMA) {
                match(COMA);
                push(NT_ELEMENT_LIST_PRIME);
                push(NT_ELEMENT);
            } else if (currentToken == R_CORCHETE) {
                push(EPSILON);
            } else {
                parse_error(sym, currentToken);
            }
            break;
        default:
            break;
    }
}

int main() {
    initLexer("fuente.txt");
    currentToken = getToken();

    push(SYM_EOF);
    push(NT_JSON);

    while (peek() != DOLLAR) {
        Symbol topSymbol = peek();

        if (topSymbol == EPSILON) {
            pop();
        } else if (topSymbol >= SYM_L_LLAVE) {
            if ((int)(topSymbol - SYM_L_LLAVE) == currentToken) {
                pop();
                currentToken = getToken();
            } else {
                printf("[ERROR] Linea %d: se esperaba '%s', pero se encontro '%s'",
                       numLinea, symbolToString(topSymbol), tokenToString(currentToken));
                currentToken = getToken();
            }
        } else {
            pop();
            applyRule(topSymbol);
        }
    }

    if (currentToken == EOF_TOKEN) {
        printf("[ta bien] JSON aceptado por el parser :).");
    } else {
        printf("[ERROR] Tokens adicionales tras analisis.");
    }

    return 0;
}

