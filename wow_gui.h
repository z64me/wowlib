/*
 * wow_gui.h
 * 
 * an easy immediate-mode GUI library
 * 
 * z64me <z64.me>
 * 
 * in one compilation unit, #define WOW_GUI_IMPLEMENTATION before
 * you #include this file; you can then #include this file
 * elsewhere without the WOW_GUI_IMPLEMENTATION, and the linker
 * will take care of the rest
 * 
 */

//#define WOW_GUI_DO_NOT_RESET_KEYBOARD_BUFFER /* threaded events */

#ifndef INCLUDE_WOW_GUI_H
#define INCLUDE_WOW_GUI_H

#include "wow.h"

#ifdef WOW_GUI_USE_PTHREAD
#	include <pthread.h>
#endif

/* fall back to default binding */
#ifndef WOW_GUI_BINDING
	#define WOW_GUI_BINDING_MINIFB
#endif

#ifndef WOW_GUI_API_PREFIX
#	define WOW_GUI_API_PREFIX
#endif

#ifndef WOWGUI_DROPDOWN_COLOR
  #define  WOWGUI_DROPDOWN_COLOR 0x000000FF
#endif

#ifndef WOWGUI_FILE_DRAG_COLOR
  #define  WOWGUI_FILE_DRAG_COLOR   0x00FF00FF
#endif

#include <stdio.h>  /* fprintf debugging */
#include <stdlib.h> /* malloc and free */
#include <string.h> /* memset, strcpy, strcat, strlen */
#include <assert.h> /* assert */
#include <stdint.h>


enum wowGui_popupChoice
{
	WOWGUI_POPUP_X = 0b0000 // used internally for user clicking X; if the user clicks X, CANCEL, NO, or YES will be returned based on which buttons were available, prioritized in that order
	, WOWGUI_POPUP_OK = 0b0001 // OK and YES are used interchangeably;
	, WOWGUI_POPUP_YES = 0b0001 // if a NO button is present, OK/YES = "Yes"
	, WOWGUI_POPUP_NO = 0b0010 // otherwise OK/YES = "OK"
	, WOWGUI_POPUP_CANCEL = 0b0100
};


enum wowGui_popupIcon
{
	WOWGUI_POPUP_ICON_ERR
	, WOWGUI_POPUP_ICON_WARN
	, WOWGUI_POPUP_ICON_INFO
	, WOWGUI_POPUP_ICON_QUESTION
};

struct wowGui_fileDropper
{
	const char *extension;
	const char *label;
	char *filename;
	int labelWidth;
	int filenameWidth;
	char isOptional;
	char isCreateMode; /* set = 1 if user is to select output file */
};

/* TODO _active equivalents of each function, which can be
        provided a 0 to indicate an unselectable label */

/* warning: must be at least 32 bits long, change if needed */
typedef unsigned int wowGui_u32_t;


/* a rectangle structure */
struct wowGui_rect
{
	int x;
	int y;
	int w;
	int h;
};


enum wowGui_blit_blend
{
	WOWGUI_BLIT_NOBLEND       = 0
	, WOWGUI_BLIT_ALPHATEST   = 1 << 1
	, WOWGUI_BLIT_ALPHABLEND  = 1 << 2
	, WOWGUI_BLIT_COLORTEST   = 1 << 3
};


enum wowGui_keyboard_control
{
	WOWGUI_KBC_CONTROL = 1
	, WOWGUI_KBC_BACKSPACE
	, WOWGUI_KBC_LEFT
	, WOWGUI_KBC_RIGHT
	, WOWGUI_KBC_UP
	, WOWGUI_KBC_DOWN
	, WOWGUI_KBC_DELETE
	, WOWGUI_KBC_CTRL
	, WOWGUI_KBC_SHIFT
	, WOWGUI_KBC_ALT
	, WOWGUI_KBC_HOME
	, WOWGUI_KBC_END
	, WOWGUI_KBC_ENTER
	, WOWGUI_KBC_ESCAPE
	, WOWGUI_KBC_TAB
};

enum wowGui_click_type {
	/* 0 means no click */
	WOWGUI_CLICKTYPE_CLICK = 1
	, WOWGUI_CLICKTYPE_HOLD = 2
};

enum wowGui_mouseclick {
	/* 0 means no mouse click */
	WOWGUI_MOUSEBUTTON_LEFT = 1
	, WOWGUI_MOUSEBUTTON_MIDDLE
	, WOWGUI_MOUSEBUTTON_RIGHT
};


/* image, used for drawing to the screen */
struct wowGui_image
{
	/* function pointers */
	/* color callback for setting color */
	void (*color)(
		struct wowGui_image *
		, wowGui_u32_t
	);
	/* destructor */
	void (*free)(struct wowGui_image *img);
	/* draw */
	void (*draw)(
		struct wowGui_image  *img
		, struct wowGui_rect *src
		, struct wowGui_rect *dst
	);
	
	/* next in linked list */
	struct wowGui_image *next;
	
	/* user data */
	void *udata;
};


/* render target (framebuffer), to limit excessive redrawing */
struct wowGui_target
{
	/* function pointers */
	
	/* destructor */
	void (*free)(struct wowGui_target *);
	
	/* draw contents on screen at specified coordinates */
	void (*draw)(
		struct wowGui_target *target
		, int x
		, int y
	);
	
	/* clears contents of framebuffer before drawing */
	void (*clear)(struct wowGui_target *);
	
	/* bind : drawing operations will alter the specified target;   *
	 *        0 means draw to screen; please do not do any clearing *
	 *        inside this callback, as bind() gets invoked multiple *
	 *        times for some windows when layering is involved      */
	void (*bind)(struct wowGui_target *);
	
	/* resize */
	void (*resize)(
		struct wowGui_target *target
		, int w
		, int h
	);
	
	/* next in linked list */
	struct wowGui_target *next;
	
	/* user data */
	void *udata;
	
	struct {
		int w;
		int h;
	}
		dim        /* current dimensions */
		, allocd   /* allocated dimensions; the idea is you need
		              only do a real resize when allocd is exceeded;
		              otherwise, just update dim on resize() */
	;
};

/* a window settings structure;
   always declare as static, as the
   wowGui functions expect the data to
   be persistent
*/
/* TODO an eventual cool feature would be window title bars,
        which can be used to move windows around; they would
        still be draw-order independent by each containing
        a linked list of occluders, which will be windows that
        are "above" the window, based on a z-index;
        wowGui_frame_start() will clear the main occluder list and
        as each window is drawn, occluders are added, etc;
        this would also include window resizing, things like,
        can_move and can_resize; this isn't a necessary feature,
        just something to add for fun eventually */
/* TODO window_push() and window_pop() instead of the current
        backup()/restore() would make the above cleaner */
struct wowGui_window
{
	struct wowGui_rect rect;
	wowGui_u32_t color;
	int refresh;     /* mostly for internal use, but if you wish     *
	                  * to force a redraw of the window's contents,  *
	                  * set refresh = 1 before calling window();     *
	                  * refresh gets reset to 0 each time window()   *
	                  * is invoked; you will generally only use      *
	                  * this for UI elements that constantly change, *
	                  * setting refresh = 1 when value(s) change     */
	
	/* internal use only */
	struct wowGui_target *target;
	struct {
		int x;
		int y;
	}
		scroll   /* window scrolling info */
		, farVec /* tiles farthest from 0 drawn, for scroll clamp */
		, cursor /* text cursor position; tracks where text drawn */
	;
	int has_init;        /* 0 if window hasn't been initialized yet */
	int dropdown_index;  /* counter; says how many times dropdown() *
	                      * has been invocated since window()       */
	int is_up_to_date;   /* 1 if target is in use and no redraw     *
	                      * conditions are met; will skip redraw in *
	                      * favor of displaying target's contents   */
	int mouse_in_window;
	int was_occluded_dd; /* 1 if it was occluded in some way by a   *
	                      * dropdown popup when drawn               */
	
	int not_scrollable;  /* 0 if window can be scrolled inside of   */
	int scroll_valuebox; /* 1 if wheel inside value box changes its *
	                      * value; applies to dropdown and ranges   */
};

/* used for keeping track of keys */
/* each is 1 bit; the flow should go like this:
    > clear struct to 0
    > test for keys pressed
    > do UI stuff
*/
#define NEWKEY(name) name:1
struct wowGui_key {
	unsigned
		NEWKEY(up)
		, NEWKEY(down)
		, NEWKEY(left)
		, NEWKEY(right)
		, NEWKEY(zero)
		, NEWKEY(enter)
		, NEWKEY(escape)
		, NEWKEY(has_pressed)  /* set = 1 when a key has been pressed */
	;
};
#undef NEWKEY


/* function prototypes */

WOW_GUI_API_PREFIX void wowGui_editcursor(const char *c, int x, int y);
WOW_GUI_API_PREFIX void wowGui_frame_end(unsigned long ms);
WOW_GUI_API_PREFIX void wowGui_quit(void);
WOW_GUI_API_PREFIX struct wowGui_key *wowGui_key(void);
WOW_GUI_API_PREFIX void wowGui_columns(int x);
WOW_GUI_API_PREFIX void wowGui_row_height(int h);
WOW_GUI_API_PREFIX void wowGui_row(void);
WOW_GUI_API_PREFIX void wowGui_seek_top(void);
WOW_GUI_API_PREFIX void wowGui_column_width(int w);
WOW_GUI_API_PREFIX void wowGui_mouse_move(int x, int y);
WOW_GUI_API_PREFIX void wowGui_file_drag(int x, int y);
WOW_GUI_API_PREFIX void wowGui_file_drop(char *file_list, int x, int y);
WOW_GUI_API_PREFIX int wowGui_dropped_file_last(void);
WOW_GUI_API_PREFIX char *wowGui_has_extension(char *str);
WOW_GUI_API_PREFIX int wowGui_is_extension(char *str, char *ext);
WOW_GUI_API_PREFIX int wowGui_force_extension(char **str, char *ext);
WOW_GUI_API_PREFIX int wowGui_change_extension(char **str, char *ext);
WOW_GUI_API_PREFIX char *wowGui_dropped_file_name(void);
WOW_GUI_API_PREFIX void wowGui_dropped_file_flush(void);
WOW_GUI_API_PREFIX void wowGui_mouse_click(
	int x
	, int y
	, enum wowGui_mouseclick button
);
WOW_GUI_API_PREFIX void wowGui_mouse_scroll_x(int i);
WOW_GUI_API_PREFIX void wowGui_mouse_scroll_y(int i);
WOW_GUI_API_PREFIX void wowGui_frame(void);
WOW_GUI_API_PREFIX void wowGui_padding(int x, int y);
WOW_GUI_API_PREFIX const char *wowGui_init(
	struct wowGui_image *image_new(unsigned char *, int w, int h)
	, void (*fill_rect)(struct wowGui_rect *rect, wowGui_u32_t color)
	, struct wowGui_target *target_new(int w, int h)
	, int textbuf_sz
	, int keybuf_sz
);
WOW_GUI_API_PREFIX void wowGui_window_end(void);
WOW_GUI_API_PREFIX int wowGui_window(struct wowGui_window *win);
WOW_GUI_API_PREFIX void wowGui_label(const char *text);
WOW_GUI_API_PREFIX enum wowGui_click_type wowGui_generic_clickable_rect(struct wowGui_rect *rect);
WOW_GUI_API_PREFIX int wowGui_clickable(char const *text);
WOW_GUI_API_PREFIX int wowGui_button(const char *text);
WOW_GUI_API_PREFIX int wowGui_checkbox(const char *text, int *on);
WOW_GUI_API_PREFIX void wowGui_list_start(void);
WOW_GUI_API_PREFIX void wowGui_list_end(void);
WOW_GUI_API_PREFIX int wowGui_list(const char *text, int *selected);
WOW_GUI_API_PREFIX void wowGui_radio_start(void);
WOW_GUI_API_PREFIX void wowGui_radio_end(void);
WOW_GUI_API_PREFIX int wowGui_radio(const char *text, int *selected);
WOW_GUI_API_PREFIX void wowGui_dropdown_end(void);
WOW_GUI_API_PREFIX void wowGui_dropdown_item(char *text);
WOW_GUI_API_PREFIX int wowGui_scrolled(void);
WOW_GUI_API_PREFIX int wowGui_rightdrag_y(void);
WOW_GUI_API_PREFIX int wowGui_rightdrag_delta_y(void);
WOW_GUI_API_PREFIX int wowGui_dropdown(
	char **text
	, int *selection
	, int w
	, int h
);
WOW_GUI_API_PREFIX int wowGui_dropdown_buf(
	char *text
	, int text_sz
	, int *selection
	, int w
	, int h
);
WOW_GUI_API_PREFIX void wowGui_viewport(int x, int y, int w, int h);
WOW_GUI_API_PREFIX void wowGui_textbox(char *buf, int buf_sz);
WOW_GUI_API_PREFIX void wowGui_textbox_only(
	char *buf
	, int buf_sz
	, char const *allowed
);
WOW_GUI_API_PREFIX int wowGui_int(int *i);
WOW_GUI_API_PREFIX int wowGui_int_range(
	int *i
	, const int min
	, const int max
	, int increment
);
WOW_GUI_API_PREFIX unsigned int wowGui_hex(unsigned int *i);
WOW_GUI_API_PREFIX unsigned int wowGui_hex_range(
	unsigned int *i
	, const unsigned int min
	, const unsigned int max
	, int increment
);
WOW_GUI_API_PREFIX int wowGui_float(float *i);
WOW_GUI_API_PREFIX float wowGui_float_range(
	float *i
	, const float min
	, const float max
	, float increment
);
WOW_GUI_API_PREFIX void wowGui_keyboard(char *pressed, int num_bytes);
WOW_GUI_API_PREFIX void wowGui_keyboard_control(
	enum wowGui_keyboard_control keycode
);
WOW_GUI_API_PREFIX void wowGui_tails(int tails);
WOW_GUI_API_PREFIX void wowGui_italic(int italic_factor);
WOW_GUI_API_PREFIX void wowGui_wait_func(
	void *func(void *progress_float)
	, void die(const char *str)
	, void bind_events(void)
	, void bind_result(void)
	, wowGui_u32_t bind_ms(void)
	, struct wowGui_rect *progress_bar
);
WOW_GUI_API_PREFIX int wowGui_popup(
	enum wowGui_popupIcon icon
	, enum wowGui_popupChoice buttons
	, const int initial
	, const char *title
	, const char *message
);
WOW_GUI_API_PREFIX int wowGui_popupf(
	enum wowGui_popupIcon icon
	, enum wowGui_popupChoice buttons
	, const int initial
	, const char *title
	, const char *fmt
	, ...
) __attribute__ ((format (printf, 5, 6)));
WOW_GUI_API_PREFIX int wowGui_fileDropper_filenameIsEmpty(
	const struct wowGui_fileDropper *v
);
WOW_GUI_API_PREFIX char * wowGui_askFilename(
	const char *ext
	, const char *path
	, const char isCreateMode
);
WOW_GUI_API_PREFIX int wowGui_fileDropper(
	struct wowGui_fileDropper *dropper
);
WOW_GUI_API_PREFIX void wowGui_bind_result(void);
WOW_GUI_API_PREFIX void wowGui_bind_clear(uint32_t rgba);
WOW_GUI_API_PREFIX wowGui_u32_t wowGui_bind_ms(void);
WOW_GUI_API_PREFIX void wowGui_delay_ms(unsigned int ms);
WOW_GUI_API_PREFIX void wowGui_bind_events(void);
WOW_GUI_API_PREFIX int wowGui_bind_should_redraw(void);
WOW_GUI_API_PREFIX void wowGui_bind_quit(void);
WOW_GUI_API_PREFIX int wowGui_bind_endmainloop(void);
WOW_GUI_API_PREFIX void wowGui_bind_resetmainloop(void);
WOW_GUI_API_PREFIX void wowGui_bind_blit_raw_scaled(
	void *raw
	, int x
	, int y
	, int w
	, int h
	, enum wowGui_blit_blend flags
	, int scale
);
WOW_GUI_API_PREFIX void wowGui_bind_blit_raw(
	void *raw
	, int x
	, int y
	, int w
	, int h
	, enum wowGui_blit_blend flags
);
WOW_GUI_API_PREFIX void wowGui_bind_set_fps(int fps);
WOW_GUI_API_PREFIX void wowGui_redrawIncrement(void);
WOW_GUI_API_PREFIX void wowGui_bind_init(char *title, int w, int h);


#endif /* INCLUDE_WOW_GUI_H */

#ifdef WOW_GUI_IMPLEMENTATION
#undef WOW_GUI_IMPLEMENTATION

#ifdef WOW_GUI_EXTERN_NATIVEFILEDIALOG
	#include <nfd.h>
#else
	#include "nativefiledialog/src/nfd_common.c"
	#ifdef _WIN32
		#include "nativefiledialog/src/nfd_win_xp.c"
	#elif defined(__linux__)
		#include "nativefiledialog/src/nfd_gtk.c"
	#endif
#endif

/* FIXME TODO it functions fine without g_ddwin_open, aside from,
 * if you edit a value box wowGui_int_range() by clicking and editing,
 * then change it by scrolling on mouseover, it flickers black; this
 * hack fixes it */
static int g_ddwin_open = 0;
static char *g_dropdown_first = 0;
static char *g_dropdown_last = 0;

static void __wowGui_unused(void);

/* test if the destination rect overlaps with active drop-down */
#define DROPDOWN_OVERLAP(RECT) \
( \
	wowGui.dropdown.parent \
	&& wowGui.win != &wowGui.dropdown.win \
	&& rects_overlap(RECT, &wowGui.dropdown.win.rect) \
)

static
void *
wowGui_malloc_die(unsigned int size)
{
	void *out = malloc(size);
	if (!out)
	{
		fprintf(stderr, "memory error\n");
		exit(EXIT_FAILURE);
	}
	return(out);
}


/* get default 2048x8 debug font */
static void *debugfont(int *surf_w, int *surf_h, int *char_w, int *char_h);
static void *font_crisp(int *surf_w, int *surf_h, int *char_w, int *char_h);

#ifdef WOW_GUI_USE_FONT_EGHBIN
	#define FONT_INIT debugfont
	#define FONT_DRAW debugfont_draw
#else /* default */
	#define FONT_INIT font_crisp
	#define FONT_DRAW font_crisp_draw
#endif
	
struct wowGui_mouse
{
	int   x;
	int   y;
	struct {
		int   x;      /* coordinates of mouse at time of click */
		int   y;
		enum wowGui_mouseclick   button; /* left/right/middle click */
		struct {
			int  x;
			int  y;
			int  frames;
			enum wowGui_mouseclick button;
		}
			down     /* unchanging start/end coordinates of click */
			, up
		;
	} click;
	struct {
		int x;
		int y;
	} lastclick;
	struct {
		int   x;
		int   y;
	} scroll;
	int in_window;
	int is_zero;
	int has_moved;
	int has_clicked;
	int has_scrolled;
};

static struct wowGui_ctx
{
	struct wowGui_window *win;
	
	struct wowGui_mouse mouse;
	
	struct
	{
		int   x;
		int   y;
	}
		scrollSpeed
		, advance
		, padding    /* offset of text from upper left of window */
		, last_label /* coordinates of last label drawn */
		, scrolldim  /* dimensions of scroll bar */
		, advance_cap
	;
	
	struct
	{
		int index;
	} list;
	
	struct
	{
		int x;
		int y;
	}
		target
		, file_drag  /* coordinates of cursor when dragging file *
		              * (negative if not dragging file)          */
		, file_drop  /* coordinates of cursor when dropped file  */
	;
	char *file_drop_name;  /* non-zero if file has been dropped */
	
	struct
	{
		struct wowGui_window  win;
		struct wowGui_window *parent;
		int index;
		int *selection;      /* index of selected item in dropdown     */
		int select_counter;  /* counts each wowGui_dropdown_item()     */
		char **text;         /* text ptr to update with chosen item    */
		int draw_only_once;  /* 1 disables all drawing functions       */
		int text_sz;         /* 0 if we update text ptr, non-zero if   *
		                      * we update data pointed to by text ptr  */
		int is_textbox;      /* 1 if it's a text edit box              */
		char *editcursor;    /* editcursor points to text being edited *
		                      * when dropdown is a textbox             */
		const
		char *only_list;     /* allow only the characters in the zero- *
		                      * terminated list, or all if 0           */
	} dropdown;
	
	struct wowGui_key key;
	
	/* pointers to user-defined functions */
	/* wowGui_image constructor */
	struct wowGui_image *(*image_new)(unsigned char *raw, int w, int h);
	/* wowGui_target constructor */
	struct wowGui_target *(*target_new)(int w, int h);
	/* shapes */
	void (*fill_rect)(struct wowGui_rect *rect, wowGui_u32_t color);
	
	/* default font */
	struct wowGui_image *font;
	
	/* column processing */
	int columns;
	int columns_counter;
	
	/* for text formatting, things like [buttons], [x]checkboxes */
	/* TODO surely we can do without this... */
	char *textbuf;
	int   textbuf_sz;
	
	/* colors associated with clickable labels */
	struct {
		wowGui_u32_t	window;
		wowGui_u32_t	mouseover;
		wowGui_u32_t	mousedown;
		wowGui_u32_t	normal;     /* makes clickables stand out */
	} color;
	
	/* scissoring rectangle */
	struct wowGui_rect scissor;
	
	/* last clickable rect */
	struct wowGui_rect last_clickable_rect;
	
	/* viewport rectangle; tracks drawable area and attempts to
	   move windows around so they fit within */
	struct wowGui_rect viewport;
	
	/* pointer to copy of wowGui structure for state backup/restore */
	struct wowGui_ctx *backup;
	
	/* force wowGui_clickable to highlight itself */
	int force_clickable_highlight;
	
	/* error string */
	const char *errstr;
	
	/* increases for each window(), decreases for each window_end();
	   is used to check if same number of calls for each are made */
	int windows_open;
	
	/* linked lists containing every allocated image/target, for
	   use during cleanup */
	struct {
		struct wowGui_image *image;
		struct wowGui_target *target;
	} allocd_list;
	
	/* font dimensions */
	int font_w;
	int font_h;
	
	/* display tail end of labels */
	int tails;
	
	/* display labels in italics */
	int italic;
	
} wowGui = {0};


/* persistent data */
static struct
{
	struct
	{
		struct wowGui_target *target;
		struct wowGui_rect rect;
		wowGui_u32_t color;
		unsigned long start_ms;
		int blink;
		int active;
		int restart;
	} editcursor;
	
	struct
	{
		char *buf;
		char *pos;
		int   sz;
		int   remaining;
	} keyboard;
	
} wowGuip = {0};


/* returns 1 if two wowGui_rect structs overlap, 0 otherwise */
static
int
rects_overlap(struct wowGui_rect *a, struct wowGui_rect *b)
{
	/* one rectangle is left of another */
	if (a->x > b->x + b->w || a->x + a->w < b->x)
		return 0;
	
	/* one rectangle is above another */
	if (a->y > b->y + b->h || a->y + a->h < b->y)
		return 0;
	
	return 1;
}


/* returns 1 if a rectangle is occluded */
static
int
rect_is_occluded(struct wowGui_rect *rect)
{
	/* TODO right now, only one occluder is allowed, and that is
	        the dropdown pop-up; eventually, there may be occluder
	        lists */
	return DROPDOWN_OVERLAP(rect);
}


/* set error string */
static
void
wowGui__error(const char *errstr)
{
	wowGui.errstr = errstr;
}


/* bind render target associated with window, if there is one */
static
void
wowGui__win_bind_target(struct wowGui_window *win)
{
	struct wowGui_target *target = win->target;
	struct wowGui_rect *rect = &win->rect;
	
	if (!target)
		return;
	
	/* attempt resize */
	target->resize(target, rect->w, rect->h);
	
	/* resize failed */
	if (!target->udata)
	{
		/* fall back to non-framebuffer method */
		win->target = 0;
		
		/* clear drawing offset */
		wowGui.target.x = 0;
		wowGui.target.y = 0;
	}
	
	/* resize was successful, so go ahead and do a bind */
	else
		target->bind(target);
}


/* backup state */
static
void
state_backup(void)
{
	memcpy(wowGui.backup, &wowGui, sizeof(wowGui));
}


