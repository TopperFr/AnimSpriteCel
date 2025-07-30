
// Argument definitions
#include "DefinitionsArguments.h"
// LoadCel()
#include "celutils.h"
// printf()
#include "stdio.h"
// SpriteCel
#include "SpriteCel.h"
// AnimSpriteCel
#include "AnimSpriteCel.h"

int32 main() {
    
    // Cel
    CCB *cel = NULL;
    // SpriteCel
    SpriteCel *spriteCel = NULL;
    
    // Load the CEL
    // printf("LoadCel()\n");
    cel = LoadCel("image.cel", MEMTYPE_DRAM);
    // If loading fails
    if (cel == NULL) {
        // Return an error
        printf("Error <- LoadCel()\n");
        return -1;
    }   
    
    // Create a SpriteCel
    // printf("SpriteCelInitialization()\n");
    spriteCel = SpriteCelInitialization(cel, 14, 14, 9);
    // If initialization fails
    if (spriteCel == NULL) {
        // Return an error
        printf("Error <- SpriteCelInitialization()\n");
        return -1;
    }   
    
    // Configure the SpriteCel
    printf("-> SpriteCelFramesConfiguration()\n");
    if (SpriteCelFramesConfiguration(
        // SpriteCel
        spriteCel,
        // Start of the list
        LIST_START,
        // frameIndex, positionX, positionY
        0, 0, 0,
        1, 14, 0,
        2, 28, 0,
        3, 42, 0,
        4, 56, 0,
        5, 70, 0,
        6, 84, 0,
        7, 98, 0,
        8, 112, 0,
        // End of the list
        LIST_END
    ) == 0) {
        printf("Error <- SpriteCelFramesConfiguration()\n");
        return -1;
    }   
    
    // Display the fifth frame
    SpriteCelSetFrame(spriteCel, 4);
    
    // Display the sixth frame
    SpriteCelNextFrame(spriteCel);
    
    // Clean up the SpriteCel
    SpriteCelCleanup(spriteCel);
    
    // Unload the Cel
    UnloadCel(cel);
    
    // Exit the program
    return 0;
}
