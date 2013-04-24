#!/bin/bash
#
# Command-line version of http://dnscheck.sidn.nl/
# Proof of concept - Created by SIDN Labs - 20130424 (md)
#
##########################################################################
#
# Issue the request (real working version would use $1 parameters and stuff...
# NOTE: X-Requested-With not really required
# We are assuming: a "status":"OK" on this request, so disregarding the output
# (we could check for it, as an improvement in a later version)
#
curl -s -H "X-Requested-With: XMLHttpRequest" --data "domain=dnssec.nl&page=1&test_type=undelegated&parameters=ns1.sidn.nl./94.198.152.68 ns2.sidn.nl./194.171.17.5" http://dnscheck1.sidn.nl/getPager.php > /dev/null
#
# Now, wait for the results...
#
RESULT=""
COUNT=0
while [[ "$RESULT" != '    "result": "OK", '  &&  $COUNT -lt 30 ]]
do 
	# "result": "IN_PROGRESS",
	RESULT=$(echo $(curl -s -H "X-Requested-With: XMLHttpRequest" --data "domain=dnssec.nl&test=undelegated&lang=&parameters=ns1.sidn.nl./94.198.152.68 ns2.sidn.nl./194.171.17.5" http://dnscheck.sidn.nl/getResult.php) | python -mjson.tool | grep '"result": ')
	echo "Please be patient..."
	let COUNT=COUNT+1
	sleep 1
done
echo $(curl -s -H "X-Requested-With: XMLHttpRequest" --data "domain=dnssec.nl&test=undelegated&lang=&parameters=ns1.sidn.nl./94.198.152.68 ns2.sidn.nl./194.171.17.5" http://dnscheck.sidn.nl/getResult.php) | python -mjson.tool

# Or use thhe GET method
#echo $(curl -s http://dnscheck1.sidn.nl/getPager.php?domain=dnssec.nl&page=1&test_type=undelegated&parameters=ns1.sidn.nl./94.198.152.68 ns2.sidn.nl./194.171.17.5) | python -mjson.tool
#echo $(curl -s http://dnscheck.sidn.nl/getResult.php?domain=dnssec.nl&test=undelegated&lang=&parameters=ns1.sidn.nl./94.198.152.68 ns2.sidn.nl./194.171.17.5) | python -mjson.tool
