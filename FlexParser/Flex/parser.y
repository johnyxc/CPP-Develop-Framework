%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Parser/parser.h"

extern int yylineno;
extern int yylex();
extern void yyset_in(FILE* _in_str);

int yyparse();

void process_global_comment(const char* fn);
void process_single_line_comment(const char* fn);
void process_index(const char* fn);
void process_create_new_file(const char* fn);
void process_take_bits(const char* fn);
void process_cmd(const char* fn);

void yyerror(const char* str) {
    fprintf(stderr, " error: %s, Line: %d\n", str, yylineno);
}

int yywrap() {
    return 1;
}

int main(int argc, char** argv)
{
	--argc;
    ++argv;
    if (argc > 0)
	{
        yyset_in(fopen(argv[0], "r"));
	}
    else
	{
        yyset_in(fopen("cmd_def.zdef", "r"));
	}

	--argc;
	++argv;
	if (argc > 0)
	{
		jf_cmd_parser_t::instance()->set_target_type(argv[0]);
	}
	else
	{
		jf_cmd_parser_t::instance()->set_target_type(g_tar_cpp.c_str());
	}

    yyparse();
	jf_cmd_parser_t::instance()->output();
	system("pause");
}
%}

%token GLOBALCMT SGLINECMT
%token INDEX FILENAME TAKEBITS
%token CMDNAME

%%
commands: /* empty */
	| commands command
	;

	command:
		global_comment
		|
		single_line_comment
		|
		index
		|
		create_new_file
		|
		get_bits
		|
		generate_command
		;

	global_comment:
		GLOBALCMT
		{
			process_global_comment((char*)yylval);
			printf("Global Comment : %s\n", (char*)yylval);
		}
		;

	single_line_comment:
		SGLINECMT
		{
			process_single_line_comment((char*)yylval);
			printf("Single Line Comment : %s\n", (char*)yylval);
		}
		;

	index:
		INDEX
		{
			process_index((char*)yylval);
		}
		;

	create_new_file:
		FILENAME
		{
			process_create_new_file((char*)yylval);
		}
		;

	get_bits:
		TAKEBITS
		{
			process_take_bits((char*)yylval);
		}
		;

	generate_command:
		CMDNAME
		{
			process_cmd((char*)yylval);
		}
		;
%%

static int stage = 1;
static int cmd_idx = 0;
static int treat_as_sigleline_cmt = 0;

void process_global_comment(const char* cont)
{
	printf("process_global_comment tasc %d cur_line %d\n", treat_as_sigleline_cmt, yylineno);
	if (treat_as_sigleline_cmt == yylineno) {
		process_single_line_comment(cont);
		treat_as_sigleline_cmt = 0;
	} else {
		jf_cmd_parser_t::instance()->set_global_comment(cont);
	}
}

void process_single_line_comment(const char* cont)
{
	jf_cmd_parser_t::instance()->set_single_line_comment(cont, yylineno);
}

void process_index(const char* cont)
{
	stage = 0;
	if (stage != 0)
	{
		yyerror("Syntax Error");
		system("pause");
		exit(0);
	}
	else
	{
		++stage;
		printf("Index : %s\n", cont);
	}
}

void process_create_new_file(const char* cont)
{
	stage = 1;
	cmd_idx = 0;
	if (stage != 1)
	{
		yyerror("Syntax Error");
		system("pause");
		exit(0);
	}
	else
	{
		++stage;
		jf_cmd_parser_t::instance()->set_file_name(cont);
		printf("New File Name : %s\n", cont);
	}
}

void process_take_bits(const char* cont)
{
	if (stage != 2)
	{
		yyerror("Syntax Error");
		system("pause");
		exit(0);
	}
	else
	{
		++stage;
		jf_cmd_parser_t::instance()->set_bits(cont);
		printf("Takes Bits : %s\n", cont);
	}
}

void process_cmd(const char* cont)
{
	if (stage != 3)
	{
		yylineno -= 1;
		yyerror("Syntax Error");
		system("pause");
		exit(0);
	}
	else
	{
		treat_as_sigleline_cmt = yylineno;
		jf_cmd_parser_t::instance()->set_cmd_name(cont, yylineno, cmd_idx++);
		printf("Cmd Name : %s\n", cont);
	}
}
