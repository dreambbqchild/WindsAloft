#include <stdio.h>
#include <stdlib.h>
#include <c_wgrib2api.h>
#include "grib_repo.h"

const char* levels[TOTAL_LEVELS] = 
{
  "1 mb", 
  "1000 mb", 
  "100 mb", 
  "10 mb", 
  "150 mb", 
  "1829 m above mean sea level", 
  "2 mb", 
  "200 mb", 
  "20 mb", 
  "250 mb", 
  "2743 m above mean sea level", 
  "3 mb", 
  "300 mb", 
  "30 mb", 
  "350 mb", 
  "3658 m above mean sea level", 
  "400 mb", 
  "450 mb", 
  "5 mb", 
  "500 mb", 
  "50 mb", 
  "550 mb", 
  "600 mb", 
  "650 mb", 
  "7 mb", 
  "700 mb", 
  "70 mb", 
  "750 mb", 
  "800 mb", 
  "850 mb", 
  "900 mb", 
  "925 mb", 
  "950 mb", 
  "975 mb"
};

char gribFile[128];
char invFile[128];

void pathToFile(char* output, const char* dataFile, const char* extension)
{
  sprintf(output, "../data/%s.%s", dataFile, extension);
}

char* colon(char* output, const char* value)
{
  sprintf(output, ":%s:", value);
  return output;
}

double extractValue(size_t bufsize, char* buffer) 
{ 
    for(int i = bufsize - 1; i >= 0; i--){
      if(buffer[i] != '=')
        continue;

      return atof(&buffer[i + 1]);
    }

    return 0.0;
}

void grib_init(const char* dataFile)
{
    pathToFile(gribFile, dataFile, "grb");
    pathToFile(invFile, dataFile, "inv");
    
    int ierr = grb2_mk_inv(gribFile, invFile);
    if (ierr) exit(1);
}

double grib_value(const char* variableName, const char* level, double lat, double lon)
{
    char vars[4][64] = {};
    size_t bufsize;
    sprintf(vars[2], "%lf", lat);
    sprintf(vars[3], "%lf", lon);
    
    int ierr = wgrib2a(gribFile,"-i_file", invFile,"-rewind_init", invFile,
            "-inv", "/dev/null",
            "-fgrep", colon(vars[0], variableName),
            "-fgrep", colon(vars[1], level),
            "-lon", vars[3], vars[2],
            "-last0", "@mem:1", 
            NULL);
            
    bufsize = wgrib2_get_mem_buffer_size(1);
    char* buffer = (char*)calloc(bufsize, sizeof(char));
    ierr = wgrib2_get_mem_buffer((unsigned char *) buffer, bufsize, 1);
    if (ierr != 0) 
    {
      printf("Error: %d\n", ierr);
      return 0;
    }

    double result = extractValue(bufsize, buffer);

    free(buffer);
    return result;
}