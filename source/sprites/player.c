#include "source/neslib_asm/neslib.h"
#include "source/sprites/player.h"
#include "source/library/bank_helpers.h"
#include "source/globals.h"
#include "source/map/map.h"
#include "source/configuration/game_states.h"
#include "source/configuration/system_constants.h"
#include "source/sprites/collision.h"
#include "source/sprites/sprite_definitions.h"
#include "source/sprites/map_sprites.h"
#include "source/menus/error.h"
#include "source/graphics/hud.h"
#include "source/graphics/game_text.h"
#include "source/sprites/map_sprites.h"
#include "source/graphics/fade_animation.h"

// Purposely hiding this in the kernel instead of this bank
const unsigned char layerBasedPalettes[] = {
    // green
    0x0f, 0x01, 0x21, 0x31, 0x0f, 0x06, 0x16, 0x26, 0x0f, 0x21, 0x19, 0x29, 0x0f, 0x08, 0x18, 0x28,

    // autumn
    0x0f, 0x01, 0x2c, 0x3c, 0x0f, 0x05, 0x06, 0x26, 0x0f, 0x3c, 0x28, 0x38, 0x0f, 0x08, 0x18, 0x28,
    
    // desert
    0x0f, 0x01, 0x22, 0x31, 0x0f, 0x07, 0x17, 0x27, 0x0f, 0x37, 0x28, 0x38, 0x0f, 0x08, 0x18, 0x28,

    // unstable
    0x0f, 0x11, 0x31, 0x30, 0x0f, 0x16, 0x26, 0x36, 0x0f, 0x20, 0x1b, 0x3b, 0x0f, 0x18, 0x38, 0x20,

    // comp
    0x0f, 0x0f, 0x31, 0x20, 0x0f, 0x0f, 0x20, 0x20, 0x0f, 0x20, 0x1a, 0x20, 0x0f, 0x18, 0x38, 0x20

};


CODE_BANK(PRG_BANK_PLAYER_SPRITE);

// Some useful global variables
ZEROPAGE_DEF(int, playerXPosition);
ZEROPAGE_DEF(int, playerYPosition);
ZEROPAGE_DEF(int, playerXVelocity);
ZEROPAGE_DEF(int, playerYVelocity);
ZEROPAGE_DEF(unsigned char, playerControlsLockTime);
ZEROPAGE_DEF(unsigned char, playerInvulnerabilityTime);
ZEROPAGE_DEF(unsigned char, playerDirection);

ZEROPAGE_DEF(unsigned char, doorPosition)

// Huge pile of temporary variables
#define rawXPosition tempChar1
#define rawYPosition tempChar2
#define rawTileId tempChar3
#define collisionTempX tempChar4
#define collisionTempY tempChar5
#define collisionTempXRight tempChar6
#define collisionTempYBottom tempChar7

#define tempSpriteCollisionX tempInt1
#define tempSpriteCollisionY tempInt2

#define collisionTempXInt tempInt3
#define collisionTempYInt tempInt4

const unsigned char* defaultText = 
                                "Hello! Be careful out there;  "
                                "this world is dangerous.      "
                                "                              "
                                "Good luck!";

const unsigned char* corruptionText = 
                                "Ht §¨r t£¤s¥¦©bª«c¬?    ";

const unsigned char* corruptionText2 = 
                                "Bt t§¨b m,¤s£n¥¦©«cbª¬!";

const unsigned char* firstPageText = 
                                "Hello! How did you get here?  " 
                                "                              "
                                "                              "

                                "Oh, I see. Well, I guess it   "
                                "can't be helped. Here is a tip"
                                "to help you.                  "

                                "There are doors like these all"
                                "over.The number of keys they  "
                                "need hovers over them.        "

                                "Good luck out there!";

const unsigned char* secondPageText = 
                                "This world is less stable than"
                                "it looks! Whoever sent you    "
                                "here must not like you...     "

                                "They trapped me here on this  "
                                "island! Those darn tiny bushes"
                                "are in the way!               "
                                
                                "The radio nearby seems very   "
                                "strange. Something might      "
                                "happen if you press A near it."

                                "What does \"press A\" mean? I   "
                                "don't know! I was told to say "
                                "this!";

const unsigned char* secondPageOtherDimensionText = 
                                "Hi there! I remember talking  "
                                "to you a many years ago!I made"
                                "this island home.             "

                                "I guess this means you must   "
                                "have never found your way     "
                                "home. I'm sorry.";

