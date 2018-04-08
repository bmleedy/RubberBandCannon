echo "////////////////////////////////////////////////" > ${1}.h
echo "//GENERATED FILE -- DO NOT HAND-MODIFY!!!!!!!!!!" >> ${1}.h
echo "////////////////////////////////////////////////" >> ${1}.h
echo "const char static_website_text[] PROGMEM = \"\\" >> ${1}.h
cat $1 | grep -v "htmldebug" | sed 's/$/\\/' | sed 's/\"/\\\"/g' | sed "s/^[ \t]*//" >> ${1}.h
echo "\";" >> ${1}.h