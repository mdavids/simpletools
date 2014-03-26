#!/bin/bash
#
# By SIDN Labs (Marco Davids)
#
# Command Line version of http://dnssectest.sidnlabs.nl
# Useful in special situations, like testing at an ISP with Unbound in 'val-permissive-mode'
#

# Make a string of 30 charaters..
randomstr=$(date | sha1sum | cut -c -30)
# (use 'shasum' on Mac OSX)

echo Testing: $randomstr

# Both, because we don't know at which of the two loadbalanced dnssectest-servers we will end up...
# Google test (change this as you please, with resolvers of choice and with or without /dev/null 2>&1)
dig +adflag +short $randomstr.$randomstr.d.sidnlabs.nl @8.8.8.8 > /dev/null 2>&1

# just for test...
#dig +short DS $randomstr.d.sidnlabs.nl

# Safety buffer
sleep 1

# Get the results:
curl  --data "qname=$randomstr" http://dnssectest.sidnlabs.nl/result.php --header "Content-Type: application/x-www-form-urlencoded"
echo 

#
# OK: Resolving OK (we saw the queries coming in), but NO DNSSEC
# OKDNSSEC: Yes, we saw DS queries!
# DBERR: Internal DB error in the test-logic
# NOK: Undecided (this is the default situation)
#