const unsigned char* whyIsTheDesert = 
                                "I have been trapped out here  "
                                "for ages. It's just nothing.  "
                                "the empty space goes forever! "

                                "I'm not getting the free      "
                                "balloon the doctor promised me"
                                "for participating in this     "

                                "experiment, am I?";

const unsigned char* bigComputer = 
                                "Sometimes I feel like I am    "
                                "trapped in a huge computer.";

const unsigned char* desssert = 
                                "I've been wandering in this   "
                                "desert forever                "
                                "                              "

                                "I should have known something "
                                "was amiss when I was offered  "
                                "endless desert.               "

                                "I also really need to learn to"
                                "spell.";

const unsigned char* escapers = 
                                "What if those monsters escape?"
                                "I'm so scared!                "
                                "                              "

                                "They promised me it would help"
                                "FoE! I guess this helps these "
                                "foes...                       "
                                
                                "English is an evil language.  ";

const unsigned char* oasis =    "Never trust a doctor that     "
                                "promises you your own oasis.  ";

const unsigned char screenKeyCounts[] = {
    1, 2, 2, 0, 3, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

const unsigned char** screenMessages[] = {
    &firstPageText,     &escapers,      0,              0,              0,              0,              0,              0,
    &secondPageText,    0,              0,              0,              &whyIsTheDesert,&bigComputer,   &desssert,      0,
    0,                  0,              0,              0,              0,              0,              0,              0,
    0,                  0,              0,              0,              &oasis,         0,              0,              0,
    0,                  0,              0,              0,              0,              0,              0,              0,
    0,                  0,              0,              0,              0,              0,              0,              0,
    0,                  0,              0,              0,              0,              0,              0,              0,
    0,                  0,              0,              0,              0,              0,              0,              0
};

const unsigned char screenDimensions[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 3, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};


// NOTE: This uses tempChar1 through tempChar3; the caller must not use these.
void update_player_sprite(void) {
    // Calculate the position of the player itself, then use these variables to build it up with 4 8x8 NES sprites.
    rawXPosition = (playerXPosition >> PLAYER_POSITION_SHIFT);
    rawYPosition = (playerYPosition >> PLAYER_POSITION_SHIFT);
    rawTileId = PLAYER_SPRITE_TILE_ID + playerDirection;

    if (playerXVelocity != 0 || playerYVelocity != 0) {
        // Does some math with the current NES frame to add either 2 or 0 to the tile id, animating the sprite.
        rawTileId += ((frameCount >> SPRITE_ANIMATION_SPEED_DIVISOR) & 0x01) << 1;
    }

    // Clamp the player's sprite X Position to 0 to make sure we don't loop over, even if technically we have.
    if (rawXPosition > (SCREEN_EDGE_RIGHT + 4)) {
        rawXPosition = 0;
    }
    
    if (gameState == GAME_STATE_GAME_OVER || (playerInvulnerabilityTime && frameCount & PLAYER_INVULNERABILITY_BLINK_MASK)) {
        // If the player is invulnerable, we hide their sprite about half the time to do a flicker animation.
        oam_spr(SPRITE_OFFSCREEN, SPRITE_OFFSCREEN, rawTileId, 0x00, PLAYER_SPRITE_INDEX);
        oam_spr(SPRITE_OFFSCREEN, SPRITE_OFFSCREEN, rawTileId + 1, 0x00, PLAYER_SPRITE_INDEX+4);
        oam_spr(SPRITE_OFFSCREEN, SPRITE_OFFSCREEN, rawTileId + 16, 0x00, PLAYER_SPRITE_INDEX+8);
        oam_spr(SPRITE_OFFSCREEN, SPRITE_OFFSCREEN, rawTileId + 17, 0x00, PLAYER_SPRITE_INDEX+12);

    } else {
        oam_spr(rawXPosition, rawYPosition, rawTileId, 0x00, PLAYER_SPRITE_INDEX);
        oam_spr(rawXPosition + NES_SPRITE_WIDTH, rawYPosition, rawTileId + 1, 0x00, PLAYER_SPRITE_INDEX+4);
        oam_spr(rawXPosition, rawYPosition + NES_SPRITE_HEIGHT, rawTileId + 16, 0x00, PLAYER_SPRITE_INDEX+8);
        oam_spr(rawXPosition + NES_SPRITE_WIDTH, rawYPosition + NES_SPRITE_HEIGHT, rawTileId + 17, 0x00, PLAYER_SPRITE_INDEX+12);
    }

}

void damage_player(unsigned char dmg, unsigned char doBounceAndSfx) {
    playerHealth -= dmg; 
    // Since playerHealth is unsigned, we need to check for wraparound damage. 
    // NOTE: If something manages to do more than 16 damage at once, this might fail.
    if (playerHealth == 0 || playerHealth > 240) {
        gameState = GAME_STATE_GAME_OVER;
        if (doBounceAndSfx) { sfx_play(SFX_HURT, SFX_CHANNEL_2); }
        music_stop();
        delay(20);
        return;
    }

    // Knock the player back
    playerControlsLockTime = PLAYER_DAMAGE_CONTROL_LOCK_TIME;
    playerInvulnerabilityTime = PLAYER_DAMAGE_INVULNERABILITY_TIME;

    if (doBounceAndSfx) {
        if (playerDirection == SPRITE_DIRECTION_LEFT) {
            // Punt them back in the opposite direction
            playerXVelocity = PLAYER_MAX_VELOCITY;
            // Reverse their velocity in the other direction, too.
            playerYVelocity = 0 - playerYVelocity;
        } else if (playerDirection == SPRITE_DIRECTION_RIGHT) {
            playerXVelocity = 0-PLAYER_MAX_VELOCITY;
            playerYVelocity = 0 - playerYVelocity;
        } else if (playerDirection == SPRITE_DIRECTION_DOWN) {
            playerYVelocity = 0-PLAYER_MAX_VELOCITY;
            playerXVelocity = 0 - playerXVelocity;
        } else { // Make being thrown downward into a catch-all, in case your direction isn't set or something.
            playerYVelocity = PLAYER_MAX_VELOCITY;
            playerXVelocity = 0 - playerXVelocity;
        }
        sfx_play(SFX_HURT, SFX_CHANNEL_2);
    }

}

void do_layer_anim(unsigned char toLayer) {
    sfx_play(SFX_EVERT, SFX_CHANNEL_1);
    fade_out();
    currentLayer = toLayer;
    // Force a sprite refresh to avoid a flash of the old state
    update_map_sprites();

    ppu_off();

    pal_bg(&(layerBasedPalettes[0]) + (currentLayer << 4));
    music_play_continue(SONG_LAYERS + currentLayer);

    ppu_on_all();

    set_chr_bank_0(CHR_BANK_TILES + currentLayer);
    fade_in();
}


void handle_player_movement(void) {
    // Using a variable, so we can change the velocity based on pressing a button, having a special item,
    // or whatever you like!
    int maxVelocity = PLAYER_MAX_VELOCITY;
    lastControllerState = controllerState;
    controllerState = pad_poll(0);

    // If Start is pressed now, and was not pressed before...
    if (controllerState & PAD_START && !(lastControllerState & PAD_START)) {
        sfx_play(SFX_PAUSE_UP, SFX_CHANNEL_1);
        gameState = GAME_STATE_PAUSED;
        return;
    }

    // Debug stuff
    #if DEBUG == 1
        if (controllerState & PAD_SELECT) {
            if (controllerState & PAD_UP && !(lastControllerState & PAD_UP)) {
                ++playerKeyCount;
            }
            if (controllerState & PAD_DOWN && !(lastControllerState & PAD_DOWN)) {
                --playerKeyCount;
            }
            if (controllerState & PAD_LEFT && !(lastControllerState & PAD_LEFT)) {
                do_layer_anim(currentLayer - 1);

            }
            if (controllerState & PAD_RIGHT && !(lastControllerState & PAD_RIGHT)) {
                do_layer_anim(currentLayer + 1);
            }

        }
    #endif

    if (playerControlsLockTime) {
        // If your controls are locked, just tick down the timer until they stop being locked. Don't read player input.
        playerControlsLockTime--;
    } else {
        if (controllerState & PAD_RIGHT && playerXVelocity >= (0 - PLAYER_VELOCITY_NUDGE)) {
            // If you're moving right, and you're not at max, speed up.
            if (playerXVelocity < maxVelocity) {
                playerXVelocity += PLAYER_VELOCITY_ACCEL;
            // If you're over max somehow, we'll slow you down a little.
            } else if (playerXVelocity > maxVelocity) {
                playerXVelocity -= PLAYER_VELOCITY_ACCEL;
            }
        } else if (controllerState & PAD_LEFT && playerXVelocity <= (0 + PLAYER_VELOCITY_NUDGE)) {
            // If you're moving left, and you're not at max, speed up.
            if (ABS(playerXVelocity) < maxVelocity) {
                playerXVelocity -= PLAYER_VELOCITY_ACCEL;
            // If you're over max, slow you down a little...
            } else if (ABS(playerXVelocity) > maxVelocity) { 
                playerXVelocity += PLAYER_VELOCITY_ACCEL;
            }
        } else if (playerXVelocity != 0) {
            // Not pressing anything? Let's slow you back down...
            if (playerXVelocity > 0) {
                playerXVelocity -= PLAYER_VELOCITY_ACCEL;
            } else {
                playerXVelocity += PLAYER_VELOCITY_ACCEL;
            }
        }

        if (controllerState & PAD_UP && playerYVelocity <= (0 + PLAYER_VELOCITY_NUDGE)) {
            if (ABS(playerYVelocity) < maxVelocity) {
                playerYVelocity -= PLAYER_VELOCITY_ACCEL;
            } else if (ABS(playerYVelocity) > maxVelocity) {
                playerYVelocity += PLAYER_VELOCITY_ACCEL;
            }
        } else if (controllerState & PAD_DOWN && playerYVelocity >= (0 - PLAYER_VELOCITY_NUDGE)) {
            if (playerYVelocity < maxVelocity) {
                playerYVelocity += PLAYER_VELOCITY_ACCEL;
            } else if (playerYVelocity > maxVelocity) {
                playerYVelocity -= PLAYER_VELOCITY_ACCEL;
            }
        } else { 
            if (playerYVelocity > 0) {
                playerYVelocity -= PLAYER_VELOCITY_ACCEL;
            } else if (playerYVelocity < 0) {
                playerYVelocity += PLAYER_VELOCITY_ACCEL;
            }
        }
        
        // Now, slowly adjust to the grid as possible, if the player isn't pressing a direction. 
        #if PLAYER_MOVEMENT_STYLE == MOVEMENT_STYLE_GRID
            if (playerYVelocity != 0 && playerXVelocity == 0 && (controllerState & (PAD_LEFT | PAD_RIGHT)) == 0) {
                if ((char)((playerXPosition + PLAYER_POSITION_TILE_MASK_MIDDLE) & PLAYER_POSITION_TILE_MASK) > (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                    playerXVelocity = 0-PLAYER_VELOCITY_NUDGE;
                } else if ((char)((playerXPosition + PLAYER_POSITION_TILE_MASK_MIDDLE) & PLAYER_POSITION_TILE_MASK) < (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                    playerXVelocity = PLAYER_VELOCITY_NUDGE;
                } // If equal, do nothing. That's our goal.
            }

            if (playerXVelocity != 0 && playerYVelocity == 0 && (controllerState & (PAD_UP | PAD_DOWN)) == 0) {
                if ((char)((playerYPosition + PLAYER_POSITION_TILE_MASK_MIDDLE + PLAYER_POSITION_TILE_MASK_EXTRA_NUDGE) & PLAYER_POSITION_TILE_MASK) > (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                    playerYVelocity = 0-PLAYER_VELOCITY_NUDGE;
                } else if ((char)((playerYPosition + PLAYER_POSITION_TILE_MASK_MIDDLE + PLAYER_POSITION_TILE_MASK_EXTRA_NUDGE) & PLAYER_POSITION_TILE_MASK) < (char)(PLAYER_POSITION_TILE_MASK_MIDDLE)) {
                    playerYVelocity = PLAYER_VELOCITY_NUDGE;
                } // If equal, do nothing. That's our goal.
            }
        #endif

    }

    // While we're at it, tick down the invulnerability timer if needed
    if (playerInvulnerabilityTime) {
        playerInvulnerabilityTime--;
    }

    // This will knock out the player's speed if they hit anything.
    test_player_tile_collision();
    handle_player_sprite_collision();

    if (gameState == GAME_STATE_GAME_OVER) {
        return;
    }

    rawXPosition = (playerXPosition >> PLAYER_POSITION_SHIFT);
    rawYPosition = (playerYPosition >> PLAYER_POSITION_SHIFT);
    // The X Position has to wrap to allow us to snap to the grid. This makes us shift when you wrap around to 255, down to 240-ish
    if (rawXPosition > (SCREEN_EDGE_RIGHT+4) && rawXPosition < SCREEN_EDGE_LEFT_WRAPPED) {
        // We use sprite direction to determine which direction to scroll in, so be sure this is set properly.
        if (playerInvulnerabilityTime) {
            playerXPosition -= playerXVelocity;
            rawXPosition = (playerXPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_LEFT;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition--;
        }
    } else if (rawXPosition > SCREEN_EDGE_RIGHT && rawXPosition < (SCREEN_EDGE_RIGHT+4)) {
        if (playerInvulnerabilityTime) {
            playerXPosition -= playerXVelocity;
            rawXPosition = (playerXPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_RIGHT;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition++;
        }
    } else if (rawYPosition > SCREEN_EDGE_BOTTOM) {
        if (playerInvulnerabilityTime) {
            playerYPosition -= playerYVelocity;
            rawYPosition = (playerYPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_DOWN;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition += 8;
        }
    } else if (rawYPosition < SCREEN_EDGE_TOP) {
        if (playerInvulnerabilityTime) {
            playerYPosition -= playerYVelocity;
            rawYPosition = (playerYPosition >> PLAYER_POSITION_SHIFT);
        } else {
            playerDirection = SPRITE_DIRECTION_UP;
            gameState = GAME_STATE_SCREEN_SCROLL;
            playerOverworldPosition -= 8;
        }
    }

}

ZEROPAGE_DEF(unsigned char, collisionTileTemp);
void test_player_tile_collision(void) {

    nearestHole = 0xff;
	if (playerYVelocity != 0) {
        collisionTempYInt = playerYPosition + PLAYER_Y_OFFSET_EXTENDED + playerYVelocity + playerGravityPullX;
        collisionTempXInt = playerXPosition + PLAYER_X_OFFSET_EXTENDED + playerGravityPullY;

		collisionTempY = ((collisionTempYInt) >> PLAYER_POSITION_SHIFT) - HUD_PIXEL_HEIGHT;
		collisionTempX = ((collisionTempXInt) >> PLAYER_POSITION_SHIFT);

        collisionTempYInt += PLAYER_HEIGHT_EXTENDED;
        collisionTempXInt += PLAYER_WIDTH_EXTENDED;

        collisionTempYBottom = ((collisionTempYInt) >> PLAYER_POSITION_SHIFT) - HUD_PIXEL_HEIGHT;
        collisionTempXRight = (collisionTempXInt) >> PLAYER_POSITION_SHIFT;
        
        // Due to how we are calculating the sprite's position, there is a slight chance we can exceed the area that our
        // map takes up near screen edges. To compensate for this, we clamp the Y position of our character to the 
        // window between 0 and 192 pixels, which we can safely test collision within.

        // If collisionTempY is > 240, it can be assumed we calculated a position less than zero, and rolled over to 255
        if (collisionTempY > 240) {
            collisionTempY = 0;
        }
        // The lowest spot we can test collision is at pixel 192 (the 12th 16x16 tile). If we're past that, bump ourselves back.
        if (collisionTempYBottom > 190) { 
            collisionTempYBottom = 190;
        }

		if (playerYVelocity < 0) {
            // We're going up - test the top left, and top right
			if (test_collision(PLAYER_MAP_POSITION(collisionTempX, collisionTempY), 1) || test_collision(PLAYER_MAP_POSITION(collisionTempXRight, collisionTempY), 1)) {
                playerYVelocity = 0;
                playerControlsLockTime = 0;
            }
            if (!playerControlsLockTime && playerYVelocity < (0-PLAYER_VELOCITY_NUDGE)) {
                playerDirection = SPRITE_DIRECTION_UP;
            }
		} else {
            // Okay, we're going down - test the bottom left and bottom right
			if (test_collision(PLAYER_MAP_POSITION(collisionTempX, collisionTempYBottom), 1) || test_collision(PLAYER_MAP_POSITION(collisionTempXRight, collisionTempYBottom), 1)) {
                playerYVelocity = 0;
                playerControlsLockTime = 0;

			}
            if (!playerControlsLockTime && playerYVelocity > PLAYER_VELOCITY_NUDGE) {
                playerDirection = SPRITE_DIRECTION_DOWN;
            }
		}
    }

	if (playerXVelocity != 0) {
        collisionTempXInt = playerXPosition + PLAYER_X_OFFSET_EXTENDED + playerXVelocity + playerGravityPullX;
        collisionTempYInt = playerYPosition + PLAYER_Y_OFFSET_EXTENDED + playerYVelocity + playerGravityPullY;
		collisionTempX = (collisionTempXInt) >> PLAYER_POSITION_SHIFT;
		collisionTempY = ((collisionTempYInt) >> PLAYER_POSITION_SHIFT) - HUD_PIXEL_HEIGHT;

        collisionTempXInt += PLAYER_WIDTH_EXTENDED;
        collisionTempYInt += PLAYER_HEIGHT_EXTENDED;

        collisionTempYBottom = ((collisionTempYInt) >> PLAYER_POSITION_SHIFT) - HUD_PIXEL_HEIGHT;
        collisionTempXRight = ((collisionTempXInt) >> PLAYER_POSITION_SHIFT);
        // The lowest spot we can test collision is at pixel 192 (the 12th 16x16 tile). If we're past that, bump ourselves back.
        if (collisionTempYBottom > 190) {
            collisionTempYBottom = 190;
        }



        // Depending on how far to the left/right the player is, there's a chance part of our bounding box falls into
        // the next column of tiles on the opposite side of the screen. This if statement prevents those collisions.
        if (collisionTempX > 2 && collisionTempX < 238) {
            if (playerXVelocity < 0) {
                // Okay, we're moving left. Need to test the top-left and bottom-left
                if (test_collision(PLAYER_MAP_POSITION(collisionTempX, collisionTempY), 1) || test_collision(PLAYER_MAP_POSITION(collisionTempX, collisionTempYBottom), 1)) {
                    playerXVelocity = 0;
                    playerControlsLockTime = 0;

                }
                if (!playerControlsLockTime && playerXVelocity < (0-PLAYER_VELOCITY_NUDGE)) {
                    playerDirection = SPRITE_DIRECTION_LEFT;
                }
            } else {
                // Going right - need to test top-right and bottom-right
                if (test_collision(PLAYER_MAP_POSITION(collisionTempXRight, collisionTempY), 1) || test_collision(PLAYER_MAP_POSITION(collisionTempXRight, collisionTempYBottom), 1)) {
                    playerXVelocity = 0;
                    playerControlsLockTime = 0;

                }
                if (!playerControlsLockTime && playerXVelocity > PLAYER_VELOCITY_NUDGE) {
                    playerDirection = SPRITE_DIRECTION_RIGHT;
                }
            }
        }
	}

    if (nearestCrack != 0xff) {

    }

    // Set gravity for next frame
    if (nearestHole != 0xff) {
        if (((nearestHole & 0x0f) << 4) > collisionTempX) { 
            //crash_error("xNudgePlus", "x", "x", 54);
            playerGravityPullX = 12;
        } else {
            playerGravityPullX = -12;
        }

        if (((nearestHole & 0xf0)) > collisionTempY) {
            playerGravityPullY = 12;
        } else {
            playerGravityPullY = -12;
        }
    } else {
        playerGravityPullX = 0;
        playerGravityPullY = 0;
    }

    // Test single-tile pull-based effects
    if (PLAYER_MAP_POSITION(collisionTempX, collisionTempY) == PLAYER_MAP_POSITION(collisionTempX + 8, collisionTempY + 6)) {
        // NOTE: Setting isPlayer to _false_ here to get collision values sprites see, for when we can walk over things like holes
        collisionTileTemp = test_collision(PLAYER_MAP_POSITION(collisionTempX, collisionTempY), 0);
        
        // hole
        if (collisionTileTemp == 3) {
            
            // Forcibly animating fall animation using oam
            sfx_play(SFX_FALLING, SFX_CHANNEL_1);

            collisionTileTemp = (nearestHole & 0xf0) + HUD_PIXEL_HEIGHT;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 0) = collisionTileTemp;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 0 + 4) = collisionTileTemp;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 0 + 8) = collisionTileTemp+8;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 0 + 12) = collisionTileTemp+8;
            collisionTileTemp = (nearestHole & 0x0f) << 4;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 3) = collisionTileTemp;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 3 + 4) = collisionTileTemp+8;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 3 + 8) = collisionTileTemp;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 3 + 12) = collisionTileTemp+8;


            delay(4);
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 1) = 0x48;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 1 + 4) = 0x49;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 1 + 8) = 0x58;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 1 + 12) = 0x59;
            delay(10);

            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 1) = 0x4a;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 1 + 4) = 0x4b;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 1 + 8) = 0x5a;
            *(unsigned char*)(0x200 + PLAYER_SPRITE_INDEX + 1 + 12) = 0x5b;
            delay(10);


            if (!playerInvulnerabilityTime) {
                damage_player(1, 0);
                if (gameState == GAME_STATE_GAME_OVER) {
                    playerXPosition = 999999;
                    playerYPosition = 999999;
                    return;
                }
            }
            // Boing
            playerXVelocity = 0 - playerXVelocity;
            playerYVelocity = 0 - playerYVelocity;
            playerGravityPullX = 0;
            playerGravityPullY = 0;

            // FIXME: This too
            
        } else if (collisionTileTemp == 4 && nearestCrack == 0xff) { // Crack
            nearestCrack = PLAYER_MAP_POSITION(collisionTempX, collisionTempY);
            crackTimer = 20;
            sfx_play(SFX_HOLE_TRIGGER, SFX_CHANNEL_2);
        }
    }

    // For lack of a better home
    if (crackTimer > 0) {
        --crackTimer;

        if (crackTimer == 0) {
            // Preserve palette, set hole
            currentMap[nearestCrack] = (currentMap[nearestCrack] & 0xc0) + 11;
            // Steal 1 frame to animate right here, right now
            collisionTileTemp = currentMap[nearestCrack] & 0x3f;
            collisionTileTemp = (((collisionTileTemp & 0xf8)) << 2) + ((collisionTileTemp & 0x07) << 1);
            screenBuffer[0] =  MSB(NTADR_A(((nearestCrack & 0x0f)<<1), ((nearestCrack & 0xf0) >>3)-0)) | NT_UPD_HORZ;
            screenBuffer[1] =  LSB(NTADR_A(((nearestCrack & 0x0f)<<1), ((nearestCrack & 0xf0) >>3)-0));
            screenBuffer[2] = 2;
            screenBuffer[3] = collisionTileTemp;
            screenBuffer[4] = collisionTileTemp+1;
            
            screenBuffer[5] =  MSB(NTADR_A((nearestCrack & 0x0f)<<1, ((nearestCrack & 0xf0) >>3)+1)) | NT_UPD_HORZ;
            screenBuffer[6] =  LSB(NTADR_A((nearestCrack & 0x0f)<<1, ((nearestCrack & 0xf0) >>3)+1));
            screenBuffer[7] = 2;
            screenBuffer[8] = collisionTileTemp + 16;
            screenBuffer[9] = collisionTileTemp + 17;
            screenBuffer[10] = NT_UPD_EOF;

            sfx_play(SFX_HOLE_FORM, SFX_CHANNEL_2);
            set_vram_update(screenBuffer);
            ppu_wait_nmi();
            set_vram_update(NULL);



            nearestCrack = 0xff;
        }
    }


    playerXPosition += playerXVelocity + playerGravityPullX;
    playerYPosition += playerYVelocity + playerGravityPullY;

}

