//Bijan Semnani bsemnani
//Ricardo Munoz riamunoz
#include "astree.h"

extern size_t next_block;

using symbol_entry = pair<const string*, symbol*>;

struct symbol {
  attr_bitset attributes;
  symbol_table* fields;
  location lloc;
  size_t block_nr;
  vector<symbol*>* parameters;
};

// Functions used by the symbol stack
void enter_block();
void leave_block();
void define_ident();
symbol* find_ident(astree* node);
void insert_symbol(symbol_table* table, astree* node);
symbol* search_symbol(symbol_table* table, astree* node);

// Functions used to typecheck and build the symbol tables
void typecheck(FILE* outfile, astree* root);
void recr_typecheck(FILE* outfile, astree* root);
void typecheck_node(FILE* outfile, astree* node);
void copyAttr(astree* parent, astree* child);
bool checkComp(astree* node1, astree* node2);
bool checkStructural(astree* node1, astree* node2);
void blockCheck(astree* node);
void checkPro(FILE* outFile, astree* node);
void checkFunc(FILE* outFile, astree* node);
void printsym(FILE* outFile, astree* node);
