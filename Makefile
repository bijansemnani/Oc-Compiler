GPP			= g++ -std=gnu++14 -g -O0 -Wall -Wextra
MKDEP		= g++ -std=gnu++14 -MM
VALGRIND	 = valgrind --leak-check=full --show-reachable=yes
#Bijan  Semnani
#Ricardo Munoz
MKFILE	 = Makefile
DEPFILE	= Makefile.dep
SOURCE1 = stringset.cpp main.cpp auxlib.cpp astree.cpp
SOURCE2 = lyutils.cpp yyparse.cpp yylex.cpp
SOURCES	= ${SOURCE1} ${SOURCE2}
HEADERS	= stringset.h auxlib.h astree.h lyutils.h yyparse.h
OBJECTS	= ${SOURCES:.cpp=.o}
EXECBIN	= oc
SRCFILES = ${HEADERS} ${SOURCES} ${MKFILE}

LSOURCES	= scanner.l
YSOURCES	= parser.y
CLGEN		 = yylex.cpp
HYGEN		 = yyparse.h
CYGEN		 = yyparse.cpp
LREPORT	 = yylex.output
YREPORT	 = yyparse.output

all : ${CLGEN} ${CYGEN} ${HYGEN} ${EXECBIN}

${EXECBIN} : ${OBJECTS}
		${GPP} ${OBJECTS} -o ${EXECBIN}



%.o : %.cpp
		${GPP} -c $<

ci :
		cid + ${SRCFILES}
		checksource ${CSOURCE}

clean :
		-rm ${OBJECTS}
		-rm ${DEPFILE}
		-rm ${CLGEN}
		-rm ${HYGEN}
		-rm ${CYGEN}
		-rm ${LREPORT}
		-rm ${YREPORT}

spotless : clean
		- rm ${EXECBIN}
		- rm *.cc
		- rm *.str
		- rm *.tok

${DEPFILE} :
		${MKDEP} ${SOURCES} >${DEPFILE}

dep :
		- rm ${DEPFILE}
		${MAKE} --no-print-directory ${DEPFILE}

${CLGEN} : ${LSOURCES}
		flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
		- grep -v ’ˆ ’ ${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
		bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

include ${DEPFILE}
