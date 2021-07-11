mkdir -p bin
PATH=$PATH:~/c/mxe/usr/bin

# test.c
# linux
gcc -o bin/test example/test.c -s -Wall -Wextra
# win32
i686-w64-mingw32.static-gcc -o bin/test.exe example/test.c -s -Wall -Wextra -mconsole -municode
# valgrind linux
#valgrind bin/test wow this is a test


# clipboard.c
# linux
gcc -o bin/clipboard example/clipboard.c -s -Wall -Wextra -Wno-unused-function `pkg-config --cflags --libs gtk+-2.0`
# win32
i686-w64-mingw32.static-gcc -o bin/clipboard.exe example/clipboard.c -s -Wall -Wextra -Wno-unused-function -mconsole


# gui.c
# linux
gcc -o bin/gui example/gui.c -s -Wall `deps/wow_gui_x11.sh`
# win32
i686-w64-mingw32.static-gcc -o bin/gui.exe example/gui.c -s -Wall `deps/wow_gui_win32.sh`

