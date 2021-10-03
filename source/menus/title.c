#include "source/neslib_asm/neslib.h"
#include "source/menus/title.h"
#include "source/globals.h"
#include "source/configuration/game_states.h"
#include "source/configuration/system_constants.h"
#include "source/menus/text_helpers.h"
#include "source/graphics/palettes.h"
#include "source/configuration/game_info.h"
#include "source/map/map.h"
#include "graphics/intro.h"
#include "source/menus/input_helpers.h"
#include "source/graphics/fade_animation.h"
#include "graphics/title.h"

CODE_BANK(PRG_BANK_TITLE);

extern const unsigned char layerBasedPalettes[];
void draw_title_screen(void) {
    ppu_off();
	pal_bg(&layerBasedPalettes[0]);
	pal_spr(mainSpritePalette);

	set_chr_bank_0(CHR_BANK_MENU);
    set_chr_bank_1(CHR_BANK_SPRITES);
	clear_screen();
	oam_clear();
/*
    
    put_str(NTADR_A(7, 5), gameName);
	
	put_str(NTADR_A(2, 26), gameAuthorContact);
	
	put_str(NTADR_A(2, 28), "Copyright");
	put_str(NTADR_A(12, 28), currentYear);
	put_str(NTADR_A(17, 28), gameAuthor);

	put_str(NTADR_A(10, 16), "Press Start!");
*/
	vram_unrle(title);
	#if DEBUG == 1
		put_str(NTADR_A(2, 28), "DEBUG MODE ENABLED");
	#endif

	oam_clear();
	tempChar1 = (frameCount & 0x10) >> 3;
    oam_spr(15<<3, 17<<3, tempChar1, 0, 0x10);
    oam_spr(16<<3, 17<<3, tempChar1 + 0x01, 0, 0x14);
    oam_spr(15<<3, 18<<3, tempChar1 + 0x10, 0, 0x18);
    oam_spr(16<<3, 18<<3, tempChar1 + 0x11, 0, 0x1c);


	ppu_on_all();


	gameState = GAME_STATE_TITLE_INPUT;
}

void handle_title_input(void) {
	if (pad_trigger(0) & PAD_START) {
		gameState = GAME_STATE_POST_TITLE;
		#if DEBUG == 1
			// DEBUG SCREEN
			if (pad_poll(0) & PAD_SELECT) {
				playerOverworldPosition = 7;
			}
		#endif

		sfx_play(SFX_PAUSE_UP, SFX_CHANNEL_1);
	}

	oam_clear();
	tempChar1 = (frameCount & 0x10) >> 3;
    oam_spr(15<<3, 17<<3, tempChar1, 0, 0x10);
    oam_spr(16<<3, 17<<3, tempChar1 + 0x01, 0, 0x14);
    oam_spr(15<<3, 18<<3, tempChar1 + 0x10, 0, 0x18);
    oam_spr(16<<3, 18<<3, tempChar1 + 0x11, 0, 0x1c);
	ppu_wait_nmi();
}

void draw_intro(void) {
	fade_out();
	ppu_off();
	oam_clear();
	pal_bg(titlePalette);
	vram_adr(0x2000);
	vram_unrle(intro);
	ppu_on_all();
	fade_in();

	banked_call(PRG_BANK_MENU_INPUT_HELPERS, wait_for_start);

	sfx_play(SFX_PAUSE_UP, SFX_CHANNEL_1);
}