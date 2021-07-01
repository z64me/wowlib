# build linux
gcc -o bin/test example/test.c -Wall -Wextra

# build win32
~/c/mxe/usr/bin/i686-w64-mingw32.shared-gcc -o bin/test.exe example/test.c -Wall -Wextra -mconsole -municode

# valgrind linux
valgrind bin/test wow this is a test

