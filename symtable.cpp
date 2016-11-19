//Bijan Semnani bsemnani
//Ricardo Munoz riamunoz
#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <string.h>
#include "symtable.h"
#include "astree.h"
#include "lyutils.h"

using namespace std;

// helper function to convert node -> symbol
symbol* createSym(astree* node, size_t next_block);

vector<symbol_table*> symbol_stack;
symbol_table* structTable = new symbol_table;
size_t next_block = 1;

//=============== SYMBOL TABLE ================= //
// converts a node -> symbol
symbol* createSym(astree* node)
{
  symbol* ret = new symbol();
  ret->attributes = node->attributes;
  ret->fields = nullptr;
  ret->lloc = node->lloc;
  ret->block_nr = node->blocknr;
  ret->parameters = nullptr;
  return ret;
}

void insert_symbol(symbol_table* table, astree* node){
  symbol* newSym = createSym(node);

  if ((table!=NULL) && (node!=NULL)){
    table->insert(make_pair(node->lexinfo, newSym));
  }

}

symbol* search_symbol(symbol_table* table, astree* node){
  if (table->count(node->lexinfo)==0){
    return nullptr;
  }
  return (table->find(node->lexinfo))->second;
}

// ===============  SYMBOL STACK =============== //
void enter_block()
{
  next_block++;
  symbol_stack.push_back(nullptr);
}

void leave_block()
{
  symbol_stack.pop_back();
}

void define_ident(astree* node)
{
  if(symbol_stack.back() == nullptr)
  {
    symbol_stack.back() = new symbol_table;
  }
  symbol* to_insert = createSym(node);
  symbol_stack.back()->insert(make_pair(node->lexinfo, to_insert));
}

symbol* find_ident(astree* node)
{
  for (auto table : symbol_stack) {
    if (table != nullptr && !(table->empty())) {
      if (search_symbol (table, node) != nullptr) {
        return search_symbol (table, node);
      }
    }
  }
    return nullptr;
}

// ===============  TYPE CHECK =============== //
void copyAttr(astree* parent, astree* child){
  for(size_t i = 0; i<ATTR_bitset_size; i++){
    if(child->attributes[i] == 1)
      parent->attributes.set(i);
  }
}
bool checkStructural(astree* node1, astree* node2){
  bool returnVal = false;
  if(node1->attributes.test(ATTR_function) ^
     node2->attributes.test(ATTR_function))
     returnVal = false;
  symbol* func1 = createSym(node1);
  symbol* func2 = createSym(node2);
  if(func1->parameters->size() == func2->parameters->size()){
    for(size_t i = 0; i<func1->parameters->size();i++){
      vector<symbol*> param1 = func1->parameters[i];
      vector<symbol*> param2 = func2->parameters[i];

      symbol *getParam1 = param1[i];
      symbol *getParam2 = param2[i];
      if(getParam1 != getParam2)
        returnVal = false;
      else returnVal = true;
    }
  }
  return returnVal;
}
bool boolVal = false;
bool checkComp(astree* node1, astree* node2){
    if(node1->attributes.test(ATTR_array) ^
       node2->attributes.test(ATTR_array))
       boolVal = false;
    switch(node1->symbol){
      case TOK_FUNCTION:
        boolVal = checkStructural(node1,node2);
        break;
      case TOK_PROTOTYPE:
        boolVal = checkStructural(node1,node2);
        break;
      case TOK_VOID:        break;
        boolVal = false;
        break;
      case TOK_INT:{
        char* intName1 =
        strdup((structTable->find(node1->lexinfo)->first)->c_str());
        char* intName2 =
        strdup((structTable->find(node2->lexinfo)->first)->c_str());
        if(strcmp(intName1,intName2) != 0) boolVal = false;
      }break;
      case TOK_NULL:{
        switch (node2->symbol) {
          case TOK_NULL:
            boolVal = true;
            break;
          case TOK_STRING:
            boolVal = true;
            break;
          case TOK_TYPEID:
            boolVal = true;
            break;
          //case TOK_BASE:
            //boolVal = true;
          default:
            boolVal = false;
            break;
        }
      }break;
      case TOK_TYPEID:{
        char* typeName1 =
        strdup((structTable->find(node1->lexinfo)->first)->c_str());
        char* typeName2 =
        strdup((structTable->find(node2->lexinfo)->first)->c_str());
        if(strcmp(typeName1,typeName2) != 0) boolVal = false;
      }break;

    }
    switch (node2->symbol) {
      case TOK_NULL:{
        switch (node2->symbol) {
          case TOK_NULL:
            boolVal = true;
            break;
          case TOK_STRING:
            boolVal = true;
            break;
          case TOK_TYPEID:
            boolVal = true;
            break;
          //case TOK_BASE:
            //boolVal = true;
          default:
            boolVal = false;
            break;
      }
    } break;
  }
  return boolVal;
}
void copyType(astree* parent, astree* child){
  for(size_t i = 0; i<ATTR_function; i++){
    if(child->attributes[i] == 1)
      parent->attributes.set(i);
  }
}

