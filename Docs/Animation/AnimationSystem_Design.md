# Animation System Design — Agent #10 Animation Agent
## Transpersonal Game Studio — Prehistoric Survival Game

### Overview
This document defines the animation architecture for the prehistoric survival game.
All animation logic is implemented via UE5 Blueprint AnimInstances and UE5 Python
configuration commands (NO C++ compilation — binary is pre-built).

---

## Character Animation States

### Player Character (Primitive Human Survivor)

#### State Machine: Locomotion
```
[Idle] ──────────────────────────────────────────────────────────────────────────
  ├─ Speed > 0.1 → [Walk]
  ├─ Speed > 300 → [Run]
  ├─ bIsCrouching → [Crouch_Idle]
  └─ bIsInAir → [Jump_Start]

[Walk] ──────────────────────────────────────────────────────────────────────────
  ├─ Speed < 0.1 → [Idle]
  ├─ Speed > 300 → [Run]
  ├─ bIsCrouching → [Crouch_Walk]
  └─ bIsInAir → [Jump_Start]

[Run] ───────────────────────────────────────────────────────────────────────────
  ├─ Speed < 150 → [Walk]
  ├─ Speed < 0.1 → [Idle]
  └─ bIsInAir → [Jump_Start]

[Jump_Start] → [Jump_Loop] → [Jump_Land] → [Idle]

[Crouch_Idle] ──────────────────────────────────────────────────────────────────
  ├─ !bIsCrouching → [Idle]
  └─ Speed > 0.1 → [Crouch_Walk]

[Crouch_Walk] ──────────────────────────────────────────────────────────────────
  ├─ Speed < 0.1 → [Crouch_Idle]
  └─ !bIsCrouching → [Walk]
```

#### Blend Space: Walk/Run
- X-axis: Speed (0 → 600 cm/s)
- Y-axis: Direction (-180 → 180 degrees)
- Samples:
  - (0, 0): Idle
  - (150, 0): Walk_Forward
  - (150, 90): Walk_Right
  - (150, -90): Walk_Left
  - (150, 180): Walk_Backward
  - (400, 0): Run_Forward
  - (400, 90): Run_Right
  - (400, -90): Run_Left

#### Additive Layers
- **Breathing**: Subtle chest/shoulder movement, always active on top of locomotion
- **Head Look**: IK-driven head rotation toward points of interest (dinosaurs, threats)
- **Foot IK**: Two-bone IK for foot placement on uneven terrain
- **Carry Weight**: Upper body lean based on inventory weight

---

## Dinosaur Animation States

### T-Rex (Tyrannosaurus Rex)
```
[Idle_Alert] — weight shifting, head scanning
[Patrol_Walk] — slow deliberate bipedal walk
[Stalk] — low crouch walk toward prey
[Chase_Run] — full sprint, tail counterbalance
[Attack_Bite] — lunge + snap
[Roar] — territorial display
[Feeding] — head down, tearing motion
[Rest] — lying down, slow breathing
```

### Triceratops
```
[Graze] — head down, slow lateral movement (PRIORITY: hub clearing pose)
[Stand_Alert] — head up, ears forward, scanning
[Charge] — lowered head, full gallop
[Threat_Display] — horn presentation, ground pawing
[Rest] — standing rest, slow breathing
```

### Raptor (Velociraptor)
```
[Idle_Curious] — head tilting, weight shifting
[Stalk_Crouch] — low body, slow deliberate steps
[Sprint] — full horizontal body, tail straight
[Leap_Attack] — airborne pounce
[Pack_Signal] — vocalisation pose
```

### Brachiosaurus
```
[Graze_High] — neck extended upward, leaf stripping
[Graze_Low] — neck curved down to water/ground
[Walk_Slow] — quadrupedal slow lumber
[Alert_Stance] — head raised, scanning
```

---

## Foot IK System

### Implementation (Blueprint-based)
- Two-bone IK on both legs
- Trace from foot bone downward (trace length: 100cm)
- Blend weight: 1.0 on ground, 0.0 in air (blend over 0.1s)
- Hip offset: calculated from average foot displacement
- Max hip drop: 30cm
- Max foot rotation: 35 degrees on each axis

