# AnimSpriteCel – Animation System for SpriteCel (3DO Cel Engine)

**Author**: Christophe Geoffroy (Topper)  
**License**: MIT  
**Last Updated**: 2025-07-30

## 🎬 Overview

**AnimSpriteCel** is an independent animation system built around the `SpriteCel` module. It handles a sequence of steps (`AnimSpriteCelStep`), each tied to a specific frame and display duration. These steps form a complete animation cycle, managed through the `AnimSpriteCel` structure and operated like a looping timeline.

Animations can move **forward**, **backward**, or alternate direction at each cycle end. They may run **once**, **multiple times**, or **infinitely**.

### ⏱️ Step Durations
- **Positive values**: fixed duration in display cycles.
- **Zero**: pauses animation; awaiting external trigger.
- **Negative values**: randomized duration between 1 and `abs(value)`, modulated by `range` (FULL, HALF, QUARTER).

### 🔁 Interactions Between AnimSpriteCels
Steps can trigger other `AnimSpriteCel`s. This enables:
- **Cascade control chains**
- **Alternating sequences**
- **Mutual animation triggering**

This creates complex coordinated behaviors between multiple animated entities.

## ⚠️ Important Notes

- The displayed frame of the associated `SpriteCel` is directly affected by `AnimSpriteCel`. For independent display, consider cloning the `SpriteCel` or reapplying its frame manually.
- One `SpriteCel` can serve multiple `AnimSpriteCel`s with distinct sequences.
- Use `AnimSpriteCelCleanup()` to free the animation. The associated `SpriteCel` must be deleted separately.

## 🏗️ Structures

### `AnimSpriteCelStep`

- `frameIndex`: Index of the frame to display from `SpriteCel`
- `frameDuration`: Display duration in cycles
- `animSpriteCelReceiver`: Pointer to another `AnimSpriteCel` to trigger if waiting 

### `AnimSpriteCel`

- `cel`: Main animated CCB 
- `spriteCel`: Reference to the source `SpriteCel`
- `loop`: Loop type (`NORMAL`, `REVERSE`, `ALTERNATE`) 
- `range`: Randomization range (`FULL`, `HALF`, `QUARTER`)
- `remainingCycles`: Execution count before moving to next step
- `iterationsCount`: Number of animation loops (`INFINITE` for endless)
- `direction`: Animation direction (`1` = forward, `-1` = backward) 
- `stepIndex`: Current step in animation
- `stepsCount`: Total number of steps
- `steps`: Array of `AnimSpriteCelStep`

## 🛠️ Core Functions

### `AnimSpriteCelInitialization()`
Initializes animation, clones the CCB, sets up step array.

### `AnimSpriteCelStepConfiguration()`
Defines an animation step: target frame, duration, optional receiver.

### `AnimSpriteCelStepsConfiguration(...)`
Sets multiple steps using variadic arguments.

### `AnimSpriteCelUpdate()`
Internal function to update visual display, called when needed.

### `AnimSpriteCelNextStep()`
Moves to next animation step. Called by `AnimateSpriteCelRun()` or a trigger.

### `AnimateSpriteCelRun()`
Controls animation progression per display cycle.

### `AnimSpriteCelTrigger()`
Triggers the next step of another waiting `AnimSpriteCel`.

### `AnimSpriteCelCleanup()`
Frees memory used by the animation structure.