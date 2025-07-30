
#include "Mathematical.h"

// ReadHardwareRandomNumber()
#include "hardware.h"

// Gets a random value between start and end
int32 GetRandomValue(int32 start, int32 end) {
    
    // Value range
    int32 range;
    // Random number
    uint32 randomNumber;
    
    // printf("*GetRandomValue()*\n");  
    
    // Initialize the expected value range
    range = end - start + 1;
    // Get a random number
    randomNumber = ReadHardwareRandomNumber();
    // Return the result
    return start + (randomNumber % range);
}
