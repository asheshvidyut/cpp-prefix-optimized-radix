# Build on OSX
gcc -dynamic -fno-common -I/Users/asheshvidyut/redis/src -I/Users/asheshvidyut/rsquare/radix -std=gnu99 -c -o rsquare.o rsquare.c -undefined dynamic_lookup
clang -o rsquare.so rsquare.o -bundle -undefined dynamic_lookup -lc
