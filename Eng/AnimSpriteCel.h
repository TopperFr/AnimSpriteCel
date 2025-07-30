#ifndef ANIMSPRITECEL_H
#define ANIMSPRITECEL_H

/******************************************************************************
**
**  AnimSpriteCel - Animation system for SpriteCel (3DO Cel Engine)
**  
**  Author: Christophe Geoffroy (Topper) - MIT License
**  Last Updated: 2025-07-30
**
**  This module provides an independent animation structure using a SpriteCel
**  composed of multiple frames. The animation is defined as a sequence 
**  of steps ("AnimSpriteCelStep"), each pointing to a frame of the SpriteCel 
**  and having its own duration. The set of steps forming the animation represents
**  an animation cycle. It is managed by the main structure "AnimSpriteCel" and
**  functions like a cyclic timeline. The animation can run forward, backward, 
**  or alternate direction at each cycle end. The cycle can run once, multiple 
**  times, or indefinitely.
**
**  The display duration of a step is defined by the number of display cycles.
**  Giving a negative value means the duration is random, with the absolute value
**  as the maximum and 0 as the minimum. The randomness can be weighted to half
**  or upper quarter of the value range.
**
**  A step can interrupt the animation cycle by setting its duration to 0. In this
**  case, the AnimSpriteCel is considered paused. Animation resumes from another
**  AnimSpriteCel, which specifies a receiver AnimSpriteCel at a given frame.
**  Multiple AnimSpriteCels can be chained for control. Two AnimSpriteCels can
**  also control each other, allowing one to run its cycle, pause, then trigger
**  the second AnimSpriteCel. In turn, it executes its cycle, then reactivates
**  the first one. This way, multiple AnimSpriteCels can interact.
**
**  Important Notes:
**
**    - The display of the used SpriteCel is directly affected by AnimSpriteCel,
**      as it changes the current frame. If a SpriteCel needs to be displayed
**      independently, it’s recommended to clone it solely for AnimSpriteCel.
**      Another solution is to store the current frame during execution and
**      call SpriteCelSetFrame() to restore the correct one after AnimSpriteCelRun().
**
**    - A single SpriteCel can be used for multiple AnimSpriteCels with their
**      own animation sequences. There’s no need to create a SpriteCel for
**      every AnimSpriteCel.
**
**    - Once an AnimSpriteCel is initialized, it should be cleaned up using AnimSpriteCelCleanup().
**      The associated SpriteCel should be deleted separately.
**
**  Structure Roles:
**
**    AnimSpriteCelStep
**      - frameIndex: index of the SpriteCel frame to display
**      - frameDuration: display duration in cycles (int32)
**                        > 1 -> fixed duration
**                        = 1 -> immediate switch
**                        = 0 -> awaiting trigger
**                        < 0 -> random duration (between 1 and abs(value)), weighted by "range"
**      - animSpriteCelReceiver: pointer to another AnimSpriteCel to trigger the next step if paused
**
**    AnimSpriteCel
**      - cel: animated CCB (copy of SpriteCel)
**      - spriteCel: reference to source SpriteCel (frame array)
**      - loop: loop type (NORMAL, REVERSE, ALTERNATE)
**      - range: value range for randomness (FULL, HALF, QUARTER)
**      - remainingCycles: executions of AnimSpriteCelRun() before next step
**      - iterationsCount: number of cycle repetitions (unlimited = INFINITE)
**      - direction: animation direction (1 = forward, -1 = backward)
**      - stepIndex: current step in the "steps" array
**      - stepsCount: total number of animation steps
**      - steps: dynamic array of "AnimSpriteCelStep"
**
**  Main Functions:
**
**    AnimSpriteCelInitialization()
**      -> Initializes animation, clones CCB, prepares steps array.
**
**    AnimSpriteCelStepConfiguration()
**      -> Defines an animation step: frame to display, duration, and pointer
**         to another AnimSpriteCel
**
**    AnimSpriteCelStepsConfiguration()
**      -> Defines multiple steps in one pass with variadic arguments.
**
**    AnimSpriteCelUpdate()
**      -> Internal function to update display.
**         Called by AnimSpriteCelNextStep() when needed.
**
**    AnimSpriteCelNextStep()
**      -> Internal function to proceed to the next step.
**         Called by AnimateSpriteCelRun() or AnimSpriteCelTrigger() 
**         when required.
**
**    AnimateSpriteCelRun()
**      -> Evolution function to call on each display cycle.
**         Manages transition to next step.
**
**    AnimSpriteCelTrigger()
**      -> Internal function to trigger the next step in another
**         waiting AnimSpriteCel.
**
**    AnimSpriteCelCleanup()
**      -> Frees memory used by the AnimSpriteCel structure
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

// Enumeration of interactive zone types
typedef enum {
    // The animation plays forward on each cycle
    NORMAL,
    // The animation plays backward on each cycle
    REVERSE,
    // The animation alternates: forward, then backward each cycle
    ALTERNATE
} AnimSpriteCelLoop;

// Random value range
typedef enum {
    // Value range between 1 and maximum
    FULL,
    // Value range between (maximum / 2) and maximum
    HALF,
    // Value range between (maximum / 4) * 3 and maximum
    QUARTER
} AnimSpriteCelRange;

typedef struct AnimSpriteCel AnimSpriteCel;

typedef struct {
    // Displayed frame
    uint32 frameIndex;
    // Frame duration
    // (A negative value indicates random duration)
    int32 frameDuration;
    // Target AnimSpriteCel for trigger dispatch
    AnimSpriteCel *animSpriteCelReceiver;
    // Animation loop type
} AnimSpriteCelStep;

struct AnimSpriteCel {
    // Main CCB of the animated sprite
    CCB *cel;
    // CCB from the SpriteCel
    SpriteCel *spriteCel;
    // Animation loop type
    AnimSpriteCelLoop loop;
    // Range of random value usage
    AnimSpriteCelRange range;
    // Remaining cycles before next change
    uint32 remainingCycles;
    // Number of animation cycle repetitions
    uint32 iterationsCount;
    // Current animation direction
    int32 direction;
    // Current step index
    int32 stepIndex;
    // Total number of steps
    uint32 stepsCount;
    // Array of animation steps
    AnimSpriteCelStep *steps;
};

// Reference to the global context
extern AnimSpriteCel animSpriteCel;

// Initialization of an AnimSpriteCel
AnimSpriteCel *AnimSpriteCelInitialization(SpriteCel *spriteCel, AnimSpriteCelLoop loop, AnimSpriteCelRange range, uint32 iterations, int32 direction, uint32 stepIndex, uint32 stepsCount);
// Configuration of a single AnimSpriteCel step
int32 AnimSpriteCelStepConfiguration(AnimSpriteCel *animSpriteCel, uint32 stepIndex, uint32 frameIndex, int32 frameDuration, AnimSpriteCel *animSpriteCelReceiver);
// Configuration of multiple AnimSpriteCel steps
int32 AnimSpriteCelStepsConfiguration(AnimSpriteCel *spriteCel, int32 start, ...);
// Updates the display of an AnimSpriteCel
void AnimSpriteCelUpdate(AnimSpriteCel *animSpriteCel);
// Advances to the next step in the animation
void AnimSpriteCelNextStep(AnimSpriteCel *animSpriteCel);
// Runs the animation
void AnimSpriteCelRun(AnimSpriteCel *animSpriteCel);
// Triggers a waiting animation
void AnimSpriteCelTrigger(AnimSpriteCel *animSpriteCel);
// Cleans up the AnimSpriteCel
int32 AnimSpriteCelCleanup(AnimSpriteCel *spriteCel);

#endif // ANIMSPRITECEL_H
