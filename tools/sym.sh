#!/bin/bash

FILE=utils/symbols.h
BAK=utils/symbols.h.bak

make()
{

	LIST=$(nm kernel.elf | grep  " T " | sed -e 's/^/{/g' -e 's/$/"},/g' -e 's/\sT\s/, "\s/g' -e 's/{/{0x/g') 

	awk -v l="$LIST" '/Generate/ { print; print l; next }1' $FILE > $BAK
	mv $BAK $FILE
}

clean()
{
	sed -i '/},/d' $FILE
}

case "$1" in
	make)
		make	
		;;
	clean)
		clean
		;;
	*)
		echo "Usage: $0 {make|clean}"
		exit 1
esac

exit $?