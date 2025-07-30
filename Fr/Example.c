
// Définitions des arguments
#include "DefinitionsArguments.h"
// LoadCel()
#include "celutils.h"
// printf()
#include "stdio.h"
// SpriteCel
#include "SpriteCel.h"
// AnimSpriteCel
#include "AnimSpriteCel.h"

int32 main(){
	
	// Cel
	CCB *cel = NULL;
	// SpriteCel
	SpriteCel *spriteCel = NULL;
	
	// Charge le CEL
	// printf("LoadCel()\n");
	cel = LoadCel("image.cel", MEMTYPE_DRAM);
	// Si c'est un échec
	if(cel == NULL){
		// Retourne une erreur
		printf("Error <- LoadCel()\n");
		return -1;
	}	
	
	// Crée un SpriteCel
	// printf("SpriteCelInitialization()\n");
	spriteCel = SpriteCelInitialization(cel, 14, 14, 9);
	// Si c'est un échec
	if(spriteCel == NULL){
		// Retourne une erreur
		printf("Error <- SpriteCelInitialization()\n");
		return -1;
	}	
	
	// Configure le SpriteCel
	printf("-> SpriteCelFramesConfiguration()\n");
	if (SpriteCelFramesConfiguration(
		// SpriteCel
		spriteCel,
		// Début de la liste
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
		// Fin de la liste
		LIST_END
	) == 0) {
		printf("Error <- SpriteCelFramesConfiguration()\n");
		return -1;
	}	
	
	// Affiche la cinquième frame
	SpriteCelSetFrame(spriteCel, 4);
	
	// Affiche la sixième frame
	SpriteCelNextFrame(spriteCel);
	
	// Supprime le SpriteCel
	SpriteCelCleanup(spriteCel);
	
	// Supprime le Cel
	UnloadCel(cel);
	
    // Termine le programme
    return 0;
}