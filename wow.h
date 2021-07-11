/* 
 * wow.h
 * 
 * a small collection of functions to make writing software easier
 * 
 * z64me <z64.me>
 *
 * in one compilation unit, #define WOW_IMPLEMENTATION before
 * you #include this file; you can then #include this file
 * elsewhere without the WOW_IMPLEMENTATION, and the linker
 * will take care of the rest
 *
 * you can also #define WOW_OVERLOAD_FILE before you #include to
 * have fopen/fread/fwrite/fclose redirected to libwow
 *
 * you can also #define WOW_OVERLOAD_ALLOCATORS before you #include
 * to have malloc/calloc/realloc/free redirected to libwow
 *
 * TODO WOW_OVERLOAD_ALL eventually
 * 
 */

#ifndef WOW_H_INCLUDED
#define WOW_H_INCLUDED

#include <stddef.h> /* size_t */
#include <stdio.h> /* file ops */
#include <stdlib.h> /* alloc */
#include <sys/stat.h> /* stat */
#include <fcntl.h> /* open */
#include <string.h> /* strdup */
#include <unistd.h> /* chdir, getcwd */
#include <stdarg.h>
#include <locale.h>
#include <errno.h>

#ifdef _WIN32
 #include <windows.h>
 #if defined(UNICODE) && !defined(_UNICODE)
  #define _UNICODE
 #endif
#endif


#define WOW_MACROCAT1(A, B) A##B
#define WOW_MACROCAT(A, B) WOW_MACROCAT1(A, B)

#if defined(_WIN32) && defined(_UNICODE)
 #define wow_main(A, B)       wmain(int A, wchar_t *WOW_MACROCAT(W, B)[])
 #define wow_main_args(A, B)  char **B = wow_conv_args(A, (void*)WOW_MACROCAT(W, B))
#else
 #define wow_main(A, B)       main(int A, char *B[])
 #define wow_main_args(A, B)  do{}while(0)
#endif


#ifndef WOW_API_PREFIX
 #define WOW_API_PREFIX
#endif

WOW_API_PREFIX
void *
wow_utf8_to_wchar(const char *str);

WOW_API_PREFIX
void *
wow_utf8_to_wchar_die(const char *str);

WOW_API_PREFIX
char *
wow_wchar_to_utf8(void *wstr);

WOW_API_PREFIX
char *
wow_wchar_to_utf8_die(void *wstr);


/* converts argv[] from wchar to char win32, in place */
WOW_API_PREFIX
void *
wow_conv_args(int argc, void *argv[]);


/* returns non-zero if path is a directory */
WOW_API_PREFIX
int
wow_is_dir(char const *path);


/* fread abstraction that falls back to buffer-based fread *
 * if a big fread fails; if that still fails, returns 0    */
WOW_API_PREFIX
size_t
wow_fread_bytes(void *ptr, size_t bytes, FILE *stream);


/* fwrite abstraction that falls back to buffer-based fwrite *
 * if a big fwrite fails; if that still fails, returns 0     */
WOW_API_PREFIX
size_t
wow_fwrite_bytes(const void *ptr, size_t bytes, FILE *stream);


/* fread abstraction that falls back to buffer-based fread *
 * if a big fread fails; if that still fails, returns 0    */
WOW_API_PREFIX
size_t
wow_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);


/* fwrite abstraction that falls back to buffer-based fwrite *
 * if a big fwrite fails; if that still fails, returns 0     */
WOW_API_PREFIX
size_t
wow_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);


/* fopen abstraction for utf8 support on windows win32 */
WOW_API_PREFIX
FILE *
wow_fopen(char const *name, char const *mode);


/* open abstraction for utf8 support on windows win32 */
WOW_API_PREFIX
int
wow_open(const char *path, int flags, int mode);


/* remove abstraction for utf8 support on windows win32 */
WOW_API_PREFIX
int
wow_remove(char const *path);


/* mkdir */
WOW_API_PREFIX
int
wow_mkdir(char const *path);


