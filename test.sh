# test.c
# linux
gcc -o bin/test example/test.c -s -Wall -Wextra
# win32
~/c/mxe/usr/bin/i686-w64-mingw32.shared-gcc -o bin/test.exe example/test.c -s -Wall -Wextra -mconsole -municode
# valgrind linux
#valgrind bin/test wow this is a test


# clipboard.c
# linux
gcc -o bin/clipboard example/clipboard.c -s -Wall -Wextra -Wno-unused-function `pkg-config --cflags --libs gtk+-2.0`
# win32
~/c/mxe/usr/bin/i686-w64-mingw32.shared-gcc -o bin/clipboard.exe example/clipboard.c -s -Wall -Wextra -Wno-unused-function -mconsole

