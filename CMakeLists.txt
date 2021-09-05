cmake_minimum_required(VERSION 3.16)
project(winds-aloft)

set(jsoncpp_DIR ~/projects/vcpkg/packages/jsoncpp_x64-linux/share/jsoncpp)

find_package(GeographicLib REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

include_directories(
        ../../grib2/c_api
        ../../grib2/wgrib2
)

link_directories(
        ../../grib2/lib
)

add_executable(winds-aloft
        ../../grib2/c_api/grb2_mk_inv.c 
        ../../grib2/c_api/grb2_inq.c 
        ../../grib2/c_api/grb2_cmd.c
        src/grib_repo.c
        src/Grib.cpp
        src/Conversions.cpp
        src/FlightPlanCalcs.cpp
        src/Route.cpp
        src/main.cpp
)

target_link_libraries(winds-aloft
        PRIVATE wgrib2
        ${GeographicLib_LIBRARIES}
        PRIVATE jsoncpp_object jsoncpp_static
)