/* chdir */
WOW_API_PREFIX
int
wow_chdir(char const *path);


/* chdir into directory of provided file */
WOW_API_PREFIX
int
wow_chdir_file(char const *path);


/* getcwd */
WOW_API_PREFIX
char *
wow_getcwd(char *buf, size_t size);


/* getcwd_die (dies on allocation failure) */
WOW_API_PREFIX
char *
wow_getcwd_die(char *buf, size_t size);


/* system */
WOW_API_PREFIX
int
wow_system(char const *path);


/* system_gui */
WOW_API_PREFIX
int
wow_system_gui(char const *name, const char *param);


/* window icon */
WOW_API_PREFIX
void
wow_windowicon(int iconId);


WOW_API_PREFIX void wow_die(const char *fmt, ...)
	__attribute__ ((format (printf, 1, 2)))
;
/* these die on allocation failure */
WOW_API_PREFIX void *wow_calloc_die(size_t nmemb, size_t size);
WOW_API_PREFIX void *wow_malloc_die(size_t size);
WOW_API_PREFIX void *wow_realloc_die(void *ptr, size_t size);
WOW_API_PREFIX char *wow_strdup_die(const char *s);
WOW_API_PREFIX void *wow_memdup_die(void *ptr, size_t size);

WOW_API_PREFIX void wow_free(void *ptr);

#ifdef WOW_IMPLEMENTATION

WOW_API_PREFIX void wow_die(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#if defined(_WIN32) && defined(_UNICODE)
	char buf[4096];
	vsprintf(buf, fmt, args);
	wchar_t *wc = wow_utf8_to_wchar_die(buf);
	setlocale(LC_ALL, "");
	fwprintf(stderr, L"%ls", wc);
	free(wc);
#else
	vfprintf(stderr, fmt, args);
#endif
	va_end(args);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

WOW_API_PREFIX void wow_free(void *ptr)
{
	free(ptr);
}

WOW_API_PREFIX void *wow_calloc_die(size_t nmemb, size_t size)
{
	void *result = (calloc)(nmemb, size);
	
	if (!result)
		wow_die("memory error");
	
	return result;
}

WOW_API_PREFIX void *wow_malloc_die(size_t size)
{
	void *result = (malloc)(size);
	
	if (!result)
		wow_die("memory error");
	
	return result;
}

WOW_API_PREFIX void *wow_realloc_die(void *ptr, size_t size)
{
	void *result = (realloc)(ptr, size);
	
	if (!result)
		wow_die("memory error");
	
	return result;
}

WOW_API_PREFIX char *wow_strdup_die(const char *s)
{
	char *result;
	int n;
	
	if (!s)
		return 0;
	
	n = strlen(s) + 1;
	
	result = wow_malloc_die(n);
	
	strcpy(result, s);
	
	return result;
}

WOW_API_PREFIX void *wow_memdup_die(void *ptr, size_t size)
{
	void *result;
	
	if (!ptr || !size)
		return 0;
	
	result = wow_malloc_die(size);
	
	memcpy(result, ptr, size);
	
	return result;
}


WOW_API_PREFIX
void *
wow_utf8_to_wchar(const char *str)
{
	if (!str)
		return 0;
#if defined(_WIN32) && defined(_UNICODE)
	const char *src = str;
	wchar_t *out = 0;
	size_t src_length;
	int length;
	
	src_length = strlen(src);
	length = MultiByteToWideChar(CP_UTF8, 0, src, src_length, 0, 0);
	out = (malloc)((length+1) * sizeof(*out));
	if (out) {
		MultiByteToWideChar(CP_UTF8, 0, src, src_length, out, length);
		out[length] = L'\0';
	}
	return out;
#else
    return strdup(str);
#endif
}


WOW_API_PREFIX
void *
wow_utf8_to_wchar_die(const char *str)
{
	void *result = wow_utf8_to_wchar(str);
	
	if (!result)
		wow_die("memory error");
	
	return result;
}

WOW_API_PREFIX
char *
wow_wchar_to_utf8_buf(void *wstr, void *dst, int dst_max)
{
#if defined(_WIN32) && defined(_UNICODE)
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, dst, dst_max, 0, 0);
    return dst;
#else
    (void)dst_max; /* unused parameter */
    return strcpy(dst, wstr);
#endif
}