void handle_player_sprite_collision(void) {
    // We store the last sprite hit when we update the sprites in `map_sprites.c`, so here all we have to do is react to it.
    if (lastPlayerSpriteCollisionId != NO_SPRITE_HIT) {
        currentMapSpriteIndex = lastPlayerSpriteCollisionId<<MAP_SPRITE_DATA_SHIFT;
        switch (currentMapSpriteData[(currentMapSpriteIndex) + MAP_SPRITE_DATA_POS_TYPE]) {
            case SPRITE_TYPE_HEALTH:
                // This if statement ensures that we don't remove hearts if you don't need them yet.
                if (playerHealth < playerMaxHealth) {
                    playerHealth += currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_HEALTH];
                    if (playerHealth > playerMaxHealth) {
                        playerHealth = playerMaxHealth;
                    }
                    // Hide the sprite now that it has been taken.
                    currentMapSpriteData[(currentMapSpriteIndex) + MAP_SPRITE_DATA_POS_TYPE] = SPRITE_TYPE_OFFSCREEN;

                    // Play the heart sound!
                    sfx_play(SFX_HEART, SFX_CHANNEL_3);

                    // Mark the sprite as collected, so we can't get it again.
                    currentMapSpritePersistance[playerOverworldPosition] |= bitToByte[lastPlayerSpriteCollisionId];
                }
                break;
            case SPRITE_TYPE_KEY:
                if (playerKeyCount < MAX_KEY_COUNT) {
                    playerKeyCount++;
                    currentMapSpriteData[(currentMapSpriteIndex) + MAP_SPRITE_DATA_POS_TYPE] = SPRITE_TYPE_OFFSCREEN;

                    sfx_play(SFX_KEY, SFX_CHANNEL_3);

                    // Mark the sprite as collected, so we can't get it again.
                    currentMapSpritePersistance[playerOverworldPosition] |= bitToByte[lastPlayerSpriteCollisionId];
                }
                break;
            case SPRITE_TYPE_REGULAR_ENEMY:
            case SPRITE_TYPE_INVULNERABLE_ENEMY:

                if (playerInvulnerabilityTime) {
                    return;
                }
                damage_player(currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_DAMAGE], 1); 
                
                break;
            case SPRITE_TYPE_DOOR: 
                // Doors without locks are very simple - they just open! Hide the sprite until the user comes back...
                // note that we intentionally *don't* store this state, so it comes back next time.
                currentMapSpriteData[(currentMapSpriteIndex) + MAP_SPRITE_DATA_POS_TYPE] = SPRITE_TYPE_OFFSCREEN;

                sfx_play(SFX_DOOR, SFX_CHANNEL_3);
                break;
            case SPRITE_TYPE_LOCKED_DOOR:
                // First off, do you have a key? If so, let's just make this go away...
                if (playerKeyCount >= screenKeyCounts[playerOverworldPosition]) {
                    playerKeyCount -= screenKeyCounts[playerOverworldPosition];
                    currentMapSpriteData[(currentMapSpriteIndex) + MAP_SPRITE_DATA_POS_TYPE] = SPRITE_TYPE_OFFSCREEN;

                    // Mark the door as gone, so it doesn't come back.
                    currentMapSpritePersistance[playerOverworldPosition] |= bitToByte[lastPlayerSpriteCollisionId];

                    sfx_play(SFX_DOOR, SFX_CHANNEL_3);
                    doorPosition = 0xff;

                    // Store a checkpoint while we're at it
                    lastCheckpointScreenId = playerOverworldPosition;
                    lastCheckpointLayer = currentLayer;
                    lastCheckpointKeyCount = playerKeyCount;
                    lastCheckpointPlayerX = playerXPosition;
                    lastCheckpointPlayerY = playerYPosition;


                    memcpy(&(lastCheckpointWorldState[0]), &(currentMapSpritePersistance[0]), 64);

                    ++playerDeathCount;

                    break;
                }
                // So you don't have a key...
                // Okay, we collided with a door before we calculated the player's movement. After being moved, does the 
                // new player position also collide? If so, stop it. Else, let it go.
                
                // This has MAJOR collision bugs! Instead, we store it and collide it using the tile collision method
                /*

                // Calculate position...
                tempSpriteCollisionX = ((currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_X]) + ((currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_X + 1]) << 8));
                tempSpriteCollisionY = ((currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_Y]) + ((currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_Y + 1]) << 8));

                // Are we colliding?
                // NOTE: We take a bit of a shortcut here and assume all doors are 16x16 (the hard-coded 16 value below)
                if (
                    playerXPosition < tempSpriteCollisionX + (16 << PLAYER_POSITION_SHIFT) &&
                    playerXPosition + PLAYER_WIDTH_EXTENDED > tempSpriteCollisionX &&
                    playerYPosition < tempSpriteCollisionY + (16 << PLAYER_POSITION_SHIFT) &&
                    playerYPosition + PLAYER_HEIGHT_EXTENDED > tempSpriteCollisionY
                ) {
                    playerXPosition -= playerXVelocity;
                    playerYPosition -= playerYVelocity;
                    playerControlsLockTime = 0;
                }
                */
                break;
            case SPRITE_TYPE_ENDGAME:
                gameState = GAME_STATE_CREDITS;
                break;
            case SPRITE_TYPE_RADIO:
                if (controllerState & PAD_A) {

                    if (currentLayer == currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_HEALTH]) {
                        do_layer_anim(currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_MOVE_SPEED]);
                    } else if (currentLayer == currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_MOVE_SPEED]) {
                        do_layer_anim(currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_HEALTH]);
                    }
                }
                break;
            case SPRITE_TYPE_NPC:
                // Okay, we collided with this NPC before we calculated the player's movement. After being moved, does the 
                // new player position also collide? If so, stop it. Else, let it go.

                // Calculate position...
                tempSpriteCollisionX = ((currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_X]) + ((currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_X + 1]) << 8));
                tempSpriteCollisionY = ((currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_Y]) + ((currentMapSpriteData[currentMapSpriteIndex + MAP_SPRITE_DATA_POS_Y + 1]) << 8));

                if (controllerState & PAD_A && !(lastControllerState & PAD_A)) {
                    // Show the text for the player on the first screen
                    /*switch (playerOverworldPosition) {
                        case 0:
                            if (currentLayer == 0) {
                                trigger_game_text(firstPageText);
                            } else {
                                trigger_game_text(corruptionText2);
                            }
                            break;
                        case 8:
                            if (currentLayer == 0) {
                                trigger_game_text(secondPageText);
                            } else if (currentLayer == 1) {
                                trigger_game_text(secondPageOtherDimensionText);
                            } else {
                                trigger_game_text(corruptionText);
                            }
                            break;
                        default:
                            trigger_game_text(defaultText);
                            break;
                    }*/

                    if (screenMessages[playerOverworldPosition] != 0) {
                        if (screenDimensions[playerOverworldPosition] == currentLayer) {
                            trigger_game_text(*screenMessages[playerOverworldPosition], 0);
                        } else {
                            trigger_game_text(*screenMessages[playerOverworldPosition], 1);
                        }
                    } else {
                        trigger_game_text(defaultText, screenDimensions[playerOverworldPosition] != currentLayer);
                    }
                }
                break;


        }

    }
}
