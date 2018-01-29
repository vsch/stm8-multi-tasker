#!/usr/bin/env bash

# CLion cannot pick up the compiler's include path for sdcc
# This workaround allows at least the files in the sdcc include directory root to be resolved.
# PITA but does give all the niceties of CLion to sdcc development.

FILES=( assert.h ctype.h ds80c390.h errno.h float.h iso646.h limits.h math.h sdcc-lib.h setjmp.h stdalign.h stdarg.h stdbool.h stddef.h stdint.h stdio.h stdlib.h stdnoreturn.h string.h time.h tinibios.h typeof.h uchar.h wchar.h )

for FILE in "${FILES[@]}"
do
    FILE_TEXT="#include </usr/local/share/sdcc/include/${FILE}>\n\n"
    echo -e ${FILE_TEXT} >${FILE}
done

#DIRS=( asm ds390 ds400 hc08 mcs51 pic14 pic16 z180 )
#
#for DIR in "${DIRS[@]}"
#do
#    ln -s /usr/local/share/sdcc/include/${DIR} ${DIR}
#done
#

