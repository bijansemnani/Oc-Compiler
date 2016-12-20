
#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>
#include <unordered_map>
using namespace std;

#include "auxlib.h"

typedef enum {
    ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
    ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
    ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval,
    ATTR_const, ATTR_vreg, ATTR_vaddr, ATTR_bitset_size,
}Flags;

struct symbol;
using symbol_table = unordered_map<const string*, symbol*>;
using attr_bitset = bitset<ATTR_bitset_size>;

struct location {
   size_t filenr;
   size_t linenr;
   size_t offset;
};
// Create a shorthand notation for the bitset

struct astree {

   // Fields.
   int symbol;               // token code
   location lloc;            // source location
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   attr_bitset attributes;
   size_t blocknr;
   symbol_table* struct_table;

   // Functions.
   astree (int symbol, const location&, const char* lexinfo);
   static void astreeFree(astree* root);
   static astree* adoptOne (astree* child1, astree* child2);
   static astree* adoptTwo (astree* child1,
     astree* child2, astree* child3);
   static astree* adoptThree (astree* child1,
     astree* child2, astree* child3, astree* child4);
   static astree* adopt_sym (astree* child, int symbol);
   static string ATtoST(astree* node);
   void dump_node (FILE*);
   void dump_tree (FILE*, int depth = 0);
   static void dump (FILE* outfile, astree* tree);
   static void print (FILE* outfile, astree* tree, int depth = 0);
};

void destroy (astree* tree1, astree* tree2 = nullptr);

void errllocprintf (const location&, const char* format, const char*);

#endif
