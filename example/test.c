#include <stdio.h>
#include <dirent.h>

#define WOW_IMPLEMENTATION
#include "wow.h"
#include "wow_dirent.h"

int wow_main(argc, argv)
{
	wow_main_args(argc, argv);
	int i;
	wow_DIR *dir;
	FILE *fp;
	
	/* print arguments */
	for (i = 0; i < argc; ++i)
	{
		fprintf(stderr, "arg[%d] '%s'\n", i, argv[i]);
	}
	
	/* enter directory of the executable */
	wow_chdir_file(argv[0]);
	
	/* print contents of directory */
	if ((dir = wow_opendir(".")))
	{
		struct wow_dirent *ep;
		
		while ((ep = wow_readdir(dir)))
		{
			fprintf(stderr, "'%s'\n", wow_dirent_dname(ep));
		}
		wow_closedir(dir);
	}
	
	/* create a file with a fancy utf8 filename */
	if ((fp = wow_fopen("wów.txt", "w")))
	{
		fprintf(fp, "wów");
		wow_fclose(fp);
	}
	
#ifdef _WIN32
	getchar();
#endif
	
	return 0;
}

