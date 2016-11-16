#include <string>
using namespace std;
#include <ctype.h>
#include <libgen.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "stringset.h"
#include "auxlib.h"
#include "lyutils.h"
#include "astree.h"
#include "symtable.h"
#include <cstddef>
#include <iostream>
#include <sys/wait.h>




const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

//Bijan Semnani bsemnani
//Ricardo Munoz riamunoz

int main (int argc, char** argv) {
  int exit_status = EXIT_SUCCESS;
   string command = ""; //the input string on command line
   int c = 0;
   yy_flex_debug = 0;
   yydebug = 0;
   char* orgFile = NULL;

   while ((c = getopt (argc, argv, "@:D:ly")) != -1)
   switch (c)
     {
     case '@':
       set_debugflags (optarg);
       break;
     case 'D':
       command = optarg;
       break;
     case 'l':
       yy_flex_debug = 1;
       break;
     case 'y':
       yydebug = 1;
       break;
     case '?':
        cerr << "not known: -"<< optopt;
      break;
     }
     printf ("yy_flex_debug = %d|yydebug = %d|@value = %s|D = %s\n",
     yy_flex_debug, yydebug, optarg,
     command.c_str());
   int index = 0;
   string file = "";// the inputed .oc file
   string extend = "";// original file extension .oc
   string filename = "";// the name of the file
   for (index = optind; index < argc; index++){
     file = argv[index];
  }
/*got from http://stackoverflow.com/questions/51949/how-to-
get-file-extension-from-string-in-c*/
    if(file.find_last_of(".") != std::string::npos){
      extend = file.substr(file.find_last_of(".")+1);

    }
    //set execname
    orgFile = strdup(file.c_str());
    set_execname(orgFile);

    if(extend == "oc"){
      filename = file.substr(0,file.size()-3);
    } else {
      fprintf(stderr, "incorrect file type: %s\n",extend.c_str());
      exit_status = EXIT_FAILURE;
      printf("exit status = %d\n", exit_status);
      return EXIT_FAILURE;
    }


    //begin tokenizing process
    const char* execname = basename (argv[0]);
    char* unfree = strdup(file.c_str());
    string newCPP = CPP + " " + unfree;
    int close = 0;

    //open a pipe to pass the file through
    yyin = popen (newCPP.c_str(), "r");

    if (yyin == NULL) {
       exit_status = EXIT_FAILURE;
       fprintf (stderr, "%s: %s: %s\n",
                execname, command.c_str(), strerror (errno));
    }else {
      //pass in the pipe, the original file and new file name
      FILE* strFile;
      string ocName = "";
      strFile = fopen((filename + ".str").c_str(), "w"); // w = write
      if(strFile == NULL){
        cerr << "FNF" << filename;
      }
      FILE* tokFile;
      tokFile = fopen((filename + ".tok").c_str(), "w"); // w = write
      if(tokFile == NULL){
        cerr << "FNF" << filename;
      }
      FILE* astFile;
      astFile = fopen((filename + ".ast").c_str(), "w"); // w = write
      if(strFile == NULL){
        cerr << "FNF" << filename;
      }
      FILE* symFile;
      symFile = fopen((filename + ".sym").c_str(), "w"); // w = write
      if(symFile == NULL){
        cerr << "FNF" << filename;
      }
      //dump tokenized output into .tok file
      lexer::newfilename (filename, tokFile);
      int yyparse_rc = yyparse();
      if(yyparse_rc ==2){
        cerr<< "yyparse failed";
      }
      else if(yyparse_rc ==1){
        cerr<< "yyparse still failed";
      }

      //dump the string set into the .str file

      string_set::dump (strFile);
      astree::print(astFile, parser::root, 0);
      typecheck(symFile, parser::root);
      fclose(astFile);
      fclose(strFile);
      fclose(tokFile);
      fclose(symFile);
      int parse_rc = yyparse();

      close = pclose (yyin);
      yylex_destroy();
      if (close !=0){
        cerr << "YYin did not close \n"<< close;
        exit_status = EXIT_FAILURE;
      }

      if (yydebug or yy_flex_debug) {
        fprintf (stderr, "Dumping parser::root:\n");
        if (parser::root != nullptr) parser::root->dump_tree (stderr);
        fprintf (stderr, "Dumping string_set:\n");
        string_set::dump (stderr);
      }
      if (parse_rc != 0) {
        errprintf ("parse failed (%d)\n", parse_rc);
        exit_status = EXIT_FAILURE;

    }


 }
 free(unfree);
 free(orgFile);
 printf("exit status = %d\n", exit_status);
 return exit_status;
}
