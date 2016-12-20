
#include <string>
#include <cstddef>
#include <iostream>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <libgen.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "symtable.h"
#include "stringset.h"

using namespace std;

//global variables
const string CPP = "/usr/bin/cpp";
int exit_status = EXIT_SUCCESS;
string command = ""; //the input string on command line
string file = "";// the inputed .oc file
string extend = "";// original file extension .oc
string filename = "";
char* orgFile = NULL;
FILE* astFile;
FILE* symFile;
FILE* strFile;
FILE* tokFile;

//Method declarations
FILE* getfile(FILE* filename, string file, string extension);
void debugOpt(int argc, char** argv);
string getExtend(string file);
string getFilename(string file, string extension);

//Method to get filename
string getFilename(string file, string extension){
  string filename = "";// the name of the file
  if(extension == "oc"){
    filename = file.substr(0,file.size()-3);
  } else {
    fprintf(stderr, "incorrect file type: %s\n",extend.c_str());
    exit_status = EXIT_FAILURE;
    printf("exit status = %d\n", exit_status);
    }
  return filename;
}

//Method to get file extension
string getExtend(string file){
  string ext = "";
  /*got from http://stackoverflow.com/questions/51949/how-to-
    get-file-extension-from-string-in-c*/
  if(file.find_last_of(".") != std::string::npos)
    ext = file.substr(file.find_last_of(".")+1);
  return ext;
}

//Method to open new file with new extension
FILE* getfile(FILE* filename,string file, string extension){
  filename = fopen((file + extension).c_str(), "w"); // w = write
  if(filename == NULL){
    cerr << "FNF" << filename;
  }
  return filename;
}

//Method for debug options
void debugOpt(int argc, char** argv){
  int c = 0;
  yy_flex_debug = 0;
  yydebug = 0;
  while ((c = getopt (argc, argv, "@:D:ly")) != -1)
  switch (c)
    {
    case '@': set_debugflags (optarg);         break;
    case 'D': command = optarg;                break;
    case 'l': yy_flex_debug = 1;               break;
    case 'y': yydebug = 1;                     break;
    case '?': cerr << "not known: -"<< optopt; break;
    }
    printf ("yy_flex_debug = %d|yydebug = %d|@value = %s|D = %s\n",
    yy_flex_debug, yydebug, optarg,command.c_str());

    //flag check
    if (yydebug or yy_flex_debug) {
      fprintf (stderr, "Dumping parser::root:\n");
      if (lexer::root != nullptr) lexer::root->dump_tree (stderr);
      fprintf (stderr, "Dumping string_set:\n");
      string_set::dump (stderr);
    }
}

int main (int argc, char** argv) {
  debugOpt(argc, argv);
  for (int index = optind; index < argc; index++)
    file = argv[index];

  //set execname
  orgFile = strdup(file.c_str());
  set_execname(orgFile);

  //set file extension
  extend = getExtend(file);

  //set filename
  filename = getFilename(file,extend);
  if(exit_status == EXIT_FAILURE) return EXIT_FAILURE;

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

    //get filenames with correct file extension
    strFile = getfile(strFile,filename,".str");
    tokFile = getfile(tokFile,filename,".tok");
    astFile = getfile(astFile,filename,".ast");
    symFile = getfile(symFile,filename,".sym");

    //dump tokenized output into .tok file
    lexer::newfilename (filename, tokFile);
    int yyparse_rc = yyparse();
    if(yyparse_rc ==2){
      cerr<< "yyparse failed";
    }
    else if(yyparse_rc ==1) cerr<< "yyparse still failed";

    //dump the string set into the .str file
    string_set::dump (strFile);
    typecheck(symFile, lexer::root);
    astree::print(astFile, lexer::root, 0);


    //close all files
    fclose(astFile);
    fclose(strFile);
    fclose(tokFile);
    fclose(symFile);

    close = pclose (yyin);
    yylex_destroy();

    //check if YYin closed
    if (close !=0){
      cerr << "YYin did not close \n"<< close;
      exit_status = EXIT_FAILURE;
    }

    if (yyparse_rc != 0) {
      cerr << "parse failed (%d)\n"<< yyparse_rc;
      exit_status = EXIT_FAILURE;
    }
 }
 free(unfree);
 free(orgFile);
 cout << "exit status = " << exit_status << endl;
 return exit_status;
}
