
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

// Initialization of an AnimSpriteCel
AnimSpriteCel *AnimSpriteCelInitialization(SpriteCel *spriteCel, AnimSpriteCelLoop loop, AnimSpriteCelRange range, uint32 iterations, int32 direction, uint32 stepIndex, uint32 stepsCount) {

    // AnimSpriteCel instance
    AnimSpriteCel *animSpriteCel = NULL;
        
    if (DEBUG_ANIMSPRITECEL_INIT == 1) { printf("*AnimSpriteCelInitialization()*\n"); }

    // If the sprite sheet doesn't exist
    if (spriteCel == NULL) {
        // Display error message
        printf("Error: SpriteCel unknown.\n");
        return NULL;
    }

    // Allocate memory for AnimSpriteCel
    animSpriteCel = (AnimSpriteCel *)AllocMem(sizeof(AnimSpriteCel), MEMTYPE_DRAM);
    // If allocation fails
    if (animSpriteCel == NULL) {
        // Display error message
        printf("Error: Failed to allocate memory for AnimSpriteCel.\n");
        return NULL;
    }

    // Parameter corrections
    // → Minimum number of steps = 2
    stepsCount = (stepsCount > 1) ? stepsCount : 2;
    // → Starting step must be within total steps
    stepIndex = (stepIndex < stepsCount) ? stepIndex : 0;
    // → Direction must be either +1 or -1
    direction = (direction == -1) ? -1 : 1;

    // Assign the source SpriteCel
    animSpriteCel->spriteCel = spriteCel;
    // Animation loop type
    animSpriteCel->loop = loop;
    // Random range configuration
    animSpriteCel->range = range;
    // Remaining cycles before next step
    animSpriteCel->remainingCycles = 0;
    // Number of iterations for the animation cycle
    animSpriteCel->iterationsCount = iterations;
    // Current animation direction
    animSpriteCel->direction = direction;
    // Current step index
    animSpriteCel->stepIndex = stepIndex;
    // Total number of steps
    animSpriteCel->stepsCount = stepsCount;

    // Clone the SpriteCel CCB (Command Control Block)
    animSpriteCel->cel = CloneCel(animSpriteCel->spriteCel->cel, CLONECEL_CCB_ONLY);
    // Enable preamble parsing on the cloned CCB
    animSpriteCel->cel->ccb_Flags |= CCB_CCBPRE;

    // Allocate memory for the step array
    animSpriteCel->steps = (AnimSpriteCelStep *)AllocMem(stepsCount * sizeof(AnimSpriteCelStep), MEMTYPE_DRAM);
    // If step allocation fails
    if (animSpriteCel->steps == NULL) {
        // Free previously allocated AnimSpriteCel
        FreeMem(animSpriteCel, sizeof(AnimSpriteCel));
        // Display error message
        printf("Error: Failed to allocate memory for AnimSpriteCel steps.\n");
        return NULL;
    }

    // Initialize step data to zero
    memset(animSpriteCel->steps, 0, (size_t)stepsCount * sizeof(AnimSpriteCelStep));

    // Return the newly created AnimSpriteCel
    return animSpriteCel;
}

// Configuration of a step in an AnimSpriteCel
int32 AnimSpriteCelStepConfiguration(AnimSpriteCel *animSpriteCel, uint32 stepIndex, uint32 frameIndex, int32 frameDuration, AnimSpriteCel *animSpriteCelReceiver) {
    
    if (DEBUG_ANIMSPRITECEL_SETUP == 1) { printf("*AnimSpriteCelStepConfiguration()*\n"); }

    // If the AnimSpriteCel is undefined
    if (animSpriteCel == NULL){
        // Return error
        printf("Error: AnimSpriteCel unknown.\n");
        return -1;  
    }
    
    // If the SpriteCel is undefined
    if (animSpriteCel->spriteCel == NULL){
        // Return error
        printf("Error: AnimSpriteCel SpriteCel unknown.\n");
        return -1;  
    }
    
    // If the steps array is undefined
    if (animSpriteCel->steps == NULL){
        // Return error
        printf("Error: AnimSpriteCel steps unknown.\n");
        return -1;
    }
    
    // If the animation has fewer than two steps
    if (animSpriteCel->stepsCount < 2){
        // Return error
        printf("Error: AnimSpriteCel needs at least two steps.\n");
        return -1;
    }

    // Clamp stepIndex if out of bounds
    if (stepIndex >= animSpriteCel->stepsCount) { 
        // Display warning
        printf("Warning: AnimSpriteCel stepIndex %u out of bounds. Clamped to last index.\n", stepIndex);
        // Adjust to the last valid index
        stepIndex = animSpriteCel->stepsCount - 1; 
    }
    
    // Configure the animation step
    animSpriteCel->steps[stepIndex].frameIndex = frameIndex;
    animSpriteCel->steps[stepIndex].frameDuration = frameDuration;
    animSpriteCel->steps[stepIndex].animSpriteCelReceiver = animSpriteCelReceiver;
    
    if (DEBUG_ANIMSPRITECEL_SETUP == 1) {
        printf("animSpriteCel->cel : %p\n", animSpriteCel->cel);
        printf("animSpriteCel->steps[%u].frameIndex : %u\n", stepIndex, animSpriteCel->steps[stepIndex].frameIndex);
        printf("animSpriteCel->steps[%u].frameDuration : %d\n", stepIndex, animSpriteCel->steps[stepIndex].frameDuration);
        printf("animSpriteCel->steps[%u].animSpriteCelReceiver : %p\n", stepIndex, animSpriteCel->steps[stepIndex].animSpriteCelReceiver);
    }
    
    // If the configured step is currently displayed
    if (stepIndex == animSpriteCel->stepIndex){
        // Update the main CCB of the AnimSpriteCel
        AnimSpriteCelUpdate(animSpriteCel);
    }
    
    // Return success
    return 1;
}

