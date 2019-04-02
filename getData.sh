#!/bin/bash

export ZHOUR=18
export TIMESTAMP=2019040218
export FORECASTS=(anl f001 f002 f003 f004 f005 f006 f007 f008 f009 f010 f011 f012 f013 f014)
export FILENAMES=(analysis hour1 hour2 hour3 hour4 hour5 hour6 hour7 hour8 hour9 hour10 hour11 hour12 hour13 hour14)

for i in `seq 0 14`;
do
	wget "https://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_0p25_1hr.pl?file=gfs.t${ZHOUR}z.pgrb2.0p25.${FORECASTS[$i]}&lev_1_mb=on&lev_1000_mb=on&lev_100_mb=on&lev_10_mb=on&lev_150_mb=on&lev_1829_m_above_mean_sea_level=on&lev_2_mb=on&lev_200_mb=on&lev_20_mb=on&lev_250_mb=on&lev_2743_m_above_mean_sea_level=on&lev_3_mb=on&lev_300_mb=on&lev_30_mb=on&lev_350_mb=on&lev_3658_m_above_mean_sea_level=on&lev_400_mb=on&lev_450_mb=on&lev_5_mb=on&lev_500_mb=on&lev_50_mb=on&lev_550_mb=on&lev_600_mb=on&lev_650_mb=on&lev_7_mb=on&lev_700_mb=on&lev_70_mb=on&lev_750_mb=on&lev_800_mb=on&lev_850_mb=on&lev_900_mb=on&lev_925_mb=on&lev_950_mb=on&lev_975_mb=on&lev_mean_sea_level=on&var_MSLET=on&var_UGRD=on&var_VGRD=on&var_TMP=on&subregion=&leftlon=-125.638281&rightlon=-64.114844&toplat=49.959211&bottomlat=23.736383&dir=%2Fgfs.${TIMESTAMP}" -O ~/public_html/flightPlan/grib/${FILENAMES[$i]}.grb
done
