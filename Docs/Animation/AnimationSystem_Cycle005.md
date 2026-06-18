# Animation System — Cycle 005 (PROD_CYCLE_AUTO_20260618_005)

## Agent #10 — Animation Agent

### Status
- Bridge: CONNECTED
- CAP: SAFE
- Map: MinPlayableMap

### Animation Architecture (Python-driven, no C++ compilation)

#### Animation Zones Deployed
| Label | Location | Purpose |
|-------|----------|---------|
| AnimZone_Idle_Savana | (500, 500, 50) | Dinosaur idle state trigger |
| AnimZone_Hunt_Forest | (-800, 300, 50) | Raptor hunt/stalk animation |
| AnimZone_Graze_Plains | (1200, -400, 50) | Herbivore grazing loop |
| AnimZone_Drink_River | (-200, -900, 50) | Drink animation at water |
| AnimZone_Combat_Arena | (0, 1500, 50) | Combat animation state |
| AnimConfig_Player_001 | (0, 0, 300) | Player animation config marker |

#### Animation State Machine Design

```
PLAYER CHARACTER STATES:
  Idle → Walk (speed > 0)
  Walk → Run (speed > 300)
  Run → Sprint (speed > 600, stamina > 20%)
  Any → Jump (space pressed, grounded)
  Any → Crouch (ctrl held)
  Crouch → CrouchWalk (WASD while crouching)
  Any → Fall (not grounded)
  Fall → Land (grounded, velocity < -500)
  Land → Idle (after 0.3s blend)

DINOSAUR STATES:
  Idle ↔ Patrol (no threat detected)
  Patrol → Alert (player within 30m)
  Alert → Hunt (player within 15m OR player runs)
  Hunt → Attack (player within 2m)
  Attack → Hunt (attack cooldown)
  Hunt → Flee (health < 20%)
  Idle → Graze (herbivore, food zone)
  Graze → Drink (water zone proximity)
  Any → Death (health = 0)
```

#### Blend Space Design (1D — Speed)

```
PLAYER LOCOMOTION BLEND SPACE:
  0.0  → Idle animation
  150  → Walk animation  
  350  → Jog animation
  600  → Run animation
  800  → Sprint animation

DINO LOCOMOTION BLEND SPACE (per species):
  T-Rex:
    0   → TRex_Idle
    100 → TRex_Walk
    400 → TRex_Run
    700 → TRex_Charge
  
  Raptor:
    0   → Raptor_Idle
    200 → Raptor_Walk
    500 → Raptor_Run
    900 → Raptor_Sprint_Attack
  
  Brachiosaurus:
    0   → Brachio_Idle
    80  → Brachio_Walk
    200 → Brachio_Trot
```

#### IK Configuration

```
FOOT IK (per character):
  - Two-bone IK on each leg
  - Trace from foot bone downward 50cm
  - Adjust pelvis height to average foot offset
  - Max foot offset: ±30cm
  - Blend weight: 1.0 on ground, 0.0 in air

LOOK-AT IK (player + dinos):
  - Head bone tracks threat/interest target
  - Max yaw: 70°, max pitch: 45°
  - Blend in: 0.2s, blend out: 0.5s
  - Priority: combat target > food > idle look
```

#### Motion Matching Parameters

```
TRAJECTORY SAMPLES: 6 (past 3, future 3)
SAMPLE INTERVAL: 0.1s
POSE FEATURES:
  - Foot positions (L/R)
  - Foot velocities (L/R)
  - Hip position
  - Hip velocity
  - Facing direction
DATABASE SIZE TARGET: 45 minutes of animation data
SEARCH FREQUENCY: Every 0.1s (10Hz)
```

### Voice Asset
- AnimationGuide_Raptor — Raptor behavior narration for tutorial/loading screen

### Next Cycle Handoff → Agent #11 (NPC Behavior)
The animation zones are placed. NPC Behavior agent should:
1. Reference AnimZone_* actors for behavior tree state transitions
2. Use AnimZone_Hunt_Forest as raptor patrol/hunt boundary
3. Use AnimZone_Idle_Savana as T-Rex territorial anchor
4. Use AnimZone_Graze_Plains for herbivore feeding behavior
5. Implement EQS queries that check proximity to animation zones

### Technical Notes
- No C++ compilation available in current editor build
- All animation setup via Python/Blueprint
- AnimBlueprint creation requires skeleton asset — audit found no pre-existing skeletons
- Recommend Agent #9 (Character Artist) to import skeletal mesh with skeleton before next anim cycle