WOW_API_PREFIX
char *
wow_wchar_to_utf8(void *wstr)
{
	if (!wstr)
		return 0;
#if defined(_WIN32) && defined(_UNICODE)
	const void *src = wstr;
	char *out = 0;
	size_t src_length = 0;
	int length;
	
	src_length = wcslen(src);
	length = WideCharToMultiByte(CP_UTF8, 0, src, src_length,
			0, 0, NULL, NULL);
	out = (malloc)((length+1) * sizeof(char));
	if (out) {
		WideCharToMultiByte(CP_UTF8, 0, src, src_length,
				out, length, NULL, NULL);
		out[length] = '\0';
	}
	return out;
#else
    return strdup(wstr);
#endif
}

WOW_API_PREFIX
char *
wow_wchar_to_utf8_die(void *wstr)
{
	char *result = wow_wchar_to_utf8(wstr);
	
	if (!result)
		wow_die("memory error");
	
	return result;
}

WOW_API_PREFIX
char *
wow_wchar_to_utf8_inplace(void *wstr)
{
#if defined(_WIN32) && defined(_UNICODE)
    char buf[4096];
    char *str;
    int wstr_len = wcslen(wstr);
    unsigned str_sz = (wstr_len + 1) * sizeof(*str);
    if (str_sz >= sizeof(buf))
        str = wow_malloc_die(str_sz);
    else
        str = buf;
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, str_sz, 0, 0);
    memcpy(wstr, str, wstr_len + 1);
    ((char*)wstr)[wstr_len+1] = '\0';
    if (str != buf)
        free(str);
    return wstr;
#else
    return wstr;
#endif
}


/* argument abstraction: converts argv[] from wchar to char win32 */
WOW_API_PREFIX
void *
wow_conv_args(int argc, void *argv[])
{
#if defined(_WIN32) && defined(_UNICODE)
	int i;
	for (i = 0; i < argc; ++i)
	{
		//fprintf(stderr, "[%d]: %s\n", i, argv[i]);
		//fwprintf(stderr, L"[%d]: %s\n", i, (wchar_t*)argv[i]);
		argv[i] = wow_wchar_to_utf8_inplace(argv[i]);
		//fwprintf(stderr, L"[%d]: %s\n", i, wow_utf8_to_wchar_die(argv[i]));
	}
#else
	(void)argc; /* unused parameter */
#endif
	return argv;
}

/* returns non-zero if path is a directory */
static
int
private_is_dir_root(void const *path)
{
	struct stat s;
#if defined(_WIN32) && defined(_UNICODE)
	if (wstat(path, &s) == 0)
#else
	if (stat(path, &s) == 0)
#endif
	{
		if (s.st_mode & S_IFDIR)
			return 1;
	}
	
	return 0;
}


/* returns non-zero if path is a directory */
WOW_API_PREFIX
int
wow_is_dir(char const *path)
{
	int rv;
	void *wpath = 0;
	
#if defined(_WIN32) && defined(_UNICODE)
	wpath = wow_utf8_to_wchar_die(path);
	rv = private_is_dir_root(wpath);
#else
	rv = private_is_dir_root(path);
#endif
	if (wpath)
		free(wpath);
	
	return rv;
}


/* fread abstraction that falls back to buffer-based fread *
 * if a big fread fails; if that still fails, returns 0    */
