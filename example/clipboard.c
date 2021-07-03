#include <stdio.h>

#include "wow_clipboard.h"

int main(void)
{
	wowClipboard_set("wow, custom clipboard text, wow");
	
	fprintf(stderr, "copied string to clipboard\n");
	
	getchar();
	
	return 0;
}

