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
  echo "////////////////////////////////////////////////" >> ${HEADER_FILENAME}
  echo "//GENERATED FILE -- DO NOT HAND-MODIFY!!!!!!!!!!" >> ${HEADER_FILENAME}
  echo "////////////////////////////////////////////////" >> ${HEADER_FILENAME}
  echo "const char ${HEADER_NAME_BASE}_text[] PROGMEM = \"\\"  >> ${HEADER_FILENAME}

  #Just escape the entire file
  cat ${HTML_FILENAME} | grep -v "^//" | sed 's/$/\\/' | sed 's/\"/\\\"/g' | sed "s/^[ \t]*//" >> ${HEADER_FILENAME}
  echo "\";" >> ${HEADER_FILENAME}.hh

  #todo: at the "////" breakpoints, parse through the list of prefetch data and make an array of strings with the prefetch field per the example in config_website.html

  echo "#endif"  >> ${HEADER_FILENAME}

done