/* backup state */
static
void
state_restore(void)
{
	/* back up persistent things */
	/* warning: do not try to do anything persistent with
	            wowGui.win, as it changes between states */
	struct wowGui_window ddwin = wowGui.dropdown.win;
	struct wowGui_mouse  mouse = wowGui.mouse;
	void *parent = wowGui.dropdown.parent;
	int select_counter = wowGui.dropdown.select_counter;
	int windows_open = wowGui.windows_open;
	
	/* restore non-persistent data */
	memcpy(&wowGui, wowGui.backup, sizeof(wowGui));
	
	/* overwrite overwritten persistent data */
	wowGui.dropdown.win = ddwin;
	wowGui.mouse = mouse;
	wowGui.dropdown.parent = parent;
	wowGui.dropdown.select_counter = select_counter;
	wowGui.windows_open = windows_open;
	
	/* restore render target */
	wowGui__win_bind_target(wowGui.win);
}


/* image ctor wrapper that confirms callbacks are non-zero
   when assertions are enabled */
static
struct wowGui_image *
wowGui__safe_image_new(unsigned char *pix, int w, int h)
{
	struct wowGui_image *image;
	
	/* confirm dimensions are positive and non-zero */
	assert(w >= 0 && "image dimensions must be >= 0");
	assert(h >= 0 && "image dimensions must be >= 0");
	
	/* confirm the callback is non-zero */
	assert(wowGui.image_new && "image ctor hasn't been set");
	
	image = wowGui.image_new(pix, w, h);
	
	if (!image || !image->udata)
	{
		wowGui__error("image ctor failed");
		return 0;
	}
	
	/* confirm each callback function pointer is non-zero */
	assert(image->free  && "image ctor missing free() callback");
	assert(image->draw  && "image ctor missing draw() callback");
	assert(image->color && "image ctor missing color() callback");
	
	/* link into linked list for eventual cleanup */
	if (!wowGui.allocd_list.image)
	{
		wowGui.allocd_list.image = image;
		image->next = 0;
	}
	else
	{
		image->next = wowGui.allocd_list.image->next;
		wowGui.allocd_list.image->next = image;
	}
	
	/* success */
	return image;
}


/* target ctor wrapper that confirms callbacks are non-zero
   when assertions are enabled */
static
struct wowGui_target *
wowGui__safe_target_new(int w, int h)
{
	struct wowGui_target *target;
	
	/* confirm dimensions are positive and non-zero */
	assert(w >= 0 && "image dimensions must be >= 0");
	assert(h >= 0 && "image dimensions must be >= 0");
	
	/* confirm the callback is non-zero */
	assert(wowGui.target_new && "target ctor hasn't been set");
	
	target = wowGui.target_new(w, h);
	
	/* no error checking on target nor its udata; target == 0
	   implies either failed allocation, or targets are simply
	   not used; either way, this is not a fatal error, b/c
	   wowGui falls back to direct drawing if this fails */
	/* the early return here is merely to skip assertions, which
	   can cause a crash when target == 0 */
	if (!target)
		return target;
	
	/* confirm each callback function pointer is non-zero */
	assert(target->free   && "target ctor missing free() callback");
	assert(target->draw   && "target ctor missing draw() callback");
	assert(target->bind   && "target ctor missing bind() callback");
	assert(target->clear  && "target ctor missing clear() callback");
	assert(target->resize && "target ctor missing resize() callback");
	
	/* link into linked list for eventual cleanup */
	if (!wowGui.allocd_list.target)
	{
		wowGui.allocd_list.target = target;
		target->next = 0;
	}
	else
	{
		target->next = wowGui.allocd_list.target->next;
		wowGui.allocd_list.target->next = target;
	}
	
	/* success */
	return target;
}


/* squeeze one line into another */
static
void
fit_into_1D(int *x1, int *w1, int x2, int w2)
{
	/* near edge */
	if (*x1 < x2)
		*x1 = x2;
	
	/* far edge */
	if (*x1 + *w1 > x2 + w2)
	{
		/* pushing near away far */
		*x1 = (x2 + w2) - *w1;
		/* keep inside */
		if (*x1 < x2)
			*x1 = x2;
		/* and width must fit */
		if(*w1 > w2)
			*w1 = w2;
	}
}


/* squeeze one rectangle into another */
static
void
wowGui__rect_fit_into(
	struct wowGui_rect *rect
	, struct wowGui_rect *into
)
{
	fit_into_1D(&rect->x, &rect->w, into->x, into->w);
	fit_into_1D(&rect->y, &rect->h, into->y, into->h);
}


/* clamp scrollbar based on scroll boundaries */
static
void
window_test_scroll_boundaries(struct wowGui_window *win)
{
	int t;
	t = -(win->farVec.y - win->rect.h + wowGui.padding.y * 2);
	if (win->scroll.y < t)
		win->scroll.y = t;
	t = -(win->farVec.x - win->rect.w + wowGui.padding.x * 2);
	if (win->scroll.x < t)
		win->scroll.x = t;
	
	if (win->scroll.y > 0)
		win->scroll.y = 0;
	if (win->scroll.x > 0)
		win->scroll.x = 0;
}


/* draw a tile that belongs inside a window */
static void
tile_draw(
	struct wowGui_image *img
	, struct wowGui_rect srcRect
	, struct wowGui_rect dstRect
)
{
	/* test how far from 0 this is */
	if (dstRect.x + wowGui.win->cursor.x >= wowGui.win->farVec.x)
		wowGui.win->farVec.x =
			dstRect.x
			+ wowGui.scrollSpeed.x
			+ wowGui.win->cursor.x
			+ (wowGui.font_w - wowGui.padding.x)
			/* TODO possibly add padding */
			/*+ wowGui.padding.x*/
		;
	if (dstRect.y + wowGui.win->cursor.y >= wowGui.win->farVec.y)
		wowGui.win->farVec.y =
			dstRect.y
			+ wowGui.scrollSpeed.y
			+ wowGui.win->cursor.y
			+ wowGui.font_h
		;
	
	/* transform destination to be within window, and account
	   for scroll and advancement */
	dstRect.x += 
		wowGui.win->rect.x
		+ wowGui.win->scroll.x
		+ wowGui.win->cursor.x
	;
	dstRect.y +=
		wowGui.win->rect.y
		+ wowGui.win->scroll.y
		+ wowGui.win->cursor.y
	;
	
	/* discard tiles that are outside of window */
	if (dstRect.x < wowGui.win->rect.x)
		return;
	if (dstRect.y < wowGui.win->rect.y)
		return;
	if (dstRect.x + dstRect.w > wowGui.win->rect.x + wowGui.win->rect.w)
		return;
	if (dstRect.y + dstRect.h > wowGui.win->rect.y + wowGui.win->rect.h)
		return;
	
	/* discard tiles that overlap with active dropdown menu */
	if (rect_is_occluded(&dstRect))
		return;
	
	/* apply render target offset */
	dstRect.x -= wowGui.target.x;
	dstRect.y -= wowGui.target.y;
	
	/* draw the text */
	wowGui.font->draw(wowGui.font, &srcRect, &dstRect);
}


WOW_GUI_API_PREFIX
void
wowGui_editcursor(const char *c, int x, int y)
{
	if (c != wowGui.dropdown.editcursor)
		return;
	
	struct wowGui_rect r;
	x +=
		wowGui.win->cursor.x
		+ wowGui.win->scroll.x
	;
	y +=
		wowGui.win->cursor.y
		+ wowGui.win->scroll.y
	;
	r.x = x - 2;
	r.y = y;
	r.w = 4;
	r.h = wowGui.advance.y;
	
	/* no-FB mode, so use absolute positioning */
	if (!wowGuip.editcursor.target)
	{
		if (r.x < -2 || r.y + r.h < -2)
			return;
		if (r.x >= wowGui.advance.x)
			return;
		r.x += wowGui.win->rect.x;
		r.y += wowGui.win->rect.y;
	}
	
	/* FB mode; defer the drawing to elsewhere */
	wowGui.fill_rect(&r, wowGuip.editcursor.color);
	wowGuip.editcursor.rect = r;
}


/* end frame */
WOW_GUI_API_PREFIX
void
wowGui_frame_end(unsigned long ms)
{
	const int speed = 1024;
	int new_editcursor_blink;
	
	/* restart blinking (on cursor move & textbox open) */
	if (wowGuip.editcursor.restart)
	{
		wowGuip.editcursor.start_ms = ms;
		wowGuip.editcursor.restart = 0;
	}
	
	/* make current ms relative to start ms */
	ms -= wowGuip.editcursor.start_ms;
	
	/* wrap; blink state changes once per 512 ms */
	ms &= (speed - 1);
	
	/* on = (ms >= 1024) */
	new_editcursor_blink = ms & (speed >> 1);
	
	/* value changed */
	if (new_editcursor_blink != wowGuip.editcursor.blink)
	{
		wowGuip.editcursor.blink = new_editcursor_blink;
		wowGuip.editcursor.color =
			(new_editcursor_blink)
			? 0x000000FF
			: 0xFFFFFFFF
		;
		
		struct wowGui_target *target = wowGuip.editcursor.target;
		if (target)
		{
			target->bind(target);
			//int on = new_editcursor_blink;
	
			wowGui.fill_rect(
				&wowGuip.editcursor.rect
				, wowGuip.editcursor.color
			);
			target->bind(0);
		}
	}
		
	/* reset keyboard input buffer after using it */
	/* NOTE/FIXME: this fixes minifb textboxes, be wary of it breaking sdl2 */
	wowGuip.keyboard.buf[0] = '\0';
	
	/* TODO the following has been moved from wow_frame() to fix *
	 *      issues with minifb.h; beware it may break sdl2.h     */
	
	/* clear key settings */
	memset(&wowGui.key, 0, sizeof(wowGui.key));
	
	/* clear mouse scroll */
	wowGui.mouse.scroll.x = 0;
	wowGui.mouse.scroll.y = 0;
	wowGui.mouse.is_zero  = 0;
	
	/* clear mouse click */
	wowGui.mouse.click.up.frames = 0;
	
	/* clear mouse booleans */
	wowGui.mouse.has_moved = 0;
	wowGui.mouse.has_clicked = 0;
	wowGui.mouse.has_scrolled = 0;
	
#ifndef WOW_GUI_DO_NOT_RESET_KEYBOARD_BUFFER
	/* keyboard buffer stuff */
	wowGuip.keyboard.pos = wowGuip.keyboard.buf;
	wowGuip.keyboard.remaining = wowGuip.keyboard.sz;
	wowGuip.keyboard.buf[0] = '\0';
#endif
	/* </TODO> end moved block */
}


/* debug font drawing */
static void
debugfont_draw(
	const char *str
	, int *w
	, int *h
)
{
	int x = 0;
	int y = 0;
	int fonttype = 0;
	int newline_adv = 8; /* advance 8 pixels on newline */
	int ox = x;
	int oy = y;
	
	/* set dimensions = 0 */
	if (w)
		*w = 0;
	if (h)
		*h = 0;
	
	/* EUC-JP test string */
	/*"\x8d\x8e\xb2\x8e\xcf\x20\x8e\xc2"
	"\x8e\xb8\x8e\xaf\x8e\xc3\x8e\xcf"
	"\x8e\xbd\x00\x00"*/
	for (int a=0; str[a]; ++a)
	{
		unsigned char c = str[a];
		int cy;
		if( (c==0x8C || c==0x8D) && ( !a || (a && ((unsigned char)str[a-1])!=0x8E) ) )
		{
			fonttype = (c==0x8D);
			continue;
		}
		switch(c) {
			case 0x0A: // newline
				/* advance detected line width before resetting */
				if (w && x - ox >= *w)
					*w = x - ox;
				y += newline_adv;
				x = ox;
				continue;
				break;
			case 0xFF: // do nothing
				break;
			default:   // text
				/* Japanese character set text; hiragana & katakana */
				if(c==0x8E) {
					a++;
					c = str[a];
					/* hiragana = add 0x20 if character >=0xC0;
					   otherwise, subtract 0x20 */
					c += ((c>=0xC0)?0x20:-0x20)*fonttype;
				}
				cy = c * 8;
				
				/* draw only if dimensions aren't being tested */
				if (!w)
				{
					/* exceeds column width */
					if (wowGui.advance_cap.x && x >= wowGui.advance_cap.x)
						break;
					
					tile_draw(
						wowGui.font
						, (struct wowGui_rect){0, cy, 8, 8}
						, (struct wowGui_rect){x, y, 8, 8}
					);
					wowGui_editcursor(str + a, x, y);
				}
				
				break;
		}
		x += 8;
	}
	
	/* detected draw dimensions */
	if (w && x - ox >= *w)
		*w = x - ox;
	if (h)
		*h = (y - oy) + newline_adv;
}


/* debug font drawing */
static void
font_crisp_draw(
	const char *str
	, int *w
	, int *h
)
{
	int x = 0;
	int y = 0;
	//int fonttype = 0;
	int newline_adv = 16; /* advance 8 pixels on newline */
	int ox = x;
	int oy = y;
	int end = strlen(str);
	
	/* set dimensions = 0 */
	if (w)
		*w = 0;
	if (h)
		*h = 0;
	
	for (int a = 0; a < end; ++a)
	{
		unsigned char c = str[a];
		
		/* utf8: make unknown characters display as '?' */
		if (0xf0 == (0xf8 & c)) {
			// 4-byte utf8 code point (began with 0b11110xxx)
			a += 3;
			c = '?';
		} else if (0xe0 == (0xf0 & c)) {
			// 3-byte utf8 code point (began with 0b1110xxxx)
			a += 2;
			c = '?';
		} else if (0xc0 == (0xe0 & c)) {
			// 2-byte utf8 code point (began with 0b110xxxxx)
			a += 1;
			c = '?';
		} else { // if (0x00 == (0x80 & *s)) {
			// 1-byte ascii (began with 0b0xxxxxxx)
		}
		
		switch(c) {
			case '\n': // newline
				/* advance detected line width before resetting */
				if (w && x - ox >= *w)
					*w = x - ox;
				y += newline_adv;
				x = ox;
				continue;
				break;
			case 0xFF: // do nothing
			case '\r':
				break;
			default:   // text
				/* draw only if dimensions aren't being tested */
				if (!w)
				{
					/* exceeds column width */
					if (wowGui.advance_cap.x && x >= wowGui.advance_cap.x)
						break;
					
					tile_draw(
						wowGui.font
						, (struct wowGui_rect){8 * (c - ' '), 0, 8, 16}
						, (struct wowGui_rect){x, y, 8, 16}
					);
					wowGui_editcursor(str + a, x, y);
				}
				
				break;
		}
		x += 8;
	}
	
	/* detected draw dimensions */
	if (w && x - ox >= *w)
		*w = x - ox;
	if (h)
		*h = (y - oy) + newline_adv;
}


/* blends two rgba8888 colors */
/* optimization: excludes alpha channel */
static
wowGui_u32_t
color_blend(wowGui_u32_t dst, wowGui_u32_t src)
{
	wowGui_u32_t result = 0;
	wowGui_u32_t val;
	float srcA = 0.0039f * (src & 0xFF);
#define COLOR_BLEND(shift) { \
val = (((src>>shift)&0xFF)*srcA)+(((dst>>shift)&0xFF)*(1.0f-srcA)); \
result |= val << shift; \
}
	COLOR_BLEND(24)
	COLOR_BLEND(16)
	COLOR_BLEND(8)
	result |= 0xFF;
	return result;
#undef COLOR_BLEND
}


/* returns 1 if point is inside rectangle */
static
int
point_in_rect(int x, int y, struct wowGui_rect *rect)
{
	return (
		x > rect->x
		&& y > rect->y
		&& x < rect->x + rect->w
		&& y < rect->y + rect->h
	);
}


/* test if region is clicked */
static
enum wowGui_click_type
mouse_clicked(struct wowGui_rect *rect)
{
//#define ERCOND { fprintf(stderr, "early return %d\n", __LINE__); return 0; }
#define ERCOND { return 0; }
	int current;
	int start;
		
	/* early return condition: wasn't a left-click */
	if (
		wowGui.mouse.click.up.button
		&& wowGui.mouse.click.up.button != WOWGUI_MOUSEBUTTON_LEFT
	)
		ERCOND
	
	/* early return conditions */
	if (
		(	wowGui.mouse.click.button != WOWGUI_MOUSEBUTTON_LEFT
			&& !wowGui.mouse.click.up.frames
		)
		|| !wowGui.mouse.in_window
	)
		ERCOND
	
	/* current mouse position in rect */
	current = point_in_rect(wowGui.mouse.x, wowGui.mouse.y, rect);
	if (!current)
		ERCOND
	
	/* click mouse position in rect */
	start =
		point_in_rect(
			wowGui.mouse.click.down.x
			, wowGui.mouse.click.down.y
			, rect
		)
	;
	if (!start)
		ERCOND
	
	/* unclick mouse position in rect */
	if (wowGui.mouse.click.up.frames)
	{
		int end =
			point_in_rect(
				wowGui.mouse.click.up.x
				, wowGui.mouse.click.up.y
				, rect
			)
		;
		
		/* mousedown and mouseup didn't happen inside same rectangle */
		if (!end)
			ERCOND
		
		/* the active click has been used and can now expire; disable it
		   so multiple things aren't clicked simultaneously, in case of
		   layering (popup/dropdown) */
		// TODO reverted to sdl2 code, is now working in minifb i think,
		//      leaving this note here until completely confirmed
		wowGui_mouse_click(0, 0, 0); // FIXME below fixes minifb, possibly breaks sdl2
		//wowGui_mouse_click(wowGui.mouse.click.x, wowGui.mouse.click.y, 0);
		
		/* mousedown and mouseup happened inside same rectangle */
		return WOWGUI_CLICKTYPE_CLICK;
	}
	else if (wowGui.mouse.click.button == WOWGUI_MOUSEBUTTON_LEFT)
	{
		/* if you got here, mousedown is in rect, cursor is in rect,
		   and mouse button is held, so this button is held down */
		return WOWGUI_CLICKTYPE_HOLD;
	}
	
	ERCOND
}
#undef ERCOND


/* get dimensions of label text */
static
void
label_dimensions(const char *text, int *w, int *h)
{
	FONT_DRAW(text, w, h);
	if (wowGui.advance.x > *w)
		*w = wowGui.advance.x;
	else if (*w > wowGui.advance.x)
		*w = wowGui.advance.x;
}


/* clamp rectangle to scissor coordinates */
static
void
scissor_rect(struct wowGui_rect *rect, struct wowGui_rect *scissor)
{
	if (rect->x < scissor->x)
	{
		rect->w -= scissor->x - rect->x;
		rect->x  = scissor->x;
	}
	if (rect->y < scissor->y)
	{
		rect->h -= scissor->y - rect->y;
		rect->y  = scissor->y;
	}
	if (rect->x + rect->w > scissor->x + scissor->w)
		rect->w = (scissor->x + scissor->w) - rect->x;
	if (rect->y + rect->h > scissor->y + scissor->h)
		rect->h = (scissor->y + scissor->h) - rect->y;
}


/* applies render target offset when filling rect */
static
void
fill_rect_offset(struct wowGui_rect *rect, wowGui_u32_t color)
{
	struct wowGui_rect r = *rect;
	
	/* apply render target offset */
	r.x -= wowGui.target.x;
	r.y -= wowGui.target.y;
	
	wowGui.fill_rect(&r, color);
}


/* fill a rectangle, but only what's inside the scissor rectangle */
static
void
fill_rect_scissor(
	struct wowGui_rect *rect
	, struct wowGui_rect *scissor
	, wowGui_u32_t color
)
{
	struct wowGui_rect rect_copy = *rect;
	
	scissor_rect(&rect_copy, scissor);
	
	fill_rect_offset(&rect_copy, color);
}


/* fill a rectangle, except for a subrectangle */
static
void
fill_rect_except(
	struct wowGui_rect *rect
	, struct wowGui_rect *except
	, wowGui_u32_t color
)
{
	struct wowGui_rect scissor;
	
	/* draw everything left of except */
	scissor.x = 0;
	scissor.y = 0;
	scissor.w = except->x;
	scissor.h = rect->y + rect->h;
	fill_rect_scissor(rect, &scissor, color);
	
	/* draw everything right of except */
	scissor.x = except->x + except->w;
	scissor.y = 0;
	scissor.w = rect->w;
	scissor.h = rect->y + rect->h;
	fill_rect_scissor(rect, &scissor, color);
	
	/* draw everything above except */
	scissor.x = except->x;
	scissor.y = 0;
	scissor.w = except->w;
	scissor.h = except->y;
	fill_rect_scissor(rect, &scissor, color);
	
	/* draw everything below except */
	scissor.x = except->x;
	scissor.y = except->y + except->h;
	scissor.w = except->w;
	scissor.h = rect->y;
	fill_rect_scissor(rect, &scissor, color);
}


/* test if mouse is over a rect before drawing */
static
int
mouse_on_rect(struct wowGui_rect *rect)
{
	//struct wowGui_rect winrect = wowGui.scissor;
	
	/* return if mouse is in rectangle */
	return point_in_rect(wowGui.mouse.x, wowGui.mouse.y, rect);
}


/* test if mousedown happened on a rect */
static
int
mousedown_on_rect(struct wowGui_rect *rect)
{
	//struct wowGui_rect winrect = wowGui.scissor;
	
	/* return if mouse is in rectangle */
	return point_in_rect(wowGui.mouse.click.x, wowGui.mouse.click.y, rect);
}


/* convert label to rect dimensions */
static
void
label_to_rect(const char *text, struct wowGui_rect *result)
{
	struct wowGui_rect rect = {0};
	struct wowGui_rect winrect = wowGui.scissor;
	
	/* zero-initialize dimensions */
	result->w = 0;
	result->h = 0;
	
	/* grab dimensions of would-be label */
	label_dimensions(text, &rect.w, &rect.h);
	
	/* derive location of rectangle in global window space */
	rect.x =
		winrect.x
		+ wowGui.win->scroll.x
		+ wowGui.win->cursor.x
	;
	rect.y =
		winrect.y
		+ wowGui.win->scroll.y
		+ wowGui.win->cursor.y
	;
	
	/* early return if this is outside of window space */
	if (!rects_overlap(&rect, &winrect))
		return;
	
	/* discard rectangles that overlap with active dropdown menu */
	if (rect_is_occluded(&rect))
		return;
	
	/* clamp edges to inside of window */
	scissor_rect(&rect, &winrect);
	
	/* last tests */
	if (rect.w <= 0 || rect.h <= 0)
		return;
	
#if 0 /* enabling this highlights clickable regions only if mouse  *
       * is inside that subwindow; it also makes dropped files not *
       * work until after the mouse has been moved, which is bad   */
	/* skip mouse collision test if mouse isn't in window */
	if (!wowGui.mouse.in_window)
		return;
#endif
	
	*result = rect;
}


/* min, integers */
static
int
min_int(int a, int b)
{
	return (a < b) ? a : b;
}


/* free any memory that wowGui is using */
WOW_GUI_API_PREFIX
void
wowGui_quit(void)
{
	struct wowGui_target *target;
	struct wowGui_image  *image;
	
	/* free all allocated images */
	image = wowGui.allocd_list.image;
	while (image)
	{
		void *next = image->next;
		image->free(image);
		image = next;
	}
	
	/* free all allocated targets */
	target = wowGui.allocd_list.target;
	while (target)
	{
		void *next = target->next;
		target->free(target);
		target = next;
	}
	
	if (wowGui.textbuf)
		free(wowGui.textbuf);
	if (wowGuip.keyboard.buf)
		free(wowGuip.keyboard.buf);
	if (wowGui.backup)
		free(wowGui.backup);
	memset(&wowGui, 0, sizeof(wowGui));
}


/* return pointer to global wowGui_keys structure */
WOW_GUI_API_PREFIX
struct wowGui_key *
wowGui_key(void)
{
	return &wowGui.key;
}


/* set number of columns per row; assumes start of row */
WOW_GUI_API_PREFIX
void
wowGui_columns(int x)
{
	wowGui.columns = x - 1;
	wowGui.columns_counter = 0;  /* return to start of row */
}


/* set pixels to advance after each row */
WOW_GUI_API_PREFIX
void
wowGui_row_height(int h)
{
	wowGui.advance.y = h;
}


