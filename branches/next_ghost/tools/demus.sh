#!/bin/bash
BASEDIR=`echo $0 | sed -e 's:[^/]*$::'`
DEMUS="${BASEDIR}demus"
MUSINFO="${BASEDIR}musinfo"

if [ $# -eq "0" ]; then
	echo "Usage: $0 files"
fi

for i in "$@"; do
	if [ -f "$i" ]; then
		echo "demus $i"
		$DEMUS "$i" | lame `$MUSINFO "$i"` -V 4 - ${i/%.MUS/.mp3}
	else
		echo "File $i not found"
	fi
done
