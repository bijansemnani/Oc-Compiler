GPP			= g++ -std=gnu++14 -g -O0 -Wall -Wextra
MKDEP		= g++ -std=gnu++14 -MM
VALGRIND	 = valgrind --leak-check=full --show-reachable=yes

MKFILE	 = Makefile
SOURCE1 = stringset.cpp main.cpp auxlib.cpp astree.cpp symtable.cpp
SOURCE2 = lyutils.cpp yyparse.cpp yylex.cpp
SOURCES	= ${SOURCE1} ${SOURCE2}
HEADERS1= stringset.h auxlib.h symtable.h
HEADERS2= astree.h lyutils.h yyparse.h
OBJECTS	= ${SOURCES:.cpp=.o}
EXECBIN	= oc
SRCFILES = ${HEADERS1} ${HEADERS2} ${SOURCES} ${MKFILE}

LSOURCES	= scanner.l
YSOURCES	= parser.y
CLGEN		 = yylex.cpp
HYGEN		 = yyparse.h
CYGEN		 = yyparse.cpp
LREPORT	 = yylex.output
YREPORT	 = yyparse.output

all : ${CLGEN} ${CYGEN} ${HYGEN} ${EXECBIN}

${CLGEN} : ${LSOURCES}
	 flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
	 - grep -v '^ ' ${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
		bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

${EXECBIN} : ${OBJECTS}
		${GPP} ${OBJECTS} -o ${EXECBIN}



%.o : %.cpp
		${GPP} -c $<

ci :
		cid + ${SRCFILES}
		checksource ${CSOURCE}

clean :
		-rm ${OBJECTS}
		-rm ${CLGEN}
		-rm ${HYGEN}
		-rm ${CYGEN}
		-rm ${LREPORT}
		-rm ${YREPORT}

spotless : clean
		- rm ${EXECBIN}
		- rm *.str
		- rm *.tok
		- rm *.ast
		- rm *.sym
