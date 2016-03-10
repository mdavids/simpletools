#!/bin/bash
#
# By SIDN Labs
#

if [ $# -eq 0 ] || [ $# -gt 2 ]
then
   echo "Usage:"
   echo "$0 <searchstring> [-d | -l]"
   echo "-d means sort by date, -l means sort by length."
   echo "minimum 4 characterstring required"
   exit 1
fi


if [ "$2" == "-d" ]
then
	# Sort with most recent date first
	
	curl -s http://retro.domain-registry.nl/search/$1 | jq -r 'to_entries[] | [.key, .value] | @csv' | \
	sed 's/"//g' | awk -F\, '{print $2";"$1}' | sort  -t"-" -rk 3 -rk 2 -rk 1 | awk -F\; '{print $2";"$1}'	
else
	if [ "$2" == "-l" ]
	then
		# Sort with shortest name first
		curl -s http://retro.domain-registry.nl/search/$1 | jq -r 'to_entries[] | [.key, .value] | @csv' | \
		sed 's/"//g' | awk -F\, '{print $2";"$1}' | awk '{ print length, $0 }' | sort -n | cut -d" " -f2-
	else
		if [ "$2" != "" ]
		then
			echo "Wrong option!"
			exit 1
		else
			# Default behaviour
			# As with the rest; skip the first line, which is a counter
			curl -s http://retro.domain-registry.nl/search/$1 | jq -r 'to_entries[] | [.key, .value] | @csv' | \
			sed 's/"//g' | awk -F\, '{print $2";"$1}'
		fi
	fi	
fi
