#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define ADD		0
#define SUB		1
#define MUL		2
#define DIV		3
#define POW		4
#define LPAREN		5
#define RPAREN		6
#define INT		7
#define NODES		1024

struct token {
	int token;
	int value;
};

struct ast_node {
	struct ast_node	*left;
	struct ast_node	*right;
	int		op;
	int		value;
};

char		*buffer_data = 0;
int		buffer_length = 0;
int		buffer_index = 0;
struct token	token;
struct ast_node nodes[NODES];
int		nodenum = 0;

struct ast_node *expression(void);
struct ast_node *unary(void);

void error(char *fmt, ...) {
	va_list ap;

	fputs("Error: ", stderr);

	va_start(ap,fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fputs("\n", stderr);
	exit(1);
}

int buffer_getc(void)
{
	if (buffer_index+1 >= buffer_length) {
		return -1;
	}
	return buffer_data[buffer_index++];
}

void buffer_ungetc(int c)
{
	if (!buffer_index) return;
	buffer_data[--buffer_index] = c;
}

void buffer_init(char *data)
{
	buffer_data = data;
	buffer_length = strlen(data);
	buffer_index = 0;
}

int scanner_skip_whitespace(void)
{
	int	c;

	c = buffer_getc();
	while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
		c = buffer_getc();
	}
	return c;
}

int scanner_parse_integer(int c)
{
	int	value = 0;
	
	while (c >= '0' && c <= '9') {
		value = value * 10 + (c - '0');
		c = buffer_getc();
	}
	/* Put the non-integer character back */
	buffer_ungetc(c);
	return value;
}

int scanner_next(struct token *tok)
{
	int c;

	/* Skip whitespace */
	c = scanner_skip_whitespace();

	/* Determine the token based on the input character */
	switch (c) {
	case EOF: /* End of input */
		tok->token = EOF;
		return 0;
	case '+':
		tok->token = ADD;
		break;
	case '-':
		tok->token = SUB;
		break;
	case '*':
		tok->token = MUL;
		break;
	case '/':
		tok->token = DIV;
		break;
	case '^':
		tok->token = POW;
		break;
	case '(':
		tok->token = LPAREN;
		break;
	case ')':
		tok->token = RPAREN;
		break;
	default: /* Try to parse integer */
		if (c < '0' || c > '9') {
			error("Invalid character '%c'", c);
		}

		tok->value = scanner_parse_integer(c);
		tok->token = INT;
		break;
	}

	return 1;
}

struct ast_node *ast_alloc_node(void)
{
	if (nodenum >= NODES) {
		error("Out of AST nodes");
	}
	return &nodes[nodenum++];
}

struct ast_node *
ast_create_node(int op, struct ast_node *left, struct ast_node *right,
		int value)
{
	struct ast_node	*node;

	node = ast_alloc_node();
	node->op = op;
	node->left = left;
	node->right = right;
	node->value = value;

	return node;
}

struct ast_node *ast_create_leaf(int op, int value)
{
	return ast_create_node(op, 0, 0, value);
}

struct ast_node *parser_primary(void)
{
	struct ast_node	*node;

	switch (token.token) {
	case INT:
		node = ast_create_leaf(INT, token.value);
		scanner_next(&token); /* Advance in token list */
		return node;
	case LPAREN:
		scanner_next(&token); /* Eat '(' */
		node = expression(); /* Parse expression in parentheses */

		if (token.token != RPAREN) {
			error("')' expected");
		}
		scanner_next(&token); /* Eat ')' */
		return node;
	default:
		error("Syntax error");
		return NULL;
	}
}

struct ast_node *parser_unary(void)
{
	struct ast_node	*node;
	int		op;
	
	if (token.token == ADD || token.token == SUB) {
		op = token.token;
		scanner_next(&token);

		node = parser_unary();
		
		if (op == SUB) {
			node = ast_create_node(SUB, ast_create_leaf(INT, 0), node, 0);
		}
		return node;
	}
	
	return parser_primary();
}

struct ast_node *parser_factor(void)
{
	struct ast_node	*left, *right;
	
	left = parser_unary();
	
	if (token.token == POW) {
		scanner_next(&token);
		right = parser_factor();
		left = ast_create_node(POW, left, right, 0);
	}
	
	return left;
}

struct ast_node *parser_term(void)
{
	struct ast_node	*left, *right;
	int		op;

	left = parser_factor();

	while (token.token == MUL || token.token == DIV) {
		op = token.token;
		
		scanner_next(&token);
		right = parser_factor();
		
		if (op > POW) error("Syntax error");
		
		left = ast_create_node(op, left, right, 0);
	}
	
	return left;
}

struct ast_node *expression(void)
{
	struct ast_node	*left, *right;
	int		op;

	left = parser_term();
	
	while (token.token == ADD || token.token == SUB) {
		op = token.token;
		
		scanner_next(&token);
		right = parser_term();
		
		if (op > POW) error("Syntax error");
		
		left = ast_create_node(op, left, right, 0);
	}

	return left;
}

int interpret(struct ast_node *node)
{
	int	left, right;
	int	res, i;

	if (node->left)
		left = interpret(node->left);
	if (node->right)
		right = interpret(node->right);

	switch (node->op) {
	case ADD:
		return left + right;
	case SUB:
		return left - right;
	case MUL:
		return left * right;
	case DIV:
		if (!right) error("Division by zero");
		return left / right;
	case POW:
		res = 1;
		for (i = 0; i < right; i++) {
			res *= left;
		}
		return res;
	case INT:
		return node->value;
	default:
		error("Invalid AST operation");
	}
	return 0;
}

int main(void)
{
	char		line[256];
	struct ast_node	*node;

	printf("Welcome to the simple C calculator\n");

	for (;;) {
		printf("Enter the expression: ");
		if(!fgets(line, sizeof(line), stdin))
			break;

		buffer_init(line);
		scanner_next(&token);
		
		node = expression();
		printf("Result: %d\n", interpret(node));
	}
	
	return 0;
}
