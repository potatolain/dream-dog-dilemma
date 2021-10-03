#include "source/menus/pause.h"
#include "source/graphics/palettes.h"
#include "source/configuration/system_constants.h"
#include "source/globals.h"
#include "source/neslib_asm/neslib.h"
#include "source/configuration/game_states.h"
#include "source/menus/text_helpers.h"
#include "source/menus/input_helpers.h"

CODE_BANK(PRG_BANK_PAUSE_MENU);


void draw_pause_screen(void) {
    ppu_off();
    clear_screen_with_border();
    // We reuse the title palette here, though we have the option of making our own if needed.
    pal_bg(titlePalette);
	pal_spr(titlePalette);
    scroll(0, 0);

	set_chr_bank_0(CHR_BANK_MENU);
    set_chr_bank_1(CHR_BANK_MENU);

    // Just write "- Paused -" on the screen... there's plenty of nicer things you could do if you wanna spend time!
    put_str(NTADR_A(11, 13), "- Paused -");

    put_str(NTADR_A(6, 18), "Continue");
    put_str(NTADR_A(6, 19), "Restore last checkpoint");


    // We purposely leave sprites off, so they do not clutter the view. 
    // This means all menu drawing must be done with background tiles - if you want to use sprites (eg for a menu item),
    // you will have to hide all sprites, then put them back after. 
    ppu_on_bg();

    set_vram_update(NULL);
    screenBuffer[0] = MSB(NTADR_A(4, 18)) | NT_UPD_VERT;
    screenBuffer[1] = LSB(NTADR_A(4, 18));
    screenBuffer[2] = 2;
    screenBuffer[3] = 0xe2;
    screenBuffer[4] = 0x00;
    screenBuffer[5] = NT_UPD_EOF;
    set_vram_update(screenBuffer);
}

void handle_pause_input(void) {
    tempChar1 = 0;
    while (1) {
        lastControllerState = controllerState;
        controllerState = pad_poll(0);

        if (controllerState & PAD_UP && !(lastControllerState & PAD_UP)) {
            tempChar1 = 0;
            screenBuffer[3] = 0xe2;
            screenBuffer[4] = 0x00;
            sfx_play(SFX_TEXT_BLIP, SFX_CHANNEL_1);
        } else if (controllerState & PAD_DOWN && !(lastControllerState & PAD_DOWN)) {
            tempChar1 = 1;
            screenBuffer[3] = 0x00;
            screenBuffer[4] = 0xe2;
            sfx_play(SFX_TEXT_BLIP, SFX_CHANNEL_1);
        }
 
        // If Start is pressed now, and was not pressed before...
        if (controllerState & (PAD_START|PAD_A) && !(lastControllerState & (PAD_START|PAD_A))) {
            break;
        }

        ppu_wait_nmi();

    }

    if (tempChar1 == 0) {
        gameState = GAME_STATE_RUNNING;
    } else {
        gameState = GAME_STATE_GAME_OVER_1;
    }
    set_vram_update(NULL);
    sfx_play(SFX_PAUSE_DOWN, SFX_CHANNEL_1);
}