#!/bin/bash
#
# By SIDN Labs
#

if [ $# -eq 0 ] || [ $# -gt 2 ]
then
   echo "Usage:"
   echo "$0 <searchstring> [-d | -l]"
   echo "-d means sort by date, -l means sort by length."
   exit 1
fi


if [ "$2" == "-d" ]
then
	# Sort with most recent date first
	echo $1 | nc retro.domain-registry.nl 43 | sed '1d' |  awk -F\; '{print $2";"$1}' | sort  -t"-" -rk 3 -rk 2 -rk 1 | awk -F\; '{print $2";"$1}'
else
	if [ "$2" == "-l" ]
	then
		# Sort with shortest name first
		echo $1 | nc retro.domain-registry.nl 43 | sed '1d' |  awk '{ print length, $0 }' | sort -n | cut -d" " -f2-
	else
		if [ "$2" != "" ]
		then
			echo "Wrong option!"
			exit 1
		else
			# Default behaviour
			# As with the rest; skip the first line, which is a counter
			echo $1 | nc retro.domain-registry.nl 43 | sed '1d'
		fi
	fi
fi
