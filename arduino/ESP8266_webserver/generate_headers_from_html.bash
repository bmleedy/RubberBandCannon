#error out if no html directory

#todo: iterate through eac
HTML_FILES=./html/*.html

#delete all generated headers that exist
rm *.html.hh

for file in ${HTML_FILES}; do
  HTML_FILENAME="${file}"
  HEADER_NAME_BASE=`echo ${file} | cut -d '/' -f3 | cut -d '.' -f1`
  HEADER_NAME_BASE_UPPER=`echo $HEADER_NAME_BASE | tr /a-z/ /A-Z/`
  HEADER_FILENAME=${HEADER_NAME_BASE}.html.hh

  echo "processing file: ${HTML_FILENAME}"

  echo "#ifndef ${HEADER_NAME_BASE_UPPER}_HTML_HH" >  ${HEADER_FILENAME}
  echo "#define ${HEADER_NAME_BASE_UPPER}_HTML_HH" >> ${HEADER_FILENAME}
  echo "/************************************************" >> ${HEADER_FILENAME}
  echo " * GENERATED FILE -- DO NOT HAND-MODIFY!!!!!!!!!!" >> ${HEADER_FILENAME}
  echo "/***********************************************/" >> ${HEADER_FILENAME}
  echo "const char ${HEADER_NAME_BASE}_text_0[] PROGMEM = \"\\"  >> ${HEADER_FILENAME}

  # Print all of the lines up to the first "////"
  echo "Pass 1"
  sed '/\/\/\/\//q' ${HTML_FILENAME} | grep -v "^//" | sed 's/$/\\/' | sed 's/\"/\\\"/g' | sed "s/^[ \t]*//" >> ${HEADER_FILENAME}
  echo "\";" >> ${HEADER_FILENAME}

  # Process all of the lines between the first "////" and the second "//END"
  echo "Pass 2"
  echo ""  >> ${HEADER_FILENAME}
  echo "const char PROGMEM ${HEADER_NAME_BASE}_prefetch[][7] = {"  >> ${HEADER_FILENAME}
  for line in `awk '/FETCHDATA_START/{flag=1;next}/FETCHDATA_END/{flag=0}flag' ${HTML_FILENAME} | grep -v "^//"`; do
    echo \"`echo ${line} | cut -d ':' -f1`\",  >> ${HEADER_FILENAME}
  done
  echo "};" >> ${HEADER_FILENAME}


  # Process all of the lines after the "//END"
  echo "Pass 3"
  echo ""  >> ${HEADER_FILENAME}
  echo "const char ${HEADER_NAME_BASE}_text_2[] PROGMEM = \"\\"  >> ${HEADER_FILENAME}
  awk '/FETCHDATA_END/{flag=1;next}flag' ${HTML_FILENAME} | grep -v "^//" | sed 's/$/\\/' | sed 's/\"/\\\"/g' | sed "s/^[ \t]*//" >> ${HEADER_FILENAME}
  echo "\";" >> ${HEADER_FILENAME}

  echo "finalizing"

  echo ""  >> ${HEADER_FILENAME}
  echo "#endif"  >> ${HEADER_FILENAME}

done


