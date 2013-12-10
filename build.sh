LIBTOOL="libtool --tag=CC --silent"
$LIBTOOL --mode=compile cc -O2 -I/usr/include/lua5.1 -lpthread -c main.c
$LIBTOOL --mode=link cc -O2 -I/usr/include/lua5.1 -lpthread  -rpath /usr/local/lib/lua/5.1 -o libmain.la main.lo
mv .libs/libmain.so.0.0.0 uinput.so
