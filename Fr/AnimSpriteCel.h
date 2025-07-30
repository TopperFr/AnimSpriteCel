#ifndef ANIMSPRITECEL_H
#define ANIMSPRITECEL_H

/******************************************************************************
**
**  AnimSpriteCel - Système d'animation pour SpriteCel (3DO Cel Engine)
**  
**  Auteur : Christophe Geoffroy (Topper) - Licence MIT
**  Dernière mise à jour : 30-07-2025
**
**  Ce module fournit une structure d'animation indépendante utilisant un SpriteCel
**  composé de plusieurs frames. L'animation est définie comme une séquence 
**  d'étapes ("AnimSpriteCelStep"), chacune pointant vers une frame du SpriteCel 
**  et possédant sa propre durée. La suite d'étapes composant l'animation représente
**  un cycle d'animation. Elle est pilotée par une structure principale 
**  "AnimSpriteCel" et s'utilise comme une timeline cyclique. L'animation peut se 
**  dérouler en avant, en arrière, ou changer de sens alternativement à chaque fin 
**  de cycle. Le cycle d'animation peut se dérouler une seule fois, plusieurs fois 
**  ou indéfiniment.
**
**  La durée d'affichage d'une étape est définine par le nombre de cycles d'affichage.
**  En donnant une valeur négative, la durée d'affichage est aléatoire en prenant la
**  valeur absolue comme maximum, et 0 comme minimum. Il est possible de pondérer la
**  durée aléatoire en réduisant à La moitié ou le quart supérieur la plage de valeurs.
**
**  Une étape peut interrompre le cycle d'animation en fixant sa durée à 0. Dans ce
**  cas, l'AnimSpriteCel est considéré en attente. La reprise de l'animation est
**  réalisée par un autre AnimSpriteCel à qui l'ont a indiqué un AnimSpriteCel
**  receveur à une frame. Il est possible de contrôler la reprise de plusieurs  
**  AnimSpriteCel en cascade. Deux AnimSpriteCels peuvent aussi se contrôler 
**  mutuellement. Ainsi, un premier AnimSpriteCel peut faire son cycle d'animation,
**  puis se mettre en attente tout en relançant le cycle d'animation du second 
**  AnimSpriteCel. A son tour, le second AnimSpriteCel réalise la même séquence et
**  relance le premier AnimSpriteCel qui est en attente. De cette manière, il est 
**  possible de faire interagir ensemble plusieurs AnimSpriteCels.
**
**  Notes importantes :
**
**    - L'affichage du SpriteCel utilisé est directement affecté par l'AnimSpriteCel
**      car celui-ci modifie la frame courante affichée. Si un SpriteCel doit être affiché
**      indépendamment, il est conseillé de créer un clone dédié uniquement au AnimSpriteCel.
**      Une autre solution est de stocker la frame courante durant l'exécution, puis
**      appeler SpriteCelSetFrame() pour rétablir la bonne position après AnimSpriteCelRun().
**
**    - Un unique SpriteCel peut être utilisé pour de multiples AnimSpriteCels qui ont leurs 
**      propres séquences d'animation. Il n'est pas nécessaire de créer autant de SpriteCels
**      que de AnimSpriteCels.
**
**    - Lorsqu'un AnimSpriteCel est initialisé, il doit être supprimé avec AnimSpriteCelCleanup().
**      Le SpriteCel qui lui a été associé doit être supprimé indépendamment.
**
**  Rôle des structures :
**
**    AnimSpriteCelStep
**      - frameIndex : index de la frame du SpriteCel à afficher
**      - frameDuration : durée d'affichage en cycles (int32)
**                        > 1 -> durée fixe
**                        = 1 -> changement immédiat
**                        = 0 -> en attente d'un déclenchement
**                        < 0 -> durée aléatoire (entre 1 et abs(valeur)), pondérée via "range"
**      - animSpriteCelReceiver : pointeur vers un autre AnimSpriteCel à qui est envoyé un 
**                                déclenchement de l'étape suivante si il est en attente
**
**    AnimSpriteCel
**      - cel : CCB principal animé (copie du SpriteCel)
**      - spriteCel : référence au SpriteCel source (tableau de frames)
**      - loop : type de boucle (NORMAL, REVERSE, ALTERNATE)
**      - range : plage de valeurs utilisée pour l'aléatoire (FULL, HALF, QUARTER)
**      - remainingCycles : nombre d'exécutions de AnimSpriteCelRun() avant l'étape suivante
**      - iterationsCount : nombre de répétitions du cycle (illimité = INFINITE)
**      - direction : sens de l'animation (1 = en avant, -1 en arrière)
**      - stepIndex : étape courante dans le tableau "steps"
**      - stepsCount : nombre total d'étapes dans l'animation
**      - steps : tableau dynamique de "AnimSpriteCelStep"
**
**  Fonctions principales :
**
**    AnimSpriteCelInitialization()
**      -> Initialise l'animation, clone le CCB et prépare le tableau des étapes.
**
**    AnimSpriteCelStepConfiguration()
**      -> Définit une étape d'animation : frame à afficher, durée associée et
**         pointeur vers un autre AnimSpriteCel
**
**    AnimSpriteCelStepsConfiguration()
**      -> Définit plusieurs étapes en une seule passe avec des arguments variadiques.
**
**    AnimSpriteCelUpdate()
**      -> Fonction interne permettant de mettre à jour l'affichage.
**         Elle est appelée par AnimSpriteCelNextStep() lorsque c'est nécessaire.
**
**    AnimSpriteCelNextStep()
**      -> Fonction interne permettant de passer à l'étape suivante.
**         Elle est appelée par AnimateSpriteCelRun() ou AnimSpriteCelTrigger() 
**         lorsque c'est nécessaire.
**
**    AnimateSpriteCelRun()
**      -> Fonction d'évolution à appeler à chaque cycle d'affichage. 
**         Contrôle le passage à l'étape suivante.
**
**    AnimSpriteCelTrigger()
**      -> Fonction interne permettant de déclencher l'étape suivante sur un 
**         autre AnimSpriteCel en attente.
**
**    AnimSpriteCelCleanup()
**      -> Libère la mémoire utilisée par la structure AnimSpriteCel
**
******************************************************************************/