/* force advance to next row */
WOW_GUI_API_PREFIX
void
wowGui_row(void)
{
	wowGui.win->cursor.y  += wowGui.advance.y;
	wowGui.win->cursor.x   = wowGui.padding.x;
	wowGui.columns_counter = 0;
}


/* force cursor to top row */
WOW_GUI_API_PREFIX
void
wowGui_seek_top(void)
{
	/* relative positioning */
	wowGui.win->cursor.x = wowGui.padding.x;
	wowGui.win->cursor.y = wowGui.padding.y;
}


/* set pixels to advance after each column */
WOW_GUI_API_PREFIX
void
wowGui_column_width(int w)
{
	wowGui.advance.x = w;
}


/* set mouse coordinates */
WOW_GUI_API_PREFIX
void
wowGui_mouse_move(int x, int y)
{
	wowGui.mouse.x = x;
	wowGui.mouse.y = y;
	wowGui.mouse.has_moved = 1;
}


/* set file drag coordinates */
WOW_GUI_API_PREFIX
void
wowGui_file_drag(int x, int y)
{
	wowGui_mouse_move(x, y);
	wowGui.file_drag.x = x;
	wowGui.file_drag.y = y;
}


/* set file drag coordinates */
WOW_GUI_API_PREFIX
void
wowGui_file_drop(char *file_list, int x, int y)
{
	if (wowGui.file_drop_name)
		free(wowGui.file_drop_name);
	
	wowGui.file_drop.x = x;
	wowGui.file_drop.y = y;
	wowGui.file_drop_name = strdup(file_list);
}


/* returns non-zero if file(s) dropped on most recent label */
WOW_GUI_API_PREFIX
int
wowGui_dropped_file_last(void)
{
	if (!wowGui.file_drop_name)
		return 0;
	
	if (point_in_rect(
			wowGui.file_drop.x
			, wowGui.file_drop.y
			, &wowGui.last_clickable_rect
		)
	)
	{
		wowGui.file_drop.x = -1;
		wowGui.file_drop.y = -1;
		return 1;
	}
	
	return 0;
}


/* get next filename in file list (returns 0 when there are none) */
/* this is literally mfb_get_dropped_file() */
static
char *wow__get_dropped_file(char *file_list)
{
    const char *delim = "\n\r";
    
    if (!file_list)
        return 0;
    
    /* a new file list has been provided: tokenize it and return first token */
    if (strchr(file_list, '\n'))
        return strtok(file_list, delim);
    
    /* return next token from previous file list */
    else
        return strtok(0, delim);
}

WOW_GUI_API_PREFIX
char *
wowGui_has_extension(char *str)
{
	char *slash = strrchr(str, '\\');
	
	if (!slash)
		slash = strrchr(str, '/');
	if (!slash)
		slash = str;
	else
		slash += 1;
	
	char *pd = strrchr(str, '.');
	
	/* no period, or period occurs before actual name */
	if (!pd || pd < slash)
		return 0;
	
	return pd + 1;
}

WOW_GUI_API_PREFIX
int
wowGui_is_extension(char *str, char *ext)
{
	if (!str)
		return 0;
	
	return !strcasecmp(str, ext);
}

/* forces string to end in extension (returns non-zero on memory failure) */
WOW_GUI_API_PREFIX
int
wowGui_force_extension(char **str, char *ext)
{
	int len;
	
	/* is already desirable extension */
	if (wowGui_is_extension(wowGui_has_extension(*str), ext))
		return 0;
	
	len = strlen(*str);
	*str = realloc(*str, len + strlen(ext) + 16);
	/* memory error */
	if (!*str)
		return 1;
	strcat(*str, ".");
	strcat(*str, ext);
	
	return 0;
}

/* forces string to end in extension (returns non-zero on memory failure) */
WOW_GUI_API_PREFIX
int
wowGui_change_extension(char **str, char *ext)
{
	char *has;
	int len;
	
	/* has no extension, so append one */
	has = wowGui_has_extension(*str);
	if (!has)
		return wowGui_force_extension(str, ext);
	
	/* existing extension is big enough */
	if (strlen(has) >= strlen(ext))
	{
		strcpy(has, ext);
		return 0;
	}
	
	len = strlen(*str);
	*str = realloc(*str, len + strlen(ext) + 16);
	/* memory error */
	if (!*str)
		return 1;
	*has = 0;
	strcat(*str, ".");
	strcat(*str, ext);
	
	return 0;
}


/* returns pointer to filename of dropped file */
/* call multiple times for multiple files, returns 0 when there are no more files */
WOW_GUI_API_PREFIX
char *
wowGui_dropped_file_name(void)
{
	return wow__get_dropped_file(wowGui.file_drop_name);
}


/* flushes any names queued in a file name list */
WOW_GUI_API_PREFIX
void
wowGui_dropped_file_flush(void)
{
	char *n = wowGui_dropped_file_name();
	while (n)
		n = wowGui_dropped_file_name();
	
	if (wowGui.file_drop_name)
		free(wowGui.file_drop_name);
	
	wowGui.file_drop_name = 0;
}


/* set mouse click */
WOW_GUI_API_PREFIX
void
wowGui_mouse_click(int x, int y, enum wowGui_mouseclick button)
{
	wowGui.mouse.has_clicked = 1;
	if (button)
	{
		wowGui.mouse.click.down.x = x;
		wowGui.mouse.click.down.y = y;
		wowGui.mouse.click.up.frames = 0;
		wowGui.mouse.click.up.button = 0;
	}
	else
	{
		wowGui.mouse.click.up.x = x;
		wowGui.mouse.click.up.y = y;
		wowGui.mouse.click.up.frames = 1;
		wowGui.mouse.click.up.button = wowGui.mouse.click.button;
	}
	wowGui.mouse.click.x = x;
	wowGui.mouse.click.y = y;
	wowGui.mouse.lastclick.x = x;
	wowGui.mouse.lastclick.y = y;
	wowGui.mouse.click.button = button;
}


/* set mouse scroll x value */
WOW_GUI_API_PREFIX
void
wowGui_mouse_scroll_x(int i)
{
	wowGui.mouse.scroll.x = i;
	wowGui.mouse.has_scrolled = 1;
}


/* set mouse scroll y value */
WOW_GUI_API_PREFIX
void
wowGui_mouse_scroll_y(int i)
{
	wowGui.mouse.scroll.y = i;
	wowGui.mouse.has_scrolled = 1;
}


/* initialize input structures */
WOW_GUI_API_PREFIX
void
wowGui_frame(void)
{
	/* make sure every window() is paired with a window_end() */
	assert(
		!wowGui.windows_open
		&& "window_end() called without window() or \
window() called without window_end()"
	);
}


/* set padding */
WOW_GUI_API_PREFIX
void
wowGui_padding(int x, int y)
{
	wowGui.padding.x = x;
	wowGui.padding.y = y;
}


/* initialize wowGui for use;
   image_new()   must point to a function that allocates an image
                 usable by your renderer, given rgba8888 pixel data
                 and the dimensions, complete with callbacks; should
                 return 0 on failure
   fill_rect()   must point to a function that draws a solid rectangle
   target_new()  can be 0 if you don't want to use framebuffers to
                 reduce excessive redrawing; otherwise, it must point
                 to a function that allocates a rendering target,
                 complete with callbacks; dimensions will never be 0;
                 return 0 on failure and wowGui will fall back to not
                 using framebuffers
   textbuf_sz    bytes to allocate for internal text buffer, for
                 formatting text; must be > 0
   keybuf_sz     bytes to allocate for keyboard event buffer, which is
                 used to pipe typed text into text edit boxes;
                 must be > 0
*/
WOW_GUI_API_PREFIX
const char *
wowGui_init(
	struct wowGui_image *image_new(unsigned char *, int w, int h)
	, void (*fill_rect)(struct wowGui_rect *rect, wowGui_u32_t color)
	, struct wowGui_target *target_new(int w, int h)
	, int textbuf_sz
	, int keybuf_sz
)
{
	unsigned char *raw;
	
	/* error checking */
	assert(image_new && "missing image ctor");
	assert(fill_rect && "missing fill rect");
	assert(textbuf_sz > 0 && "textbuf_sz must be > 0");
	assert(keybuf_sz > 0 && "keybuf_sz must be > 0");
	
	/* set up function pointers */
	wowGui.image_new     = image_new;
	wowGui.fill_rect     = fill_rect;
	wowGui.target_new    = target_new;
	
	/* allocate text buffer */
	wowGui.textbuf = calloc(1, textbuf_sz);
	wowGui.textbuf_sz = textbuf_sz;
	if (!wowGui.textbuf)
		return "memory allocation failure";
	
	/* allocate intermediate keyboard buffer */
	wowGuip.keyboard.buf = calloc(1, keybuf_sz);
	wowGuip.keyboard.sz = keybuf_sz;
	if (!wowGuip.keyboard.buf)
		return "memory allocation failure";
	
	/* allocate backup buffer */
	wowGui.backup = calloc(1, sizeof(*wowGui.backup));
	if (!wowGui.backup)
		return "memory allocation failure";
	
	/* initialize default debug font */
	int surf_w;
	int surf_h;
	int font_w;
	int font_h;
	raw = FONT_INIT(&surf_w, &surf_h, &font_w, &font_h);
	if (!raw)
		return "failed to initialize debug font";
	
	wowGui.font = wowGui__safe_image_new(raw, surf_w, surf_h);
	free(raw);
	if (!wowGui.font)
		return wowGui.errstr;
	
	/* TODO make these customizable eventually, perhaps? */
	wowGui.scrollSpeed.x = font_w;
	wowGui.scrollSpeed.y = font_h;
	
	/* default advancement is font dimensions */
	wowGui.advance.x = font_w;
	wowGui.advance.y = font_h;
	
	/* default padding */
	wowGui.padding.x = font_w;
	wowGui.padding.y = font_h;
	
	/* font dimensions */
	wowGui.font_w = font_w;
	wowGui.font_h = font_h;
	
	/* dimensions of scroll bar */
	wowGui.scrolldim.x = 4;
	wowGui.scrolldim.y = 4;
	
	/* success */
	return 0;
	
	(void)__wowGui_unused;
}


/* finish drawing window */
WOW_GUI_API_PREFIX
void
wowGui_window_end(void)
{
	struct wowGui_window *win;
	struct wowGui_target *target;
	
	win = wowGui.win;
	
	assert(
		win
		&& "wowGui_window_end() must be paired with wowGui_window()"
	);
	
	/* window_end() call increments this for later error checking */
	wowGui.windows_open -= 1;
	
	target = win->target;
	if (!target)
		return;
	
	/* display window target's contents onto the screen */
	target->bind(0);
	target->draw(target, wowGui.target.x, wowGui.target.y);
}


/* set window region where drawing will take place */
/* if it returns 1, display your window contents, and follow up
   with wowGui_window_end(); otherwise, skip them */
/* note: any windows you pass in should be persistent, aka no
         local variables unless declared as static, and each
         should be zero-initialized before its first use with
         this function; declaring as static guarantees this */
WOW_GUI_API_PREFIX
int
wowGui_window(struct wowGui_window *win)
{	
	struct wowGui_window *ddwin = &wowGui.dropdown.win;
	struct wowGui_rect *ddrect = &ddwin->rect;
	struct wowGui_rect  winrect;
	int mouse_was_in_window = 0;
	int force_redraw = 0;
	int mouse_inactive = 0;
	
	/* bind global window structure */
	wowGui.win = win;
	
	/* window() call increments this for later error checking */
	wowGui.windows_open += 1;
	
	/* early return conditions */
	if (
		!win
		|| win->rect.w <= 0
		|| win->rect.h <= 0
	)
		return 0;
	
	/* initialization on first encounter */
	if (!win->has_init)
	{
		win->has_init = 1;
		
		/* attempt to allocate render target */
		win->target = 0;
		if (wowGui.target_new)
		{
			struct wowGui_target *target;
			
			target = wowGui__safe_target_new(win->rect.w, win->rect.h);
			
			win->target = target;
		}
		
		/* force to draw once on initialization */
		force_redraw = __LINE__;
	}
	
	/* if window MUST be redrawn due to an external update */
	if (win->refresh)
	{
//		fprintf(stderr, "REFRESH\n");
		force_redraw = __LINE__;
		win->refresh = 0;
	}
	
	/* squeeze it into the viewport */
	wowGui__rect_fit_into(&win->rect, &wowGui.viewport);
	
	/* if we are operating with render targets, translate
	   drawing operations to map upper-left corner of window
	   to (0, 0) */
	wowGui.target.x = 0;
	wowGui.target.y = 0;
	if (win->target)
	{
		wowGui.target.x = win->rect.x;
		wowGui.target.y = win->rect.y;
	}
	
	/* make a copy to keep code concise */
	winrect = win->rect;
	
	/* relative positioning */
	win->cursor.x = wowGui.padding.x;
	win->cursor.y = wowGui.padding.y;
	
	/* if dropdown menu is active, we allow mouse operations
	   only on the dropdown menu */
	if (wowGui.dropdown.parent && win != ddwin)
	{
		/* disable mouse */
		mouse_inactive = 1;
	}
	
	/* if user left-clicks outside of dropdown window, close it */
	if (win == wowGui.dropdown.parent)
	{
		/* test left-click outside of region */
		if (
			wowGui.mouse.click.up.frames
			&& wowGui.mouse.click.up.button == WOWGUI_MOUSEBUTTON_LEFT
			&&	!point_in_rect(
				wowGui.mouse.click.down.x
				, wowGui.mouse.click.down.y
				, ddrect
			)
		)
		{
			/* close dropdown window */
			wowGui.dropdown.parent = 0;
			wowGuip.editcursor.active = 0;
			g_ddwin_open = 0;
			
			/* expire the mouse click so it doesn't fall through */
		// TODO reverted to sdl2 code, is now working in minifb i think,
		//      leaving this note here until completely confirmed
			wowGui_mouse_click(0, 0, 0); // FIXME below fixes minifb, possibly breaks sdl2
			//wowGui_mouse_click(wowGui.mouse.click.x, wowGui.mouse.click.y, 0);
		}
	}
	
	/* fixes problems with ddwin             *
	 * overriding scrolling in value boxes   *
	 * after closing a ddwin (FIXME someday) */
	if (g_ddwin_open == 0 && ddwin == win)
	{
		memset(&winrect, 0, sizeof(winrect));
	}
	
	/* test if mouse is in this window */
	wowGui.mouse.in_window = 0;
	if (point_in_rect(wowGui.mouse.x, wowGui.mouse.y, &winrect))
	{
		wowGui.mouse.in_window = 1;
		win->mouse_in_window = 1;
	}
	else if (win->mouse_in_window)
	{
		/* mouse was in window previous frame */
		mouse_was_in_window = win->mouse_in_window;
		win->mouse_in_window = 0;
	}
	
	/* force inactive mouse */
	if (mouse_inactive)
		wowGui.mouse.in_window = 0;
	
	/* handle render targets */
	wowGui__win_bind_target(win);
	
	/* bind was successful */
	if (win->target)
	{
		/* clear dropdown rect dimensions if it isn't used */
		if (!wowGui.dropdown.parent)
			memset(ddrect, 0, sizeof(*ddrect));
		
		/* this is the parent of the dropdown window, so if dropdown
		   must be redrawn, this window must also be redrawn */
		if (win == wowGui.dropdown.parent)
		{
			/* events happening inside dropdown window */
			if (
				/* if the dropdown doesn't have a framebuffer, we have
				   no choice but to redraw this window in order to
				   display it */
				ddwin->target == 0
				
				/* key presses guarantee dropdown must be redrawn,
				   whether or not the mouse is within its boundaries */
				|| wowGui.key.has_pressed
				
				/* or the following */
				||
				(
					/* mouse in dropdown window */
					point_in_rect(
						wowGui.mouse.x
						, wowGui.mouse.y
						, ddrect
					)
					
					/* and mouse has changed in some way */
					&&
					(
						wowGui.mouse.has_moved
						|| wowGui.mouse.has_clicked
						|| wowGui.mouse.has_scrolled
					)
				)
			)
			{
				force_redraw = __LINE__;
			}
		}
		
		/* if the window is the dropdown menu */
		else if (win == ddwin)
		{
			/* force redraw on every key press */
			if (wowGui.key.has_pressed)
				force_redraw = __LINE__;
		}
		
		/* if window was previously occluded by dropdown menu, which
		   is now inactive */
		if (win->was_occluded_dd && !wowGui.dropdown.parent)
		{
			force_redraw = __LINE__;
		}
		
		/* if a previous redraw condition has not been met, check
		   if mouse or key events have happened within its bounds, and
		   if they have, this is a redraw event */
		if (
			force_redraw == 0
			
			&&
			(
				/* (mouse OR mouse_prev) in window */
				(
					wowGui.mouse.in_window
					|| mouse_was_in_window
				)
				
				/* AND (mouse or keys have changed) */
				&&
				(
					wowGui.mouse.has_moved
					|| wowGui.mouse.has_clicked
					|| wowGui.key.has_pressed
					|| wowGui.mouse.has_scrolled
				)
				
				/* AND (win is dropdown, or mouse is not occluded by it) */
				&&
				(
					win == ddwin
					|| !point_in_rect(
						wowGui.mouse.x
						, wowGui.mouse.y
						, ddrect
					)
				)
			)
		)
		{
			force_redraw = __LINE__;
		}
		
		/* if redraw conditions are not met, we can just display the
		   contents already rendered onto this window's framebuffer */
		if (force_redraw == 0)
		{
			wowGui_window_end();
			
			/* because we return 0 here, the window's contents are
			   not processed, and if this window contains an active
			   dropdown, it will not be drawn, so do the drawing
			   right here */
			if (win == wowGui.dropdown.parent)
			{
				/* display window target's contents onto the screen */
				struct wowGui_target *target = ddwin->target;
				target->bind(0);
				target->draw(target, ddrect->x, ddrect->y);
			}
			
			return 0;
		}
		
		/* debugging stuff */
#if 0
		fprintf(stderr, "force_redraw = %d\n", force_redraw);
#endif
		
		/* clearing and binding are separate for a reason */
		win->target->clear(win->target);
	}
	
	/* reset dropdown counter to 0 */
	win->dropdown_index = 0;
	
	/* default scissor rectangle */
	wowGui.scissor = winrect;
	
	/* derive label highlights from window color */
	wowGui.color.window    = win->color;
	wowGui.color.mouseover = color_blend(win->color, 0xFFFFFF80);
	wowGui.color.mousedown = color_blend(win->color, 0x00000080);
	wowGui.color.normal    = color_blend(win->color, 0xFFFFFF40);
	
	/* if the dropdown menu is visible, this window is not it, and
	   this  window overlaps it, draw only the part of the window
	   that don't intersect with it */
	if (
		wowGui.dropdown.parent
		&& win != ddwin
		&& rects_overlap(ddrect, &winrect)
	)
	{
		fill_rect_except(&winrect, ddrect, win->color);
		win->was_occluded_dd = 1;
	}
	
	/* draw rect of requested color */
	else
	{
		fill_rect_offset(&winrect, win->color);
		win->was_occluded_dd = 0;
	}
	
	/* check for arrow key input and scroll the window */
	if (wowGui.mouse.in_window && win->not_scrollable == 0)
	{
		/* allow these keyboard shortcuts only if we aren't
		   doing cursor editing */
		if (wowGuip.editcursor.active == 0)
		{
			/* directional scrolling */
			if (wowGui.key.up)
				win->scroll.y += wowGui.scrollSpeed.y;
			
			if (wowGui.key.down)
				win->scroll.y -= wowGui.scrollSpeed.y;
			
			if (wowGui.key.left)
				win->scroll.x += wowGui.scrollSpeed.x;
			
			if (wowGui.key.right)
				win->scroll.x -= wowGui.scrollSpeed.x;
			
			/* reset to zero */
			if (wowGui.key.zero)
			{
				win->scroll.x = 0;
				win->scroll.y = 0;
			}
		}
		
		/* scrolling via mouse wheel */
		if (wowGui.mouse.has_scrolled)
		{
			if (wowGui.mouse.scroll.x)
				win->scroll.x += wowGui.mouse.scroll.x * wowGui.scrollSpeed.x;
			if (wowGui.mouse.scroll.y)
				win->scroll.y += wowGui.mouse.scroll.y * wowGui.scrollSpeed.y;
			wowGui.mouse.has_scrolled = 0;
		}
		
		/* scrolling via click-drag */
		if (wowGui.mouse.click.button == WOWGUI_MOUSEBUTTON_RIGHT)
		{
			win->scroll.x += wowGui.mouse.x - wowGui.mouse.click.x;
			win->scroll.y += wowGui.mouse.y - wowGui.mouse.click.y;
			wowGui.mouse.click.x = wowGui.mouse.x;
			wowGui.mouse.click.y = wowGui.mouse.y;
		}
		
		/* boundaries */
		window_test_scroll_boundaries(win);
	
		/* draw scroll bars */
		struct wowGui_rect rect;
		float f;
		/* horizontal */
		if (win->farVec.x > winrect.w)
		{
			/* derive length of bar */
			f  = winrect.w - wowGui.padding.x * 2;
			f /= win->farVec.x;
			rect.w = f * winrect.w;
			rect.h = wowGui.scrolldim.y;
			/* derive offset */
			f = -win->scroll.x;
			rect.x = winrect.x + f / (win->farVec.x) * winrect.w;
			rect.y = winrect.y + winrect.h - rect.h;
			fill_rect_offset(&rect, 0xFFFFFFFF);
			/* update scissor rectangle so scrollbar isn't drawn over */
			wowGui.scissor.h -= rect.h;
		}
		/* vertical */
		if (win->farVec.y > winrect.h)
		{
			/* derive length of bar */
			f  = winrect.h - wowGui.padding.y * 2;
			f /= win->farVec.y;
			rect.h = f * winrect.h;
			rect.w = wowGui.scrolldim.x;
			/* derive offset */
			f = -win->scroll.y;
			rect.y = winrect.y + f / (win->farVec.y) * winrect.h;
			rect.x = winrect.x + winrect.w - rect.w;
			fill_rect_offset(&rect, 0xFFFFFFFF);
			/* update scissor rectangle so scrollbar isn't drawn over */
			wowGui.scissor.w -= rect.w;
		}
	}
	
	return 1;
}


/* display text */
WOW_GUI_API_PREFIX
void
wowGui_label(const char *text)
{
	/* early return condition: don't draw if invalid window */
	if (!wowGui.win)
		return;
	
	/* back up position of label */
	wowGui.last_label.x = wowGui.win->cursor.x;
	wowGui.last_label.y = wowGui.win->cursor.y;
	
	/* draw text */
	FONT_DRAW(text, 0, 0);
	
	wowGui.columns_counter += 1;
	
	/* reached max columns per row, advance vertically */
	if (wowGui.columns_counter > wowGui.columns)
	{
		wowGui.win->cursor.y  += wowGui.advance.y;
		wowGui.win->cursor.x   = wowGui.padding.x;
		wowGui.columns_counter = 0;
	}
	/* advance horizontally to next column */
	else
	{
		wowGui.win->cursor.x += wowGui.advance.x + wowGui.padding.x;
	}
}


/* display clickable rect */
WOW_GUI_API_PREFIX
enum wowGui_click_type
wowGui_generic_clickable_rect(struct wowGui_rect *rect)
{
	int mouseover;
	wowGui_u32_t color;
	enum wowGui_click_type click;
	wowGui.last_clickable_rect = *rect;
	
	mouseover = mouse_on_rect(rect);
	click = mouse_clicked(rect);
	color = wowGui.color.normal;
	if (wowGui.force_clickable_highlight)
		color = wowGui.color.mousedown;
	else if (click == WOWGUI_CLICKTYPE_HOLD)
		color = wowGui.color.mousedown;
	else if (mouseover)
		color = wowGui.color.mouseover;
	if (point_in_rect(wowGui.file_drag.x, wowGui.file_drag.y, rect))
		color = WOWGUI_FILE_DRAG_COLOR;
	fill_rect_offset(rect, color);
	
	/* force the window to redraw on click (updates checkboxes
	   in the framebuffer */
	if (click == WOWGUI_CLICKTYPE_CLICK)
		wowGui.win->refresh = 1;
	
	return click;
}


