
#include "AnimSpriteCel.h"

// UNDEFINED, LIST_START, LIST_END
#include "DefinitionsArguments.h"
// AllocMem(), FreeMem(), MEMTYPE_DRAM
#include "mem.h"
// CloneCel()
#include "celutils.h"
// GetRandomValue()
#include "Mathematical.h"
// memset()
#include "string.h"
// printf()
#include "stdio.h"

// Initialisation d'un AnimSpriteCel
AnimSpriteCel *AnimSpriteCelInitialization(SpriteCel *spriteCel, AnimSpriteCelLoop loop, AnimSpriteCelRange range, uint32 iterations, int32 direction, uint32 stepIndex, uint32 stepsCount) {

	// AnimSpriteCel
	AnimSpriteCel *animSpriteCel = NULL;
		
	if (DEBUG_ANIMSPRITECEL_INIT == 1) { printf("*AnimSpriteCelInitialization()*\n");	}
	
	// Si le sprite sheet n'existe pas
    if (spriteCel == NULL){
		// Affiche un message d'erreur
		printf("Error : SpriteCel unknow.\n");
        return NULL;
	} 	
	
	// Alloue de la mémoire pour le AnimSpriteCel
	animSpriteCel = (AnimSpriteCel *)AllocMem(sizeof(AnimSpriteCel), MEMTYPE_DRAM);
	// Si c'est un échec
    if (animSpriteCel == NULL) {
		// Affiche un message d'erreur
        printf("Error : Failed to allocate memory for AnimSpriteCel.\n");
        return NULL;
    }

	// Corrige les paramètres 
	// -> Nombre minimal d'étapes = 2
	stepsCount = (stepsCount > 1) ? stepsCount : 2;
	// -> Etape de départ < étapes totales
	stepIndex = (stepIndex < stepsCount) ? stepIndex : 0;
	// -> +1 ou -1
	direction = (direction == -1) ? -1 : 1;
	
	// CCB du SpriteCel
	animSpriteCel->spriteCel = spriteCel;
	// Type de boucle d'animation
	animSpriteCel->loop = loop;
	// Plage de valeurs de l'aléatoire
	animSpriteCel->range = range;
	// Nombre de cycles restants avant le changement
	animSpriteCel->remainingCycles = 0;
	// Répétitions du cycle d'animation
	animSpriteCel->iterationsCount = iterations;
	// Sens d'animation en cours
	animSpriteCel->direction = direction;
	// Etape en cours
	animSpriteCel->stepIndex = stepIndex;
	// Nombre total d'étapes
    animSpriteCel->stepsCount = stepsCount;	
	// Copie le CCB du SpriteCel
	animSpriteCel->cel = animSpriteCel->spriteCel->cel;
	
	// Copie le CCB du SpriteCel
	animSpriteCel->cel = CloneCel(animSpriteCel->spriteCel->cel, CLONECEL_CCB_ONLY);
	// Force la lecture des préambules dans le CCB
	animSpriteCel->cel->ccb_Flags |= CCB_CCBPRE;

    // Alloue de la mémoire pour le tableau d'étapes
    animSpriteCel->steps = (AnimSpriteCelStep *)AllocMem(stepsCount * sizeof(AnimSpriteCelStep), MEMTYPE_DRAM);
	// Si c'est un échec
    if (animSpriteCel->steps == NULL) {
		// Libère la mémoire précédemment allouée
        FreeMem(animSpriteCel, sizeof(AnimSpriteCel));
		// Affiche un message d'erreur
        printf("Error : Failed to allocate memory for AnimSpriteCel steps.\n");
        return NULL;
    }

    // Initialise les valeurs des étapes à 0 
    memset(animSpriteCel->steps, 0, (size_t)stepsCount * sizeof(AnimSpriteCelStep));

	// Retourne le AnimSpriteCel créé
    return animSpriteCel;
}

