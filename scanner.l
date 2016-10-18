%{
#include <stdlib.h>
#include "auxlib.h"
#include "lyutils.h"


#define YY_USER_ACTION	{ lexer::advance(); }
#define RETURN(SYMBOL) { \
	 yylval = new astree (SYMBOL, lexer::lloc, yytext); \
	 return SYMBOL; \
}

%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn

LETTER					[A-Za-z_]
DIGIT					  [0-9]
MANTISSA				({DIGIT}+\.?{DIGIT}*|\.{DIGIT}+)
EXPONENT				([Ee][+-]?{DIGIT}+)
NUMBER					({MANTISSA}{EXPONENT}?)
NOTNUMBER			  ({MANTISSA}[Ee][+-]?)
IDENT					  ({LETTER}({LETTER}|{DIGIT})*)
CHARCONST				('([^\\'\n]|\\[\\'"0nt])')
STRING					(\"([^\\"\n]|\\[\\'"0nt])*\")

%%

"#".*					  { lexer::include(); }
[ \t]+					{ }
\n							{ lexer::newline(); }

"char"				 { RETURN (TOK_CHAR); }
"else"				 { RETURN (TOK_ELSE); }
"if"					 { RETURN (TOK_IF); }
"int"					 { RETURN (TOK_INT); }
"new"					 { RETURN (TOK_NEW); }
"null"				 { RETURN (TOK_NULL); }
"return"			 { RETURN (TOK_RETURN); }
"string"			 { RETURN (TOK_STRING); }
"struct"			 { RETURN (TOK_STRUCT); }
"void"				 { RETURN (TOK_VOID); }
"while"				 { RETURN (TOK_WHILE); }

{CHARCONST}		 { RETURN (TOK_CHARCON); }
{STRING}			 { RETURN (TOK_STRINGCON); }
{NUMBER}			 { RETURN (TOK_INTCON); }
{IDENT}				 { RETURN (TOK_IDENT); }
"[]"					 { RETURN (TOK_ARRAY); }
"["						 { RETURN ('['); }
"]"						 { RETURN (']'); }
"="						 { RETURN ('='); }
"+"						 { RETURN ('+'); }
"-"						 { RETURN ('-'); }
"*"						 { RETURN ('*'); }
"/"						 { RETURN ('/'); }
"^"						 { RETURN ('^'); }
"("						 { RETURN ('('); }
")"						 { RETURN (')'); }
";"						 { RETURN (';'); }
"{"						 { RETURN ('{'); }
"}"						 { RETURN ('}'); }
"."						 { RETURN ('.'); }
"!"						 { RETURN ('!'); }
"%"						 { RETURN ('%'); }
","						 { RETURN (','); }

"=="					 { RETURN (TOK_EQ); }
"<="					 { RETURN (TOK_LE); }
">="					 { RETURN (TOK_GE); }
"!="					 { RETURN (TOK_NE); }
"<"						 { RETURN (TOK_LT); }
">"						 { RETURN (TOK_GT); }




{NOTNUMBER}		 { lexer::badtoken (yytext);  }
.							 { lexer::badchar (*yytext); }

%%
