// $Id: astree.cpp,v 1.8 2016-09-21 17:13:03-07 - - $
//Bijan Semnani bsemnani
//Ricardo Munoz riamunoz
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"

location function_lloc = {0,0,0};
location prototype_lloc = {0,0,0};
bool func_lloc = false;
bool pro_lloc = false;
astree::astree (int symbol_, const location& lloc_, const char* info):
  attributes(0), blocknr(0), struct_table(nullptr)
 {
   symbol = symbol_;
   if(strcmp(get_yytname(symbol), "TOK_FUNCTION")== 0){
     if(!func_lloc){
       function_lloc = lloc_;
       func_lloc = true;
     }
     lloc = function_lloc;
     lexinfo = string_set::intern (info);
     return;

   }else if(strcmp(get_yytname(symbol), "TOK_PROTOTYPE")== 0){
     if(!pro_lloc){
       prototype_lloc = lloc_;
       pro_lloc = true;
     }
     lloc = prototype_lloc;
     lexinfo = string_set::intern (info);
     return;
   }

   lloc = lloc_;
   lexinfo = string_set::intern (info);
   //lexer::dump(symbol);
   // vector defaults to empty -- no children
}

void astree::astreeFree(astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      astreeFree(child);
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      //astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
   delete root;
}

astree* astree::adoptOne (astree* child1, astree* child2) {
   if (child1 != nullptr) child1->children.push_back (child2);

   return child1;
}
astree* astree::adoptTwo (astree* child1,
  astree* child2, astree* child3) {
  astree::adoptOne(child1, child2);
  astree::adoptOne(child1, child3);
  return child1;
}
astree* astree::adoptThree (astree* child1,
  astree* child2, astree* child3, astree* child4) {
  astree::adoptOne(child1, child2);
  astree::adoptOne(child1, child3);
  astree::adoptOne(child1, child4);
  return child1;
}
astree* astree::adopt_sym (astree* child, int symbol_) {
   child->symbol = symbol_;
   return child;
}


void astree::dump_node (FILE* outfile) {
   fprintf (outfile, "%p->{%s %zd.%zd.%zd \"%s\":",
            this, parser::get_tname (symbol),
            lloc.filenr, lloc.linenr, lloc.offset,
            lexinfo->c_str());
   for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }
}

void astree::dump_tree (FILE* outfile, int depth) {
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (NULL);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_node (outfile);
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   char *tname = strdup(parser::get_tname (tree->symbol));
   if (strstr (tname, "TOK_") == tname)
      tname += 4;
   for(int i = 0; i< depth; i++) fprintf(outfile, "|%s", " ");
   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd) {%zu}\n",
            tname, tree->lexinfo->c_str(),
            tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset,tree->blocknr);
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
}

void destroy (astree* tree1, astree* tree2) {
   if (tree1 != nullptr) delete tree1;
   if (tree2 != nullptr) delete tree2;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s",
              lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
              buffer);
}
