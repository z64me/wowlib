/* 
 * wow_clipboard.h
 * 
 * easy cross-platform clipboard magic
 * 
 * z64me <z64.me>
 *
 * linux dependencies: `pkg-config --cflags --libs gtk+-2.0`
 * 
 */

#ifndef WOW_CLIPBOARD_H
#define WOW_CLIPBOARD_H

#include <stdlib.h> /* malloc */
#include <string.h> /* strlen, strcpy */

/* copy a given string to the system clipboard */
static int wowClipboard_set(const char *string);

/* copy contents of a given file to the clipboard, as a text string */
static int wowClipboard_set_FILE(FILE *fp)
{
	size_t ofs;
	size_t len;
	char *string = 0;
	
	/* error: no file */
	if (!fp)
		goto L_fail;
	
	/* back up original offset */
	ofs = ftell(fp);
	
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	
	/* error: zero length file */
	if (!len)
		goto L_fail;
	
	/* error: no memory */
	if (!(string = malloc(len + 1)))
		goto L_fail;
	
	/* read file into buffer; doing it one character at a time
	 * can be slower, but it doesn't have the same limitations
	 * that sometimes accompany fread
	 */
	fseek(fp, 0, SEEK_SET);
	len = 0;
	while (!feof(fp))
		string[len++] = fgetc(fp);
	string[len] = '\0'; /* string terminator */
	
	/* copy file string to clipboard */
	wowClipboard_set(string);
	
	/* cleanup */
	free(string);
	fseek(fp, ofs, SEEK_SET); /* revert to original offset */
	return 0;
L_fail:
	if (string)
		free(string);
	if (fp)
		fseek(fp, ofs, SEEK_SET); /* revert to original offset */
	return 1;
}

#ifdef _WIN32

#include <windows.h>

static int wowClipboard_set(const char *string)
{
	HGLOBAL hMem = 0;
	
	/* allocate and ensure allocation */
	hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, strlen(string)+1);
	if (hMem == NULL)
		return 1;
	
	strcpy(GlobalLock(hMem), string);
	GlobalUnlock(hMem);
	
	if (OpenClipboard(0) == 0)
		return 1;
	if (EmptyClipboard() == 0)
		return 1;
	if (SetClipboardData(CF_TEXT, hMem) == NULL)
		return 1;
	if (CloseClipboard() == 0)
		return 1;
	
	return 0;
}

/* end win32 */

#else /* linux */

#include <gtk/gtk.h>

static int wowClipboard_set(const char *string)
{
	gtk_init_check(0, 0);
	GtkClipboard *clip = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_text(clip, string, -1);
	
	gtk_clipboard_set_can_store(clip, 0, 0);
	
	gtk_clipboard_store(clip);
	
	while(gtk_events_pending())
		gtk_main_iteration();
	
	return 0;
}

#endif /* linux */

#endif /* WOW_CLIPBOARD_H */