// Configuration d'une étape d'un AnimSpriteCel
int32 AnimSpriteCelStepConfiguration(AnimSpriteCel *animSpriteCel, uint32 stepIndex, uint32 frameIndex, int32 frameDuration, AnimSpriteCel *animSpriteCelReceiver) {
	
 	if (DEBUG_ANIMSPRITECEL_SETUP == 1) { printf("*AnimSpriteCelStepConfiguration()*\n"); }	

	// Si l'animation est inconnue
	if (animSpriteCel == NULL){
		// Retourne une erreur
		printf("Error : AnimSpriteCel unknow.\n");
		return -1;	
	}
	
	// Si le SpriteCel est inconnu
	if (animSpriteCel->spriteCel == NULL){
		// Retourne une erreur
		printf("Error : AnimSpriteCel SpriteCel unknow.\n");
		return -1;	
	}
	
	// Si le tableau d'étapes est inconnu
	if (animSpriteCel->steps == NULL){
		// Retourne une erreur
		printf("Error : AnimSpriteCel steps unknow.\n");
		return -1;
		
	}
	
	// Si l'animation ne comporte pas au moins deux étapes
	if (animSpriteCel->stepsCount < 2){
		// Retourne une erreur
		printf("Error : AnimSpriteCel needs at least two steps.\n");
		return -1;
		
	}

	// Corrige les paramètres
	if (stepIndex >= animSpriteCel->stepsCount) { 
		// Affiche un avertissement
		printf("Warning : AnimSpriteCel stepIndex %u out of bounds. Clamped to last index.\n", stepIndex);
		// Modifie l'index au dernier disponible
		stepIndex = animSpriteCel->stepsCount - 1; 
	}
	
    // Configure l'étape
    animSpriteCel->steps[stepIndex].frameIndex = frameIndex;
    animSpriteCel->steps[stepIndex].frameDuration = frameDuration;
    animSpriteCel->steps[stepIndex].animSpriteCelReceiver = animSpriteCelReceiver;
	
	if (DEBUG_ANIMSPRITECEL_SETUP == 1) { 
		printf("animSpriteCel->cel : %p\n", animSpriteCel->cel);
		printf("animSpriteCel->steps[%u].frameIndex : %u\n", stepIndex, animSpriteCel->steps[stepIndex].frameIndex);
		printf("animSpriteCel->steps[%u].frameDuration : %d\n", stepIndex, animSpriteCel->steps[stepIndex].frameDuration);
		printf("animSpriteCel->steps[%u].animSpriteCelReceiver : %p\n", stepIndex, animSpriteCel->steps[stepIndex].animSpriteCelReceiver);
	}
	
	// Si l'étape configurée est celle affichée
	if(stepIndex == animSpriteCel->stepIndex){
		// Mets à jour le CCB principal du AnimSpriteCel
		AnimSpriteCelUpdate(animSpriteCel);
	}
	
	// Retourne un succès
	return 1;
}

// Configuration des étapes d'un AnimSpriteCel
int32 AnimSpriteCelStepsConfiguration(AnimSpriteCel *animSpriteCel, int32 start, ...) {

	// Liste des arguments de la fonction
    va_list args;
	// Index de l'étape
	uint32 stepIndex = 0;
	// Coordonnées
    uint32 frameIndex = 0;
	int32 frameDuration = 1;
	AnimSpriteCel *animSpriteCelReceiver = NULL;
	
	if (DEBUG_ANIMSPRITECEL_SETUP == 1) { printf("*AnimSpriteCelStepsConfiguration()*\n"); }		

    // Débute la lecture des arguments
    va_start(args, start);

    // Boucle jusqu'à la valeur sentinelle
    while (1) {

        // Récupère l'index
        stepIndex = va_arg(args, uint32);

        // Si c'est la fin de la liste
        if (stepIndex == LIST_END) {
			// Sort de la boucle
            break;
        }

        // Récupère le reste des arguments
		frameIndex = va_arg(args, uint32);
		frameDuration = va_arg(args, int32);
		animSpriteCelReceiver = va_arg(args, AnimSpriteCel *);

		// Configuration d'une step d'un sprite
		if (DEBUG_ANIMSPRITECEL_SETUP == 1) { printf("-> AnimSpriteCelStepConfiguration(%p, %u, %u, %d, %p)\n", animSpriteCel, stepIndex, frameIndex, frameDuration, animSpriteCelReceiver); }
		if (AnimSpriteCelStepConfiguration(animSpriteCel, stepIndex, frameIndex, frameDuration, animSpriteCelReceiver) < 0) {
			// Fin de la liste des arguments 
			va_end(args);
            // Retourne une erreur
            printf("Error <- AnimSpriteCelStepConfiguration()\n");
            return -1;
        }
    }

    // Fin de la liste des arguments 
    va_end(args);

    // Retourne un succès
    return 1;
}