/* display clickable text */
WOW_GUI_API_PREFIX
int
wowGui_clickable(char const *text)
{
	struct wowGui_rect rect = {0};
	enum wowGui_click_type click;
	
	/* early return condition: don't draw if invalid window */
	if (!wowGui.win)
		return 0;
	
	label_to_rect(text, &rect);
	
	click = wowGui_generic_clickable_rect(&rect);
	
	
	/* clickable label() width should be capped at column width! */
	if (wowGui.tails)
	{
		/* display tail end of string */
		/* TODO this trick works only with fixed-width fonts */
		int w;
		int h;
		FONT_DRAW(text, &w, &h);
		if (w > wowGui.advance.x)
		{
			text += (w - wowGui.advance.x) / wowGui.font_w;
		}
	}
	wowGui.advance_cap.x = wowGui.advance.x;
	wowGui_label(text);
	wowGui.advance_cap.x = 0;
	
	/* return 1 if clicked */
	return (click == WOWGUI_CLICKTYPE_CLICK);
}


/* display button */
WOW_GUI_API_PREFIX
int
wowGui_button(const char *text)
{
	char *carr = wowGui.textbuf;
	
	strcpy(carr, "[");
	strcat(carr, text);
	strcat(carr, "]");
	
	return wowGui_clickable(carr);
}


/* display checkbox (returns 1 if value changes) */
WOW_GUI_API_PREFIX
int
wowGui_checkbox(const char *text, int *on)
{
	char *carr = wowGui.textbuf;
	
	strcpy(carr, "[ ] ");
	strcat(carr, text);
	
	if (*on)
		carr[1] = 'x';
	
	if (wowGui_clickable(carr))
	{
		*on = !*on;
		return 1;
	}
	
	return 0;
}


/* initialize clickable list */
WOW_GUI_API_PREFIX
void
wowGui_list_start(void)
{
	wowGui.list.index = 0;
}


/* finish clickable list */
WOW_GUI_API_PREFIX
void
wowGui_list_end(void)
{
}


/* display clickable list item (returns 1 if value changes) */
WOW_GUI_API_PREFIX
int
wowGui_list(const char *text, int *selected)
{
	if (wowGui_clickable(text))
	{
		*selected = wowGui.list.index;
		wowGui.list.index += 1;
		return 1;
	}
	
	wowGui.list.index += 1;
	return 0;
}


/* initialize radio list */
WOW_GUI_API_PREFIX
void
wowGui_radio_start(void)
{
	wowGui_list_start();
}


/* finish radio list */
WOW_GUI_API_PREFIX
void
wowGui_radio_end(void)
{
}


/* display radio box (returns 1 if value changes) */
WOW_GUI_API_PREFIX
int
wowGui_radio(const char *text, int *selected)
{
	char *carr = wowGui.textbuf;
	
	strcpy(carr, "( ) ");
	strcat(carr, text);
	
	if (*selected == wowGui.list.index)
		carr[1] = 'x';
	
	return wowGui_list(carr, selected);
}


/* done with dropdown menu
   note: you need only call this if wowGui_dropdown() returns 1 */
/* TODO perhaps add some assertions to ensure the above */
WOW_GUI_API_PREFIX
void
wowGui_dropdown_end(void)
{
	wowGui_window_end();
	
	/* ensure selection does not exceed list max */
	if (wowGui.dropdown.selection/* && *wowGui.dropdown.selection >= wowGui.dropdown.select_counter*/)
	{
		int *selection = wowGui.dropdown.selection;
		int Mselection = wowGui.dropdown.select_counter;
	//	*wowGui.dropdown.selection = 0;//wowGui.dropdown.select_counter - 1;
		
		/* wrap back to beginning */
		if (*selection >= Mselection)
		{
			*selection = 0;
			wowGui.dropdown.index = 0;
			*wowGui.dropdown.text = g_dropdown_first;
		}
		if (*selection < 0)
		{
			*selection = Mselection - 1;
			wowGui.dropdown.index = Mselection - 1;
			*wowGui.dropdown.text = g_dropdown_last;
		}
	}
	
	state_restore();
	
	/* if we are only drawing it once, terminate it now */
	if (wowGui.dropdown.draw_only_once)
	{
		/* zztexview r1 bug: clicking a drop-down menu, then clicking
		 * an edit box, makes edit boxes break */
//		fprintf(stderr, "draw_only_once\n");
		wowGui.dropdown.draw_only_once = 0;
		wowGui.dropdown.parent = 0;
		wowGuip.editcursor.active = 0;
		g_ddwin_open = 0;
	}
	
	/* reset to 0 after each dropdown */
	wowGui.dropdown.text_sz = 0;
}


/* item in a dropdown menu */
WOW_GUI_API_PREFIX
void
wowGui_dropdown_item(char *text)
{
	int clicked;
	int is_selected;
	char *dest = *wowGui.dropdown.text;
	int dest_sz = wowGui.dropdown.text_sz;
	int overwrite_dest = 0;
	if (!g_dropdown_first)
		g_dropdown_first = text;
	g_dropdown_last = text;
	
	/* is the current item the one that is selected? */
	is_selected =
	wowGui.dropdown.select_counter == *wowGui.dropdown.selection
	;
	
	/* force highlight the selected item*/
	wowGui.force_clickable_highlight = is_selected;
	clicked = 0;
	if (wowGui.dropdown.draw_only_once == 0)
		clicked = wowGui_clickable(text);
	wowGui.force_clickable_highlight = 0;
	
	/* whatever element is selected from the dropdown list, should
	   be displayed as the dropdown */
	if (is_selected)
	{
		static const char *last = 0;
		
		/* force redraw on change */
		if (text != last)
			wowGui.dropdown.parent->refresh = 1;
		
		last = text;
		
		/* dropdown text is a pointer */
		if (!dest_sz)
			*wowGui.dropdown.text = text;
		
		/* dropdown text is an array */
		else
		{
			/* TODO there may be a way to save a refresh here */
			overwrite_dest = 1;
			wowGui.dropdown.parent->refresh = 1;
		}
	}
	
	/* when a selection has been made, close the dropdown */
	if (clicked)
	{
		/* update dropdown text array */
		if (dest_sz)
			overwrite_dest = 1;
		
		/* dropdown text is a pointer */
		else
			*wowGui.dropdown.text = text;
		
		*wowGui.dropdown.selection = wowGui.dropdown.select_counter;
		wowGui.dropdown.parent = 0;
		wowGuip.editcursor.active = 0;
		g_ddwin_open = 0;
	}
	
	if (overwrite_dest)
	{
		int len = strlen(text);
		dest_sz -= 2;
		
		if (len >= dest_sz)
			len = dest_sz - 1;
		
		memcpy(dest, text, len);
		dest[len] = '\0';
	}
	
	wowGui.dropdown.select_counter += 1;
}


/* test if user scrolled inside last clickable rect */
/* returns 0 if no scroll, otherwise, scroll direction (1, -1) */
WOW_GUI_API_PREFIX
int
wowGui_scrolled(void)
{
	if (
		wowGui.win->scroll_valuebox
		&& wowGui.mouse.has_scrolled
		&& wowGui.mouse.scroll.y
		&& mouse_on_rect(&wowGui.last_clickable_rect)
	)
	{
		wowGui.mouse.has_scrolled = 0;
		return (wowGui.mouse.scroll.y > 0) ? 1 : -1;
	}
	
	return 0;
}


/* tests if user right-click-dragged inside last clickable rect */
/* returns 0 on false, or the drag distance since mousedown */
WOW_GUI_API_PREFIX
int
wowGui_rightdrag_y(void)
{
	if (
		mousedown_on_rect(&wowGui.last_clickable_rect)
		&& wowGui.mouse.click.button == WOWGUI_MOUSEBUTTON_RIGHT
	)
	{
		return wowGui.mouse.y - wowGui.mouse.click.y;
	}
	
	return 0;
}


/* tests if user right-click-dragged inside last clickable rect */
/* returns 0 on false, or the drag distance since mousedown */
WOW_GUI_API_PREFIX
int
wowGui_rightdrag_delta_y(void)
{
	if (
		mousedown_on_rect(&wowGui.last_clickable_rect)
		&& wowGui.mouse.click.button == WOWGUI_MOUSEBUTTON_RIGHT
	)
	{
		int rval = wowGui.mouse.y - wowGui.mouse.lastclick.y;
		wowGui.mouse.lastclick.y = wowGui.mouse.y;
		return rval;
	}
	
	return 0;
}


/* create dropdown menu */
WOW_GUI_API_PREFIX
int
wowGui_dropdown(char **text, int *selection, int w, int h)
{
	static int win_drawn_once  = 0;
	static int win_want_scroll = 0;
	
	char *carr = wowGui.textbuf;
	int rval = 0;
	int clicked;
	int scrolled;
	
	struct wowGui_window *ddwin = &wowGui.dropdown.win;
	struct wowGui_window *win = wowGui.win;
	int selection_max  = wowGui.dropdown.select_counter - 1;
	g_dropdown_first = 0;
	g_dropdown_last = 0;
	
	/* advance dropdown index */
	win->dropdown_index += 1;
	
	/* confirm dimensions are positive and non-zero */
	assert(w >= 0 && "dropdown dimensions must be >= 0");
	assert(h >= 0 && "dropdown dimensions must be >= 0");
	
	/* confirm text */
	assert(text && "must provide pointer to pointer to char");
	
	/* confirm selection */
	assert(selection && "must provide pointer to selection");
	
	/* confirm dropdown() isn't being called inside dropdown window */
	assert(ddwin != win && "cannot use dropdown() inside dropdown");
	
	strcpy(carr, "[");
	
	/* if text pointer points to something */
	if (*text)
		strcat(carr, *text);
#if 0
	strcat(carr, "|v]");
#else
	strcat(carr, "][v]");
#endif
	
	clicked = wowGui_clickable(carr);
	
	wowGui.dropdown.text = text;
	
	/* special case: must propagate contents on init */
	/* !*text takes care of an uninitialized pointer, whereas
	   (*text && !**text) takes care of an uninitialized array */
	if (!*text || (*text && !**text))
	{
L_update:
		/* disable drawing */
		wowGui.dropdown.draw_only_once = 1;
		
		/* this is the bare minimum needed to make the dropdown
		   window contents get processed */
		state_backup();                        /* back up current win */
		wowGui.dropdown.parent = win;
		g_ddwin_open = 0; /* yes, 0... fixes problems with ddwin   *
		                   * overriding scrolling in value boxes   *
		                   * after closing a ddwin (FIXME someday) */
		ddwin->refresh = 1;                    /* force redraw        */
		win->refresh = 1;
		wowGui.dropdown.selection = selection; /* store selection     */
		wowGui_window(ddwin);                  /* make window         */
		wowGui.dropdown.select_counter = 0;    /* reset counter       */
		
		return 1;
	}
	
	/* test if this dropdown is the one that is active */
	else if (
		wowGui.dropdown.parent == win
		&& win->dropdown_index == wowGui.dropdown.index
	)
	{
		int selection_prev = *selection;
		
		/* enter and escape keys close dropdown menu */
		if (wowGui.key.enter
			|| wowGui.key.escape
		)
		{
			wowGui.dropdown.parent = 0;
			wowGuip.editcursor.active = 0;
			g_ddwin_open = 0;
			goto L_early_return;
		}
		
		/* up/down keys update selection in real time */
		if (wowGui.key.down)
			*selection += *selection < selection_max;
		if (wowGui.key.up)
			*selection -= *selection > 0;
		
		/* automatically derive scroll (window contents must
		   have been drawn once before for this to work */
		if (
			(win_drawn_once && win_want_scroll)
			|| *selection != selection_prev
		)
		{
			/* scroll window so selected item is at the top */
			ddwin->scroll.y = -(wowGui.advance.y * *selection);
			
			/* additional scroll offset to  vertically center the
			   selection, so the user can see what is beyond the
			   selected item: in both directions */
			ddwin->scroll.y += ddwin->rect.h / 2;
			window_test_scroll_boundaries(ddwin);
			
			/* force it to redraw once more after auto-scroll */
			if (win_want_scroll)
			{
				ddwin->refresh = 1;
				win->refresh = 1;
				win_want_scroll = 0;
			}
		}
		else if (!win_drawn_once)
		{
			ddwin->refresh = 1;
			win->refresh = 1;
			win_drawn_once += 1;
		}
		
		/* reset item counter */
		wowGui.dropdown.selection = selection;
		
		/* backup the current window state */
		state_backup();
		
		/* if dropdown window doesn't have a render target but current
		   window does, manually switch to no-target mode */
		if (!ddwin->target && win->target)
			win->target->bind(0);
		
		/* initialize a new window */
		wowGui_columns(1);
		wowGui_column_width(ddwin->rect.w);
		
		/* window opened */
		/* will return non-zero at end of this function, so the
		   program knows to do item list and call end() */
		rval = wowGui_window(ddwin);
		if (rval)
		{
			wowGui.dropdown.select_counter = 0;
		}
		
		/* window didn't open, so restore previous state */
		else
		{
			state_restore();
		}
	}
	
	/* dropdown is clicked, open it and make active */
	else if (clicked)
	{
		/* back up ddwin's contents */
		struct wowGui_window ddwin_backup = *ddwin;
		
		/* makes active */
		wowGui.dropdown.parent = win;
		wowGui.dropdown.index = win->dropdown_index;
		g_ddwin_open = 1;
		
		/* initialize auto-scroll inside this window so it jumps
		   to whatever item was initially selected */
		win_drawn_once  = 0;
		win_want_scroll = 1;
		ddwin->refresh = 1;
		win->refresh = 1;
		
		/* initialize window settings */
		memset(ddwin, 0, sizeof(*ddwin));
		ddwin->color = WOWGUI_DROPDOWN_COLOR;
		
		/* restore presistent settings */
		ddwin->target = ddwin_backup.target;
		ddwin->has_init = ddwin_backup.has_init;
		
		/* draw at x of label, and at y after label */
		ddwin->rect.x =
			win->rect.x
			+ wowGui.last_label.x
			+ win->scroll.x
		;
		ddwin->rect.y =
			win->rect.y
			+ wowGui.last_label.y
			+ wowGui.advance.y
			+ win->scroll.y
		;
		/* do not want it to fall off left/upper edge of window */
		if (ddwin->rect.x < win->rect.x)
			ddwin->rect.x = win->rect.x;
		if (ddwin->rect.y < win->rect.y)
			ddwin->rect.y = win->rect.y;
		ddwin->rect.w = w;
		ddwin->rect.h = h;
	}
	
	/* test if mouse wheel'd on */
	/* allow this only if scrolling fields enabled */
	else if ((scrolled = wowGui_scrolled()))
	{
		//static int i = 0;
		//fprintf(stderr, "selection_max = %d (%d)\n", selection_max, i++);
		*selection += -scrolled;
		//if (*selection < 0)
		//	*selection = 0;
		//*selection = 100;
		
		goto L_update;
	}
	
L_early_return:
	
	return rval;
}


/* create dropdown menu; selection text is stored in a buffer
   that has been pre-allocated by the user, of the length text_sz;
   on first use, at least the first character of `text` should be 0 */
WOW_GUI_API_PREFIX
int
wowGui_dropdown_buf(
	char *text
	, int text_sz
	, int *selection
	, int w
	, int h
)
{
	/* buffering with a static pointer is necessary due to
	   dropdown() requiring a pointer-to-pointer */
	static char *buf;
	int rval;
	
	buf = text;
	
	wowGui.dropdown.text_sz = text_sz;
	
	assert(text_sz > 0 && "text_sz must be > 0");
	
	rval = wowGui_dropdown(&buf, selection, w, h);
	
	/* dropdown() did not initialize, so dropdown_end() will not be
	   called, so set text_sz = 0 manually here */
	if (!rval)
	{
		wowGui.dropdown.text_sz = 0;
	}
	
	return rval;
}


/* set viewport rectangle */
WOW_GUI_API_PREFIX
void
wowGui_viewport(int x, int y, int w, int h)
{
	wowGui.viewport.x = x;
	wowGui.viewport.y = y;
	wowGui.viewport.w = w;
	wowGui.viewport.h = h;
}


/* text edit box */
WOW_GUI_API_PREFIX
void
wowGui_textbox(char *buf, int buf_sz)
{
	assert(buf);
	
	static int win_drawn_once = 0;
	
	/* the last few bytes are reserved */
	buf_sz -= 2;
	
	char *carr = wowGui.textbuf;
	struct wowGui_window *ddwin = &wowGui.dropdown.win;
	struct wowGui_window *win = wowGui.win;
	int clicked;
	int rval = 0;
	int buf_len = strlen(buf);
	//int buf_min = min_int(buf_len + 1, buf_sz);
	int buf_max = buf_sz - 2;
	
	char *edit = wowGui.dropdown.editcursor;
	
	strcpy(carr, "{");
	strcat(carr, buf);
	strcat(carr, "}");
	
	clicked = wowGui_clickable(carr);
	
	/* advance dropdown index */
	win->dropdown_index += 1;
	
	/* test if this dropdown is the one that is active */
	if (
		wowGui.dropdown.parent == win
		&& win->dropdown_index == wowGui.dropdown.index
	)
	{
		int scrolled = 0;
		char *k;
		const char *allowed = wowGui.dropdown.only_list;
		
		for (k = wowGuip.keyboard.buf; *k; ++k)
		{
			/* control characters */
			if (*k == WOWGUI_KBC_CONTROL)
			{
				++k;
				
				switch (*k)
				{
					/* scroll left */
					case WOWGUI_KBC_LEFT:
						if (edit > buf)
						{
							wowGui.dropdown.editcursor -= 1;
							scrolled = 1;
						}
						break;
					
					/* scroll right */
					case WOWGUI_KBC_RIGHT:
						if (edit < buf + buf_len)
						{
							wowGui.dropdown.editcursor += 1;
							scrolled = 1;
						}
						break;
					
					/* delete next character */
					case WOWGUI_KBC_DELETE:
						if (edit < buf + buf_len)
						{
							memmove(edit, edit + 1, buf_len - (edit - buf));
							buf_len -= 1;
							scrolled = 1;
						}
						break;
					
					/* delete previous character */
					case WOWGUI_KBC_BACKSPACE:
						if (edit > buf)
						{
							memmove(edit - 1, edit, buf_len - (edit - buf));
							edit -= 1;
							wowGui.dropdown.editcursor -= 1;
							buf_len -= 1;
							scrolled = 1;
						}
						break;
					
					/* home takes us to the start of the edit box */
					case WOWGUI_KBC_HOME:
						wowGui.dropdown.editcursor = buf;
						edit = buf;
						scrolled = 1;
						break;
					
					/* end takes us to the end of the edit box */
					case WOWGUI_KBC_END:
						edit = buf + buf_len;
						wowGui.dropdown.editcursor = edit;
						scrolled = 1;
						break;
		
					/* enter and escape keys close dropdown menu */
					case WOWGUI_KBC_ENTER:
					case WOWGUI_KBC_ESCAPE:
						wowGui.dropdown.parent = 0;
						wowGuip.editcursor.target = 0;
						wowGuip.editcursor.active = 0;
						g_ddwin_open = 0;
						return;
					
					default:
						break;
				}
				
				continue;
			}
			
			/* standard characters */
			
			/* skip characters that are not in allowed list */
			if (allowed && !strchr(allowed, *k))
				continue;
			
			/* insert standard character at current position,
			   but only if there is room */
			if (buf_len < buf_max)
			{
				memmove(edit + 1, edit, buf_len - (edit - buf));
				*edit = *k;
				wowGui.dropdown.editcursor += 1;
				buf_len += 1;
				scrolled = 1;
			}
		}
		
		/* this hack is so the blinking cursor can be placed at the
		   end of the string */
		if (buf_len < buf_max)
		{
			buf[buf_len] = ' ';
			buf[buf_len+1] = '\0';
		}
		
		/* automatically derive scroll (window contents must
		   have been drawn once before for this to work */
		if (win_drawn_once && scrolled)
		{
			/* restart blinking cursor */
			wowGuip.editcursor.restart = 1;
			/* scroll window along */
			ddwin->scroll.x = -(wowGui.font_w * (edit - buf));
			
			/* additional scroll offset to center the
			   selection, so the user can see what is beyond the
			   selected item: in all directions */
			ddwin->scroll.x += ddwin->rect.w / 2;
			ddwin->scroll.y += ddwin->rect.h / 2;
			window_test_scroll_boundaries(ddwin);
		}
		else if (!win_drawn_once)
		{
			ddwin->refresh = 1;
			win->refresh = 1;
			win_drawn_once += 1;
		}
		
		/* backup the current window state */
		state_backup();
		
		/* if dropdown window doesn't have a render target but current
		   window does, manually switch to no-target mode */
		if (!ddwin->target && win->target)
			win->target->bind(0);
		
		/* initialize a new window */
		wowGui_columns(2);
		//wowGui_column_width(ddwin->rect.w);
		wowGui.padding.x = 0;
		wowGui.padding.y = 0;
		wowGui.font_h = 0;
		
		/* window opened */
		/* will return non-zero at end of this function, so the
		   program knows to do item list and call end() */
		rval = wowGui_window(ddwin);
		if (rval)
		{
			wowGui.dropdown.select_counter = 0;
			wowGuip.editcursor.target = ddwin->target;
			wowGuip.editcursor.active = 1;
		}
		
		/* window didn't open, so restore previous state */
		else
		{
			state_restore();
		}
	}
	
	/* dropdown is clicked, open it and make active */
	else if (clicked)
	{
		/* editing text */
		wowGui.dropdown.editcursor = buf;
		wowGuip.editcursor.restart = 1;
		
		/* back up ddwin's contents */
		struct wowGui_window ddwin_backup = *ddwin;
		
		/* makes active */
		wowGui.dropdown.parent = win;
		wowGui.dropdown.index = win->dropdown_index;
		g_ddwin_open = 1;
		
		/* initialize auto-scroll inside this window so it jumps
		   to whatever item was initially selected */
		win_drawn_once  = 0;
		ddwin->refresh = 1;
		win->refresh = 1;
		
		/* initialize window settings */
		memset(ddwin, 0, sizeof(*ddwin));
		ddwin->color = WOWGUI_DROPDOWN_COLOR;
		
		/* restore presistent settings */
		ddwin->target = ddwin_backup.target;
		ddwin->has_init = ddwin_backup.has_init;
		
		/* draw at coordinates of label */
		ddwin->rect.x =
			win->rect.x
			+ wowGui.last_label.x
			+ win->scroll.x
		;
		ddwin->rect.y =
			win->rect.y
			+ wowGui.last_label.y
			+ win->scroll.y
		;
		/* do not want it to fall off left/upper edge of window */
		if (ddwin->rect.x < win->rect.x)
			ddwin->rect.x = win->rect.x;
		if (ddwin->rect.y < win->rect.y)
			ddwin->rect.y = win->rect.y;
		ddwin->rect.w = wowGui.advance.x;
		/* TODO + advance.y * (lines - 1) or some such */
		ddwin->rect.h = wowGui.font_h + wowGui.scrolldim.y;//wowGui.advance.y;
	}
	
	if (rval)
	{
		wowGui_label(buf);
		
		/* zztv r1 new code fixes textbox() issue after clicking dropdown */
		wowGui_window_end();
		state_restore();
		/* old broken equivalent */
		//wowGui_dropdown_end();
	}
		
	buf[buf_len] = '\0';
}


/* text edit box allowing only specified characters */
WOW_GUI_API_PREFIX
void
wowGui_textbox_only(char *buf, int buf_sz, char const *allowed)
{
	wowGui.dropdown.only_list = allowed;
	wowGui_textbox(buf, buf_sz);
	wowGui.dropdown.only_list = 0;
}


/* int edit box */
WOW_GUI_API_PREFIX
int
wowGui_int(int *i)
{
	assert(i);
	
	static char buf[16] = {0};
	struct wowGui_window *win = wowGui.win;
	const char *allowed = "-0123456789";
	
	/* if this is the active dropdown, update value */
	if (
		wowGui.dropdown.parent == win
		&& win->dropdown_index + 1 == wowGui.dropdown.index
	)
	{
		int old = *i;
		wowGui_textbox_only(buf, sizeof(buf), allowed);
		sscanf(buf, "%d", i);
		if (*i != old)
			return 1;
	}
	
	/* one is active, but not this one */
	else if (wowGui.dropdown.parent)
	{
		char *carr = wowGui.textbuf;
		sprintf(carr, "{%d}", *i);
		wowGui_label(carr);
		
		/* we don't call textbox() here, so increment as if we had */
		win->dropdown_index += 1;
	}
	
	/* none is active yet */
	else
	{
		sprintf(buf, "%d", *i);
		wowGui_textbox_only(buf, sizeof(buf), allowed);
	}
	
	return 0;
}


