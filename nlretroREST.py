#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# By SIDN Labs
#

import sys
import requests

API_URL = "http://retro.domain-registry.nl/"

if len(sys.argv) == 1:
   print "Usage: %s searchstring" % sys.argv[0]
   sys.exit(1)

try:
   res = requests.get(API_URL + "search/acropolis")

   if res.status_code != 200:
      print "Error (%i):" % res.status_code
      if res.status_code == 403:
          print "Access forbidden - are you whitelisted?"
      else:
          try:
              print res.json()["ErrorString"]
          except:
              print "undefined error"
      sys.exit(1)

   for key,value in res.json().iteritems():
       try:
           print "Naam: %s,datum: %s" % (key, value)
       except:
           print "[key-value error]"

except ValueError:
   print "Error: Decoding of JSON has failed."
   sys.exit(1)
   
