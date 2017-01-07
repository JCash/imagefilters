mkdir -p build

#clang -o test -g -O0 -m64 -std=c99 -Wall -Weverything -Wno-float-equal -pedantic -Isrc test/list.c
#clang++ -o ./build/test -g -O3 -m64 -Wall -Weverything -pedantic -Isrc test/main.cpp
#clang++ -o main.txt -g -O0 -m64 -Wall -Weverything -pedantic -Isrc -E test/main.cpp

clang -o ./build/imgfilters -g -O3 -m64 -Wall -Weverything -pedantic -Wno-cast-align -Wno-cast-qual -Wno-missing-prototypes -Wno-padded -Wno-double-promotion -Wno-unused-parameter -Wno-conversion -Wno-disabled-macro-expansion -Wno-sign-conversion -Isrc src/main.c
clang -o ./build/test -std=c99 -g -O0 -m64 -Wall -Weverything -pedantic -Isrc test/test.c
