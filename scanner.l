
%{
#include <stdlib.h>
#include "auxlib.h"
#include "lyutils.h"


#define YY_USER_ACTION  { lexer::advance(); }
#define RETURN(SYMBOL) { \
   yylval = new astree (SYMBOL, lexer::lloc, yytext); \
   lexer::dump(SYMBOL);\
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

LETTER          [A-Za-z_]
NUMBER          ([0-9]+)
NOTIDENT        ({NUMBER}{IDENT})
NOTCHAR         ('([^\\'\n]|\\[\\'"0nt])[^'])
NOTSTRING       (\"([^\\"\n]|\\[\\'"0nt])+[^\"])
IDENT           ({LETTER}({LETTER}|{NUMBER})*)
CHAR            ('([^\\'\n]|\\[\\'"0nt])')
STRING          (\"([^\\"\n]|\\[\\'"0nt])*\")

%%

"#".*            { lexer::include(); }
[ \t]+          { }
\n              { lexer::newline(); }

"char"         { RETURN (TOK_CHAR); }
"else"         { RETURN (TOK_ELSE); }
"if"           { RETURN (TOK_IF); }
"int"          { RETURN (TOK_INT); }
"new"          { RETURN (TOK_NEW); }
"null"         { RETURN (TOK_NULL); }
"return"       { RETURN (TOK_RETURN); }
"string"       { RETURN (TOK_STRING); }
"struct"       { RETURN (TOK_STRUCT); }
"void"         { RETURN (TOK_VOID); }
"while"        { RETURN (TOK_WHILE); }

{CHAR}          { RETURN (TOK_CHARCON); }
{STRING}        { RETURN (TOK_STRINGCON); }
{NUMBER}        { RETURN (TOK_INTCON); }
{IDENT}         { RETURN (TOK_IDENT); }
"[]"            { RETURN (TOK_ARRAY); }
"["             { RETURN ('['); }
"]"             { RETURN (']'); }
"="             { RETURN ('='); }
"+"             { RETURN ('+'); }
"-"             { RETURN ('-'); }
"*"             { RETURN ('*'); }
"/"             { RETURN ('/'); }
"^"             { RETURN ('^'); }
"("             { RETURN ('('); }
")"             { RETURN (')'); }
";"             { RETURN (';'); }
"{"             { RETURN ('{'); }
"}"             { RETURN ('}'); }
"."             { RETURN ('.'); }
"!"             { RETURN ('!'); }
"%"             { RETURN ('%'); }
","             { RETURN (','); }

"=="           { RETURN (TOK_EQ); }
"<="           { RETURN (TOK_LE); }
">="           { RETURN (TOK_GE); }
"!="           { RETURN (TOK_NE); }
"<"            { RETURN (TOK_LT); }
">"            { RETURN (TOK_GT); }



{NOTIDENT}     { lexer::badtoken (yytext);  }
{NOTCHAR}       { lexer::badtoken (yytext);  }
{NOTSTRING}     { lexer::badchar (*yytext);  }
.               { lexer::badchar (*yytext); }

%%
