# Winds Aloft

Dependencies:
* https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/compile_questions.html
* https://geographiclib.sourceforge.io (You'll also need to install the magnetic model wmm2015)
* https://github.com/open-source-parsers/jsoncpp

Aims to build a flight plan in 10nm intervals using data extracted from the GFS weather model (See getData.sh for a pointer script on how to get that).

The C++ is designed to be complied into a CGI program, then the JavaScript n' HTML can be used on an appropriately configured webserver to build a flight plan. What that appropriate configuration is, will be contingent upon your setup so I ain't goin' to venture to guess.

It would be a bad idea to face this as is on a webserver on the Interent cause it offers oppertunity to p0wn said server for an attacker. Virtual Machine recommended.

Lastly, this is a research/educational project. If used for actual flight planning, the user would be demonstrating questionable risk management skills, and ADM. So, don't do that eh.