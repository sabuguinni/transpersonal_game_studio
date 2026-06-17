# Animation System Design — Agent #10
## Transpersonal Game Studio | Prehistoric Survival Game

### Overview
The animation system for the prehistoric survival game uses UE5's Motion Matching and Animation Blueprint systems to deliver fluid, weight-appropriate movement for the player character and dinosaur NPCs.

---

## Player Character Animation States

### Locomotion State Machine
```
[IDLE] ──speed>0──► [WALK] ──speed>300──► [RUN] ──sprint_input──► [SPRINT]
  │                    │                     │
  └──crouch──► [CROUCH_IDLE] ──speed>0──► [CROUCH_WALK]
  │
  └──jump──► [JUMP_START] ──► [JUMP_APEX] ──► [FALL] ──land──► [LAND_RECOVER]
  │
  └──water──► [SWIM_IDLE] ──speed>0──► [SWIM_FORWARD]
  │
  └──climb──► [CLIMB_IDLE] ──speed>0──► [CLIMB_UP/DOWN]
```

### Blend Space Parameters
| BlendSpace | Axis X | Axis Y | Description |
|------------|--------|--------|-------------|
| BS_Locomotion | Speed (0-600) | Direction (-180/180) | Main movement blend |
| BS_Crouch | Speed (0-200) | Direction (-180/180) | Crouched movement |
| BS_Swim | Speed (0-300) | Pitch (-90/90) | Swimming in water |
| BS_Climb | Speed (0-150) | Vertical (-1/1) | Climbing surfaces |

### IK Settings
- **Foot IK**: Two-bone IK on both legs, traces 80cm below foot socket
- **Hand IK**: Weapon/tool hold IK, blends with animation curves
- **Look IK**: Head and spine rotation toward look target, max 60° yaw, 30° pitch
- **Lean IK**: Spine lean during turns, driven by angular velocity

---

## Dinosaur Animation States

### T-Rex (Tyrannosaurus Rex)
```
[IDLE_SCAN] ──detect_prey──► [ALERT] ──chase_start──► [CHASE_RUN]
     │                           │                          │
     └──hunger──► [PATROL]       └──lose_sight──► [SEARCH]  └──attack_range──► [ATTACK_BITE]
     │                                                                              │
     └──rest──► [SLEEP]                                                        [ROAR_VICTORY]
```

**Key Animations:**
- Idle: Weight shift, head scan, tail sway (loop 8s)
- Walk: Bipedal, counter-balance tail, 2.5m/s
- Run: Full sprint, 12m/s, ground shake on footsteps
- Bite: 0.8s windup, 0.3s strike, 0.5s recovery
- Roar: 3.2s full body animation, triggers audio event

### Raptor (Velociraptor)
```
[IDLE] ──► [PATROL] ──► [STALK] ──► [POUNCE_CHARGE] ──► [POUNCE_LEAP] ──► [CLAW_ATTACK]
              │                                                                    │
              └──pack_signal──► [PACK_COORDINATE]                            [BITE_HOLD]
```

**Key Animations:**
- Pack coordination: Head bob signals (3 types: attack, flank, retreat)
- Stalk: Low crouch, slow deliberate steps
- Pounce: 4m leap, claw-first landing

### Brachiosaurus
```
[GRAZE] ──► [WALK_SLOW] ──► [DRINK] ──► [ALERT_STOMP] ──► [FLEE_STAMPEDE]
```

---

## Animation Blueprint Architecture

### ABP_SurvivorCharacter
```
AnimGraph:
  ├── LocomotionStateMachine (BlendSpace driven)
  ├── UpperBodyLayer (additive, weapon/tool animations)
  ├── IKRig_FootPlacement (post-process)
  ├── IKRig_LookAt (post-process)
  └── FullBodyIK_Override (climbing/swimming)
```

### Motion Matching Configuration
- **Pose History**: 10 frames at 30fps
- **Trajectory Prediction**: 0.5s ahead, 5 sample points
- **Feature Channels**: Position, velocity, facing direction, foot contacts
- **Database**: ~200 clips, 45min total motion data
- **Responsiveness**: 0.15s blend time for direction changes

---

## Animation Zone System (In-World Markers)

Zone markers placed in MinPlayableMap define areas where animation behavior changes:

| Zone Label | Location | Effect |
|------------|----------|--------|
| AnimZone_Stealth_001 | (-800, 600, 150) | Forces crouch blend, reduces footstep volume |
| AnimZone_Sprint_001 | (1200, -400, 150) | Enables sprint, increases stride length |
| AnimZone_Danger_001 | (2000, 1500, 150) | Triggers alert posture, head scan increase |
| AnimZone_Water_001 | (-1500, -1200, 80) | Activates swim state machine |

---

## Foot IK Implementation

```python
# UE5 Python — Configure foot IK on character
# Foot trace settings
FOOT_IK_TRACE_DISTANCE = 80.0  # cm below foot socket
FOOT_IK_BLEND_SPEED = 12.0     # blend speed for terrain adaptation
FOOT_IK_MAX_CORRECTION = 45.0  # max degrees of ankle rotation

# Sockets required on skeleton:
# - foot_l (left foot)
# - foot_r (right foot)  
# - ik_foot_l (left IK target)
# - ik_foot_r (right IK target)
```

---

## Performance Budget

| System | Target Cost (ms) | Max Actors |
|--------|-----------------|------------|
| Player AnimBP | 0.8ms | 1 |
| T-Rex AnimBP | 1.2ms | 3 |
| Raptor AnimBP | 0.6ms | 12 |
| Brachio AnimBP | 0.9ms | 2 |
| Crowd AnimBP (LOD2) | 0.1ms | 50 |
| **Total Animation** | **< 4ms** | — |

---

## Audio-Animation Integration

Animation Notify events trigger audio:
- `AN_Footstep_L/R` → Surface-dependent footstep sound
- `AN_WeaponSwing` → Weapon whoosh SFX
- `AN_DinoRoar` → Dinosaur vocalization
- `AN_ImpactLand` → Landing thud based on fall height
- `AN_WaterSplash` → Water entry/exit sounds

Voice line: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781717359693_AnimationNarrator.mp3

---

## Deliverables This Cycle

- Animation zone markers spawned in MinPlayableMap (4 zones)
- Animation system design documentation
- State machine diagrams for player + 3 dinosaur species
- Blend space parameter tables
- IK configuration specs
- Performance budget defined
- Audio-animation integration events listed

## Next Agent (#11 NPC Behavior)
- Use AnimZone markers as behavior trigger zones
- T-Rex patrol path should route through AnimZone_Danger_001
- Raptor pack should use AnimZone_Stealth_001 for stalk behavior
- Reference animation notify events (AN_*) for behavior tree audio triggers
