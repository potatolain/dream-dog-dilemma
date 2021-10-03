#include "source/menus/credits.h"
#include "source/graphics/palettes.h"
#include "source/configuration/system_constants.h"
#include "source/globals.h"
#include "source/neslib_asm/neslib.h"
#include "source/configuration/game_states.h"
#include "source/menus/text_helpers.h"
#include "source/configuration/game_info.h"

CODE_BANK(PRG_BANK_CREDITS_MENU);

#include "graphics/ending.h"

void draw_win_screen(void) {
    music_play(6);
    set_vram_update(NULL);
    ppu_off();
    clear_screen();
    // We reuse the title palette here, though we have the option of making our own if needed.
    pal_bg(titlePalette);
	pal_spr(titlePalette);
    scroll(0, 0);

	set_chr_bank_0(CHR_BANK_MENU);
    set_chr_bank_1(CHR_BANK_SPRITES);
/*
    // Add whatever you want here; NTADR_A just picks a position on the screen for you. Your options are 0, 0 to 32, 30
    put_str(NTADR_A(7, 2), "- Congratulations -");

    put_str(NTADR_A(4, 8), "You escaped the corrupted");
    put_str(NTADR_A(4, 9), "world and made it home!");
*/
    vram_adr(0x2000);
    vram_unrle(ending);


    // Hide all existing sprites
    oam_clear();
    oam_spr(14<<3, 20<<3, 0x4c, 2, 0x10);
    oam_spr(15<<3, 20<<3, 0x4d, 2, 0x14);
    oam_spr(14<<3, 21<<3, 0x5c, 2, 0x18);
    oam_spr(15<<3, 21<<3, 0x5d, 2, 0x1c);

    ppu_on_all();

}

void draw_credits_screen(void) {
    set_vram_update(NULL);
    ppu_off();
    clear_screen();
    oam_clear();
    // We reuse the title palette here, though we have the option of making our own if needed.
    pal_bg(titlePalette);
	pal_spr(titlePalette);
    scroll(0, 0);

	set_chr_bank_0(CHR_BANK_MENU);
    set_chr_bank_1(CHR_BANK_MENU);

    // Add whatever you want here; NTADR_A just picks a position on the screen for you. Your options are 0, 0 to 32, 30
    put_str(NTADR_A(11, 2), "- Credits -");

    put_str(NTADR_A(2, 6), "Game Logic/Art/Everything");
    put_str(NTADR_A(4, 8), gameAuthor);

    put_str(NTADR_A(2, 11), "Tools Used");
    put_str(NTADR_A(4, 13), "nes-starter-kit");
    put_str(NTADR_A(4, 14), "2Bit Tile Generator");
    put_str(NTADR_A(4, 15), "2BitCharactersGenerator");
    put_str(NTADR_A(4, 16), "Famitracker");
    put_str(NTADR_A(4, 17), "NEXXT Editor");
    put_str(NTADR_A(4, 18), "Tiled");


    put_str(NTADR_A(4, 24), "Created in 48 hours for");
    put_str(NTADR_A(8, 26), "Ludum Dare 49");


    // Hide all existing sprites
    oam_clear();
    ppu_on_all();
}