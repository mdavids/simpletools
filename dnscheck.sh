#!/bin/bash
#
# Command-line version of http://dnscheck.sidn.nl/
# Proof of concept - Created by SIDN Labs - 20130424 (md)
#
##########################################################################
#
# NOTE: X-Requested-With not really required
# We are assuming: a "status":"OK" on this request, so disregarding the output
# (we could check for it, as an improvement in a later version)
# TODO: perhaps not issue a new request, but first check if one is 'IN_PROGRESS' already
#
PATIENCE=60
DOMAIN=$1
if [ "$DOMAIN" == "" ]
then
	echo "Usage: ./dnscheck.sh example.nl"
	exit 1
fi
echo "Checking $DOMAIN..."
# Issue the request
curl -s -H "X-Requested-With: XMLHttpRequest" --data "domain=$DOMAIN&test=standard&lang=&parameters=" http://dnscheck.sidn.nl/getPager.php > /dev/null
#
# Now, wait for the results...
#
# Pretent as if we are already 'IN_PROGRESS'
RESULT='    "result": "IN_PROGRESS", '
COUNT=1
while [[ "$RESULT" == '    "result": "IN_PROGRESS", '  &&  $COUNT -lt $PATIENCE ]]
do 
	# "result": "IN_PROGRESS",
	RESULT=$(echo $(curl -s -H "X-Requested-With: XMLHttpRequest" --data "domain=$DOMAIN&test=standard&lang=&parameters=" http://dnscheck.sidn.nl/getResult.php) | python -mjson.tool | grep '"result": ')
	echo "Please be patient...($COUNT seconds elapsed from $PATIENCE)"
	let COUNT=COUNT+1
	sleep 1
done
echo $(curl -s -H "X-Requested-With: XMLHttpRequest" --data "domain=$DOMAIN&test=standard&lang=&parameters=" http://dnscheck.sidn.nl/getResult.php) | python -mjson.tool
if [ $COUNT -eq $PATIENCE ]
then
	echo "No results, test obviously still in progress..."
fi
if [ "$RESULT" == '    "result": "WARNING", ' ]
then
	echo "Please be advised of the WARNING!"
fi
# Or use the GET method
# (example for undelegated check)
#echo $(curl -s http://dnscheck.sidn.nl/getPager.php?domain=dnssec.nl&page=1&test_type=undelegated&parameters=ns1.sidn.nl./94.198.152.68 ns2.sidn.nl./194.171.17.5) | python -mjson.tool
#echo $(curl -s http://dnscheck.sidn.nl/getResult.php?domain=dnssec.nl&test=undelegated&lang=&parameters=ns1.sidn.nl./94.198.152.68 ns2.sidn.nl./194.171.17.5) | python -mjson.tool
