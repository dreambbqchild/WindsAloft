#!/bin/bash

TIMESTAMP=20210905
ZHOUR=06

rm -rf ./data
mkdir data

echo "{" > registry.json

for i in `seq 0 24`;
do
	if [ $i -gt 0 ]
	then
		echo "," >> registry.json
		FORECAST=$(printf "f%03d" $i)
	else
		FORECAST="anl"
	fi

	HOUR_OFFSET=$(($ZHOUR + $i))
	FOR_TIME=$(date --date "TZ=\"UTC\" ${TIMESTAMP} +${HOUR_OFFSET} hours" --iso-8601=seconds)

	echo -n "   \"${FORECAST}\": \"${FOR_TIME}\"" >> registry.json
	wget "https://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_0p25_1hr.pl?file=gfs.t${ZHOUR}z.pgrb2.0p25.${FORECAST}&lev_1_mb=on&lev_1000_mb=on&lev_100_mb=on&lev_10_mb=on&lev_150_mb=on&lev_1829_m_above_mean_sea_level=on&lev_2_mb=on&lev_200_mb=on&lev_20_mb=on&lev_250_mb=on&lev_2743_m_above_mean_sea_level=on&lev_3_mb=on&lev_300_mb=on&lev_30_mb=on&lev_350_mb=on&lev_3658_m_above_mean_sea_level=on&lev_400_mb=on&lev_450_mb=on&lev_5_mb=on&lev_500_mb=on&lev_50_mb=on&lev_550_mb=on&lev_600_mb=on&lev_650_mb=on&lev_7_mb=on&lev_700_mb=on&lev_70_mb=on&lev_750_mb=on&lev_800_mb=on&lev_850_mb=on&lev_900_mb=on&lev_925_mb=on&lev_950_mb=on&lev_975_mb=on&lev_mean_sea_level=on&var_MSLET=on&var_UGRD=on&var_VGRD=on&var_TMP=on&subregion=&leftlon=-125.638281&rightlon=-64.114844&toplat=49.959211&bottomlat=23.736383&dir=%2Fgfs.${TIMESTAMP}%2F${ZHOUR}%2Fatmos" -O ./data/${FORECAST}.grb
done

echo '' >> registry.json
echo -n '}' >> registry.json