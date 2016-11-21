%{
//Bijan Semnani bsemnani
//Ricardo Munoz riamunoz

#include <cassert>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%printer { astree::dump (yyoutput, $$); } <>

%initial-action {
   lexer::root = new astree (TOK_ROOT, {0, 0, 0}, "<<ROOT>>");
}


%token TOK_VOID TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_INITDECL TOK_FALSE TOK_TRUE
%token TOK_POS TOK_NEG TOK_NEWARRAY
%token TOK_ORD TOK_CHR

%token TOK_ROOT TOK_DECLID TOK_TYPEID TOK_FIELD TOK_INDEX
%token TOK_CALL TOK_NEWSTRING TOK_IFELSE TOK_RETURNVOID
%token TOK_BLOCK TOK_VARDECL TOK_FUNCTION TOK_PARAMLIST TOK_PROTOTYPE

%nonassoc then
%right  TOK_IF TOK_ELSE
%right  '='
%left   TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left   '+' '-'
%left   '*' '/' '%'
%right  TOK_POS TOK_NEG '!' TOK_NEW TOK_ORD TOK_CHR
%left   TOK_ARRAY TOK_FIELD TOK_FUNCTION
%left   '[' '.'

%nonassoc '('
%start start

%%
start    : program              { lexer::root = $1; }
         ;

program  : program structdef    { $$ = astree::adoptOne ($1, $2); }
         | program function     { $$ = astree::adoptOne ($1, $2); }
         | program statement    { $$ = astree::adoptOne ($1, $2); }
         | program error '}'    { $$ = $1; }
         | program error ';'    { $$ = $1; }
         |                      { $$ = lexer::root; }
         ;

structdef: TOK_STRUCT TOK_IDENT '{' '}'
              {
                astree::astreeFree($3);
                astree::astreeFree($4);
                $2 = astree::adopt_sym($2, TOK_TYPEID);
                $$ = astree::adoptOne($1,$2);
              }
         | TOK_STRUCT TOK_IDENT field '}'
              {
                astree::astreeFree($4);
                $2 = astree::adopt_sym($2, TOK_TYPEID);
                $$ = astree::adoptTwo($1,$2,$3);
              }
field    : '{' fielddecl ';'
              {
                astree::astreeFree($3);
                $$ = astree::adoptOne($1,$2);
              }

         | field fielddecl ';'
              {
                astree::astreeFree($3);
                $$ = astree::adoptOne($1,$2);
              }
         ;

fielddecl: basetype TOK_ARRAY TOK_IDENT
              {
                $3 = astree::adopt_sym($3, TOK_TYPEID);
                $$ = astree::adoptTwo($2, $1, $3);
              }

         | basetype TOK_IDENT
              {
                $2 = astree::adopt_sym($2, TOK_TYPEID);
                $$ = astree::adoptOne($1, $2);
              }
         ;

basetype : TOK_VOID    { $$ = $1;}
         | TOK_CHAR    { $$ = $1;}
         | TOK_INT     { $$ = $1;}
         | TOK_STRING  { $$ = $1;}
         | TOK_IDENT   { $$ = astree::adopt_sym($1, TOK_TYPEID);}
         ;

function : identdecl '(' ')' ';'
                {
                  astree::astreeFree($3);
                  astree::astreeFree($4);
                  $2 = astree::adopt_sym($2, TOK_PARAMLIST);
                  $$ = new astree(TOK_PROTOTYPE,$1->lloc, "");
                  $$ = astree::adoptTwo($$, $1, $2);
                }
         | identdecl '(' ')' block
                {
                  astree::astreeFree($3);
                  $2 = astree::adopt_sym($2, TOK_PARAMLIST);
                  $$ = new astree(TOK_FUNCTION, $1->lloc, "");
                  $$ = astree::adoptThree($$, $1, $2, $4);
                }
         | identdecl param ')' ';'
                {
                  astree::astreeFree($3);
                  astree::astreeFree($4);
                  $$ = new astree(TOK_PROTOTYPE, $1->lloc, "");
                  $$ = astree::adoptTwo($$, $1, $2);
                }
         | identdecl param ')' block
                {
                  astree::astreeFree($3);
                  $$ = new astree(TOK_FUNCTION, $1->lloc, "");
                  $$ = astree::adoptThree($$,$1,$2, $4);
                }
         ;

param    : '(' identdecl
                {
                  $1 = astree::adopt_sym($1, TOK_PARAMLIST);
                  $$ = astree::adoptOne($1,$2);
                }
         | param ',' identdecl
                {
                  astree::astreeFree($2);
                  $$ = astree::adoptOne($1, $3);
                }
         ;

identdecl: basetype TOK_ARRAY TOK_IDENT
                {
                  $3 = astree::adopt_sym($3, TOK_DECLID);
                  $$ = astree::adoptTwo($2, $1, $3);
                }
         | basetype TOK_IDENT
                {
                  $2 = astree::adopt_sym($2, TOK_DECLID);
                  $$ = astree::adoptOne($1, $2);
                }
         ;

block    : '{' '}'
                {
                  astree::astreeFree($2);
                  $$ = astree::adopt_sym($1, TOK_BLOCK);
                }
         | state '}'
                {
                  astree::astreeFree($2);
                  $$ = astree::adopt_sym($1, TOK_BLOCK);
                }
         ;

state    : '{' statement
                {
                  astree::adopt_sym($1, TOK_BLOCK);
                  $$ = astree::adoptOne($1, $2);
                }
         | state statement
                { $$ = astree::adoptOne($1, $2); }
         ;

statement: block      { $$ = $1;}
         | vardecl    { $$ = $1;}
         | while      { $$ = $1;}
         | ifelse     { $$ = $1;}
         | return     { $$ = $1;}
         | expr ';'   {
                        astree::astreeFree($2);
                        $$ = $1;
                      }
         | ';'        { $$ = $1;}
         ;

vardecl  : identdecl '=' expr ';'
                {
                  astree::astreeFree($4);
                  $2 = astree::adopt_sym($2, TOK_VARDECL);
                  $$ = astree::adoptTwo($2, $1, $3);
                }
         ;

while    : TOK_WHILE '(' expr ')' statement
                {
                  astree::astreeFree($2);
                  astree::astreeFree($4);
                  $$ = astree::adoptTwo($1, $3, $5);
                }
         ;

ifelse    : TOK_IF '(' expr ')' statement %prec TOK_ELSE
                {
                  astree::astreeFree($2);
                  astree::astreeFree($4);
                  $$ = astree::adoptTwo($1, $3, $5);
                }

          | TOK_IF '(' expr ')' statement TOK_ELSE statement
                {
                  astree::astreeFree($2);
                  astree::astreeFree($4);
                  $1 = astree::adopt_sym($1, TOK_IFELSE);
                  $$ = astree::adoptThree($1, $3, $5, $7);
                }
          ;

return    : TOK_RETURN ';'
                {
                  astree::astreeFree($2);
                  $1 = astree::adopt_sym($1, TOK_RETURNVOID);
                }

          | TOK_RETURN expr ';'
                {
                  astree::astreeFree($3);
                  $$ = astree::adoptOne($1, $2);
                }
          ;

expr      : allocator           { $$ = $1; }
          | call                { $$ = $1; }
          | '(' expr ')'
                                {
                                  astree::astreeFree($1);
                                  astree::astreeFree($3);
                                  $$ = $2;
                                }
          | variable            { $$ = $1; }
          | constant            { $$ = $1; }


          | expr TOK_GT expr    { $$ = astree::adoptTwo($2,$1,$3); }
          | expr TOK_LT expr    { $$ = astree::adoptTwo($2,$1,$3); }
          | expr TOK_LE expr    { $$ = astree::adoptTwo($2,$1,$3); }
          | expr TOK_GE expr    { $$ = astree::adoptTwo($2,$1,$3); }
          | expr TOK_EQ expr    { $$ = astree::adoptTwo($2,$1,$3); }
          | expr TOK_NE expr    { $$ = astree::adoptTwo($2,$1,$3); }
          | expr '+' expr       { $$ = astree::adoptTwo($2,$1,$3); }
          | expr '-' expr       { $$ = astree::adoptTwo($2,$1,$3); }
          | expr '/' expr       { $$ = astree::adoptTwo($2,$1,$3); }
          | expr '*' expr       { $$ = astree::adoptTwo($2,$1,$3); }
          | expr '=' expr       { $$ = astree::adoptTwo($2,$1,$3); }
          | '+' expr %prec TOK_POS
                                {
                                  $$ = $1;
                                }
          | '-' expr %prec TOK_NEG
                                {
                                  $$ = $1;
                                }
          | '!' expr            { $$ = astree::adoptOne($1,$2); }
          | TOK_ORD expr        { $$ = $1; }
          | TOK_CHR             { $$ = $1; }
          | TOK_NEW             { $$ = $1; }
          ;



allocator : TOK_NEW TOK_IDENT '(' ')'
                {
                  astree::astreeFree($3);
                  astree::astreeFree($4);
                  $2 = astree::adopt_sym($2, TOK_TYPEID);
                  $$ = astree::adoptOne($1, $2);
                }
          | TOK_NEW TOK_STRING '('expr ')'
                {
                  astree::astreeFree($2);
                  astree::astreeFree($3);
                  astree::astreeFree($5);
                  $1 = astree::adopt_sym($1, TOK_NEWSTRING);
                  $$ = astree::adoptOne($1, $4);
                }
          | TOK_NEW basetype '[' expr ']'
                {
                  astree::astreeFree($3);
                  astree::astreeFree($5);
                  $1 = astree::adopt_sym($1, TOK_NEWARRAY);
                  $$ = astree::adoptTwo($1, $2, $4);
                }
          ;

call      : TOK_IDENT '(' ')'
                {
                  astree::astreeFree($3);
                  $2 = astree::adopt_sym($2, TOK_CALL);
                  $$ = astree::adoptOne($2, $1);
                }
          | EXP ')'
                {
                  astree::astreeFree($2);
                  $$ = $1;
                }
          ;

EXP       : TOK_IDENT '(' expr
                {
                  $2 = astree::adopt_sym($2, TOK_CALL);
                  $$ = astree::adoptTwo($2, $1, $3);
                }
          | EXP ',' expr
                {
                  astree::astreeFree($2);
                  $$ = astree::adoptOne($1, $3);
                }
          ;

variable  : TOK_IDENT           { $$ = $1; }
          | expr '[' expr ']'
                {
                  astree::astreeFree($4);
                  $2 = astree::adopt_sym($2, TOK_INDEX);
                  $$ = astree::adoptTwo($2, $1, $3);
                }
          | expr '.' TOK_IDENT
                {
                  $3 = astree::adopt_sym($3, TOK_FIELD);
                  $$ = astree::adoptTwo($2, $1, $3);
                }
          ;

constant  : TOK_INTCON          { $$ = $1; }
          | TOK_CHARCON         { $$ = $1; }
          | TOK_STRINGCON       { $$ = $1; }
          | TOK_NULL            { $$ = $1; }
          | TOK_FALSE           { $$ = $1; }
          | TOK_TRUE            { $$ = $1; }
          ;
%%


const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}