// Configuration of multiple steps in an AnimSpriteCel
int32 AnimSpriteCelStepsConfiguration(AnimSpriteCel *animSpriteCel, int32 start, ...) {

    // Function argument list
    va_list args;
    // Step index
    uint32 stepIndex = 0;
    // Frame data
    uint32 frameIndex = 0;
    int32 frameDuration = 1;
    AnimSpriteCel *animSpriteCelReceiver = NULL;

    if (DEBUG_ANIMSPRITECEL_SETUP == 1) { printf("*AnimSpriteCelStepsConfiguration()*\n"); }

    // Begin reading arguments
    va_start(args, start);

    // Loop until sentinel value
    while (1) {

        // Retrieve step index
        stepIndex = va_arg(args, uint32);

        // End of argument list
        if (stepIndex == LIST_END) {
            // Exit loop
            break;
        }

        // Retrieve remaining step parameters
        frameIndex = va_arg(args, uint32);
        frameDuration = va_arg(args, int32);
        animSpriteCelReceiver = va_arg(args, AnimSpriteCel *);

        // Step configuration
        if (DEBUG_ANIMSPRITECEL_SETUP == 1) {
            printf("-> AnimSpriteCelStepConfiguration(%p, %u, %u, %d, %p)\n", animSpriteCel, stepIndex, frameIndex, frameDuration, animSpriteCelReceiver);
        }
        if (AnimSpriteCelStepConfiguration(animSpriteCel, stepIndex, frameIndex, frameDuration, animSpriteCelReceiver) < 0) {
            // End argument processing
            va_end(args);
            // Return error
            printf("Error <- AnimSpriteCelStepConfiguration()\n");
            return -1;
        }
    }

    // End argument processing
    va_end(args);

    // Return success
    return 1;
}

// Updates the display of an AnimSpriteCel
void AnimSpriteCelUpdate(AnimSpriteCel *animSpriteCel) {
    
    // Maximum value for the random range
    uint32 randomRangeMax = 0;
    
    if (DEBUG_ANIMSPRITECEL_FUNCT == 1) { printf("*AnimSpriteCelUpdate()*\n"); }

    // Set the current frame in the SpriteCel CCB
    SpriteCelSetFrame(animSpriteCel->spriteCel, animSpriteCel->steps[animSpriteCel->stepIndex].frameIndex); 
    
    // Copy CCB data from SpriteCel to AnimSpriteCel
    animSpriteCel->cel->ccb_PRE0 = animSpriteCel->spriteCel->cel->ccb_PRE0;
    animSpriteCel->cel->ccb_PRE1 = animSpriteCel->spriteCel->cel->ccb_PRE1;
    animSpriteCel->cel->ccb_SourcePtr = animSpriteCel->spriteCel->cel->ccb_SourcePtr;

    // If frame duration is positive (> 1)
    if (animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration > 1) {
        // Use specified duration
        animSpriteCel->remainingCycles = animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration;

    // If frame duration is negative (< 0)
    } else if (animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration < 1) {
        // Convert to positive max range
        randomRangeMax = 0 - animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration;

        // Choose random value depending on configured range
        switch (animSpriteCel->range) {
            // Range between 1 and max
            case FULL:    
                animSpriteCel->remainingCycles = GetRandomValue(1, randomRangeMax); 
                break;
            // Range between (max / 2) and max
            case HALF:    
                animSpriteCel->remainingCycles = GetRandomValue(randomRangeMax / 2, randomRangeMax);  
                break;
            // Range between (max * 3/4) and max
            case QUARTER: 
                animSpriteCel->remainingCycles = GetRandomValue(randomRangeMax - (randomRangeMax / 4), randomRangeMax);
                break;
        }

    // Otherwise, duration is zero — waiting for trigger
    } else {
        animSpriteCel->remainingCycles = 0;
    }   
}