WOW_API_PREFIX
size_t
wow_fread_bytes(void *ptr, size_t bytes, FILE *stream)
{
	if (!stream || !ptr || !bytes)
		return 0;
	
	unsigned char *ptr8 = ptr;
	size_t Oofs = ftell(stream);
	size_t bufsz = 1024 * 1024; /* 1 mb at a time */
	size_t Obytes = bytes;
	size_t rem;
	
	fseek(stream, 0, SEEK_END);
	rem = ftell(stream) - Oofs;
	fseek(stream, Oofs, SEEK_SET);
	
	if (bytes > rem)
		bytes = rem;
	
	/* everything worked */
	if ((fread)(ptr, 1, bytes, stream) == bytes)
		return Obytes;
	
	/* failed: try falling back to slower buffered read */
	fseek(stream, Oofs, SEEK_SET);
	while (bytes)
	{
		/* don't read past end */
		if (bytes < bufsz)
			bufsz = bytes;
		if (bufsz > rem)
		{
			bytes = rem;
			bufsz = rem;
		}
		
		/* still failed */
		if ((fread)(ptr8, 1, bufsz, stream) != bufsz)
			return 0;
		
		/* advance */
		ptr8 += bufsz;
		bytes -= bufsz;
		rem -= bufsz;
	}
	
	/* success */
	return Obytes;
}


/* fwrite abstraction that falls back to buffer-based fwrite *
 * if a big fwrite fails; if that still fails, returns 0     */
WOW_API_PREFIX
size_t
wow_fwrite_bytes(const void *ptr, size_t bytes, FILE *stream)
{
	if (!stream || !ptr || !bytes)
		return 0;
	
	const unsigned char *ptr8 = ptr;
	size_t bufsz = 1024 * 1024; /* 1 mb at a time */
	size_t Obytes = bytes;
	
	/* everything worked */
	if ((fwrite)(ptr, 1, bytes, stream) == bytes)
		return bytes;
	
	/* failed: try falling back to slower buffered read */
	while (bytes)
	{
		/* don't read past end */
		if (bytes < bufsz)
			bufsz = bytes;
		
		/* still failed */
		if ((fwrite)(ptr8, 1, bufsz, stream) != bufsz)
			return 0;
		
		/* advance */
		ptr8 += bufsz;
		bytes -= bufsz;
	}
	
	/* success */
	return Obytes;
}


/* fread abstraction that falls back to buffer-based fread *
 * if a big fread fails; if that still fails, returns 0    */
WOW_API_PREFIX
size_t
wow_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if (!stream || !ptr || !size || !nmemb)
		return 0;
	
	if (wow_fread_bytes(ptr, size * nmemb, stream) == size * nmemb)
		return nmemb;
	
	return 0;
}


/* fwrite abstraction that falls back to buffer-based fwrite *
 * if a big fwrite fails; if that still fails, returns 0     */
WOW_API_PREFIX
size_t
wow_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if (!stream || !ptr || !size || !nmemb)
		return 0;
	
	if (wow_fwrite_bytes(ptr, size * nmemb, stream) == size * nmemb)
		return nmemb;
	
	return 0;
}


/* fopen abstraction for utf8 support on windows win32 */
WOW_API_PREFIX
FILE *
wow_fopen(char const *name, char const *mode)
{
#if defined(_WIN32) && defined(_UNICODE)
	void *wname = 0;
	void *wmode = 0;
	FILE *fp = 0;
	
	wname = wow_utf8_to_wchar_die(name);
	if (!wname)
		goto L_cleanup;
	
	/* TODO eventually, an error message would be cool */
	if (private_is_dir_root(wname))
		goto L_cleanup;
	
	wmode = wow_utf8_to_wchar_die(mode);
	if (!wmode)
		goto L_cleanup;
	
	fp = _wfopen(wname, wmode);
	
L_cleanup:
	if (wname) free(wname);
	if (wmode) free(wmode);
	if (fp)
		return fp;
	return 0;
#else
	/* TODO eventually, an error message would be cool */
	if (private_is_dir_root(name))
		return 0;
	return (fopen)(name, mode);
#endif
}


/* open abstraction for utf8 support on windows win32 */
WOW_API_PREFIX
int
wow_open(const char *path, int flags, int mode)
{
	int fd;
	
#if defined(_WIN32) && defined(_UNICODE)
	void *p = wow_utf8_to_wchar_die(path);
	fd = _wopen(p, flags, mode);
	free(p);
#else
	fd = open(path, flags, mode);
#endif
	return fd;
}


