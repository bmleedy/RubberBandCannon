echo "////////////////////////////////////////////////" > ${1}.hh
echo "//GENERATED FILE -- DO NOT HAND-MODIFY!!!!!!!!!!" >> ${1}.hh
echo "////////////////////////////////////////////////" >> ${1}.hh
echo "const char static_website_text[] PROGMEM = \"\\" >> ${1}.hh
cat $1 | grep -v "htmldebug" | sed 's/$/\\/' | sed 's/\"/\\\"/g' | sed "s/^[ \t]*//" >> ${1}.hh
echo "\";" >> ${1}.hh