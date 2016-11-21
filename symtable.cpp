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

//used in type check tok_struct see if struct val was
//just called or initiallized
symbol* foundInsert = nullptr;
symbol* nullInsert = nullptr;

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

  if (table && node)
    table->insert(make_pair(node->lexinfo, newSym));
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
  if(symbol_stack.empty())
    symbol_stack.push_back(nullptr);
  if(symbol_stack.back() == nullptr)
  {
    symbol_stack.back() = new symbol_table;
  }
  symbol* to_insert = createSym(node);
  symbol_stack.back()->insert(make_pair(node->lexinfo, to_insert));
}

symbol* find_ident(astree* node)
{
  for (symbol_table* table : symbol_stack) {
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
void printsym (FILE* outfile, astree* node) {
  astree* current = nullptr;
  if (node->attributes[ATTR_struct]) {
      fprintf (outfile, "\n");
  } else {
      fprintf (outfile, "    ");
  }

  if (node->attributes[ATTR_field]) {
      fprintf (outfile, "%s (%zu.%zu.%zu) field {%s} ",
          node->lexinfo->c_str(),
          node->lloc.linenr, node->lloc.filenr, node->lloc.offset,
          (current->lexinfo)->c_str());
  } else {
      fprintf (outfile, "%s (%zu.%zu.%zu) {%zu} ",
          (node->lexinfo)->c_str(),
          node->lloc.linenr, node->lloc.filenr, node->lloc.offset,
          node->blocknr);
  }
  if (node->attributes[ATTR_struct]) {
        fprintf (outfile, "struct \"%s\" ",
            (node->lexinfo)->c_str());
        current = node;
    }

    fprintf (outfile, "%s\n", astree::ATtoST (node).c_str());
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
void blockCheck(astree* node){
  if(node->symbol == TOK_BLOCK)
    enter_block();
  node->blocknr = next_block;
  for(auto child: node->children)
    blockCheck(child);
}
void checkPro(FILE* outFile, astree* node){
  node->children[0]->children[0]->attributes.set (ATTR_function);
  insert_symbol (symbol_stack[0], node->children[0]->children[0]);
  printsym (outFile, node->children[0]->children[0]);
  enter_block();
  for (auto child : node->children[1]->children) {
    child->children[0]->attributes.set (ATTR_variable);
    child->children[0]->attributes.set (ATTR_lval);
    child->children[0]->attributes.set (ATTR_param);
    child->children[0]->blocknr = next_block;
    define_ident (child);
    printsym (outFile, child->children[0]);
    }
  leave_block();
}

void checkFunc(FILE* outFile, astree* node){
  node->children[0]->children[0]->attributes.set (ATTR_function);
  insert_symbol (symbol_stack[0], node->children[0]->children[0]);
  printsym (outFile, node->children[0]->children[0]);
  enter_block();
  for (auto child : node->children[1]->children) {
    child->children[0]->attributes.set (ATTR_variable);
    child->children[0]->attributes.set (ATTR_lval);
    child->children[0]->attributes.set (ATTR_param);
    child->children[0]->blocknr = next_block;
    define_ident (child);
    printsym (outFile, child->children[0]);
    }
  blockCheck(node->children[2]);
  leave_block();
}
bool boolVal = false;

bool checkComp(astree* node1, astree* node2){
    if(node2)
      if(!(node1->attributes.test(ATTR_array) ^
        node2->attributes.test(ATTR_array)))
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
      default:
        break;
    }
    if(node2){
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
}
  return boolVal;
}
void copyType(astree* parent, astree* child){
  for(size_t i = 0; i<ATTR_function; i++){
    if(child->attributes[i] == 1)
      parent->attributes.set(i);
  }
}

void typecheck_node(FILE* outFile, astree* node)
{
  cout << *(node->lexinfo) + "\n";
  astree *left = nullptr;
  astree *right = nullptr;
  symbol* sym = nullptr;

  if(node->children.size() >= 1)
    left = node->children[0];
  if(node->children.size() == 2)
    right = node->children[1];

    switch(node->symbol){
      case TOK_VOID:
        left->attributes.set(ATTR_void);
        break;
      case TOK_CHAR:
        if(left == nullptr)
          break;
        left->attributes.set(ATTR_char);
        copyType(node, left);
        break;
      case TOK_INT:
        if(left == nullptr)
          break;
        left->attributes.set(ATTR_int);
        copyType(node, left);
        break;
      case TOK_STRING:
        if (left == nullptr)
          break;
        left->attributes.set(ATTR_string);
        copyType(node, left);
        break;
      case TOK_IF:
      case TOK_IFELSE:
        if(!left->attributes.test(ATTR_bool))
          errprintf ("Error bust be a bool expr (%zu.%zu.%zu)\n",
          node->lloc.filenr, node->lloc.linenr,node->lloc.offset);
        break;
      case TOK_WHILE:
        if(!left->attributes.test(ATTR_bool))
        errprintf ("Error must be a bool expr (%zu.%zu.%zu)\n",
        node->lloc.filenr, node->lloc.linenr,node->lloc.offset);
        break;
      case TOK_RETURN:                      break;
      case TOK_STRUCT:
        left->attributes.set(ATTR_struct);
        if((*structTable)[left->lexinfo]){
          foundInsert = createSym(left);
          foundInsert->block_nr = 0;
          structTable->insert(make_pair(left->lexinfo,foundInsert));
          printsym(outFile,left);
          //populate the struct fields
          symbol* populate = search_symbol(structTable, left);
          for(auto field = left->children.begin()+1;
              field != left->children.end(); field++){
            insert_symbol(populate->fields, *field);
            printsym(outFile, (*field)->children[0]);
          }
        } else{
          nullInsert = createSym(left);
          nullInsert->block_nr = 0;
          structTable->insert(make_pair(left->lexinfo,nullInsert));
          printsym(outFile,left);
        }
        break;
      case TOK_TRUE:
      case TOK_FALSE:
        node->attributes.set(ATTR_bool);
        node->attributes.set(ATTR_const);
        break;
      case TOK_NULL:
        node->attributes.set(ATTR_null);
        node->attributes.set(ATTR_const);
        break;
      case TOK_NEW:
        copyAttr(node, left);
        break;
      case TOK_ARRAY:
        left->attributes.set(ATTR_array);
        if(left == nullptr || left->children.empty())
          break;
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
          errprintf ("Error (%zu.%zu.%zu)\n ",
          node->lloc.filenr, node->lloc.linenr,node->lloc.offset);
        }
        break;
      case TOK_IDENT:
        sym = find_ident(node);
        if(sym == nullptr){
          search_symbol(structTable,node);
        }
        if(sym == nullptr){
          errprintf ("Not defined or out of scope (%zu.%zu.%zu): %s\n",
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
        node->attributes.set(ATTR_const);
        break;
      case TOK_CALL:
        sym = search_symbol(symbol_stack[0], node->children.back());
        if(sym == nullptr){
          errprintf ("Not defined or out of scope (%zu.%zu.%zu): %s\n",
          node->lloc.filenr, node->lloc.linenr,node->lloc.offset,
          node->children.back()->lexinfo->c_str());
          break;
        }
        for(size_t i = 0; i<ATTR_function; i++){
          if(sym->attributes[i] == 1)
            node->attributes.set(i);
        }
        break;
      case TOK_BLOCK:
        blockCheck (node);
        leave_block();
        break;
      case TOK_NEWARRAY:
          node->attributes.set(ATTR_vreg);
          node->attributes.set(ATTR_array);
          copyType(node, left);
          break;
      case TOK_TYPEID:
          node->attributes.set(ATTR_typeid);
          break;
      case TOK_FIELD:
          node->attributes.set(ATTR_field);
          if(left != nullptr){
            left->attributes.set(ATTR_field);
            copyType(node, left);
          }
          break;
      case TOK_ORD:
          node->attributes.set(ATTR_int);
          node->attributes.set(ATTR_vreg);
          if(!left->attributes[ATTR_char])
          errprintf ("Error wrong ord type (%zu.%zu.%zu): %s\n",
          node->lloc.filenr, node->lloc.linenr,node->lloc.offset);
          break;
      case TOK_CHR:
          node->attributes.set(ATTR_char);
          node->attributes.set(ATTR_vreg);
          if(!left->attributes[ATTR_char])
          errprintf ("Error wrong chr type (%zu.%zu.%zu): %s\n",
          node->lloc.filenr, node->lloc.linenr,node->lloc.offset);
          break;
      case TOK_ROOT:
      case TOK_PARAMLIST:
          break;
      case TOK_PROTOTYPE:
          checkPro(outFile, node);
          break;
      case TOK_FUNCTION:
          enter_block();
          checkFunc(outFile, node);
          printsym(outFile, node);
          break;
     case TOK_DECLID:
          break;
     case TOK_INDEX:
          node->attributes.set(ATTR_lval);
          node->attributes.set(ATTR_vaddr);
          break;
     case TOK_NEWSTRING:
          node->attributes.set(ATTR_vreg);
          node->attributes.set(ATTR_string);
          break;
     case TOK_RETURNVOID:
          break;
     case TOK_VARDECL:
          left->children[0]->attributes.set(ATTR_lval);
          left->children[0]->attributes.set(ATTR_variable);
          copyAttr(node, left);
          if (find_ident (left->children[0])) {
          errprintf ("Error (%zu.%zu.%zu), variable already declared\n",
                    node->lloc.filenr, node->lloc.linenr,
                    node->lloc.offset, left->children[0]->lexinfo);
            break;
          }
          define_ident(left->children[0]);
          printsym(outFile, left->children[0]);
          break;
     case '=':
          if(left == nullptr)
            break;
          if(left->attributes[ATTR_lval]){
            copyType(node, left);
            node->attributes.set(ATTR_vreg);
          } else{
                errprintf ("Error (%zu.%zu.%zu), incompatiable types\n",
                    node->lloc.filenr, node->lloc.linenr,
                    node->lloc.offset);
          }
          break;
     case '+':
     case '-':
          node->attributes.set(ATTR_vreg);
          node->attributes.set(ATTR_int);
          if(right == nullptr){
            if(left == nullptr)
              break;
            if(!(left->attributes[ATTR_int])){
                errprintf ("Error (%zu.%zu.%zu), int type required\n",
                    node->lloc.filenr, node->lloc.linenr,
                    node->lloc.offset);
            }
          }
          break;
     case '*':
     case '/':
     case '%':
          node->attributes.set(ATTR_vreg);
          node->attributes.set(ATTR_int);
          if (!(left->attributes[ATTR_int]) ||
              !(right->attributes[ATTR_int])) {
                errprintf ("Error (%zu.%zu.%zu),int type required\n",
                node->lloc.filenr,
                node->lloc.linenr, node->lloc.offset);
            }
          break;
    case '!':
          node->attributes.set(ATTR_vreg);
          node->attributes.set(ATTR_bool);
          if (!(left->attributes[ATTR_int]) ||
                !(right->attributes[ATTR_int])) {
                errprintf ("Error (%zu.%zu.%zu), bool type required\n",
                node->lloc.filenr,
                node->lloc.linenr, node->lloc.offset);
            }
          break;
   case '.':
          node->attributes.set(ATTR_lval);
          node->attributes.set(ATTR_vaddr);
          sym = search_symbol(structTable, node);
          copyType(node, left);
          break;
  default:
          errprintf ("Error, invalid token \"%s\"",
          get_yytname (node->symbol));
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