/* remove abstraction for utf8 support on windows win32 */
WOW_API_PREFIX
int
wow_remove(char const *path)
{
#if defined(_WIN32) && defined(_UNICODE)
	void *wpath = 0;
	int rval;
	
	wpath = wow_utf8_to_wchar_die(path);
	if (!wpath)
		return -1;
	
	rval = _wremove(wpath);
	free(wpath);
	return rval;
#else
	return remove(path);
#endif
}


/* mkdir */
WOW_API_PREFIX
int
wow_mkdir(char const *path)
{
#if defined(_WIN32) && defined(_UNICODE)
extern int _wmkdir(const wchar_t *);
	void *wname = 0;
	int rval;
	
	wname = wow_utf8_to_wchar_die(path);
	if (!wname)
		return -1;
	
	rval = _wmkdir(wname);
	
	if (wname)
		free(wname);
	
	return rval;
#elif defined(_WIN32) /* win32 no unicode */
extern int _mkdir(const char *);
	return _mkdir(path);
#else /* ! _WIN32 */
	return mkdir(path, 0777);
#endif
}


/* chdir */
WOW_API_PREFIX
int
wow_chdir(char const *path)
{
#if defined(_WIN32) && defined(_UNICODE)
extern int _wchdir(const wchar_t *);
	void *wname = 0;
	int rval;
	
	wname = wow_utf8_to_wchar_die(path);
	if (!wname)
		return -1;
	
	rval = _wchdir(wname);
	
	if (wname)
		free(wname);
	
	return rval;
#elif defined(_WIN32) /* win32 no unicode */
extern int _chdir(const char *);
	return _chdir(path);
#else /* ! _WIN32 */
	return chdir(path);
#endif
}


/* chdir into directory of provided file */
WOW_API_PREFIX
int
wow_chdir_file(char const *path)
{
	char *p = wow_strdup_die(path);
	char *slash = 0;
	char *slash1 = 0;
	int rval;
	
	/* account for 'C:\Program Files\wow\wow.exe'
	 * as well as '/home/wow/wow'
	 * and even relative paths like './wow', 'bin/wow', or 'wow'
	 */
	slash = strrchr(p, '/');
	slash1 = strrchr(p, '\\');
	slash = (slash1 > slash) ? slash1 : slash;
	if (!slash) /* already in the same directory */
		return 0;
	*slash = '\0';
	
	rval = wow_chdir(p);
	free(p);
	
	return rval;
}


/* getcwd */
WOW_API_PREFIX
char *
wow_getcwd(char *buf, size_t size)
{
#if defined(_WIN32) && defined(_UNICODE)
extern int _wgetcwd(const wchar_t *, int);
	wchar_t wname[4096];
	
	if (!buf || !size)
		return 0;
	
	if (!_wgetcwd(wname, sizeof(wname) / sizeof(wname[0])))
		return 0;
	
	return wow_wchar_to_utf8_buf(wname, buf, size);
#elif defined(_WIN32) /* win32 no unicode */
//extern char *_getcwd(char *, int);
	return _getcwd(buf, size);
#else /* ! _WIN32 */
	return getcwd(buf, size);
#endif
}


/* getcwd_die */
WOW_API_PREFIX
char *
wow_getcwd_die(char *buf, size_t size)
{
	char *result = wow_getcwd(buf, size);
	
	if (!result)
		wow_die("failed to get current working directory");
	
	return result;
}


/* system */
WOW_API_PREFIX
int
wow_system(char const *path)
{
#if defined(_WIN32) && defined(_UNICODE)
	void *wname = 0;
	int rval;
	
	wname = wow_utf8_to_wchar_die(path);
	if (!wname)
		return -1;
	
	rval = _wsystem(wname);
	
	if (wname)
		free(wname);
	
	return rval;
#else /* not win32 unicode */
	return system(path);
#endif
}


