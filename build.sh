#!/bin/bash
rm a.out
clang++ -std=c++14 main.cpp Conversions.cpp GribParser.cpp -l Geographic -l jsoncpp -l curl -l stdc++fs
