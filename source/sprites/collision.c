#include "source/sprites/collision.h"
#include "source/library/bank_helpers.h"
#include "source/globals.h"
#include "source/sprites/player.h"
#include "source/menus/error.h"
#include "source/map/map.h"
#include "source/configuration/game_states.h"

CODE_BANK(PRG_BANK_SPRITE_COLLISION);
ZEROPAGE_DEF(unsigned char, collisionTemp);

// 0: no collision, 1: Solid, 2: liquid (treated as solid for now), 3: hole, 4: collapsible (persists as hole), 5: quicksand
// NOTE: 32 could be used if we feel so inclined. would be 160 tiles so no need to change anything outside this
// and the collision method below for its shift
const unsigned char tileCollisions[] = {
    // 32 for each layer - first is "normal"
    0, 0, 2, 2, 0, 0, 2, 2,
    1, 1, 1, 3, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // green2
    0, 0, 2, 2, 0, 0, 2, 2,
    1, 1, 1, 3, 1, 0, 1, 0,
    0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // desert
    0, 0, 0, 0, 0, 0, 2, 2,
    1, 1, 1, 3, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // dark desert/comp peek
    0, 0, 0, 0, 0, 0, 2, 2,
    1, 1, 0, 3, 1, 0, 1, 4,
    0, 0, 0, 0, 0, 0, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    // Computer
    0, 0, 0, 0, 0, 0, 0, 2,
    1, 1, 0, 3, 1, 0, 1, 3,
    0, 0, 0, 0, 0, 0, 3, 4,
    0, 0, 0, 0, 0, 0, 0, 0
};


unsigned char test_collision(unsigned char tilePos, unsigned char isPlayer) {
    // The top 2 bits of the tile id are the palette color. We don't care about that here, so skip them.
    collisionTemp = currentMap[tilePos] & 0x3f;

    if (tilePos == doorPosition) {
        return 1;
    }

    // Jump up to pos in tileCollisios above
    collisionTemp += (currentLayer << 5);

    collisionTemp = tileCollisions[collisionTemp];

    if (collisionTemp == 3) {
        if (isPlayer) {
            if (nearestHole == 0xff) {
                nearestHole = tilePos;
            }
            return 0;
        }
    }

    if (collisionTemp == 4) {
        if (isPlayer) {
            return 0;
        }
    }


    return collisionTemp;
}