/* system_gui */
WOW_API_PREFIX
int
wow_system_gui(char const *name, const char *param)
{
#if defined(_WIN32)
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	int rval = 0 /*success */;
//extern int ShellExecuteA(void *hwnd, void *op, void *file, void *param, void *dir, int cmd);
//extern int ShellExecuteW(void *hwnd, void *op, void *file, void *param, void *dir, int cmd);
//const int SW_SHOWNORMAL = 1;
	#if defined(_UNICODE)
		void *wname = 0;
		void *wparam = 0;
		
		wname = wow_utf8_to_wchar_die(name);
		if (!wname)
		{
			return -1;
		}
		wparam = wow_utf8_to_wchar_die(param);
		if (!wparam)
		{
			free(wname);
			return -1;
		}
		
#if 0
		if (CreateProcessW(
			wname, wparam
			, NULL, NULL
			, FALSE
			, CREATE_NO_WINDOW
			, NULL
			, NULL
			, &si, &pi)
		)
		{
		//WaitForSingleObject(pi.hProcess, INFINITE);
		//CloseHandle(pi.hProcess);
		//CloseHandle(pi.hThread);
		}
		else
			rval = 1;
#else
		rval = (int)ShellExecuteW(NULL, L"open", wname, wparam, L".", SW_SHOWNORMAL);
		rval = rval <= 32;
#endif
		
		free(wname);
		free(wparam);
	#else /* win32 non-unicode */
#if 0
		if (CreateProcessA(
			name, x
			, NULL, NULL
			, FALSE
			, CREATE_NO_WINDOW
			, NULL
			, NULL
			, &si, &pi)
		)
		{
		//WaitForSingleObject(pi.hProcess, INFINITE);
		//CloseHandle(pi.hProcess);
		//CloseHandle(pi.hThread);
		}
		else
			rval = 1;
#else
		rval = (int)ShellExecuteA(NULL, "open", name, param, ".", SW_SHOWNORMAL);
		rval = rval <= 32;
#endif
	#endif
	return rval;//rval <= 32;
#else /* not win32 unicode */
	char *x = wow_malloc_die(strlen(name) + strlen(param) + 128);
	if (!x)
		return -1;
	strcpy(x, "\"");
	strcat(x, name);
	strcat(x, "\" ");
	strcat(x, param);
	int rval = system(x);
	free(x);
	return rval;
#endif
}


/* window icon */
WOW_API_PREFIX
void
wow_windowicon(int iconId)
{
#ifdef _WIN32
	HWND win = GetActiveWindow();
	if( win )
	{
		SendMessage(
			win
			, WM_SETICON
			, ICON_BIG
			, (LPARAM)LoadImage(
					GetModuleHandle(NULL)
					, MAKEINTRESOURCE(iconId)
					, IMAGE_ICON
					, 32//GetSystemMetrics(SM_CXSMICON)
					, 32//GetSystemMetrics(SM_CXSMICON)
					, 0
				)
		);
		SendMessage(
			win
			, WM_SETICON
			, ICON_SMALL
			, (LPARAM)LoadImage(
					GetModuleHandle(NULL)
					, MAKEINTRESOURCE(iconId)
					, IMAGE_ICON
					, 16//GetSystemMetrics(SM_CXSMICON)
					, 16//GetSystemMetrics(SM_CXSMICON)
					, 0
				)
		);
	}
#elif defined(__linux__)
	/* TODO */
#endif
}

#endif /* WOW_IMPLEMENTATION */

#ifdef WOW_OVERLOAD_FILE
 #define  fopen       wow_fopen
 #define  fread       wow_fread
 #define  fwrite      wow_fwrite
 #define  remove      wow_remove
#endif

#ifdef WOW_OVERLOAD_ALLOCATORS
 #define  malloc      wow_malloc_die
 #define  calloc      wow_calloc_die
 #define  realloc     wow_realloc_die
 #define  free        wow_free
#endif

#define  wow_fclose  fclose

#endif /* WOW_H_INCLUDED */


