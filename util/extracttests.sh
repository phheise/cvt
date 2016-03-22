#! /bin/bash

if [ $# -ne 2 ]
then
    name=$(basename $0);
    echo "usage: $name lib header";
    exit 1;
fi

if [ ! -f $1 ]
then
    echo "No such file!";
    exit 1;
fi

echo $OSTYPE;
case $OSTYPE in
    "linux-gnu") 
		symextractor="objdump -t $1";
		rm -f $2;
		echo -e "#include <cvt/util/CVTTest.h>\n\nextern \"C\" { " >> $2;
		$symextractor | awk '/.*_test/ {print "\tbool "$6"( void );"}' >> $2;
		echo -e "}\n" >> $2;

		echo -e "\nCVTTest _tests[] = {" >> $2;
		$symextractor | awk '/.*_test/ {print "\t{ "$6", \""$6"\" }," }' >> $2;
		echo -e "\t{ NULL, NULL}\n};" >> $2;;
    "darwin10.0"|"darwin11"|"darwin12"|"darwin13"|"darwin14")
		symextractor="nm -g $1";
		rm -f $2;
		echo -e "#include <cvt/util/CVTTest.h>\n\nextern \"C\" { " >> $2;
		$symextractor | awk '/.*_test/ {print "\tbool "substr($3, 2)"( void );"}' >> $2;
		echo -e "}\n" >> $2;

		echo -e "\nCVTTest _tests[] = {" >> $2;
		$symextractor | awk '/.*_test/ {print "\t{ "substr($3, 2)", \""substr($3, 2)"\" }," }' >> $2;
		echo -e "\t{ NULL, NULL}\n};" >> $2;;
    *) echo "Unknown OS";
       exit 1;;
esac



