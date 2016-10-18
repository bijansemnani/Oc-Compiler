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
#include <cstddef>
#include <iostream>
#include <sys/wait.h>




const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;
void chomp (char* string, char delim);
void cpplines (FILE* pipe, const char* filename, const char* newName);

//Bijan Semnani bsemnani
//Ricardo Munoz riamunoz
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

// Run cpp against the lines of the file.
void cpplines (FILE* pipe, const char* filename, const char* newName) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
      /*int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
                              &linenr, inputname);*/
      /*if (sscanf_rc == 2) {
         continue;
      }*/
      lexer::newfilename (filename);
      int symbol = 0;
      while((symbol = yylex()) != YYEOF){
        cout << parser::get_tname (symbol)<<'\n';
        lexer::advance();
      }
      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
        //tokenize the chars per line
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         string_set::intern (token);
      }
      //dump the string set into the .str file
      FILE* newFile;
      newFile = fopen(newName, "w"); // w = write
      if(newFile == NULL){
        cerr << "FNF" << newName;
            }
      string_set::dump (newFile);
      fclose(newFile);
      ++linenr;
   }
}


int main (int argc, char** argv) {
  int exit_status = EXIT_SUCCESS;
   string command = ""; //the input string on command line
   int c = 0;
   yy_flex_debug = 0;
   yydebug = 0;
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
   string filename1 = "";// the name of the file
   string filename2 = "";
   for (index = optind; index < argc; index++){
     file = argv[index];
  }
/*got from http://stackoverflow.com/questions/51949/how-to-
get-file-extension-from-string-in-c*/
    if(file.find_last_of(".") != std::string::npos){
      extend = file.substr(file.find_last_of(".")+1);

    }

    if(extend == "oc"){
      filename1 = file.substr(0,file.size()-3) + ".str";
      filename2 = file.substr(0,file.size()-3) + ".tok";
    } else {
      fprintf(stderr, "incorrect file type: %s\n",extend.c_str());
      exit_status = EXIT_FAILURE;
      printf("exit status = %d\n", exit_status);
      return EXIT_FAILURE;
    }
    //new file as an .str file
    printf("New File: %s\n", filename1.c_str());
    //new file as an .tok file
    printf("New File: %s\n", filename2.c_str());

    //begin tokenizing process
    const char* execname = basename (argv[0]);
    char* unfree = strdup(file.c_str());
    string newCPP = CPP + " " + unfree;

    //printf ("command=\"%s\"\n", newCPP.c_str());

    //open a pipe to pass the file through
    yyin = popen (newCPP.c_str(), "r");

    if (yyin == NULL) {
       exit_status = EXIT_FAILURE;
       fprintf (stderr, "%s: %s: %s\n",
                execname, command.c_str(), strerror (errno));
    }else {
      //pass in the pipe, the original file and new file name
      cpplines (yyin, extend.c_str(), filename1.c_str());

      int parse_rc = yyparse();
      int pcloseint = 0;
      yylex_destroy();
      pclose (yyin);
      printf("exit status = %d\n", exit_status);
      if (yydebug or yy_flex_debug) {
        fprintf (stderr, "Dumping parser::root:\n");
        if (parser::root != nullptr) parser::root->dump_tree (stderr);
        fprintf (stderr, "Dumping string_set:\n");
        string_set::dump (stderr);
      }
      if (parse_rc != 0) {
        errprintf ("parse failed (%d)\n", parse_rc);

    }
     if (pclose(yyin) !=0){
       cerr << "YYin did not close \n"<< pcloseint;
       exit_status = EXIT_FAILURE;
     }

 }
 free(unfree);
 return exit_status;
}