// CCB
#include "graphics.h"
// int32
#include "types.h"
// SpriteCel
#include "SpriteCel.h"

// Debug
#define DEBUG_ANIMSPRITECEL_INIT 0
#define DEBUG_ANIMSPRITECEL_SETUP 0
#define DEBUG_ANIMSPRITECEL_FUNCT 0
#define DEBUG_ANIMSPRITECEL_CLEAN 0

// Enumération des types de zones interactives
typedef enum {
	// L'animation se déroule en avant à chaque cycle
    NORMAL,
	// L'animation se déroule en arrière à chaque cycle
    REVERSE,
	// L'animation se déroule en avant, puis en arrière au cycle suivant
    ALTERNATE
} AnimSpriteCelLoop;

// Plage de valeurs de l'aléatoire
typedef enum {
	// Plage de valeur entre 1 et le maximum
    FULL,
	// Plage de valeur entre (maximum / 2) et le maximum
    HALF,
	// Plage de valeur entre (maximum / 4) * 3 et le maximum
    QUARTER
} AnimSpriteCelRange;

typedef struct AnimSpriteCel AnimSpriteCel;

typedef struct {
	// Frame affichée
	uint32 frameIndex;
	// Durée de la frame
	// (Une valeur négative correspond à une valeur aléatoire)
	int32 frameDuration;
	// AnimSpriteCel vers lequel envoyé un déclenchement
    AnimSpriteCel *animSpriteCelReceiver;
	// Type de boucle d'animation
} AnimSpriteCelStep;

struct AnimSpriteCel {
	// CCB principal du sprite animé
	CCB *cel;
	// CCB du SpriteCel
    SpriteCel *spriteCel;
	// Type de boucle d'animation
	AnimSpriteCelLoop loop;
	// Plage de valeurs de l'aléatoire
	AnimSpriteCelRange range;
	// Nombre de cycles restants avant le changement
	uint32 remainingCycles;
	// Répétitions du cycle d'animation
	uint32 iterationsCount;
	// Sens d'animation en cours
	int32 direction;
	// Etape en cours
	int32 stepIndex;
	// Nombre total d'étapes
    uint32 stepsCount;
	// Tableau d'étapes
    AnimSpriteCelStep *steps;
};

// Référence au contexte global
extern AnimSpriteCel animSpriteCel;

// Initialisation d'un AnimSpriteCel
AnimSpriteCel *AnimSpriteCelInitialization(SpriteCel *spriteCel, AnimSpriteCelLoop loop, AnimSpriteCelRange range, uint32 iterations, int32 direction, uint32 stepIndex, uint32 stepsCount);
// Configuration d'une étape d'un AnimSpriteCel
int32 AnimSpriteCelStepConfiguration(AnimSpriteCel *animSpriteCel, uint32 stepIndex, uint32 frameIndex, int32 frameDuration, AnimSpriteCel *animSpriteCelReceiver);
// Configuration des étapes d'un AnimSpriteCel
int32 AnimSpriteCelStepsConfiguration(AnimSpriteCel *spriteCel, int32 start, ...);
// Mets à jour l'affichage d'un AnimSpriteCel
void AnimSpriteCelUpdate(AnimSpriteCel *animSpriteCel);
// Passe à l'étape suivante de l'animation
void AnimSpriteCelNextStep(AnimSpriteCel *animSpriteCel);
// Exécution de l'animation
void AnimSpriteCelRun(AnimSpriteCel *animSpriteCel);
// Déclencheur de l'animation en attente
void AnimSpriteCelTrigger(AnimSpriteCel *animSpriteCel);
// Supprime le AnimSpriteCel
int32 AnimSpriteCelCleanup(AnimSpriteCel *spriteCel);

#endif // ANIMSPRITECEL_H