/* int edit box, with range */
WOW_GUI_API_PREFIX
int
wowGui_int_range(int *i, const int min, const int max, int increment)
{
	assert(i);
//	assert(min < max);
	
	int changed = wowGui_int(i);
	
	/* if user scrolled or right-dragged on value box */
	int scrolled;
	int rdrag;
	if ((scrolled = wowGui_scrolled()))
	{
		*i += scrolled * increment;
		changed = 1;
	}
	else if ((rdrag = wowGui_rightdrag_delta_y()))
	{
		*i -= rdrag * increment;
		changed = 1;
	}
	
	if (*i < min)
		*i = min;
	
	else if (*i > max)
		*i = max;
	
	return changed;
}


/* hex edit box */
WOW_GUI_API_PREFIX
unsigned int
wowGui_hex(unsigned int *i)
{
	assert(i);
	
	static char buf[16] = {0};
	struct wowGui_window *win = wowGui.win;
	const char *allowed = "Xx0123456789ABCDEFabcdef";
	
	/* if this is the active dropdown, update value */
	if (
		wowGui.dropdown.parent == win
		&& win->dropdown_index + 1 == wowGui.dropdown.index
	)
	{
		int old = *i;
		wowGui_textbox_only(buf, sizeof(buf), allowed);
		sscanf(buf, "%X", i);
		if (*i != old)
			return 1;
	}
	
	/* one is active, but not this one */
	else if (wowGui.dropdown.parent)
	{
		char *carr = wowGui.textbuf;
		sprintf(carr, "{0x%08X}", *i);
		wowGui_label(carr);
		
		/* we don't call textbox() here, so increment as if we had */
		win->dropdown_index += 1;
	}
	
	/* none is active yet */
	else
	{
		sprintf(buf, "0x%08X", *i);
		wowGui_textbox_only(buf, sizeof(buf), allowed);
	}
	
	return 0;
}


/* hexadecimal edit box, with range */
WOW_GUI_API_PREFIX
unsigned int
wowGui_hex_range(
	unsigned int *i
	, const unsigned int min
	, const unsigned int max
	, int increment
)
{
	assert(i);
//	assert(min < max);
	
	int changed = wowGui_hex(i);
	
	/* if user scrolled or right-dragged on value box */
	int scrolled;
	int rdrag;
	if ((scrolled = wowGui_scrolled()))
	{
		if (scrolled < 0 && increment > *i)
			*i = 0;
		else
			*i += scrolled * increment;
		changed = 1;
	}
	else if ((rdrag = wowGui_rightdrag_delta_y()))
	{
		/* unsigned overflow test */
		if (rdrag > 0 && (*i - rdrag * increment) > *i)
			*i = 0;
		else
			*i -= rdrag * increment;
		changed = 1;
	}
	
	if (*i < min)
		*i = min;
	
	else if (*i > max)
		*i = max;
	
	return changed;
}


/* float edit box */
WOW_GUI_API_PREFIX
int
wowGui_float(float *i)
{
	assert(i);
	
	static char buf[16] = {0};
	struct wowGui_window *win = wowGui.win;
	const char *allowed = "-.0123456789";
	
	/* if this is the active dropdown, update value */
	if (
		wowGui.dropdown.parent == win
		&& win->dropdown_index + 1 == wowGui.dropdown.index
	)
	{
		int old = *i;
		wowGui_textbox_only(buf, sizeof(buf), allowed);
		sscanf(buf, "%f", i);
		if (*i != old)
			return 1;
	}
	
	/* one is active, but not this one */
	else if (wowGui.dropdown.parent)
	{
		char *carr = wowGui.textbuf;
		sprintf(carr, "{%f}", *i);
		wowGui_label(carr);
		
		/* we don't call textbox() here, so increment as if we had */
		win->dropdown_index += 1;
	}
	
	/* none is active yet */
	else
	{
		sprintf(buf, "%f", *i);
		wowGui_textbox_only(buf, sizeof(buf), allowed);
	}
	
	return 0;
}


/* float edit box, with range */
WOW_GUI_API_PREFIX
float
wowGui_float_range(
	float *i
	, const float min
	, const float max
	, float increment
)
{
	assert(i);
//	assert(min < max);
	
	float changed = wowGui_float(i);
	
	/* if user scrolled or right-dragged on value box */
	float scrolled;
	float rdrag;
	if ((scrolled = wowGui_scrolled()))
	{
		*i += scrolled * increment;
		changed = 1;
	}
	else if ((rdrag = wowGui_rightdrag_delta_y()))
	{
		*i -= rdrag * increment;
		changed = 1;
	}
	
	if (*i < min)
		*i = min;
	
	else if (*i > max)
		*i = max;
	
	return changed;
}


/* push characters onto keyboard buffer */
WOW_GUI_API_PREFIX
void
wowGui_keyboard(char *pressed, int num_bytes)
{
	if (num_bytes > wowGuip.keyboard.remaining || !wowGuip.keyboard.buf)
		return;
	
	assert(pressed);
	assert(num_bytes);
	
	wowGui.key.has_pressed = 1;
	
	if (*pressed == '0')
		wowGui.key.zero = 1;
	
	memcpy(wowGuip.keyboard.pos, pressed, num_bytes);
	
	wowGuip.keyboard.pos += num_bytes;
	wowGuip.keyboard.remaining -= num_bytes;
	*wowGuip.keyboard.pos = '\0';
}


/* push keyboard control characters */
WOW_GUI_API_PREFIX
void
wowGui_keyboard_control(enum wowGui_keyboard_control keycode)
{
	char arr[2] = { WOWGUI_KBC_CONTROL, keycode };
	struct wowGui_key *key = &wowGui.key;
	
	switch (keycode)
	{
		case WOWGUI_KBC_LEFT:
			key->left = 1;
			break;
		
		case WOWGUI_KBC_RIGHT:
			key->right = 1;
			break;
		
		case WOWGUI_KBC_UP:
			key->up = 1;
			break;
		
		case WOWGUI_KBC_DOWN:
			key->down = 1;
			break;
		
		case WOWGUI_KBC_ENTER:
			key->enter = 1;
			break;
		
		case WOWGUI_KBC_ESCAPE:
			key->escape = 1;
			break;
		
		default:
			break;
	}
	
	wowGui_keyboard(arr, 2);
}


