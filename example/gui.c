#define WOW_IMPLEMENTATION
#include <wow.h>
#define WOW_GUI_IMPLEMENTATION
#include <wow_gui.h>

#define WIN_W (42 *  8)
#define WIN_H (31 * 16)

#define WIN_SCROLLABLE_H (4 * 16)

#define FULL_W WIN_W
#define FULL_H (WIN_H + WIN_SCROLLABLE_H)

int wow_main(argc, argv)
{
	wow_main_args(argc, argv);
	wowGui_bind_init("wow_gui.h example", FULL_W, FULL_H);
	
	while (1)
	{
		/* wowGui_frame() must be called before you do any input */
		wowGui_frame();
		
		/* events */
		wowGui_bind_events();
		
		if (wowGui_bind_should_redraw())
		{
			wowGui_viewport(0, 0, FULL_W, FULL_H);
			wowGui_padding(8, 8);
			
			static struct wowGui_window win = {
				.rect = {
					.x = 0
					, .y = 0
					, .w = WIN_W
					, .h = WIN_H
				}
				, .color = 0x301818FF
				, .not_scrollable = 1
				, .scroll_valuebox = 1
			};
			if (wowGui_window(&win))
			{
				const char UHHUH[] = 
					"UHHUHUHHUHUho:::///:::::/hNNddo+/:------\0"
					"UHHUHUHHdy/:::://::::::+mMNdhhys/:------\0"
					"UHHUHUHy/+hs/::::::/+odmMNmdddmddmms/---\0"
					"UHHUHmy+hNMm/://////+ymmmhhhhhdy-+ymy---\0"
					"UHHUdo/o/sNNho/:----oms:mmmmmmNd. `:o:--\0"
					"UHHd//sNMd/+/------sm+ yNNUHHUHd- `-----\0"
					"UHy+sddhhho-------:m:` +mUHHUHmy..------\0"
					"Uy/shdhm+-y-------:/.``-smmmmds:--------\0"
					"dosoymmm.`o--------------:::::----------\0"
					"Mm+`dNNs`.------------------------------\0"
					"my`:mNm-----::::------------------------\0"
					"ms-/y+:-----:++//:----::----------------\0"
					"d:----------:///:-----:+y:--------------\0"
					"do-------:---------:::/oso:-------------\0"
					"ds-------o---::+/++ooo+-----------------\0"
					"d+-------ssso/------------------------:/\0"
					"Ud/--------------------------------:/+oo\0"
					"UHh+-----------------------------:/+osUH\0"
					"UHHms:--------------------------/++sHHUH\0"
					"UHHUHds:----------------------:/+osUHHUH\0\0"
				;
				const char *i;
				static int toggle;
				static int intVal;
				static float floatVal;
				
				wowGui_column_width(WIN_W);
				wowGui_row_height(20);
				
				wowGui_columns(1);
				{
					wowGui_label("Hello, wowlib GUI!");
					wowGui_italic(2);
						wowGui_label("This text is italic.");
					wowGui_italic(0);
					wowGui_label("* Try scrolling or right-click-dragging");
					wowGui_label("  in the two fields below this line.");
				}
				
				wowGui_columns(2);
				wowGui_column_width(WIN_W / 2);
				{
					wowGui_label("Integer Range:");
					wowGui_int_range(&intVal, -3000, 3000, 10);
					
					wowGui_label("Float Range:");
					wowGui_float_range(&floatVal, -100, 100, 0.1f);
					
					if (wowGui_button("Click for a popup"))
					{
						wowGui_popup(
							WOWGUI_POPUP_ICON_INFO
							, WOWGUI_POPUP_OK /* choices */
							, WOWGUI_POPUP_OK /* initial selection */
							, "What's this?"
							, "A wild popup appeared!"
						);
					}
					
					wowGui_checkbox("UHHUH", &toggle);
				}
				
				/* do a file dropper */
				wowGui_columns(3);
				static struct wowGui_fileDropper txtFile = {
					.label = "Text File (.txt)"
					, .labelWidth = 128
					, .filenameWidth = 160
					, .extension = "txt"
					, .isOptional = 1
					, .isCreateMode = 1 /* ask for save file name */
				};
				if (wowGui_fileDropper(&txtFile))
				{
					/* load file here or do whatever you want */
					
					if (!wowGui_fileDropper_filenameIsEmpty(&txtFile))
					{
						wowGui_popupf(
							WOWGUI_POPUP_ICON_INFO
							, WOWGUI_POPUP_OK /* choices */
							, WOWGUI_POPUP_OK /* initial selection */
							, "You selected or dropped a file!"
							, "Thanks for the\n'%s'!\nI will eat it later!"
							, txtFile.filename
						);
					}
				}
				
				if (toggle)
				{
					wowGui_columns(1);
					wowGui_column_width(WIN_W);
					wowGui_row_height(16);
					for (i = UHHUH; *i; i += strlen(i) + 1)
						wowGui_label(i);
				}
				
				wowGui_window_end();
			}
			static struct wowGui_window scrollable = {
				.rect = {
					.x = 0
					, .y = WIN_H
					, .w = WIN_W
					, .h = WIN_SCROLLABLE_H
				}
				, .color = 0x401818FF
				, .not_scrollable = 0
				, .scroll_valuebox = 1
			};
			if (wowGui_window(&scrollable))
			{
				wowGui_column_width(WIN_W);
				wowGui_row_height(20);
				
				wowGui_columns(1);
				{
					int i;
					wowGui_label("Try scrolling here!");
					wowGui_label("You can also right-click-drag!");
					wowGui_label("Or use the arrow keys while hovering!");
					
					for (i = 'A'; i <= 'Z'; ++i)
					{
						char ok[] = { i, '\0' };
						wowGui_label(ok);
					}
				}
				
				wowGui_window_end();
			}
		} /* wowGui_bind_should_redraw */
		
		wowGui_frame_end(wowGui_bind_ms());
		
		/* display */
		wowGui_bind_result();
		
		/* loop exit condition */
		if (wowGui_bind_endmainloop())
			break;
	}
	
	wowGui_bind_quit();
	
	return 0;
}

