#error out if no html directory

function stringcat(){
echo "catting";
CHARSTRING=$CHARSTRING$(cat -);
}


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
  echo " @file ${HEADER_FILENAME}" >> ${HEADER_FILENAME}
  echo " * GENERATED FILE -- DO NOT HAND-MODIFY!!!!!!!!!!" >> ${HEADER_FILENAME}
  echo " ***********************************************/" >> ${HEADER_FILENAME}
  echo " /*! @var ${HEADER_NAME_BASE}_text_0" >> ${HEADER_FILENAME}
  echo "  *  @brief generated variable  ${HEADER_NAME_BASE}_text_0" >> ${HEADER_FILENAME}
  echo "  */ " >> ${HEADER_FILENAME}
  echo "const char ${HEADER_NAME_BASE}_text_0[] PROGMEM = \"\\"  >> ${HEADER_FILENAME}

  # Print all of the lines up to the first "////"
  echo "Pass 1"


  # Write the lines to the header file
  sed '/\/\/FETCHDATA_START/q' ${HTML_FILENAME} | grep -v "^//" | sed "s/^[ \t]*//" | sed 's/\\/\\\\/g' | sed 's/$/\\/' | sed 's/\"/\\\"/g' >> ${HEADER_FILENAME}
  echo "\";" >> ${HEADER_FILENAME}

  # Now, find the number of characters in that section and write that to a variable
  unset CHARSTRING
  CHARSTRING=$(sed '/\/\/FETCHDATA_START/q' ${HTML_FILENAME} | grep -v "^//" | sed "s/^[ \t]*//" | awk '{printf("%s",$0)}')
  echo "" >> ${HEADER_FILENAME}
  echo "static const int ${HEADER_NAME_BASE}_text_0_len = $(echo $CHARSTRING | wc -m);" >> ${HEADER_FILENAME}

  # Process all of the lines between the first "////" and the second "//END"
  echo "Pass 2"
  echo ""  >> ${HEADER_FILENAME}
  echo ""  >> ${HEADER_FILENAME}
  echo " /*! @var ${HEADER_NAME_BASE}_prefetch" >> ${HEADER_FILENAME}
  echo "  *  @brief generated variable  ${HEADER_NAME_BASE}_prefetch" >> ${HEADER_FILENAME}
  echo "  */ " >> ${HEADER_FILENAME}
  ITERATOR=0
  for line in `awk '/FETCHDATA_START/{flag=1;next}/FETCHDATA_END/{flag=0}flag' ${HTML_FILENAME} | grep -v "^//"`; do
    echo "const char prefetch_field_${ITERATOR}[] PROGMEM = \"$(echo ${line} | cut -d ':' -f1)\";"  >> ${HEADER_FILENAME}
    (( ITERATOR+=1 ))
  done
  ITERATOR=0
  echo "const char* const ${HEADER_NAME_BASE}_prefetch[] PROGMEM = {"  >> ${HEADER_FILENAME}
  for line in `awk '/FETCHDATA_START/{flag=1;next}/FETCHDATA_END/{flag=0}flag' ${HTML_FILENAME} | grep -v "^//"`; do
    echo "prefetch_field_${ITERATOR},"  >> ${HEADER_FILENAME}
    (( ITERATOR+=1 ))
  done
  echo "};" >> ${HEADER_FILENAME}
  echo ""   >> ${HEADER_FILENAME}
  echo "#define ${HEADER_NAME_BASE}_PREFETCH_LEN ${ITERATOR}" >> ${HEADER_FILENAME}


  # Process all of the lines after the "//END"
  echo "Pass 3"
  echo ""  >> ${HEADER_FILENAME}
  echo " /*! @var ${HEADER_NAME_BASE}_text_2" >> ${HEADER_FILENAME}
  echo "  *  @brief generated variable  ${HEADER_NAME_BASE}_text_2" >> ${HEADER_FILENAME}
  echo "  */ " >> ${HEADER_FILENAME}
  echo "const char ${HEADER_NAME_BASE}_text_2[] PROGMEM = \"\\"  >> ${HEADER_FILENAME}
  awk '/FETCHDATA_END/{flag=1;next}flag' ${HTML_FILENAME} | grep -v "^//" | sed 's/\\/\\\\/g' | sed 's/$/\\/' | sed 's/\"/\\\"/g' | sed "s/^[ \t]*//" >> ${HEADER_FILENAME}
  echo "\";" >> ${HEADER_FILENAME}
  echo ""  >> ${HEADER_FILENAME}
  # Write out the number of characters in that string
  unset CHARSTRING
  CHARSTRING=$(awk '/FETCHDATA_END/{flag=1;next}flag' ${HTML_FILENAME} | grep -v "^//" | sed "s/^[ \t]*//" | awk '{printf("%s",$0)}')
  echo "" >> ${HEADER_FILENAME}
  echo "static const int ${HEADER_NAME_BASE}_text_2_len = $(echo $CHARSTRING | wc -m);" >> ${HEADER_FILENAME}



  echo "finalizing"

  echo ""  >> ${HEADER_FILENAME}
  echo "#endif"  >> ${HEADER_FILENAME}

done