/* this is the old font */
static
unsigned char
__wowGui_EGh_bin[] = {
0x00, 0xdf, 0xfd, 0x00, 0x0a, 0xee, 0xff, 0xa0, 0x0d, 0xf2, 0x2d, 0xd0,
0x06, 0x61, 0x1d, 0xc0, 0x01, 0x12, 0x2d, 0xd0, 0x06, 0x71, 0x99, 0x00,
0x01, 0x1e, 0xed, 0x10, 0x07, 0x7e, 0xf7, 0x00, 0x01, 0x56, 0x29, 0x90,
0x05, 0x58, 0x97, 0x60, 0x0d, 0xd2, 0x29, 0x90, 0x05, 0x59, 0x97, 0x70,
0x04, 0xdf, 0xfd, 0x40, 0x02, 0x6e, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x08, 0xbf, 0xfb, 0x00, 0x0e, 0xff, 0xff, 0xc0,
0x0b, 0xf0, 0x0f, 0xb0, 0x0f, 0xf0, 0x03, 0x30, 0x0f, 0xf0, 0x0f, 0xf0,
0x0f, 0xf0, 0x02, 0x20, 0x0c, 0xfb, 0xbf, 0x60, 0x0f, 0xfc, 0xce, 0x20,
0x0d, 0xd4, 0x4f, 0xf0, 0x0f, 0xf0, 0x02, 0x20, 0x0f, 0xf0, 0x0f, 0xf0,
0x0f, 0xf0, 0x03, 0x30, 0x0c, 0xfb, 0xbf, 0x40, 0x0e, 0xf7, 0x77, 0x40,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdf, 0xfd, 0x00,
0x0a, 0xee, 0xff, 0xa0, 0x0d, 0xf2, 0x2d, 0xd0, 0x06, 0x61, 0x1d, 0xc0,
0x01, 0x12, 0x2d, 0xd0, 0x06, 0x71, 0x99, 0x00, 0x01, 0x1e, 0xed, 0x10,
0x07, 0x7e, 0xf7, 0x00, 0x01, 0x56, 0x29, 0x90, 0x05, 0x58, 0x97, 0x60,
0x0d, 0xd2, 0x29, 0x90, 0x05, 0x59, 0x97, 0x70, 0x04, 0xdf, 0xfd, 0x40,
0x02, 0x6e, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0xbf, 0xfb, 0x00, 0x00, 0x0d, 0xe0, 0x00, 0x0b, 0xf0, 0x0f, 0xb0,
0x00, 0x5d, 0xe6, 0x00, 0x0f, 0xf0, 0x0f, 0xf0, 0x05, 0x5c, 0xc6, 0x60,
0x0c, 0xfb, 0xbf, 0x60, 0x77, 0x3f, 0xf3, 0x77, 0x0d, 0xd4, 0x4f, 0xf0,
0xbb, 0x3f, 0xf3, 0xbb, 0x0f, 0xf0, 0x0f, 0xf0, 0x09, 0x9c, 0xca, 0xa0,
0x0c, 0xfb, 0xbf, 0x40, 0x00, 0x9d, 0xea, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0d, 0xe0, 0x00, 0x04, 0xc2, 0x2c, 0x40, 0x02, 0x8d, 0x50, 0x20,
0x0c, 0xca, 0xac, 0xc0, 0x21, 0xf9, 0x17, 0x10, 0x04, 0xc2, 0x2c, 0x40,
0x12, 0x49, 0x34, 0x00, 0x00, 0x82, 0x08, 0x00, 0x01, 0x97, 0x51, 0x10,
0x08, 0x8a, 0x88, 0x80, 0x04, 0x61, 0x52, 0x41, 0x00, 0x80, 0x08, 0x00,
0x43, 0x11, 0x75, 0x30, 0x00, 0xa2, 0x08, 0x00, 0x60, 0x05, 0x56, 0x00,
0x00, 0x00, 0x00, 0x00, 0x04, 0x40, 0x00, 0x40, 0x00, 0x22, 0x11, 0x00,
0x00, 0x00, 0x00, 0x80, 0x00, 0x0f, 0xb0, 0x00, 0x00, 0x00, 0x08, 0x80,
0x04, 0x0d, 0xa4, 0x00, 0x00, 0x00, 0x88, 0x00, 0x08, 0xcd, 0xe8, 0x80,
0x02, 0x2a, 0xa2, 0x20, 0x08, 0xcd, 0xe8, 0x80, 0x02, 0xaa, 0x22, 0x20,
0x04, 0x0d, 0xa4, 0x00, 0x0c, 0xd1, 0x00, 0x00, 0x00, 0x0f, 0xb0, 0x00,
0x8c, 0x51, 0x00, 0x00, 0x00, 0x22, 0x11, 0x00, 0x81, 0x10, 0x00, 0x00,
0x00, 0xdf, 0xfd, 0x00, 0x0a, 0xee, 0xff, 0xa0, 0x0d, 0xf2, 0x2d, 0xd0,
0x06, 0x61, 0x1d, 0xc0, 0x01, 0x12, 0x2d, 0xd0, 0x06, 0x71, 0x99, 0x00,
0x01, 0x1e, 0xed, 0x10, 0x07, 0x7e, 0xf7, 0x00, 0x01, 0x56, 0x29, 0x90,
0x05, 0x58, 0x97, 0x60, 0x0d, 0xd2, 0x29, 0x90, 0x05, 0x59, 0x97, 0x70,
0x04, 0xdf, 0xfd, 0x40, 0x02, 0x6e, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x00, 0x04, 0x48, 0x99, 0x80,
0x03, 0x3c, 0xc3, 0x30, 0x00, 0xcd, 0x10, 0x88, 0x03, 0x3c, 0xc3, 0x30,
0x02, 0xbf, 0x62, 0xa8, 0x00, 0x33, 0x33, 0x20, 0x01, 0x10, 0x4c, 0x80,
0x01, 0x10, 0x03, 0x30, 0x00, 0x15, 0xc8, 0x00, 0x03, 0x3c, 0xc3, 0x30,
0x02, 0x67, 0x32, 0x20, 0x00, 0x3f, 0xf3, 0x00, 0x04, 0x40, 0x99, 0x00,
0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0xdf, 0xfd, 0x10,
0x07, 0xff, 0xff, 0x60, 0x1c, 0xe0, 0x0e, 0xc1, 0x0f, 0xf0, 0x09, 0x90,
0x1e, 0xe1, 0x16, 0x61, 0x0f, 0xf0, 0x01, 0x10, 0x1e, 0xf4, 0x56, 0x21,
0x0f, 0xf6, 0x67, 0x10, 0x1e, 0xf2, 0x36, 0x61, 0x0f, 0xf0, 0x89, 0x90,
0x1e, 0xf1, 0x0f, 0xe1, 0x0f, 0xf0, 0x09, 0x90, 0x16, 0xec, 0xce, 0x21,
0x07, 0xfb, 0xbb, 0x20, 0x01, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00,
0x09, 0xb6, 0x6f, 0xd0, 0x27, 0xd8, 0x8e, 0x60, 0x09, 0x92, 0xed, 0x10,
0x2f, 0xf0, 0x2e, 0xe0, 0x09, 0x9a, 0xe5, 0x10, 0x2f, 0xf6, 0x2e, 0xe0,
0x09, 0x9b, 0x75, 0x10, 0x2f, 0xd6, 0x4e, 0xe0, 0x0d, 0xda, 0xe5, 0x10,
0x2f, 0xd0, 0x4e, 0xe0, 0x0d, 0xd2, 0xed, 0x10, 0x2f, 0xd0, 0x0e, 0xe0,
0x09, 0xf6, 0x6f, 0x90, 0x27, 0xd9, 0x9f, 0x70, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0x00, 0x8f, 0x71, 0x1f, 0xf0,
0x2f, 0xd0, 0x0f, 0xf0, 0x8f, 0x71, 0x1f, 0xf0, 0x2f, 0xd0, 0x07, 0x70,
0x8e, 0x61, 0x1e, 0xe0, 0x27, 0xdd, 0xdf, 0x60, 0x8e, 0x69, 0x1e, 0xe0,
0x27, 0x76, 0x4a, 0xa0, 0x8e, 0xe9, 0x9e, 0xe0, 0x2f, 0xd0, 0x6e, 0x80,
0x8a, 0xe7, 0xfe, 0xa0, 0x07, 0xfa, 0x8e, 0x60, 0x88, 0x27, 0x7a, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x7c, 0xcf, 0xf0,
0x13, 0x26, 0x60, 0x11, 0x07, 0x7c, 0xcf, 0xf0, 0x03, 0x76, 0x65, 0x10,
0x02, 0x39, 0xd7, 0x20, 0x04, 0x53, 0x35, 0x40, 0x00, 0x2f, 0xf2, 0x00,
0x01, 0x13, 0x31, 0x10, 0x00, 0x5f, 0xb1, 0x00, 0x00, 0x03, 0x30, 0x00,
0x05, 0x5e, 0xe5, 0x50, 0x01, 0x13, 0x31, 0x10, 0x05, 0x5e, 0xed, 0xd0,
0x02, 0x23, 0x30, 0x00, 0x00, 0x08, 0x88, 0x80, 0x8a, 0xab, 0xb8, 0x88,
0x00, 0x00, 0x11, 0x00, 0x00, 0x04, 0x45, 0x10, 0x04, 0x62, 0x33, 0x20,
0x00, 0x44, 0x01, 0x10, 0x04, 0xc8, 0x9a, 0xa0, 0x00, 0xee, 0xab, 0x10,
0x0c, 0xe6, 0x67, 0x20, 0x0e, 0xf5, 0x5f, 0xb0, 0x0e, 0xe0, 0x06, 0x60,
0x0b, 0xf6, 0x2b, 0x90, 0x0e, 0xe0, 0x06, 0x60, 0x03, 0xfc, 0x89, 0x90,
0x04, 0xee, 0xee, 0xa0, 0x00, 0x77, 0x3b, 0xb0, 0x00, 0x00, 0x00, 0x00,
0x08, 0x88, 0x88, 0x00, 0x09, 0x90, 0x00, 0x00, 0x00, 0x11, 0x10, 0x00,
0x09, 0x92, 0x24, 0x40, 0x00, 0x01, 0x10, 0x00, 0x09, 0x90, 0x88, 0x00,
0x26, 0xef, 0xde, 0x20, 0x09, 0x9b, 0xb5, 0x40, 0x2e, 0xc3, 0x3c, 0xe2,
0x0d, 0x9a, 0x25, 0x50, 0x2e, 0xc3, 0x3c, 0xe2, 0x0d, 0xda, 0xa5, 0x50,
0x2e, 0xc3, 0x3c, 0xe2, 0x09, 0xd6, 0xed, 0x10, 0x26, 0xcb, 0xbc, 0x62,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00,
0x05, 0xfb, 0xff, 0xe0, 0x8e, 0x61, 0x16, 0xe8, 0x0f, 0xf4, 0x03, 0x30,
0x8f, 0x71, 0x17, 0xf8, 0x07, 0xfc, 0x8b, 0x30, 0x8e, 0x69, 0x96, 0xe8,
0x05, 0x73, 0x3b, 0xa0, 0x8a, 0x6d, 0xd6, 0xa8, 0x0d, 0xd8, 0x8a, 0x20,
0x08, 0xa7, 0x79, 0xb2, 0x01, 0x10, 0x02, 0x20, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x80, 0x8a, 0x01, 0x10, 0x00, 0x00, 0x00, 0x08, 0x00,
0x80, 0xa1, 0x10, 0x00, 0x07, 0x74, 0x4f, 0x70, 0x80, 0xa9, 0x90, 0x00,
0x02, 0x31, 0xdf, 0x20, 0x84, 0xe6, 0x00, 0x04, 0x00, 0x27, 0xda, 0x20,
0xc8, 0xaa, 0x4c, 0x40, 0x00, 0x57, 0x3b, 0x20, 0x00, 0xa1, 0x18, 0x00,
0x05, 0x54, 0x6f, 0x50, 0x00, 0xa9, 0x98, 0x00, 0x02, 0x22, 0x20, 0x80,
0x02, 0x00, 0x18, 0x88, 0x00, 0x04, 0x44, 0x40, 0x00, 0x04, 0x00, 0x00,
0x00, 0x04, 0x44, 0x40, 0x0c, 0x44, 0x44, 0x00, 0x00, 0x04, 0x40, 0x00,
0x88, 0xc0, 0x00, 0x00, 0x00, 0x0c, 0xc0, 0x00, 0x0c, 0x46, 0xa4, 0x40,
0x00, 0x0c, 0xc0, 0x00, 0x08, 0x8e, 0xe0, 0x00, 0x02, 0x08, 0x80, 0x00,
0x80, 0xd0, 0x88, 0x00, 0x28, 0xa8, 0x80, 0x00, 0x88, 0xcd, 0x4c, 0x40,
0x0a, 0x88, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x06, 0xe0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
0x08, 0x88, 0x00, 0x80, 0x01, 0x06, 0x10, 0x00, 0x56, 0xe7, 0x50, 0x80,
0x02, 0x1f, 0xf1, 0x00, 0x38, 0x8c, 0xb8, 0x00, 0x0b, 0xf6, 0x0b, 0x00,
0x94, 0xc0, 0x28, 0x00, 0x06, 0x07, 0x6a, 0x00, 0xcb, 0xa6, 0xc8, 0x00,
0x00, 0x47, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0a, 0x80, 0x00, 0x00, 0x39, 0x14, 0x20, 0x02, 0x22, 0x24, 0x00,
0x08, 0xae, 0xa8, 0x60, 0x04, 0x28, 0x99, 0x70, 0x07, 0x75, 0xd1, 0x04,
0x0f, 0xb3, 0x33, 0xd0, 0x00, 0xae, 0xbe, 0xa4, 0x25, 0x15, 0x20, 0xa0,
0x02, 0x61, 0x0c, 0x02, 0x20, 0x42, 0x08, 0x20, 0x2c, 0x30, 0x14, 0x02,
0x02, 0x28, 0x82, 0x00, 0x03, 0xac, 0xc1, 0x30, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x12, 0x00, 0x08, 0x00, 0x28, 0x00,
0x0a, 0xcf, 0xee, 0x20, 0x0b, 0x62, 0x2e, 0x20, 0x02, 0x10, 0x82, 0x40,
0x01, 0x44, 0xe4, 0x40, 0x03, 0x00, 0x0e, 0x00, 0x8d, 0xea, 0xac, 0x00,
0x02, 0x10, 0x0a, 0x00, 0x01, 0xe0, 0x24, 0x00, 0x0c, 0x21, 0x02, 0x00,
0x09, 0x42, 0x21, 0x00, 0x00, 0xcc, 0xf4, 0x40, 0x02, 0xbf, 0xd4, 0x40,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x44, 0x40,
0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x44, 0x40, 0x00, 0x00, 0x00, 0x00,
0x00, 0x04, 0x40, 0x00, 0x0c, 0xcc, 0xc4, 0x40, 0x00, 0x0c, 0xc0, 0x00,
0x00, 0x02, 0xa0, 0x40, 0x00, 0x0c, 0xc0, 0x00, 0x04, 0xce, 0x64, 0x40,
0x02, 0x08, 0x80, 0x00, 0x00, 0x90, 0x00, 0x40, 0x28, 0xa8, 0x80, 0x00,
0x08, 0x01, 0x04, 0x00, 0x0a, 0x88, 0x80, 0x00, 0x04, 0x44, 0x40, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x54, 0x44, 0x00,
0xee, 0xfe, 0xe0, 0x00, 0x09, 0x3b, 0x3f, 0x00, 0x21, 0xd8, 0x20, 0x00,
0x00, 0x54, 0x4f, 0x00, 0x18, 0x58, 0x20, 0x00, 0x00, 0x01, 0x86, 0x00,
0xc6, 0x7e, 0x40, 0x00, 0x00, 0xef, 0x66, 0x20, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04, 0x00, 0x00, 0xc0, 0x20, 0x00,
0xaa, 0xaa, 0xea, 0x20, 0xef, 0xff, 0xff, 0x00, 0x80, 0x44, 0x19, 0x30,
0x00, 0x49, 0x24, 0x00, 0xc5, 0x35, 0x1b, 0x10, 0x00, 0x4b, 0x24, 0x00,
0x01, 0x35, 0xa0, 0x00, 0x8c, 0xa9, 0xac, 0x80, 0x00, 0x2c, 0x00, 0x00,
0x04, 0x21, 0xa4, 0x00, 0x2a, 0x84, 0x00, 0x00, 0x73, 0x11, 0xf1, 0x10,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x11, 0x19, 0x00,
0x00, 0x40, 0x00, 0x00, 0x8f, 0xee, 0xef, 0xe0, 0x0b, 0x76, 0x66, 0xd0,
0x1a, 0x00, 0x0b, 0x40, 0x4c, 0x40, 0x02, 0xd0, 0x28, 0x00, 0x1a, 0x40,
0x01, 0xd0, 0x2c, 0x10, 0x00, 0x00, 0x38, 0x40, 0x00, 0x40, 0x28, 0x10,
0x00, 0x01, 0xa0, 0x40, 0x00, 0x42, 0x83, 0x00, 0x05, 0xfe, 0x44, 0x40,
0x03, 0xfd, 0x54, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x09, 0x99, 0x9b, 0x00, 0x00, 0x10, 0x20, 0x00, 0x07, 0x26, 0x21, 0x40,
0x2a, 0xfe, 0xee, 0xa0, 0x8d, 0x8c, 0xa9, 0xc0, 0x00, 0x10, 0x20, 0x80,
0x32, 0x33, 0xb3, 0x60, 0x00, 0x19, 0x28, 0x00, 0x00, 0x00, 0xa1, 0x40,
0x00, 0x10, 0xb1, 0x00, 0x00, 0x08, 0x34, 0x00, 0x00, 0x1a, 0x08, 0x00,
0x05, 0xf7, 0x40, 0x00, 0x8e, 0xf4, 0x44, 0xc0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x08, 0x14, 0x02, 0x80, 0x00, 0x04, 0x00, 0x00,
0x1d, 0x11, 0xdb, 0x00, 0xdd, 0xfd, 0xdd, 0xd0, 0x0c, 0x88, 0x07, 0x00,
0x02, 0x06, 0x00, 0x90, 0x48, 0x00, 0x34, 0x00, 0x2c, 0x04, 0x2c, 0x10,
0x48, 0x11, 0x21, 0x40, 0x04, 0x84, 0x83, 0x40, 0x59, 0x03, 0x00, 0x50,
0x40, 0x0c, 0x10, 0x60, 0x42, 0xa9, 0x88, 0xc0, 0x40, 0x15, 0x80, 0x40,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x02, 0x00, 0x40,
0x08, 0x98, 0x88, 0x80, 0x08, 0xf9, 0x98, 0xc0, 0x06, 0x77, 0x75, 0x50,
0x02, 0x0c, 0x05, 0x00, 0x19, 0x98, 0xa8, 0xd0, 0x0b, 0x99, 0xca, 0x80,
0x04, 0x54, 0x65, 0xc0, 0x20, 0x08, 0x50, 0x20, 0x00, 0x10, 0x20, 0xc0,
0x31, 0x1c, 0x04, 0x20, 0x00, 0x01, 0x28, 0x40, 0x26, 0x63, 0xbb, 0xe0,
0x26, 0xef, 0xe6, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x02, 0x01, 0x00, 0xc8, 0xc0, 0x00, 0x00, 0x0f, 0x8a, 0x89, 0x80,
0xc3, 0xf3, 0x11, 0x30, 0x0f, 0x02, 0x01, 0x80, 0xc9, 0xc0, 0x00, 0x30,
0x0f, 0x02, 0x05, 0xa0, 0x00, 0x00, 0x00, 0x30, 0x0e, 0x02, 0x05, 0xa0,
0x00, 0x00, 0x00, 0x30, 0x0e, 0x02, 0x52, 0x80, 0x00, 0x00, 0x03, 0x00,
0x2c, 0xdf, 0xa8, 0x80, 0x02, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x88, 0x00, 0x01, 0x02, 0x80, 0x00,
0x03, 0xff, 0xf7, 0x00, 0x0f, 0x26, 0xe4, 0x72, 0xcc, 0x38, 0x00, 0x40,
0x0c, 0x38, 0x99, 0x00, 0x03, 0x0a, 0x31, 0x50, 0x0c, 0xb1, 0x82, 0x80,
0x03, 0x28, 0x06, 0x00, 0x87, 0x88, 0x2a, 0xa0, 0x01, 0x05, 0xc2, 0x00,
0x85, 0x82, 0xc2, 0x80, 0x10, 0x00, 0x39, 0x10, 0x08, 0x51, 0xbf, 0x40,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04, 0x00,
0x48, 0x9d, 0xcc, 0x40, 0xc9, 0xe6, 0x7f, 0x40, 0x40, 0x00, 0x94, 0x00,
0x5b, 0x21, 0x0c, 0xb0, 0x48, 0xae, 0xcc, 0x40, 0xe1, 0x30, 0x0c, 0x30,
0x43, 0x01, 0xa4, 0x00, 0xe1, 0x24, 0x5d, 0x30, 0x78, 0x8c, 0xd6, 0x10,
0xf1, 0x60, 0x94, 0x70, 0xd0, 0x40, 0x9c, 0x70, 0x0b, 0x8c, 0x53, 0x00,
0x0c, 0x9d, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0x39, 0x50, 0x00, 0x00, 0x88, 0xf0, 0x00, 0x2e, 0xaf, 0xc6, 0x00,
0x03, 0x01, 0x77, 0x60, 0x04, 0xf0, 0x41, 0x60, 0x03, 0x92, 0xf8, 0x12,
0x0f, 0xbd, 0x91, 0x40, 0x1b, 0x28, 0x60, 0x92, 0x70, 0xf4, 0x01, 0xf0,
0x0a, 0xd4, 0x65, 0x82, 0x53, 0xe0, 0x01, 0xe0, 0x04, 0x10, 0x68, 0x60,
0x04, 0x2a, 0xbe, 0x00, 0x00, 0x4f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x02, 0x3a, 0xee, 0x00, 0xc8, 0xc0, 0x00, 0x00,
0x0d, 0x84, 0xa5, 0x00, 0xc1, 0xc2, 0x11, 0x00, 0x45, 0x0e, 0x27, 0x00,
0xd9, 0xc3, 0x00, 0x10, 0x07, 0xf8, 0x8d, 0x20, 0x01, 0x30, 0x00, 0x10,
0xac, 0x02, 0x25, 0xa0, 0x01, 0x22, 0x00, 0x10, 0x44, 0x20, 0x16, 0xa0,
0x13, 0x02, 0x00, 0x30, 0x04, 0x1b, 0xaa, 0x40, 0x21, 0x00, 0x23, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


/* get default 2048x8 debug font */
static
void *
debugfont(int *surf_w, int *surf_h, int *char_w, int *char_h)
{
	long int bytes = sizeof(__wowGui_EGh_bin) * 8 * 4;
	unsigned char *result = wowGui_malloc_die(bytes * 4);
	if (!result)
		return 0;
	
	unsigned char *w;
	long int a;
	for (a = 0; a < 64 * 256; a++) {
		int x = a % 64;
		int y = a / 64;
		x = a & 7;
		y = a / 64;
		y += (64-8) * (a / (64*8));
		y += 8 * ((a%64) / 8);
		w = result + y * 8 * 4 + x * 4;
		int byte=((a&7)/2)+4*(a/32), bit=((a%32)/8)+4*!(a&1);
		if ((__wowGui_EGh_bin[byte]>>bit)&0x1)
			w[0] = w[1] = w[2] = w[3] = 0xFF;
		else
			w[0] = w[1] = w[2] = w[3] = 0x00;
	}
	
	*surf_w = 8;
	*surf_h = 2048;
	*char_w = 8;
	*char_h = 8;
	
	return result;
}


static
void
rgba8888_from_1bit(uint32_t *dst, const unsigned char *src, int num)
{
	uint32_t arr[] = {0, 0xFFFFFFFF};
	num /= 8;
	while (num--)
	{
		dst[0] = arr[(*src>>7)&1];
		dst[1] = arr[(*src>>6)&1];
		dst[2] = arr[(*src>>5)&1];
		dst[3] = arr[(*src>>4)&1];
		dst[4] = arr[(*src>>3)&1];
		dst[5] = arr[(*src>>2)&1];
		dst[6] = arr[(*src>>1)&1];
		dst[7] = arr[(*src>>0)&1];
		src += 1;
		dst += 8;
	}
}


/* drop black shadow behind white font */
static
void
font_dropshadow(uint32_t *px, int w, int h)
{
	/* skip the last row on purpose */
	int y = 0;
	while (y < h - 1)
	{
		/* skip the last column on purpose */
		int x = 0;
		while (x < w - 1)
		{
			uint32_t *dst = px + (y + 1) * w + x + 1;
			uint32_t *src = px +  y      * w + x    ;
			if (!*dst && *src == 0xFFFFFFFF)
				*dst = 0x000000FF;
			++x;
		}
		++y;
	}
}


/* get pixels for font crisp */
static
void *
font_crisp(int *surf_w, int *surf_h, int *char_w, int *char_h)
{
	static const int crisp_png_w = 768;
	static const int crisp_png_h = 16;
	static const unsigned char crisp_png_pix[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 0, 0, 0, 0, 8, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 36, 0, 8, 0, 0, 8, 4, 16, 0, 0, 0, 0, 0, 2, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 28, 64, 28, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 8, 48, 0, 0, 0, 8, 36
	, 0, 8, 32, 0, 8, 8, 8, 0, 0, 0, 0, 0, 2, 60, 8, 60, 60, 4, 126
	, 60, 126, 60, 60, 0, 0, 0, 0, 0, 28, 0, 60, 124, 30, 124, 126
	, 126, 30, 66, 62, 28, 65, 64, 65, 98, 28, 124, 28, 124, 62, 127
	, 65, 65, 65, 65, 65, 127, 16, 64, 4, 0, 0, 2, 0, 64, 0, 2, 0, 14
	, 0, 64, 8, 4, 64, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 8, 8, 8, 0, 0, 0, 8, 36, 20, 62, 81, 56, 8, 8, 8, 0, 0, 0, 0, 0
	, 4, 66, 56, 66, 66, 12, 64, 64, 66, 66, 66, 0, 0, 0, 0, 0, 34, 28
	, 66, 66, 33, 66, 64, 64, 33, 66, 8, 4, 66, 64, 65, 98, 34, 66, 34
	, 66, 65, 8, 65, 65, 65, 65, 65, 1, 16, 32, 4, 0, 0, 0, 0, 64, 0, 2
	, 0, 16, 0, 64, 8, 4, 64, 16, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0
	, 0, 0, 8, 8, 8, 0, 0, 0, 8, 0, 20, 73, 82, 68, 0, 16, 4, 0, 0, 0
	, 0, 0, 4, 70, 8, 2, 2, 20, 64, 64, 4, 66, 66, 8, 8, 4, 0, 32, 2
	, 34, 66, 66, 64, 65, 64, 64, 64, 66, 8, 4, 68, 64, 99, 82, 65, 66
	, 65, 66, 64, 8, 65, 65, 65, 34, 34, 2, 16, 32, 4, 0, 0, 0, 0, 64
	, 0, 2, 0, 16, 0, 64, 0, 0, 64, 16, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0
	, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 8, 0, 127, 72, 36, 68, 0, 16, 4, 8
	, 8, 0, 0, 0, 8, 78, 8, 2, 2, 36, 124, 124, 4, 66, 66, 8, 8, 8, 0
	, 16, 2, 73, 66, 66, 64, 65, 64, 64, 64, 66, 8, 4, 72, 64, 99, 82
	, 65, 66, 65, 66, 64, 8, 65, 34, 65, 20, 20, 4, 16, 16, 4, 8, 0, 0
	, 60, 124, 60, 62, 60, 60, 62, 124, 56, 28, 68, 16, 118, 124, 60
	, 124, 62, 30, 60, 60, 66, 34, 65, 98, 66, 126, 8, 8, 8, 0, 0, 0
	, 8, 0, 20, 62, 8, 56, 0, 16, 4, 42, 8, 0, 0, 0, 8, 90, 8, 60, 28
	, 68, 2, 66, 8, 60, 66, 0, 0, 16, 62, 8, 4, 85, 66, 124, 64, 65
	, 124, 124, 64, 126, 8, 4, 112, 64, 85, 74, 65, 124, 65, 124, 62
	, 8, 65, 34, 73, 8, 8, 8, 16, 16, 4, 20, 0, 0, 2, 66, 66, 66, 66
	, 16, 66, 66, 8, 4, 72, 16, 73, 66, 66, 66, 66, 32, 64, 16, 66, 34
	, 65, 20, 66, 4, 8, 8, 8, 50, 0, 0, 8, 0, 20, 9, 18, 72, 0, 16, 4
	, 28, 62, 0, 62, 0, 16, 114, 8, 64, 2, 126, 2, 66, 8, 66, 62, 0, 0
	, 32, 0, 4, 8, 85, 126, 66, 64, 65, 64, 64, 71, 66, 8, 4, 72, 64
	, 85, 74, 65, 64, 65, 72, 1, 8, 65, 20, 73, 20, 8, 16, 16, 8, 4
	, 34, 0, 0, 62, 66, 64, 66, 126, 16, 66, 66, 8, 4, 112, 16, 73, 66
	, 66, 66, 66, 32, 60, 16, 66, 34, 73, 8, 66, 8, 48, 8, 6, 76, 0, 0
	, 0, 0, 127, 73, 37, 69, 0, 16, 4, 42, 8, 0, 0, 0, 16, 98, 8, 64
	, 2, 4, 2, 66, 16, 66, 2, 0, 0, 16, 62, 8, 0, 74, 66, 66, 64, 65
	, 64, 64, 65, 66, 8, 4, 68, 64, 73, 70, 65, 64, 65, 68, 1, 8, 65
	, 20, 73, 34, 8, 32, 16, 8, 4, 0, 0, 0, 66, 66, 64, 66, 64, 16, 66
	, 66, 8, 4, 72, 16, 73, 66, 66, 66, 66, 32, 2, 16, 66, 34, 73, 24
	, 66, 16, 8, 8, 8, 0, 0, 0, 8, 0, 20, 62, 69, 66, 0, 16, 4, 8, 8
	, 8, 0, 8, 32, 66, 8, 64, 66, 4, 66, 66, 16, 66, 2, 8, 8, 8, 0, 16
	, 8, 32, 66, 66, 33, 66, 64, 64, 33, 66, 8, 4, 66, 64, 73, 70, 34
	, 64, 34, 66, 65, 8, 34, 8, 73, 65, 8, 64, 16, 4, 4, 0, 0, 0, 66
	, 66, 66, 66, 66, 16, 66, 66, 8, 4, 68, 16, 73, 66, 66, 66, 66, 32
	, 66, 16, 66, 20, 73, 36, 66, 32, 8, 8, 8, 0, 0, 0, 8, 0, 20, 8, 2
	, 61, 0, 16, 4, 0, 0, 8, 0, 8, 32, 60, 62, 126, 60, 4, 60, 60, 16
	, 60, 60, 8, 8, 4, 0, 32, 8, 30, 66, 124, 30, 124, 126, 64, 30, 66
	, 62, 56, 65, 126, 65, 66, 28, 64, 28, 65, 62, 8, 28, 8, 54, 65
	, 8, 127, 16, 4, 4, 0, 126, 0, 62, 124, 60, 62, 60, 16, 62, 66, 62
	, 4, 66, 12, 73, 66, 60, 124, 62, 32, 60, 12, 62, 8, 54, 67, 62
	, 126, 8, 8, 8, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 8, 8, 0, 0, 16, 0, 0
	, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 16, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 4, 0, 0, 0, 0
	, 0, 64, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 8, 8, 8, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 8, 8, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 2, 4, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 2, 0, 0, 4, 0, 0, 0, 0, 0, 64, 2, 0, 0, 0, 0, 0, 0
	, 0, 2, 0, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 16, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 28, 0, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60, 0, 0, 56
	, 0, 0, 0, 0, 0, 64, 2, 0, 0, 0, 0, 0, 0, 0, 60, 0, 6, 8, 48, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	void *result = wowGui_malloc_die(crisp_png_w * crisp_png_h * 4);
	
	rgba8888_from_1bit(result, crisp_png_pix, crisp_png_w * crisp_png_h);
	
	*surf_w = crisp_png_w;
	*surf_h = crisp_png_h;
	*char_w = 8;
	*char_h = 16;
	
//	font_dropshadow(result, *surf_w, *surf_h);
	
	return result;
}


WOW_GUI_API_PREFIX
void
wowGui_tails(int tails)
{
	wowGui.tails = tails;
}


WOW_GUI_API_PREFIX
void
wowGui_italic(int italic_factor)
{
	wowGui.italic = italic_factor;
}

WOW_GUI_API_PREFIX
void
wowGui_wait_func(
	void *func(void *progress_float)
	, void die(const char *str)
	, void bind_events(void)
	, void bind_result(void)
	, wowGui_u32_t bind_ms(void)
	, struct wowGui_rect *progress_bar
)
{
	assert(func);
	assert(die);
	assert(bind_events);
	assert(bind_result);
	assert(bind_ms);
	
	float progress = 0;
	
#ifdef WOW_GUI_USE_PTHREAD
	pthread_t pt;
	if (pthread_create(&pt, 0, func, &progress))
		die("threading error");
	
	while (progress < 1)
	{
		/* wowGui_frame() must be called before you do any input */
		wowGui_frame();
		
		/* events */
		bind_events();
		
		/* end frame */
		wowGui_frame_end(bind_ms());
		
		/* display progress bar */
		wowGui.fill_rect(progress_bar, 0x000000FF);
		struct wowGui_rect update = *progress_bar;
		update.w *= progress;
		wowGui.fill_rect(&update, 0xFFFFFFFF);
		
		/* display */
		bind_result();
	}
	
	if (pthread_join(pt, 0))
		die("threading error");
#else
	func(&progress);
#endif
	
	/* clear out any junk thrown at it by the user */
	wowGui_dropped_file_flush();
}



/*
 *
 * wowGui_popup
 *
 */
#ifdef _WIN32

WOW_GUI_API_PREFIX
int
wowGui_popup(
	enum wowGui_popupIcon icon
	, enum wowGui_popupChoice buttons
	, const int initial
	, const char *title
	, const char *message
)
{
    void *wtitle = wow_utf8_to_wchar_die(title);
    void *wmessage = wow_utf8_to_wchar_die(message);
    int rval = 0;
    int defbutton = MB_DEFBUTTON1, type, winicon=0;
    switch(icon) {
        case WOWGUI_POPUP_ICON_ERR:
            winicon = MB_ICONERROR; break;
        case WOWGUI_POPUP_ICON_WARN:
            winicon = MB_ICONWARNING; break;
        case WOWGUI_POPUP_ICON_INFO:
            winicon = MB_ICONINFORMATION; break;
        case WOWGUI_POPUP_ICON_QUESTION:
            winicon = MB_ICONQUESTION; break;
    }
    switch(initial) {
        case WOWGUI_POPUP_NO:
            defbutton = MB_DEFBUTTON2;
            break;
        case WOWGUI_POPUP_CANCEL:
            if(buttons==(WOWGUI_POPUP_YES|WOWGUI_POPUP_NO|WOWGUI_POPUP_CANCEL))
                defbutton = MB_DEFBUTTON3;
            else
                defbutton = MB_DEFBUTTON2;
            break;
    }
    switch((int)buttons) {
        case WOWGUI_POPUP_YES|WOWGUI_POPUP_CANCEL:
            type = MB_OKCANCEL; break;
        case WOWGUI_POPUP_YES|WOWGUI_POPUP_NO:
            type = MB_YESNO; break;
        case WOWGUI_POPUP_YES|WOWGUI_POPUP_NO|WOWGUI_POPUP_CANCEL:
            type = MB_YESNOCANCEL; break;
        case WOWGUI_POPUP_YES: default:
            type = MB_OK; break;
    }
    switch( MessageBox(NULL, wmessage, wtitle, type | defbutton | winicon ) ) {
        case IDYES:
		case IDOK:
            rval = WOWGUI_POPUP_YES;
            break;
        case IDNO:
            rval = WOWGUI_POPUP_NO;
            break;
        case IDCANCEL:
            rval = WOWGUI_POPUP_CANCEL;
            break;
    }
    free(wtitle);
    free(wmessage);
    return rval;
}

#else // end _WIN32, start GTK fallback

#include <gtk/gtk.h>

static int __wowGuiPopup_rval; // the value returned by wowGui_popup()

void __wowGuiPopup_close(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

static void __wowGuiPopup_yes(GtkWidget *widget, gpointer data) {
	__wowGuiPopup_rval = WOWGUI_POPUP_YES;
	gtk_widget_destroy (GTK_WIDGET (data));
}

static void __wowGuiPopup_no(GtkWidget *widget, gpointer data) {
	__wowGuiPopup_rval = WOWGUI_POPUP_NO;
	gtk_widget_destroy (GTK_WIDGET (data));
}

static void __wowGuiPopup_cancel(GtkWidget *widget, gpointer data) {
	__wowGuiPopup_rval = WOWGUI_POPUP_CANCEL;
	gtk_widget_destroy (GTK_WIDGET (data));
}

WOW_GUI_API_PREFIX
int
wowGui_popup(
	enum wowGui_popupIcon icon
	, enum wowGui_popupChoice buttons
	, const int initial
	, const char *title
	, const char *message
)
{
	// TODO icons

	// initialize GTK
	gtk_init(NULL, NULL);

	__wowGuiPopup_rval = WOWGUI_POPUP_X;

	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *dialog_window;
	void (*func_yes)() = __wowGuiPopup_yes;
	void (*func_no)() = __wowGuiPopup_no;
	void (*func_cancel)() = __wowGuiPopup_cancel;

	// create dialog
	dialog_window = gtk_dialog_new();

	// hook window close signal
	gtk_signal_connect (GTK_OBJECT(dialog_window), "destroy",
					GTK_SIGNAL_FUNC (__wowGuiPopup_close),
					&dialog_window);

	// set title
	gtk_window_set_title(GTK_WINDOW (dialog_window), title);

	// border please
	gtk_container_border_width(GTK_CONTAINER (dialog_window), 5);

	// create a label with a message
	label = gtk_label_new(message);

	// label padding
	gtk_misc_set_padding(GTK_MISC (label), 10, 10);

	// add to dialog
	gtk_box_pack_start(GTK_BOX (GTK_DIALOG (dialog_window)->vbox),
						label, TRUE, TRUE, 0);

	// show label
	gtk_widget_show(label);

	// "Yes" button
	if( buttons & WOWGUI_POPUP_YES ) {
		// create
		if( buttons & WOWGUI_POPUP_NO ) // if a "No" button is present, "Yes"
			button = gtk_button_new_with_label("Yes");
		else // otherwise, "OK"
			button = gtk_button_new_with_label("OK");

		gtk_signal_connect(GTK_OBJECT (button), "clicked",
						GTK_SIGNAL_FUNC (func_yes),
						dialog_window);

		GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

		// add to dialog
		gtk_box_pack_start(GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
				  button, TRUE, TRUE, 0);

		// is this the default button
		if( initial == WOWGUI_POPUP_YES ) {
			gtk_widget_grab_default(button);
		}

		gtk_widget_show(button);
	}

	// "No" button
	if( buttons & WOWGUI_POPUP_NO ) {
		// create
		button = gtk_button_new_with_label("No");

		gtk_signal_connect(GTK_OBJECT (button), "clicked",
						GTK_SIGNAL_FUNC (func_no),
						dialog_window);

		GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

		// add to dialog
		gtk_box_pack_start(GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
				  button, TRUE, TRUE, 0);

		// is this the default button
		if( initial == WOWGUI_POPUP_NO ) {
			gtk_widget_grab_default(button);
		}

		gtk_widget_show(button);
	}

	// "Cancel" button
	if( buttons & WOWGUI_POPUP_CANCEL ) {
		// create
		button = gtk_button_new_with_label("Cancel");

		gtk_signal_connect(GTK_OBJECT (button), "clicked",
						GTK_SIGNAL_FUNC (func_cancel),
						dialog_window);

		GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

		// add to dialog
		gtk_box_pack_start(GTK_BOX (GTK_DIALOG (dialog_window)->action_area),
				  button, TRUE, TRUE, 0);

		// is this the default button
		if( initial == WOWGUI_POPUP_CANCEL ) {
			gtk_widget_grab_default(button);
		}

		gtk_widget_show(button);
	}

	// show dialog
	gtk_widget_show(dialog_window);

	// only this window can be used for now
	gtk_grab_add(dialog_window);

	// GTK main loop
	gtk_main();

	// user clicked X button
	if(__wowGuiPopup_rval == WOWGUI_POPUP_X) {
		if( buttons & WOWGUI_POPUP_CANCEL )
			__wowGuiPopup_rval = WOWGUI_POPUP_CANCEL;
		else if( buttons & WOWGUI_POPUP_NO )
			__wowGuiPopup_rval = WOWGUI_POPUP_NO;
		else
			__wowGuiPopup_rval = WOWGUI_POPUP_YES;
	}

	return __wowGuiPopup_rval;
}

#endif

WOW_GUI_API_PREFIX
int
wowGui_popupf(
	enum wowGui_popupIcon icon
	, enum wowGui_popupChoice buttons
	, const int initial
	, const char *title
	, const char *fmt
	, ...
)
{
	char message[4096];
	va_list args;
	va_start(args, fmt);
	vsnprintf(message, sizeof(message), fmt, args);
	va_end(args);
	return wowGui_popup(icon, buttons, initial, title, message);
}
/*
 *
 * end wowGui_popup
 *
 */


/*
 *
 * generic file dropper magic
 *
 */

static char *__wowGui_fileDropper_optional = "(optional)";

static int __wowGui_fileDropper_empty(const char *str)
{
	return !str || (str == __wowGui_fileDropper_optional) || !strlen(str);
}

WOW_GUI_API_PREFIX
int
wowGui_fileDropper_filenameIsEmpty(const struct wowGui_fileDropper *v)
{
	assert(v);
	
	return __wowGui_fileDropper_empty(v->filename);
}

WOW_GUI_API_PREFIX
char *
wowGui_askFilename(
	const char *ext
	, const char *path
	, const char isCreateMode
)
{
	nfdchar_t *ofn = 0;
	nfdresult_t result;
	
	if (isCreateMode)
		result = NFD_SaveDialog(ext, path, &ofn);
	else
		result = NFD_OpenDialog(ext, path, &ofn);
	
	if (result != NFD_OKAY)
	{
		return 0;
	}
	
	return ofn;
}

static char *__wowGui_fileDropper_invalidExtension(
	const char *str
	, const char *list
)
{
	char buf[32];
	char *tok;
	strcpy(buf, list);
	char *ext = strrchr(str, '.');
	const char *slash = strrchr(str, '/');
	if (!slash) slash = strrchr(str, '\\');
	if (!slash) slash = str;
	else ++slash;
	
	/* unknown extension */
	if (!ext || ext <= slash)
		return "unknown";
	
	/* test every extension in list */
	for (tok = strtok(buf, ","), ++ext; tok; tok = strtok(0, ","))
		/* return 0 on match */
		if (!strcasecmp(tok, ext))
			return 0;
	
	/* no match found, return extension */
	return ext;
}

static inline int
__wowGui_fileDropper_assert_extension(const char *dfn, const char *ext)
{
	char *invalid;
	if ((invalid = __wowGui_fileDropper_invalidExtension(dfn, ext)))
	{
		char err[256];
		snprintf(
			err
			, sizeof(err)
			, "Invalid extension (expected '%s', got '%s')"
			, ext, invalid
		);
		wowGui_popup(
			WOWGUI_POPUP_ICON_WARN
			, WOWGUI_POPUP_OK
			, WOWGUI_POPUP_OK
			, "Error"
			, err
		);
		return 1;
	}
	return 0;
}

/* generic file dropper */
WOW_GUI_API_PREFIX
int
wowGui_fileDropper(struct wowGui_fileDropper *dropper)
{
	const char *ext;
	char **str;
	char *x;
	int changed = 0;
	int tailsOld = wowGui.tails;
	
	assert(dropper);
	
	ext = dropper->extension;
	str = &dropper->filename;
	
	if (!*str)
	{
		if (dropper->isOptional)
			*str = __wowGui_fileDropper_optional;
		else
			*str = "";
	}
	
	wowGui_column_width(dropper->labelWidth);
	wowGui_label(dropper->label);
	wowGui_column_width(dropper->filenameWidth);
	wowGui_tails(1);
	if (wowGui_clickable(*str)
		&& (x = wowGui_askFilename(ext, 0, dropper->isCreateMode))
	)
	{
		/* if invalid extension, discard */
		if (__wowGui_fileDropper_assert_extension(x, ext))
		{
			free(x);
			changed = 0;
			goto done;
		}
		
		if (x)
		{
			if (!__wowGui_fileDropper_empty(*str))
				free(*str);
			*str = x;
			changed = 1;
		}
	}
	else if (wowGui_dropped_file_last())
	{
		char *dfn = wowGui_dropped_file_name();
		
		/* if invalid extension, discard */
		if (__wowGui_fileDropper_assert_extension(dfn, ext))
		{
			changed = 0;
			goto done;
		}
		
		/* ask user is it okay to overwrite (if it exists) */
		if (dropper->isCreateMode
			&& wowGui_popupf(
				WOWGUI_POPUP_ICON_WARN
				, WOWGUI_POPUP_YES | WOWGUI_POPUP_NO
				, WOWGUI_POPUP_NO
				, "This file already exists!"
				, "Is it okay to overwrite '%s'?"
				, dfn
			) == WOWGUI_POPUP_NO
		)
		{
			changed = 0;
			goto done;
		}
		
		if (!__wowGui_fileDropper_empty(*str))
			free(*str);
		*str = strdup(dfn);
		changed = 1;
	}
	wowGui_column_width(8 * 3);
	if (wowGui_button("x"))
	{
		if (!__wowGui_fileDropper_empty(*str))
			free(*str);
		*str = "";
		if (dropper->isOptional)
			*str = __wowGui_fileDropper_optional;
		changed = 1;
	}
	
done:
	wowGui_tails(tailsOld);
	return changed;
}


/*
 *
 * end generic file dropper magic
 *
 */



/*
 *
 * binding: MiniFB
 *
 */

#ifdef WOW_GUI_BINDING_MINIFB

#ifdef WOW_GUI_EXTERN_MINIFB
	#include <MiniFB.h>
#else
	#include "minifb/include/MiniFB.h"
	#include "minifb/src/MiniFB_internal.h"
	#ifdef _WIN32
		#include "minifb/src/windows/WinMiniFB.c"
	#elif defined(__linux__)
		#include "minifb/src/x11/X11MiniFB.c"
	#endif
	#include "minifb/src/MiniFB_timer.c"
	#include "minifb/src/MiniFB_linux.c"
	#include "minifb/src/MiniFB_common.c"
	#include "minifb/src/MiniFB_internal.c"
#endif

/* this is for keeping CPU usage down (framerate regulation) */
//static const int frame_delay = 50; /* milliseconds */
static int __wowGui_mfbFramerate = 10; /* run at 60 fps */

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <time.h>

#define __wowGuiMfb_kUnused(var)	(void) var

#define COLORCORRECT_ARGB 1

#define COLORCORRECT  COLORCORRECT_ARGB

#define WOWMFB_BlitSurface_AlphaBlend        WOWMFB_BlitSurface
#define WOWMFB_BlitSurface_AlphaTest         WOWMFB_BlitSurface
#define WOWMFB_BlitSurface_ColorTest         WOWMFB_BlitSurface
#define WOWMFB_BlitSurfaceScaled_AlphaBlend  WOWMFB_BlitSurfaceScaled
#define WOWMFB_BlitSurfaceScaled_AlphaTest   WOWMFB_BlitSurfaceScaled
#define WOWMFB_BlitSurfaceScaled_ColorTest   WOWMFB_BlitSurfaceScaled

static int __wowGui_redraw = 1;

static
inline
uint32_t
fix_color(uint32_t color)
{
#if (COLORCORRECT == COLORCORRECT_NONE)
#define SHIFT_R  24
#define SHIFT_G  16
#define SHIFT_B   8
#define SHIFT_A   0
	return color;
#else
	unsigned char r = color >> 24;
	unsigned char g = color >> 16;
	unsigned char b = color >>  8;
	unsigned char a = color;
	#if (COLORCORRECT == COLORCORRECT_ARGB)
		#define SHIFT_R  16
		#define SHIFT_G   8
		#define SHIFT_B   0
		#define SHIFT_A  24
	#endif
	return (a << SHIFT_A) | (r << SHIFT_R) | (g << SHIFT_G) | (b << SHIFT_B);
#endif
#define CHANNEL_R(x) ((x >> SHIFT_R) & 0xFF)
#define CHANNEL_G(x) ((x >> SHIFT_G) & 0xFF)
#define CHANNEL_B(x) ((x >> SHIFT_B) & 0xFF)
#define CHANNEL_A(x) ((x >> SHIFT_A) & 0xFF)
}

static
uint32_t
blendfunc_alphablend(uint32_t dst, uint32_t src)
{
	if (CHANNEL_A(src))
	{
		int sR = CHANNEL_R(src);
		int sG = CHANNEL_G(src);
		int sB = CHANNEL_B(src);
		int sA = CHANNEL_A(src);
		
		int dR = CHANNEL_R(dst);
		int dG = CHANNEL_G(dst);
		int dB = CHANNEL_B(dst);
		
		dR = ((sR * sA) + (dR * (255 - sA))) / 255;
		dG = ((sG * sA) + (dG * (255 - sA))) / 255;
		dB = ((sB * sA) + (dB * (255 - sA))) / 255;
		return (0xFF << SHIFT_A) | (dR << SHIFT_R) | (dG << SHIFT_G) | (dB << SHIFT_B);
	}
	
	return dst;
}

static
uint32_t
blendfunc_alphatest(uint32_t dst, uint32_t src)
{
	if (CHANNEL_A(src))
		return src;
	
	return dst;
}

static
uint32_t
blendfunc_colortest(uint32_t dst, uint32_t src)
{
	if (src)
		return src;
	
	return dst;
}

static
uint32_t
blendfunc_noblend(uint32_t dst, uint32_t src)
{
	return src;
}

static
void
fix_color_array(void *_pix, unsigned long num)
{
	uint32_t *pix = _pix;
	while (num--)
	{
		*pix = fix_color(*pix);
		 ++pix;
	}
}

struct {
	int x, y;
} __wowGuiMfb_mouse = {0};

typedef struct {
	int x, y, w, h;
} WOWMFB_Rect;

typedef struct {
	uint32_t format;
	//WOWMFB_Palette *palette;
	uint8_t BitsPerPixel, BytesPerPixel;
	uint32_t Rmask, Gmask, Bmask, Amask;
	uint8_t Rloss, Gloss, Bloss, Aloss, Rshift, Gshift, Bshift, Ashift;
	int refcount;
	//WOWMFB_PixelFormat *next;
} WOWMFB_PixelFormat;

typedef struct {
	WOWMFB_PixelFormat *format;
	int w, h;
	int pitch;
	void *pixels;
	void *userdata;
	int locked;
	void *lock_data;
	WOWMFB_Rect clip_rect;
	//WOWMFB_BlitMap *map;
	int refcount;
	int comp;
	int index;
} WOWMFB_Surface;

static struct mfb_window *__wowGui_mfbWindow = 0;
static int __wowGui_mfbActive = 1;

static int __wowGui_mfbWinW;
static int __wowGui_mfbWinH;
static int __wowGui_mfbRunning = 1;
static WOWMFB_Surface *g_buffer = 0;

static
void *
memdup_safe(void *data, unsigned int size)
{
	void *out = wow_malloc_die(size);
	memcpy(out, data, size);
	return out;
}


static
char
keycode_to_char(int sym, mfb_key_mod mod)
{
	const char shift_lut[] = {
		[KB_KEY_GRAVE_ACCENT] = '~',
		[KB_KEY_1] = '!',
		[KB_KEY_2] = '@',
		[KB_KEY_3] = '#',
		[KB_KEY_4] = '$',
		[KB_KEY_5] = '%',
		[KB_KEY_6] = '^',
		[KB_KEY_7] = '&',
		[KB_KEY_8] = '*',
		[KB_KEY_9] = '(',
		[KB_KEY_0] = ')',
		[KB_KEY_MINUS] = '_',
		[KB_KEY_EQUAL] = '+',
		[KB_KEY_LEFT_BRACKET] = '{',
		[KB_KEY_RIGHT_BRACKET] = '}',
		[KB_KEY_BACKSLASH] = '|',
		[KB_KEY_SEMICOLON] = ':',
		[KB_KEY_APOSTROPHE] = '"',
		[KB_KEY_COMMA] = '<',
		[KB_KEY_PERIOD] = '>',
		[KB_KEY_SLASH] = '?',
		
		[KB_KEY_SPACE] = ' ',
		[KB_KEY_A] = 'A',
		[KB_KEY_B] = 'B',
		[KB_KEY_C] = 'C',
		[KB_KEY_D] = 'D',
		[KB_KEY_E] = 'E',
		[KB_KEY_F] = 'F',
		[KB_KEY_G] = 'G',
		[KB_KEY_H] = 'H',
		[KB_KEY_I] = 'I',
		[KB_KEY_J] = 'J',
		[KB_KEY_K] = 'K',
		[KB_KEY_L] = 'L',
		[KB_KEY_M] = 'M',
		[KB_KEY_N] = 'N',
		[KB_KEY_O] = 'O',
		[KB_KEY_P] = 'P',
		[KB_KEY_Q] = 'Q',
		[KB_KEY_R] = 'R',
		[KB_KEY_S] = 'S',
		[KB_KEY_T] = 'T',
		[KB_KEY_U] = 'U',
		[KB_KEY_V] = 'V',
		[KB_KEY_W] = 'W',
		[KB_KEY_X] = 'X',
		[KB_KEY_Y] = 'Y',
		[KB_KEY_Z] = 'Z',
		
		[KB_KEY_KP_1] = '1',
		[KB_KEY_KP_2] = '2',
		[KB_KEY_KP_3] = '3',
		[KB_KEY_KP_4] = '4',
		[KB_KEY_KP_5] = '5',
		[KB_KEY_KP_6] = '6',
		[KB_KEY_KP_7] = '7',
		[KB_KEY_KP_8] = '8',
		[KB_KEY_KP_9] = '9',
		[KB_KEY_KP_0] = '0',
		[KB_KEY_KP_DECIMAL] = '.',
		[KB_KEY_KP_DIVIDE] = '/',
		[KB_KEY_KP_MULTIPLY] = '*',
		[KB_KEY_KP_SUBTRACT] = '-',
		[KB_KEY_KP_ADD] = '+',
		[KB_KEY_KP_EQUAL] = '=',
		
		[KB_KEY_LAST] = '\0'
	};
	const char char_lut[] = {
		[KB_KEY_GRAVE_ACCENT] = '`',
		[KB_KEY_1] = '1',
		[KB_KEY_2] = '2',
		[KB_KEY_3] = '3',
		[KB_KEY_4] = '4',
		[KB_KEY_5] = '5',
		[KB_KEY_6] = '6',
		[KB_KEY_7] = '7',
		[KB_KEY_8] = '8',
		[KB_KEY_9] = '9',
		[KB_KEY_0] = '0',
		[KB_KEY_MINUS] = '-',
		[KB_KEY_EQUAL] = '=',
		[KB_KEY_LEFT_BRACKET] = '[',
		[KB_KEY_RIGHT_BRACKET] = ']',
		[KB_KEY_BACKSLASH] = '\\',
		[KB_KEY_SEMICOLON] = ';',
		[KB_KEY_APOSTROPHE] = '\'',
		[KB_KEY_COMMA] = ',',
		[KB_KEY_PERIOD] = '.',
		[KB_KEY_SLASH] = '?',
		
		[KB_KEY_SPACE] = ' ',
		[KB_KEY_A] = 'a',
		[KB_KEY_B] = 'b',
		[KB_KEY_C] = 'c',
		[KB_KEY_D] = 'd',
		[KB_KEY_E] = 'e',
		[KB_KEY_F] = 'f',
		[KB_KEY_G] = 'g',
		[KB_KEY_H] = 'h',
		[KB_KEY_I] = 'i',
		[KB_KEY_J] = 'j',
		[KB_KEY_K] = 'k',
		[KB_KEY_L] = 'l',
		[KB_KEY_M] = 'm',
		[KB_KEY_N] = 'n',
		[KB_KEY_O] = 'o',
		[KB_KEY_P] = 'p',
		[KB_KEY_Q] = 'q',
		[KB_KEY_R] = 'r',
		[KB_KEY_S] = 's',
		[KB_KEY_T] = 't',
		[KB_KEY_U] = 'u',
		[KB_KEY_V] = 'v',
		[KB_KEY_W] = 'w',
		[KB_KEY_X] = 'x',
		[KB_KEY_Y] = 'y',
		[KB_KEY_Z] = 'z',
		
		[KB_KEY_KP_1] = '1',
		[KB_KEY_KP_2] = '2',
		[KB_KEY_KP_3] = '3',
		[KB_KEY_KP_4] = '4',
		[KB_KEY_KP_5] = '5',
		[KB_KEY_KP_6] = '6',
		[KB_KEY_KP_7] = '7',
		[KB_KEY_KP_8] = '8',
		[KB_KEY_KP_9] = '9',
		[KB_KEY_KP_0] = '0',
		[KB_KEY_KP_DECIMAL] = '.',
		[KB_KEY_KP_DIVIDE] = '/',
		[KB_KEY_KP_MULTIPLY] = '*',
		[KB_KEY_KP_SUBTRACT] = '-',
		[KB_KEY_KP_ADD] = '+',
		[KB_KEY_KP_EQUAL] = '=',
		
		[KB_KEY_LAST] = '\0'
	};
	
	if (mod & KB_MOD_SHIFT)
		return shift_lut[sym];
	
	else
		return char_lut[sym];
	
	return 0;
}

void cb_active(struct mfb_window *window, bool isActive) {
#if 0
	const char *window_title = "";
	if(window) {
		window_title = (const char *) mfb_get_user_data(window);
	}
	fprintf(stdout, "%s > active: %d\n", window_title, isActive);
#endif
	__wowGui_mfbActive = isActive;
	__wowGui_redraw = 1;
}

void cb_resize(struct mfb_window *window, int width, int height) {
	uint32_t x = 0;
	uint32_t y = 0;
#if 0
	const char *window_title = "";
	if(window) {
		window_title = (const char *) mfb_get_user_data(window);
	}

	fprintf(stdout, "%s > resize: %d, %d\n", window_title, width, height);
#endif
	if(width > __wowGui_mfbWinW) {
		x = (width - __wowGui_mfbWinW) >> 1;
		width = __wowGui_mfbWinW;
	}
	if(height > __wowGui_mfbWinH) {
		y = (height - __wowGui_mfbWinH) >> 1;
		height = __wowGui_mfbWinH;
	}
	mfb_set_viewport(window, x, y, width, height);
	__wowGui_redraw = 1;
}

void cb_keyboard(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool isPressed)
{
#if 0
	const char *window_title = "";
	if(window) {
		window_title = (const char *) mfb_get_user_data(window);
	}
	fprintf(stdout, "%s > keyboard: key: %s (pressed: %d) [mfb_key_mod: %x]\n", window_title, mfb_get_key_name(key), isPressed, mod);
#endif
	__wowGui_redraw = 1;
	
	if (!isPressed)
		return;
	
#if 0	/* WOW_GUI_DO_NOT_RESET_KEYBOARD_BUFFER */
	wowGuip.keyboard.pos = wowGuip.keyboard.buf;
	wowGuip.keyboard.remaining = wowGuip.keyboard.sz;
	wowGuip.keyboard.buf[0] = '\0';
#endif
	
	//static unsigned int last_ms = 0;
	
	int key_control = 0;
	char key_other;
	
	/* keyboard buffer */
	
	switch (key)
	{
		case KB_KEY_UP:
			key_control = WOWGUI_KBC_UP;
			break;
		
		case KB_KEY_DOWN:
			key_control = WOWGUI_KBC_DOWN;
			break;
		
		case KB_KEY_LEFT:
			key_control = WOWGUI_KBC_LEFT;
			break;
		
		case KB_KEY_RIGHT:
			key_control = WOWGUI_KBC_RIGHT;
			break;
		
		case KB_KEY_ENTER:
		case KB_KEY_KP_ENTER:
			key_control = WOWGUI_KBC_ENTER;
			break;
		
		case KB_KEY_ESCAPE:
			key_control = WOWGUI_KBC_ESCAPE;
			break;
					
		case KB_KEY_HOME:
			key_control = WOWGUI_KBC_HOME;
			break;
		
		case KB_KEY_END:
			key_control = WOWGUI_KBC_END;
			break;
		
		case KB_KEY_DELETE:
			key_control = WOWGUI_KBC_DELETE;
			break;
		
		case KB_KEY_BACKSPACE:
			key_control = WOWGUI_KBC_BACKSPACE;
			break;
					
		default:
			key_other = keycode_to_char(key, mod);
			break;
	}
	
	if (key_control)
		wowGui_keyboard_control(key_control);
				
	else if (key_other)
	{
		/*fprintf(stderr, "key = '%c'\n", key_other);*/
		wowGui_keyboard(&key_other, 1);
	}
	
#if 0
	if(key == KB_KEY_ESCAPE) {
		mfb_close(window);
	}	
	if (key == KB_KEY_UP)
		fprintf(stderr, "kb_key_up\n");
#endif
}

void cb_char_input(struct mfb_window *window, unsigned int charCode) {
#if 0
	const char *window_title = "";
	if(window) {
		window_title = (const char *) mfb_get_user_data(window);
	}
	fprintf(stdout, "%s > charCode: %d\n", window_title, charCode);
#endif
	__wowGui_redraw = 1;
}

void cb_mouse_btn(struct mfb_window *window, mfb_mouse_button button, mfb_key_mod mod, bool isPressed) {
#if 0
	const char *window_title = "";
	if(window) {
		window_title = (const char *) mfb_get_user_data(window);
	}
#endif
	int left, middle, right;
#ifdef _WIN32
	left = 1;
	middle = 3;
	right = 2;
#else
	left = 1;
	middle = 2;
	right = 3;
#endif
	wowGui_mouse_click(
		__wowGuiMfb_mouse.x
		, __wowGuiMfb_mouse.y
		, (
			(button == left)   ? WOWGUI_MOUSEBUTTON_LEFT :
			(button == middle) ? WOWGUI_MOUSEBUTTON_MIDDLE :
			(button == right)  ? WOWGUI_MOUSEBUTTON_RIGHT : 0
		) * isPressed
	);
	__wowGui_redraw = 2;
//	fprintf(stdout, "%s > mouse_btn: button: %d (pressed: %d) [mfb_key_mod: %x]\n", window_title, button, isPressed, mod);
}

void cb_mouse_move(struct mfb_window *window, int x, int y) {
	__wowGuiMfb_kUnused(window);
	__wowGuiMfb_mouse.x = x;
	__wowGuiMfb_mouse.y = y;
	wowGui_mouse_move(x, y);
	__wowGuiMfb_kUnused(x);
	__wowGuiMfb_kUnused(y);
	__wowGui_redraw = 1;
	// const char *window_title = "";
	// if(window) {
	//	 window_t(const char *) itle = mfb_get_user_data(window);
	// }
	//fprintf(stdout, "%s > mouse_move: %d, %d\n", window_title, x, y);
}

void cb_file_drag(struct mfb_window *window, int x, int y) {
	__wowGuiMfb_kUnused(window);
	__wowGuiMfb_mouse.x = x;
	__wowGuiMfb_mouse.y = y;
	wowGui_file_drag(x, y);
	__wowGuiMfb_kUnused(x);
	__wowGuiMfb_kUnused(y);
	__wowGui_redraw = 1;
}

void cb_file_drop(struct mfb_window *window, char *file_list, int x, int y)
{
	__wowGuiMfb_kUnused(window);
//	fwprintf(stderr, L"file_list = '%s'\n", wowGui_utf8_to_wchar(file_list));
	wowGui_file_drop(file_list, x, y);
	__wowGui_redraw = 1;
}

void cb_mouse_scroll(struct mfb_window *window, mfb_key_mod mod, float deltaX, float deltaY) {
	__wowGuiMfb_kUnused(window);
#if 0
	const char *window_title = "";
	if(window) {
		window_title = (const char *) mfb_get_user_data(window);
	}
//	fprintf(stdout, "%s > mouse_scroll: x: %f, y: %f [mfb_key_mod: %x]\n", window_title, deltaX, deltaY, mod);
#endif
	wowGui_mouse_scroll_x(deltaX);
	wowGui_mouse_scroll_y(deltaY);
	__wowGui_redraw = 2;
}


/* WOWMFB_surf.h */
#define SDL_FLIP_HORIZONTAL 1
#define SDL_FLIP_VERTICAL 2
#define WOWMFB_MUSTLOCK(WOWMFBml) 0
#define WOWMFB_LockSurface(WOWMFBls) WOWMFB_void()
#define WOWMFB_UnlockSurface(WOWMFBls) WOWMFB_void()
#include <stdint.h>

#define WOWMFBMIN(minA,minB) (((minA)<(minB))?(minA):(minB))

void WOWMFB_void(void);

WOWMFB_Surface *WOWMFB_LoadBMP(const char *filename);
WOWMFB_Surface *WOWMFB_IMG_Load(const char *filename);
WOWMFB_Surface *WOWMFB_CreateRGBSurfaceFrom(void* pixels, int w, int h, int depth, int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
WOWMFB_Surface *WOWMFB_CreateRGBSurface(uint32_t flags, int w, int h, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask);
void WOWMFB_FreeSurface(WOWMFB_Surface *surf);
void WOWMFB_SaveBMP(WOWMFB_Surface *s, const char *name);
static inline __attribute__((always_inline)) void
WOWMFB_BlitSurface(WOWMFB_Surface *src, WOWMFB_Rect *srcrect, WOWMFB_Surface *dst, WOWMFB_Rect *dstrect, uint32_t blendfunc(uint32_t dst, uint32_t src));
WOWMFB_Surface *WOWMFB_IMG_Load_POT(const char *filename);
uint32_t get_pixel32( WOWMFB_Surface *surface, uint16_t x, uint16_t y );
void put_pixel32( WOWMFB_Surface *surface, uint16_t x, uint16_t y, uint32_t pixel );
/* end WOWMFB_surf.h */

/* WOWMFB_surf.c */

static
void
WOWMFB_ClearPixels(void *_pix, unsigned int num, uint32_t rgba)
{
	uint32_t *pix = _pix;
	while (num)
	{
		*pix = rgba;
		++pix;
		num--;
	}
}

static
void
WOWMFB_FillRect(WOWMFB_Surface *dst, uint32_t rgba, WOWMFB_Rect *dstrect)
{
	WOWMFB_Rect dstrectC={0,0,dst->w,dst->h};
	if(dstrect!=NULL)
		dstrectC=*dstrect;
	if(dstrectC.x+dstrectC.w>dst->w) dstrectC.w=dst->w-dstrectC.x;
	if(dstrectC.y+dstrectC.h>dst->h) dstrectC.h=dst->w-dstrectC.y;
	//printf("dst %d %d %d %d\n",srcrectC.x,srcrectC.y,srcrectC.w,srcrectC.h);
	int y;
	int BPP = dst->format->BytesPerPixel;
	unsigned char *dstP = dst->pixels;
	rgba = fix_color(rgba);
	for(y=0;y<dstrectC.h;y++)
	{
		WOWMFB_ClearPixels(
			dstP + dst->pitch * dstrectC.y + dstrectC.x * BPP + dst->pitch * y,
			dstrectC.w,
			rgba
		);
	}
}

void WOWMFB_void(void) {
}

static WOWMFB_Surface *iWOWMFB_NewSurface(void) {
	WOWMFB_Surface *o=(WOWMFB_Surface*)malloc(sizeof(WOWMFB_Surface));
	o->pixels=NULL;
	o->format=(WOWMFB_PixelFormat*)malloc(sizeof(WOWMFB_PixelFormat));
	o->format->BytesPerPixel=4;
	o->format->BitsPerPixel=32;
	o->format->Amask=0xFF000000; o->format->Rloss=0; o->format->Ashift=24;
	o->format->Bmask=0xFF0000; o->format->Gloss=0; o->format->Bshift=16;
	o->format->Gmask=0xFF00; o->format->Bloss=0; o->format->Gshift=8;
	o->format->Rmask=0xFF; o->format->Aloss=0; o->format->Rshift=0;
	return o;
}

#if 0
static void iWOWMFB_SurfaceSetProperties(WOWMFB_Surface *s) {
	//printf("comp: %d\n",s->comp);
	s->format->BytesPerPixel=s->comp; // STBI_rgb = 3, STBI_rgb_alpha = 4
	s->pitch=s->format->BytesPerPixel*s->w;
}
#endif

WOWMFB_Surface *WOWMFB_CreateRGBSurface(uint32_t flags, int w, int h, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
	WOWMFB_Surface *o=iWOWMFB_NewSurface();
	o->pixels=(uint8_t*)malloc(w*h*o->format->BytesPerPixel);
	o->pitch=o->format->BytesPerPixel*w;
	o->w=w;
	o->h=h;
	return o;
}

WOWMFB_Surface *WOWMFB_CreateRGBSurfaceFrom(void* pixels, int w, int h, int depth, int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
	WOWMFB_Surface *o=iWOWMFB_NewSurface();
	o->pixels=pixels;//(uint8_t*)malloc(w*h*o->format->BytesPerPixel);
	o->pitch=o->format->BytesPerPixel*w;
	o->w=w;
	o->h=h;
	return o;
}

void WOWMFB_FreeSurface(WOWMFB_Surface *s) {
	if(s!=NULL) {
		if(s->pixels!=NULL) {
			free(s->pixels);
			s->pixels=NULL;
		}
		if(s->format!=NULL) {
			free(s->format);
			s->format=NULL;
		}
		free(s);
	}
}

static
void
WOWMFB_BlitSurfaceScaled(
	WOWMFB_Surface *src
	, WOWMFB_Rect *srcrect
	, WOWMFB_Surface *dst
	, WOWMFB_Rect *dstrect
	, uint32_t blendfunc(uint32_t dst, uint32_t src)
	, int scale
)
{
	WOWMFB_Rect srcrectC={0,0,src->w,src->h}, dstrectC={0,0,dst->w,dst->h};
	if(srcrect!=NULL) srcrectC=*srcrect;
	if(dstrect!=NULL) dstrectC=*dstrect;
	if(srcrectC.x+srcrectC.w>src->w) srcrectC.w=src->w-srcrectC.x;
	if(dstrectC.x+dstrectC.w>dst->w) dstrectC.w=dst->w-dstrectC.x;
	if(srcrectC.y+srcrectC.h>src->h) srcrectC.h=src->w-srcrectC.y;
	if(dstrectC.y+dstrectC.h>dst->h) dstrectC.h=dst->w-dstrectC.y;
	if(!srcrectC.w || !dstrectC.w) return;
	//printf("dst %d %d %d %d\n",srcrectC.x,srcrectC.y,srcrectC.w,srcrectC.h);
	int y;
	int BPP = src->format->BytesPerPixel;
	unsigned char *dstP = dst->pixels;
	unsigned char *srcP = src->pixels;
	int italic_offset = wowGui.italic;
	for(y=0;y<WOWMFBMIN(srcrectC.h,dstrectC.h);y++)
	{
		if (wowGui.italic)
		{
			if (!italic_offset)
			{
				dstP += BPP;
				italic_offset = wowGui.italic;
			}
			italic_offset -= 1;
		}
		uint32_t *D = (uint32_t*)(dstP + dst->pitch * dstrectC.y + dstrectC.x * BPP + dst->pitch * y * scale);
		uint32_t *S = (uint32_t*)(srcP + src->pitch * srcrectC.y + srcrectC.x * BPP + src->pitch * y);
		uint32_t *OD = D;
		int x;
		/* run blendfunc on each pixel */
		for (x = 0; x < srcrectC.w; ++x)
		{
			uint32_t x = blendfunc(*D, *S);
			int i;
			for (i=0; i < scale; ++i)
				D[i] = x;
			++S;
			D += scale;
		}
		if (scale > 1)
		{
			int i;
			for (i=1 ; i < scale; ++i)
			{
				memcpy(
					((unsigned char*)OD) + dst->pitch * i
					, OD
					, srcrectC.w * BPP * scale
				);
			}
		}
	}
	#undef BPP
}


static
void
WOWMFB_BlitSurface(
	WOWMFB_Surface *src
	, WOWMFB_Rect *srcrect
	, WOWMFB_Surface *dst
	, WOWMFB_Rect *dstrect
	, uint32_t blendfunc(uint32_t dst, uint32_t src)
)
{
	WOWMFB_BlitSurfaceScaled(src, srcrect, dst, dstrect, blendfunc, 1);
}

/*WOWMFB_Surface *WOWMFB_LoadBMP(const char *filename) {
	WOWMFB_Surface *out=iWOWMFB_NewSurface();
	out->pixels=stbi_load(filename,&out->w,&out->h,&out->comp,STBI_rgb_alpha);
	out->comp=STBI_rgb_alpha; // because &out->comp will set what stbi_load *would* have used, so force 4
	iWOWMFB_SurfaceSetProperties(out);
	//printf("loaded \"%s\": %d x %d\n",filename,out->w,out->h);
	return out;
}*/

void WOWMFB_SaveBMP(WOWMFB_Surface *s, const char *name) {
	/*stbi_write_png(name,s->w,s->h,4,s->pixels,0);
	return;*/
    unsigned char header[54] = {
        0x42, 0x4D, 0x22, 0x69, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00,
        0x00, 0x00, 0x28, 0x00, 0x00, 0x00, ((s->w))&0xFF, (s->w>>8)&0xFF, 0x00, 0x00, ((s->h))&0xFF, (s->h>>8)&0xFF,
        0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x68,
        0x02, 0x00, 0x74, 0x12, 0x00, 0x00, 0x74, 0x12, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    FILE *a;
    a=fopen(name,"wb");
    assert(a);
    fwrite(header,1,54,a);
    unsigned int x, y;
    for(y=0;y<s->h;y++) {
        for(x=0;x<s->w;x++) {
            uint32_t pixel=get_pixel32(s,x,(s->h-1)-y);
            fputc((pixel>>16)&0xFF,a); // G
            fputc((pixel>>8)&0xFF,a); // R
            fputc((pixel)&0xFF,a); // B
            fputc((pixel>>24)&0xFF,a); // alpha
        }
    }
    fclose(a);
}

uint32_t get_pixel32( WOWMFB_Surface *surface, uint16_t x, uint16_t y )
{
    uint32_t *pixels = (uint32_t *)surface->pixels;
    return pixels[ ( y * surface->w ) + x ];
}

void put_pixel32( WOWMFB_Surface *surface, uint16_t x, uint16_t y, uint32_t pixel )
{
    uint32_t *pixels = (uint32_t *)surface->pixels;
    pixels[ ( y * surface->w ) + x ] = pixel;
}
/* end WOWMFB_surf.c */


static
void fill_rect(struct wowGui_rect *rect, unsigned int color)
{
	/* early return condition */
	if (rect->w <= 0 || rect->h <= 0)
		return;
	
	WOWMFB_Rect r = {rect->x, rect->y, rect->w, rect->h};
	
	WOWMFB_FillRect(g_buffer, color, &r);
	
	#if 0
	TODO
	SDL_Rect r = {rect->x, rect->y, rect->w, rect->h};
	SDL_SetRenderDrawColor(renderer, color>>24, color>>16, color>>8, color&0xFF);
	SDL_RenderFillRect(renderer, &r);
	#endif
}


static
void
image_free(struct wowGui_image *img)
{
	if (!img)
		return;
	
	if (!img->udata)
		return;
	
	WOWMFB_Surface *surf = img->udata;
	
	WOWMFB_FreeSurface(surf);
	free(img);
}


static
void
image_color(struct wowGui_image *img, wowGui_u32_t rgba8888)
{
	if (!img)
		return;
	
	WOWMFB_Surface *surf = img->udata;
	
	if (!surf)
		return;
	
	#if 0
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	
	r = rgba8888 >> 24;
	g = rgba8888 >> 16;
	b = rgba8888 >>  8;
	a = rgba8888;
	
	TODO
	SDL_SetTextureColorMod(tex, r, g, b);
	SDL_SetTextureAlphaMod(tex, a);
	#endif
}


static
void
image_draw(
	struct wowGui_image *img
	, struct wowGui_rect *src
	, struct wowGui_rect *dst
)
{
	WOWMFB_Surface *surf = img->udata;
	WOWMFB_Rect srcRect = {src->x, src->y, src->w, src->h};
	WOWMFB_Rect dstRect = {dst->x, dst->y, dst->w, dst->h};
	
//	fprintf(stderr, "srcRect = {%d, %d, %d, %d}\n", src->x, src->y, src->w, src->h);
//	fprintf(stderr, "dstRect = {%d, %d, %d, %d}\n", dst->x, dst->y, dst->w, dst->h);
//	fprintf(stderr, "dstBuf  = %d x %d\n", g_buffer->w, g_buffer->h);
	/* this will only ever be used for fonts, so use the fastest blendfunc w/ alpha */
	WOWMFB_BlitSurface(surf, &srcRect, g_buffer, &dstRect, blendfunc_colortest);
}


static
struct wowGui_image *
image_new(unsigned char *rgba8888, int w, int h)
{
	struct wowGui_image *sprite;
	WOWMFB_Surface *surf;
	uint32_t rmask, gmask, bmask, amask;
	
	/* no pixels provided, so nothing to be done */
	if (!rgba8888 || w <= 0 || h <= 0)
		return 0;

	/* SDL interprets each pixel as a 32-bit number, so our masks
	   must depend on the endianness (byte order) of the machine */
#if 1//SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	
	/* create surface from pixels */
	surf =
	WOWMFB_CreateRGBSurfaceFrom(
		memdup_safe(rgba8888, w * h * 4)
		, w, h
		, 32
		, w * 4
		, rmask, gmask, bmask, amask
	);
	if (!surf)
		return 0;
	fix_color_array(surf->pixels, w * h);
	
	/*SDL_SaveBMP(surface, "wow.bmp");*/
	
	/* allocate sprite */
	sprite = malloc(sizeof(*sprite));
	if (!sprite)
		return 0;
	
	/* enable texture blending */
#if 0
TODO
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
#endif
	
	/* every image contains these callbacks */
	sprite->draw = image_draw;
	sprite->free = image_free;
	sprite->color = image_color;
	
	/* other data */
	sprite->udata = surf;
	
	/* success */
	return sprite;
}

#if 0
TODO low priority
static
void
target_free(struct wowGui_target *target)
{
	/* can't free non-existent target */
	if (!target)
		return;
	
	SDL_Texture *tex = target->udata;
	
	/* free udata if it exists */
	if (tex)
		SDL_DestroyTexture(tex);
	
	/* free target */
	free(target);
}


static
void
target_draw(struct wowGui_target *target, int x, int y)
{
	/* can't draw non-existent target */
	if (!target || !target->udata)
		return;
	
	SDL_Texture *tex = target->udata;
	SDL_Rect src = {0, 0, target->dim.w, target->dim.h};
	SDL_Rect dst = {x, y, target->dim.w, target->dim.h};
	
	SDL_RenderCopy(renderer, tex, &src, &dst);
}


static
void
target_bind(struct wowGui_target *target)
{
	SDL_Texture *tex = 0;
	
	if (target && target->udata)
		tex = target->udata;
	
	SDL_SetRenderTarget(renderer, tex);
}


static
void
target_clear(struct wowGui_target *target)
{
	/* can't clear a blank target */
	if (!target || !target->udata)
		return;
	
	/* back up render target */
	SDL_Texture *backup = SDL_GetRenderTarget(renderer);
	SDL_Texture *tex = target->udata;
	
	/* clear render target */
	SDL_SetRenderTarget(renderer, tex);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	
	/* restore render target */
	SDL_SetRenderTarget(renderer, backup);
}


static
void
target_resize(struct wowGui_target *target, int w, int h)
{
	/* can't resize non-existent target */
	if (!target || !target->udata)
		return;
	
	SDL_Texture *tex = target->udata;
	
	/* no need to resize the actual texture if the requested
	   size is smaller than or equal to it */
	if (w <= target->allocd.w && h <= target->allocd.h)
	{
		goto L_update_dim;
	}
	
	/* destroy existing texture */
	SDL_DestroyTexture(tex);
	
	/* create a new one of the requested dimensions */
	tex =
	SDL_CreateTexture(
		renderer
		, SDL_PIXELFORMAT_RGBA8888
		, SDL_TEXTUREACCESS_TARGET
		, w
		, h
	);
	
	/* only modify texture if it was created successfully */
	if (tex)
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

	/* note: on failure, tex should be 0 by now so udata = 0 happens;
	         this way, wowGui knows the resize failed */
	target->udata = tex;
	target->allocd.w = w;
	target->allocd.h = h;
	
L_update_dim:
	target->dim.w = w;
	target->dim.h = h;
}


static
struct wowGui_target *
target_new(int w, int h)
{
	struct wowGui_target *target;
	SDL_Texture *tex;
	
	tex =
	SDL_CreateTexture(
		renderer
		, SDL_PIXELFORMAT_RGBA8888
		, SDL_TEXTUREACCESS_TARGET
		, w
		, h
	);
	
	if (!tex)
		return 0;
	
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	
	target = malloc(sizeof(*target));
	
	if (!target)
		return 0;
	
	
	/* every target contains these callbacks */
	target->draw   = target_draw;
	target->free   = target_free;
	target->bind   = target_bind;
	target->clear  = target_clear;
	target->resize = target_resize;
	
	/* other data */
	target->udata = tex;
	target->dim.w = w;
	target->dim.h = h;
	target->allocd.w = w;
	target->allocd.h = h;
	
	return target;
}
#endif


/* public functions */
WOW_GUI_API_PREFIX
void
wowGui_bind_result(void)
{
	mfb_update_state state;
	state = mfb_update(__wowGui_mfbWindow, g_buffer->pixels);
	if (state != STATE_OK)
		__wowGui_mfbRunning = 0;
}

WOW_GUI_API_PREFIX
void
wowGui_bind_clear(uint32_t rgba)
{
	uint32_t *pix = g_buffer->pixels;
	unsigned int count;
	rgba = fix_color(rgba);
	for (count = 0; count < __wowGui_mfbWinW * __wowGui_mfbWinH; ++count)
	{
		*pix = rgba;
		++pix;
	}
}

WOW_GUI_API_PREFIX
wowGui_u32_t
wowGui_bind_ms(void)
{
//#ifdef _WIN32
	/* TODO */
//#else
	static int has_init = 0;
	static struct timespec start;
	struct timespec now;
	if (!has_init)
	{
		clock_gettime(CLOCK_MONOTONIC, &start);
		has_init = 1;
	}
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / 1000000;
//#endif
}

WOW_GUI_API_PREFIX
void
wowGui_delay_ms(unsigned int ms)
{
#ifdef _WIN32
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

WOW_GUI_API_PREFIX
void
wowGui_bind_events(void)
{
	wowGui_u32_t ms = wowGui_bind_ms();
//	static int c = 0; fprintf(stderr, "events %d\n", c++);
#if 0 /* FPS counter */
	static float frame = 0;
	float seconds = ms * 0.001f;
	float fps = 0;
	if (seconds)
		fps = frame / seconds;
	fprintf(stderr, "fps = %f (%d ms)\n", fps, ms);
	frame += 0.5f;
#endif
	/* this is for keeping CPU usage down (frame-rate regulation) */
	static wowGui_u32_t ms_last = 0;
	wowGui_u32_t delta = ms - ms_last;
	if (delta <= (1000 / __wowGui_mfbFramerate))
		wowGui_delay_ms((1000 / __wowGui_mfbFramerate) - delta);
	ms_last = ms;

/* TODO this causes nothing to display on win32 */
#ifdef _WIN32
	mfb_update(__wowGui_mfbWindow, g_buffer->pixels);
#else
//#ifndef _WIN32
	mfb_update_events(__wowGui_mfbWindow);
#endif
}

WOW_GUI_API_PREFIX
int
wowGui_bind_should_redraw(void)
{
	int o_redraw = __wowGui_redraw;
	__wowGui_redraw -= __wowGui_redraw > 0;
	
	return o_redraw;
}


WOW_GUI_API_PREFIX
void
wowGui_bind_quit(void)
{
	/* wowGui */
	wowGui_quit();
	
	/* WOWMFB_surf */
	WOWMFB_FreeSurface(g_buffer);
}

WOW_GUI_API_PREFIX
int
wowGui_bind_endmainloop(void)
{
	return !__wowGui_mfbRunning;
}

WOW_GUI_API_PREFIX
void
wowGui_bind_resetmainloop(void)
{
	__wowGui_mfbRunning = 1;
}


WOW_GUI_API_PREFIX
void
wowGui_bind_blit_raw_scaled(
	void *raw
	, int x
	, int y
	, int w
	, int h
	, enum wowGui_blit_blend flags
	, int scale
)
{
	WOWMFB_Surface surf = {0};
	WOWMFB_Rect srcRect = {0, 0, w, h};
	WOWMFB_Rect dstRect = {x, y, w, h};
	
	WOWMFB_PixelFormat fmt = {
		.BytesPerPixel = 4
		, .BitsPerPixel  = 32
	};
	surf.format = &fmt;
	surf.w = w;
	surf.h = h;
	surf.pitch = w * 4;
	surf.pixels = raw;
	
	if (flags & WOWGUI_BLIT_ALPHABLEND)
		WOWMFB_BlitSurfaceScaled(
			&surf
			, &srcRect
			, g_buffer
			, &dstRect
			, blendfunc_alphablend
			, scale
		);
	else if (flags & WOWGUI_BLIT_ALPHATEST)
		WOWMFB_BlitSurfaceScaled(
			&surf
			, &srcRect
			, g_buffer
			, &dstRect
			, blendfunc_alphatest
			, scale
		);
	else if (flags & WOWGUI_BLIT_COLORTEST)
		WOWMFB_BlitSurfaceScaled(
			&surf
			, &srcRect
			, g_buffer
			, &dstRect
			, blendfunc_colortest
			, scale
		);
	else /* WOWGUI_BLIT_NOBLEND */
		WOWMFB_BlitSurfaceScaled(
			&surf
			, &srcRect
			, g_buffer
			, &dstRect
			, blendfunc_noblend
			, scale
		);
}


WOW_GUI_API_PREFIX
void
wowGui_bind_blit_raw(
	void *raw
	, int x
	, int y
	, int w
	, int h
	, enum wowGui_blit_blend flags
)
{
	wowGui_bind_blit_raw_scaled(raw, x, y, w, h, flags, 1);
}

WOW_GUI_API_PREFIX
void
wowGui_bind_set_fps(int fps)
{
	__wowGui_mfbFramerate = fps;
}


WOW_GUI_API_PREFIX
void
wowGui_redrawIncrement(void)
{
	__wowGui_redraw += 1;
}


WOW_GUI_API_PREFIX
void
wowGui_bind_init(char *title, int w, int h)
{
	const char *errstr;
	__wowGui_mfbWinW = w;
	__wowGui_mfbWinH = h;

	__wowGui_mfbWindow = mfb_open_ex(title, w, h, 0);

	/* SDL interprets each pixel as a 32-bit number, so our masks
	   must depend on the endianness (byte order) of the machine */
	uint32_t rmask, gmask, bmask, amask;
#if 1//SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	
	uint32_t *buffer_pix = malloc(w * h * sizeof(*buffer_pix));
	
	g_buffer =
	WOWMFB_CreateRGBSurfaceFrom(
		buffer_pix
		, w, h
		, 32
		, w * 4
		, rmask, gmask, bmask, amask
	);
	if (!g_buffer || !__wowGui_mfbWindow)
	{
		fprintf(stderr, "something went wrong\n");
		exit(EXIT_FAILURE);
	}
	
	/* initialize wowGui */
	errstr = wowGui_init(
		image_new
		, fill_rect
		, 0/*target_new*/
		, 256 /* text format buffer */
		, 256 /* keyboard buffer */
	);
	if (errstr)
	{
		fprintf(stderr, "init failure: %s\n", errstr);
		exit(EXIT_FAILURE);
	}

	mfb_set_active_callback(__wowGui_mfbWindow, cb_active);
	mfb_set_resize_callback(__wowGui_mfbWindow, cb_resize);
	mfb_set_keyboard_callback(__wowGui_mfbWindow, cb_keyboard);
	mfb_set_char_input_callback(__wowGui_mfbWindow, cb_char_input);
	mfb_set_mouse_button_callback(__wowGui_mfbWindow, cb_mouse_btn);
	mfb_set_mouse_move_callback(__wowGui_mfbWindow, cb_mouse_move);
	mfb_set_mouse_scroll_callback(__wowGui_mfbWindow, cb_mouse_scroll);
	mfb_set_file_drag_callback(__wowGui_mfbWindow, cb_file_drag);
	mfb_set_file_drop_callback(__wowGui_mfbWindow, cb_file_drop);

	mfb_set_user_data(__wowGui_mfbWindow, (void *) "title");
}

#else /* WOW_GUI_BINDING_MINIFB */
	#error wow_gui.h: no binding selected
#endif


/*
 *
 * end bindings
 *
 */




static void __wowGui_unused(void)
{
	(void)debugfont;
	(void)debugfont_draw;
	(void)font_crisp;
	(void)font_crisp_draw;
	(void)font_dropshadow;
	(void)min_int;
}

#endif /* WOW_GUI_IMPLEMENTATION */