### Terrain Adaptation
- Works on slopes up to 45 degrees
- Disabled during jump/fall
- Blends out during run (performance optimization)

---

## Motion Matching Configuration

### Database Entries (Priority Order)
1. Locomotion (walk/run all directions) — 60% of database
2. Idle variations (alert, relaxed, scanning) — 15%
3. Transitions (start/stop) — 10%
4. Jump/land — 10%
5. Crouch locomotion — 5%

### Trajectory Prediction
- Look-ahead: 0.5 seconds
- History: 0.3 seconds
- Sample rate: 10 Hz
- Cost weights:
  - Position: 1.0
  - Velocity: 0.8
  - Facing: 0.6

---

## Dinosaur Pose System (Hub Clearing — X=2100, Y=2400)

### Static Pose Overrides for Hero Screenshot
Each dinosaur in the hub clearing gets a frozen animation frame:

| Actor Label | Pose | Bone Overrides |
|-------------|------|----------------|
| TRex_Savana_001 | Standing Alert | Head +15° yaw, tail -10° pitch |
| Trike_Hub_001 | Grazing | Head -45° pitch (down), neck -20° pitch |
| Raptor_Hub_001 | Mid-stride | Left leg forward, right leg back, body lean |
| Raptor_Hub_002 | Curious | Head +30° yaw, weight on right leg |
| Brachio_Hub_001 | Neck raised | Neck +60° pitch upward |

### Implementation Method
Applied via UE5 Python: set_editor_property on SkeletalMeshComponent
with custom AnimationMode = UseAnimationAsset + specific animation frame.

---

## Performance Budget

| System | CPU Budget | Notes |
|--------|-----------|-------|
| Player AnimBP | 0.8ms | Full state machine + IK |
| Dinosaur AnimBP (per dino) | 0.4ms | Simplified state machine |
| Foot IK (player) | 0.2ms | Two traces per frame |
| Motion Matching | 0.5ms | Database query |
| Crowd animations | 0.1ms/agent | LOD-based, simplified |
| **Total budget** | **4ms** | At 60fps target |

---

## Animation Assets Required (from Character Artist #9)

- [ ] Player SkeletalMesh with UE5 Mannequin-compatible skeleton
- [ ] T-Rex SkeletalMesh with custom skeleton (bipedal, large)
- [ ] Triceratops SkeletalMesh (quadrupedal)
- [ ] Raptor SkeletalMesh (bipedal, small)
- [ ] Brachiosaurus SkeletalMesh (quadrupedal, long neck)

### Interim Solution (Current)
Using UE5 primitive shapes (boxes/capsules) as placeholders with:
- Bone rotation applied via Python to simulate poses
- Static mesh actors positioned to suggest dinosaur silhouettes

---

## Handoff to NPC Behavior Agent (#11)

### Animation Notifies for AI Integration
- `AN_FootstepLeft` / `AN_FootstepRight` — triggers audio + ground decal
- `AN_AttackStart` — enables hit detection window
- `AN_AttackEnd` — disables hit detection
- `AN_Roar_Peak` — triggers fear response in nearby NPCs
- `AN_AlertTransition` — signals behavior tree state change

### Blend Tree Hooks for Behavior Tree
- `bIsAggressive` (bool) — blends to aggressive locomotion set
- `bIsFeeding` (bool) — locks to feeding animation
- `bIsFleeing` (bool) — enables panic run blend
- `ThreatDirection` (float) — drives head look-at IK

---

## Files Created This Cycle
- `Docs/Animation/AnimationSystem_Design.md` — This document
- `Docs/Animation/DinosaurPoseConfig.json` — Hub clearing pose data

## Next Steps for Agent #11 (NPC Behavior)
1. Reference `AN_Roar_Peak` notify for fear propagation radius
2. Use `bIsAggressive` blend parameter to drive behavior tree transitions
3. Implement daily routine system that triggers animation state changes
4. Raptor pack coordination: sync animation phases between pack members