// Mets à jour l'affichage d'un AnimSpriteCel
void AnimSpriteCelUpdate(AnimSpriteCel *animSpriteCel) {
	
	// Valeur maximale de la plage de valeurs aléatoires
	uint32 randomRangeMax = 0;
	
	if (DEBUG_ANIMSPRITECEL_FUNCT == 1) { printf("*AnimSpriteCelUpdate()*\n"); }

	// Affiche le CCB de l'étape en cours
	SpriteCelSetFrame(animSpriteCel->spriteCel, animSpriteCel->steps[animSpriteCel->stepIndex].frameIndex);	
	
	// Copie le contenu du CCB du SpriteCel vers AnimSpriteCel
    animSpriteCel->cel->ccb_PRE0 = animSpriteCel->spriteCel->cel->ccb_PRE0;
    animSpriteCel->cel->ccb_PRE1 = animSpriteCel->spriteCel->cel->ccb_PRE1;
    animSpriteCel->cel->ccb_SourcePtr = animSpriteCel->spriteCel->cel->ccb_SourcePtr;

	// Si la durée de la frame est positive
	if (animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration > 1) {
		// Récupère la durée
		animSpriteCel->remainingCycles = animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration;
		
	// Si la durée de la frame est négative
	} else if (animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration < 1) {
		// Récupère la valeur maximale
		randomRangeMax = 0 - animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration;
		// Selon la plage de valeurs
		switch (animSpriteCel->range) {
			// Plage de valeur entre 1 et le maximum
			case FULL:    
				animSpriteCel->remainingCycles = GetRandomValue(1, randomRangeMax); 
				break;
			// Plage de valeur entre (maximum / 2) et le maximum
			case HALF:    
				animSpriteCel->remainingCycles = GetRandomValue(randomRangeMax / 2, randomRangeMax);  
				break;
			// Plage de valeur entre (maximum / 4) * 3 et le maximum
			case QUARTER: 
				animSpriteCel->remainingCycles = GetRandomValue(randomRangeMax - (randomRangeMax / 4), randomRangeMax);
				break;
		}
	// Sinon la durée est 0
	} else {
		// L'animation est en attente de déclenchement
		animSpriteCel->remainingCycles = 0; 
	}	
}

// Passe à l'étape suivante de l'animation
void AnimSpriteCelNextStep(AnimSpriteCel *animSpriteCel) {
	
	// Témoin de fin de cycle
	uint32 cycleEnd = 0;
	
	if (DEBUG_ANIMSPRITECEL_FUNCT == 1) { printf("*AnimSpriteCelNextStep()*\n"); }

	// Selon le mode
	switch (animSpriteCel->loop) {
		// L'animation se déroule en avant à chaque cycle
		case NORMAL:   
			// Avance
			animSpriteCel->stepIndex++; 
			// Si l'étape dépasse le maximum
			if (animSpriteCel->stepIndex >= animSpriteCel->stepsCount) { 
				// Reviens au début
				animSpriteCel->stepIndex = 0;
				// Indique que la fin du cycle a été atteint
				cycleEnd = 1;
			}
			break;
			
		// L'animation se déroule en arrière à chaque cycle	
		case REVERSE:  
			// Recule
			animSpriteCel->stepIndex--; 
			// Si l'étape dépasse le minimum
			if (animSpriteCel->stepIndex < 0) { 
				// Retourne à la fin
				animSpriteCel->stepIndex = animSpriteCel->stepsCount - 1;
				// Indique que la fin du cycle a été atteint
				cycleEnd = 1;
			}
			break;
			
		// L'animation se déroule en avant, puis en arrière au cycle suivant	
		case ALTERNATE:
			// Avance ou recule selon la direction
			animSpriteCel->stepIndex += animSpriteCel->direction;

			// Si l'étape dépasse le maximum
			if (animSpriteCel->stepIndex >= animSpriteCel->stepsCount) { 
				// Reviens au début
				animSpriteCel->stepIndex = 0;
				// Change la direction
				animSpriteCel->direction *= -1;
				// Indique que la fin du cycle a été atteint
				cycleEnd = 1;
			// Si l'étape dépasse le minimum
			}else if (animSpriteCel->stepIndex < 0) { 
				// Retourne à la fin
				animSpriteCel->stepIndex = animSpriteCel->stepsCount - 1;
				// Change la direction
				animSpriteCel->direction *= -1;
				// Indique que la fin du cycle a été atteint
				cycleEnd = 1;
			}
			break;
	}

	// Mets à jour le CCB principal du AnimSpriteCel
	AnimSpriteCelUpdate(animSpriteCel);
	
	// Si c'est la fin d'un cycle et l'animation n'est pas infinie
	if ((cycleEnd == 1) && (animSpriteCel->iterationsCount != INFINITE)) { 
		// Décrémente le compteur de cycles restants
		animSpriteCel->iterationsCount--; 
	}
	
	// Si il y a une animation à contrôler
	if (animSpriteCel->steps[animSpriteCel->stepIndex].animSpriteCelReceiver != NULL) {
		// Envoie un déclenchement de la suite  
		AnimSpriteCelTrigger(animSpriteCel->steps[animSpriteCel->stepIndex].animSpriteCelReceiver);
	}
}

