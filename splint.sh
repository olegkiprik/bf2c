#!/bin/sh
splint \
\
+posixstrictlib \
+voidabstract \
+charindex \
+matchanyintegral \
+charint \
+ignorequals \
-sysunrecog \
-preproc \
-compdef \
-mustfreeonly \
-temptrans \
-varuse \
-nullpass \
-nullderef \
-usereleased \
-nullret \
-mustfreefresh \
-branchstate \
-paramuse \
-fcnuse \
-unrecog \
-globs \
-usedef \
-mayaliasunique \
-shiftnegative \
-redef \
\
*.c