void typecheck_node(FILE* outfile, astree* node)
{
  printf("%s\n", node->lexinfo->c_str() );
  astree *left;
  astree *right;
  symbol* sym;

  if(!node->children.empty()){
    if(node->children[0]){
      left = node->children[0];
    }
    if(node->children[1]){
      right = node->children[1];
    }
  }
    switch(node->symbol){
      case TOK_VOID:
        left->attributes.set(ATTR_void);    break;
      case TOK_CHAR:
        if(left == nullptr)
          left->attributes.set(ATTR_char);
        copyType(node, left);               break;
      case TOK_INT:
        if(left == nullptr)
          left->attributes.set(ATTR_int);
        copyType(node, left);               break;
      case TOK_STRING:
        if (left == nullptr)
          left->attributes.set(ATTR_string);
        copyType(node, left);               break;
      case TOK_IF:
      case TOK_IFELSE:
        if(!left->attributes.test(ATTR_bool))
          errprintf ("Error bust be a bool expr (%zu.%zu.%zu) ",
          node->lloc.filenr, node->lloc.linenr,node->lloc.offset);
                                            break;
      case TOK_WHILE:
        if(!left->attributes.test(ATTR_bool))
        errprintf ("Error bust be a bool expr (%zu.%zu.%zu) ",
        node->lloc.filenr, node->lloc.linenr,node->lloc.offset);
                                            break;
      case TOK_RETURN:                      break;
      case TOK_STRUCT:/* WRITE!!!! */       break;
      case TOK_TRUE:
      case TOK_FALSE:
        node->attributes.set(ATTR_bool);
        node->attributes.set(ATTR_const);   break;
      case TOK_NULL:
        node->attributes.set(ATTR_null);
        node->attributes.set(ATTR_const);   break;
      case TOK_NEW:
        copyAttr(node, left);               break;
      case TOK_ARRAY:
        left->attributes.set(ATTR_array);
        if(left == nullptr || left->children.empty())
          left->children[0]->attributes.set(ATTR_array);
                                            break;
      case TOK_EQ:
      case TOK_NE:
      case TOK_LT:
      case TOK_LE:
      case TOK_GT:
      case TOK_GE:
        if(checkComp(left, right)){
          node->attributes.set(ATTR_int);
          node->attributes.set(ATTR_vreg);
        } else{
          errprintf ("Error (%zu.%zu.%zu) ",
          node->lloc.filenr, node->lloc.linenr,node->lloc.offset);
        }                                   break;
      case TOK_IDENT:
        sym = find_ident(node);
        if(sym == nullptr){
          search_symbol(structTable,node);
        }
        if(sym == nullptr){
          errprintf ("Not defined or out of scope (%zu.%zu.%zu): %s ",
          node->lloc.filenr, node->lloc.linenr,node->lloc.offset,
          node->lexinfo->c_str());
          break;
        }
        node->attributes = sym->attributes;
                                            break;
      case TOK_INTCON:
      case TOK_CHARCON:
        node->attributes.set(ATTR_int);
        node->attributes.set(ATTR_const);
                                            break;
      case TOK_STRINGCON:
        node->attributes.set(ATTR_string);
        node->attributes.set(ATTR_const);   break;
      case TOK_CALL:
        sym = search_symbol(symbol_stack[0], node->children.back());
        if(sym == nullptr){
          errprintf ("Not defined or out of scope (%zu.%zu.%zu): %s ",
          node->lloc.filenr, node->lloc.linenr,node->lloc.offset,
          node->children.back()->lexinfo->c_str());
          break;
        }
        for(size_t i = 0; i<ATTR_function; i++){
          if(sym->attributes[i] == 1)
            node->attributes.set(i);
        }
        break;
    }

}

void recr_typecheck(FILE* outfile, astree* node)
{
  for(auto child: node->children) {
    recr_typecheck(outfile, child);
  }
  typecheck_node(outfile, node);
}

void typecheck(FILE* outfile, astree* root)
{
  recr_typecheck(outfile, root);
  while(!symbol_stack.empty()) {
    leave_block();
  }
}