// Exécution de l'animation
void AnimSpriteCelRun(AnimSpriteCel *animSpriteCel) {
	
	if (DEBUG_ANIMSPRITECEL_FUNCT == 1) { printf("*AnimSpriteCelRun()*\n"); }

	// Si l'animation est inconnue
	if (animSpriteCel == NULL){
		// Retourne une erreur
		printf("Error : AnimSpriteCel unknow.\n");
		return;	
	}
	
	// Si le SpriteCel est inconnu
	if (animSpriteCel->spriteCel == NULL){
		// Retourne une erreur
		printf("Error : AnimSpriteCel SpriteCel unknow.\n");
		return;	
	}
	
	// Si le tableau d'étapes est inconnu
	if (animSpriteCel->steps == NULL){
		// Retourne une erreur
		printf("Error : AnimSpriteCel steps unknow.\n");
		return;
		
	}
	
	// Si l'animation est en attente d'un déclencheur
	if (animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration == 0){
		// Quitte prématurément
		return;
	}
	
	// Si toutes les itérations ont été réalisées
	if (animSpriteCel->iterationsCount == 0){
		// Quitte prématurément
		return;
	}
	
	// Si ce n'est pas le moment de changer d'étape
	if(animSpriteCel->remainingCycles > 0){	
		// Décrémente le nombre de cycles d'affichage
		animSpriteCel->remainingCycles--;
		// Quitte prématurément
		return;
	}

	// Passe à l'étape suivante de l'animation
	AnimSpriteCelNextStep(animSpriteCel);
	
}

// Déclencheur de l'animation en attente
void AnimSpriteCelTrigger(AnimSpriteCel *animSpriteCel) {
	
	if (DEBUG_ANIMSPRITECEL_FUNCT == 1) { printf("*AnimSpriteCelTrigger()*\n"); }

	// Si l'animation est inconnue
	if (animSpriteCel == NULL){
		// Retourne une erreur
		printf("Error : AnimSpriteCel unknow.\n");
		return;	
	}
	
	// Si le SpriteCel est inconnu
	if (animSpriteCel->spriteCel == NULL){
		// Retourne une erreur
		printf("Error : AnimSpriteCel SpriteCel unknow.\n");
		return;	
	}
	
	// Si le tableau d'étapes est inconnu
	if (animSpriteCel->steps == NULL){
		// Retourne une erreur
		printf("Error : AnimSpriteCel steps unknow.\n");
		return;
		
	}
	
	// Si l'animation n'est pas en attente d'un déclencheur
	if (animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration != 0){
		// Quitte prématurément
		return;
	}
	
	// Si toutes les itérations ont été réalisées
	if (animSpriteCel->iterationsCount == 0){
		// Quitte prématurément
		return;
	}

	// Passe à l'étape suivante de l'animation
	AnimSpriteCelNextStep(animSpriteCel);
	
}

// Supprime le AnimSpriteCel
int32 AnimSpriteCelCleanup(AnimSpriteCel *animSpriteCel) {
		
	if (DEBUG_ANIMSPRITECEL_CLEAN == 1) { printf("*AnimSpriteCelCleanup()*\n"); }	
	
	// Si le sprite n'existe pas
    if (animSpriteCel == NULL){
		// Affiche une erreur
		printf("Error : AnimSpriteCel unknow.\n");
		return -1;	
	} 

	// Si il y a un Cel
    if (animSpriteCel->cel != NULL) {
		// Supprime le Cel du sprite
		DeleteCel(animSpriteCel->cel);
		animSpriteCel->cel = NULL;
    }
	
	// Si il y a des steps
    if (animSpriteCel->steps != NULL) {
		// Libère la mémoire utilisée pour le tableau de steps
        FreeMem(animSpriteCel->steps, animSpriteCel->stepsCount * sizeof(AnimSpriteCelStep));
        animSpriteCel->steps = NULL;
    }
	
	// Libère la mémoire utilisée pour le AnimSpriteCel
    FreeMem(animSpriteCel, sizeof(AnimSpriteCel));
	animSpriteCel->spriteCel = NULL;
	
	// Finalise le nettoyage
	animSpriteCel = NULL;

	// Retourne un succès
    return 1;
}