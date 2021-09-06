# Winds Aloft

Dependencies:
* https://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/compile_questions.html
* https://geographiclib.sourceforge.io (You'll also need to install the magnetic model wmm2020)
* https://github.com/open-source-parsers/jsoncpp

Uses cmake so to build (after installing prerequsites)
* Create a build folder from the top level of the repo.
* cd build
* cmake ..
* make

Aims to build a flight plan with wind corrections sourced from the GFS weather model (See getData.sh for a pointer script on how to get that). It also uses data on Earth's magnetic field to report deviation values.

Lastly, this is a research/educational project. If used for actual flight planning, the user would be demonstrating questionable risk management skills, and ADM. So, don't do that eh.