// Advances to the next step in the animation
void AnimSpriteCelNextStep(AnimSpriteCel *animSpriteCel) {
    
    // End-of-cycle flag
    uint32 cycleEnd = 0;

    if (DEBUG_ANIMSPRITECEL_FUNCT == 1) { printf("*AnimSpriteCelNextStep()*\n"); }

    // Based on loop mode
    switch (animSpriteCel->loop) {
        
        // Animation plays forward each cycle
        case NORMAL:
            // Advance
            animSpriteCel->stepIndex++;
            // If step exceeds bounds
            if (animSpriteCel->stepIndex >= animSpriteCel->stepsCount) {
                // Wrap around to beginning
                animSpriteCel->stepIndex = 0;
                // Flag cycle completion
                cycleEnd = 1;
            }
            break;

        // Animation plays backward each cycle
        case REVERSE:
            // Rewind
            animSpriteCel->stepIndex--;
            // If step index drops below zero
            if (animSpriteCel->stepIndex < 0) {
                // Wrap to last step
                animSpriteCel->stepIndex = animSpriteCel->stepsCount - 1;
                // Flag cycle completion
                cycleEnd = 1;
            }
            break;

        // Animation alternates forward and backward each cycle
        case ALTERNATE:
            // Move forward or backward based on direction
            animSpriteCel->stepIndex += animSpriteCel->direction;

            // If step exceeds bounds
            if (animSpriteCel->stepIndex >= animSpriteCel->stepsCount) {
                // Reset to start
                animSpriteCel->stepIndex = 0;
                // Reverse direction
                animSpriteCel->direction *= -1;
                // Flag cycle completion
                cycleEnd = 1;
            }
            // If step index drops below zero
            else if (animSpriteCel->stepIndex < 0) {
                // Wrap to last step
                animSpriteCel->stepIndex = animSpriteCel->stepsCount - 1;
                // Reverse direction
                animSpriteCel->direction *= -1;
                // Flag cycle completion
                cycleEnd = 1;
            }
            break;
    }

    // Update main CCB of the AnimSpriteCel
    AnimSpriteCelUpdate(animSpriteCel);

    // If cycle ended and animation is not infinite
    if ((cycleEnd == 1) && (animSpriteCel->iterationsCount != INFINITE)) {
        // Decrement remaining cycle count
        animSpriteCel->iterationsCount--;
    }

    // If this step controls another animation
    if (animSpriteCel->steps[animSpriteCel->stepIndex].animSpriteCelReceiver != NULL) {
        // Trigger next step on receiver
        AnimSpriteCelTrigger(animSpriteCel->steps[animSpriteCel->stepIndex].animSpriteCelReceiver);
    }
}

// Executes the animation
void AnimSpriteCelRun(AnimSpriteCel *animSpriteCel) {

    if (DEBUG_ANIMSPRITECEL_FUNCT == 1) { printf("*AnimSpriteCelRun()*\n"); }

    // If the animation is undefined
    if (animSpriteCel == NULL) {
        // Log error
        printf("Error: AnimSpriteCel unknown.\n");
        return;
    }

    // If the SpriteCel is undefined
    if (animSpriteCel->spriteCel == NULL) {
        // Log error
        printf("Error: AnimSpriteCel SpriteCel unknown.\n");
        return;
    }

    // If the steps array is undefined
    if (animSpriteCel->steps == NULL) {
        // Log error
        printf("Error: AnimSpriteCel steps unknown.\n");
        return;
    }

    // If the animation is waiting for a trigger
    if (animSpriteCel->steps[animSpriteCel->stepIndex].frameDuration == 0) {
        // Exit early
        return;
    }

    // If all iterations have been completed
    if (animSpriteCel->iterationsCount == 0) {
        // Exit early
        return;
    }

    // If it's not time to change steps yet
    if (animSpriteCel->remainingCycles > 0) {
        // Decrement the remaining display cycles
        animSpriteCel->remainingCycles--;
        // Exit early
        return;
    }

    // Advance to the next animation step
    AnimSpriteCelNextStep(animSpriteCel);
}

// Deletes the AnimSpriteCel
int32 AnimSpriteCelCleanup(AnimSpriteCel *animSpriteCel) {

    if (DEBUG_ANIMSPRITECEL_CLEAN == 1) { printf("*AnimSpriteCelCleanup()*\n"); }

    // If the AnimSpriteCel is undefined
    if (animSpriteCel == NULL) {
        printf("Error: AnimSpriteCel unknown.\n");
        return -1;
    }

    // Free the Cel if present
    if (animSpriteCel->cel != NULL) {
        DeleteCel(animSpriteCel->cel);
        animSpriteCel->cel = NULL;
    }

    // Free the step array if present
    if (animSpriteCel->steps != NULL) {
        FreeMem(animSpriteCel->steps, animSpriteCel->stepsCount * sizeof(AnimSpriteCelStep));
        animSpriteCel->steps = NULL;
    }

    // Free the AnimSpriteCel structure itself
    FreeMem(animSpriteCel, sizeof(AnimSpriteCel));
    animSpriteCel->spriteCel = NULL;

    // Finalize cleanup
    animSpriteCel = NULL;

    // Return success
    return 1;
}
