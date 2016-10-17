GPP			= g++ -std=gnu++14 -g -O0 -Wall -Wextra
MKDEP		= g++ -std=gnu++14 -MM

MKFILE	 = Makefile
DEPFILE	= Makefile.dep
SOURCES	= stringset.cpp main.cpp auxlib.cpp astree.cpp lyutils.cpp yyparse.cpp yylex.cpp
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

all : ${CLGEN} ${EXECBIN}

${EXECBIN} : ${OBJECTS}
		${GPP} ${OBJECTS} -o ${EXECBIN}

${CLGEN} : ${LSOURCES}
		flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}
		-grep -v ’ˆ ’ ${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
		bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

%.o : %.cpp
		${GPP} -c $<

ci :
		cid + ${SRCFILES}

clean :
		-rm ${OBJECTS} ${DEPFILE}

spotless : clean
		- rm ${EXECBIN}

${DEPFILE} :
		${MKDEP} ${SOURCES} >${DEPFILE}

dep :
		- rm ${DEPFILE}
		${MAKE} --no-print-directory ${DEPFILE}

include ${DEPFILE}
