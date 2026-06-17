# Animation System — Agent #10 — Cycle PROD_CYCLE_AUTO_20260617_007

## Overview
Complete animation architecture documentation for the prehistoric survival game.
All animation work is executed via UE5 Python (no C++ compilation required).

## Animation State Machine Design

### Player Character States
```
IDLE (speed=0)
  ↓ speed > 50
WALK (speed=50-200)
  ↓ speed > 200
RUN (speed=200-450)
  ↓ speed > 450
SPRINT (speed=450+)

IDLE ↔ CROUCH (input: crouch key)
ANY_STATE → JUMP (input: jump key, not grounded)
ANY_STATE → COMBAT_IDLE (weapon drawn)
COMBAT_IDLE → ATTACK_LIGHT / ATTACK_HEAVY / BLOCK
ANY_STATE → DEATH (health = 0)
```

### Blend Space 1D — BS_Locomotion_Speed
| Speed (cm/s) | Animation |
|---|---|
| 0 | Idle_Prehistoric |
| 150 | Walk_Prehistoric |
| 400 | Run_Prehistoric |
| 600 | Sprint_Prehistoric |

### Blend Space 2D — BS_Locomotion_Direction (future)
- X axis: Speed (0-600)
- Y axis: Direction (-180 to +180)
- Enables strafe animations and directional movement

## IK System

### Foot IK
- Two-bone IK on left and right foot
- Trace from foot bone downward 50cm
- Adjust pelvis height based on average foot height
- Blend weight: 1.0 on ground, 0.0 in air

### Hand IK (Weapon)
- Right hand IK target follows weapon grip socket
- Left hand IK target follows weapon support socket
- Blend in/out based on weapon equipped state

## Animation Zones in MinPlayableMap

| Label | Location | Purpose |
|---|---|---|
| AnimZone_Idle_Camp | (0, 0, 50) | Safe zone — idle animations |
| AnimZone_Walk_Forest | (500, 300, 50) | Forest traversal animations |
| AnimZone_Run_Plains | (1000, 0, 50) | Open plains sprint zone |
| AnimZone_Crouch_Stealth | (-500, 400, 50) | Stealth/crouch zone near predators |
| AnimZone_Combat_Arena | (800, -600, 50) | Combat animation testing area |

## IK Reference Points

| Label | Location | Purpose |
|---|---|---|
| IK_FootL_Reference | (0, -30, 0) | Left foot IK ground reference |
| IK_FootR_Reference | (0, 30, 0) | Right foot IK ground reference |
| IK_Hand_Weapon_Reference | (50, 0, 100) | Weapon grip IK reference |

## Voice Samples Generated

1. **Survivor_Whisper** — Danger/stealth alert
   - URL: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781701997304_Survivor_Whisper.mp3
   - Text: "The beast is close. Move slowly. Keep low. One wrong step and the hunt begins — and we are the prey."

2. **Survivor_Alert** — Run/escape trigger
   - URL: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702028523_Survivor_Alert.mp3
   - Text: "Run. Do not look back. The valley is not safe after dark. Find high ground before the sun drops below the ridge."

## Animation Blueprint Architecture (ABP_PlayerCharacter)

### Event Graph
- On Begin Play: Initialize speed/direction variables
- On Update Animation: Read character velocity, compute speed float, compute direction float

### AnimGraph
```
[Output Pose]
    ↑
[Apply Additive: Upper Body Layer]
    ↑
[Layered Blend Per Bone: Upper/Lower split at spine_01]
    ↑
[State Machine: Locomotion]
    States: Idle → Walk → Run → Sprint → Jump → Fall → Land
    ↑
[Two Bone IK: Left Foot]
    ↑
[Two Bone IK: Right Foot]
```

### Variables Required
- `Speed` (float) — character ground speed
- `Direction` (float) — movement direction relative to facing
- `bIsInAir` (bool) — from character movement component
- `bIsCrouching` (bool) — from character crouch state
- `bIsArmed` (bool) — weapon equipped state
- `bIsDead` (bool) — health <= 0

## Montages

### AM_Attack_Spear
- Section: WindUp (0.0-0.3s)
- Section: Strike (0.3-0.6s)
- Section: Recovery (0.6-1.0s)
- Notify: DealDamage at 0.45s
- Notify: FootstepSound at 0.1s

### AM_Death_Forward
- Single section, no looping
- Notify: PlayDeathSound at 0.1s
- Notify: EnableRagdoll at 0.8s

### AM_Jump_Start / AM_Jump_Loop / AM_Jump_Land
- Triggered from locomotion state machine
- Jump_Land has camera shake notify

## Next Steps for Agent #11 (NPC Behavior)
1. Use AnimZone_Combat_Arena for dinosaur behavior testing
2. Reference IK targets for NPC foot placement on terrain
3. NPC dinosaurs need separate animation blueprints (ABP_TRex, ABP_Raptor, ABP_Brachio)
4. Behavior tree nodes should trigger animation montages via AnimInstance interface
5. Predator attack animations need damage notify events synced with hitbox activation

## Technical Notes
- UE5 editor running pre-compiled binary — all work via Python API
- Skeleton discovery: 0 custom skeletons found in /Game (mannequin paths not present)
- Animation Blueprint creation requires valid Skeleton asset — pending character mesh import
- All zone markers and IK targets successfully placed in MinPlayableMap
