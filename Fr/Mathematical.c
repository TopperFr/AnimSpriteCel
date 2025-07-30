
#include "Mathematical.h"

// ReadHardwareRandomNumber()
#include "hardware.h"

// Obtient une valeur aléatoire entre start et end
int32 GetRandomValue(int32 start, int32 end) {
	
	// Plage de valeurs
    int32 range;
	// Nombre aléatoire
    uint32 randomNumber;
	
	// printf("*GetRandomValue()*\n");	
	
	// Initialise la plage de valeurs attendue
    range = end - start + 1;
	// Obtient un nombre aléatoire
    randomNumber = ReadHardwareRandomNumber();
	// Renvoie le résultat
    return start + (randomNumber % range);
}