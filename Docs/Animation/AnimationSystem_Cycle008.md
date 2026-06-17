# Animation System — Cycle 008 Documentation
**Agent #10 — Animation Agent | PROD_CYCLE_AUTO_20260617_008**

## Animation State Machine Design

### Character States (TranspersonalCharacter)

```
[IDLE] ──speed > 0──► [WALK] ──speed > 200──► [RUN] ──sprint input──► [SPRINT]
  │                      │                       │
  │                      ▼                       ▼
  └──crouch input──► [CROUCH_IDLE]          [CROUCH_WALK]
                         │
                         └──sneak input──► [SNEAK]

[ANY STATE] ──jump input──► [JUMP_START] ──► [JUMP_LOOP] ──► [LAND]
[ANY STATE] ──near wall──►  [CLIMB_START] ──► [CLIMB_LOOP] ──► [CLIMB_END]
```

### Blend Space Configuration

**BS_Locomotion (2D Blend Space)**
- X Axis: Speed (0–600 cm/s)
- Y Axis: Direction (-180° to 180°)
- Samples:
  - (0, 0): Idle
  - (150, 0): Walk_Fwd
  - (150, 90): Walk_Right
  - (150, -90): Walk_Left
  - (150, 180): Walk_Back
  - (350, 0): Run_Fwd
  - (600, 0): Sprint_Fwd

### IK System — Foot Placement

**Two-Bone IK Chain:**
- Root: Pelvis
- Mid: Knee
- End: Foot (L/R)

**Trace Configuration:**
- Trace Start: Foot bone + 50cm up
- Trace End: Foot bone - 75cm down
- Trace Channel: ECC_WorldStatic
- Interpolation Speed: 15.0 (smooth terrain adaptation)

**Foot IK Actors Placed in MinPlayableMap:**
- IK_FootL_Flat_001 — flat terrain reference (0, 0, 0)
- IK_FootR_Flat_001 — flat terrain reference (50, 0, 0)
- IK_FootL_Slope_001 — slope terrain test (300, 200, 30)
- IK_FootR_Slope_001 — slope terrain test (350, 200, 30)
- IK_FootL_Rock_001 — rocky terrain test (-200, 400, 60)
- IK_FootR_Rock_001 — rocky terrain test (-150, 400, 60)
- IK_Hand_Climb_L_001 — climbing reference (-800, -600, 200)
- IK_Hand_Climb_R_001 — climbing reference (-750, -600, 200)

### Animation Trigger Zones

**Placed in MinPlayableMap:**
- AnimZone_Crouch_001 — triggers crouch animation state (500, 500, 100)
- AnimZone_Sneak_001 — triggers sneak locomotion (-500, 800, 100)
- AnimZone_Sprint_001 — sprint corridor (1200, -300, 100)
- AnimZone_Climb_001 — climbing wall zone (-800, -600, 150)

## Assets Created This Cycle

| Asset | Path | Type |
|-------|------|------|
| ABP_TranspersonalCharacter | /Game/Animation/ | AnimBlueprint |
| BP_AnimationConfig | /Game/Animation/ | Blueprint |
| AnimZone_Crouch_001 | MinPlayableMap | TriggerBox |
| AnimZone_Sneak_001 | MinPlayableMap | TriggerBox |
| AnimZone_Sprint_001 | MinPlayableMap | TriggerBox |
| AnimZone_Climb_001 | MinPlayableMap | TriggerBox |
| IK_FootL/R_* (6 actors) | MinPlayableMap | TargetPoint |
| IK_Hand_Climb_* (2 actors) | MinPlayableMap | TargetPoint |

## Voice Asset
- **SurvivalNarrator** — movement/survival guidance audio
- URL: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781707447693_SurvivalNarrator.mp3
- Duration: ~18s
- Use: Tutorial/onboarding narration for movement mechanics

## Technical Notes

### Motion Matching Parameters (for future implementation)
- Trajectory samples: 6 points (past 0.2s, 0.1s; future 0.1s, 0.2s, 0.3s, 0.5s)
- Pose cost weight: 0.7
- Trajectory cost weight: 0.3
- Database update rate: 30Hz

### Dinosaur Animation Considerations
- T-Rex: bipedal, tail counterbalance, head bob at walk
- Raptor: agile, low center of gravity, pack coordination
- Brachiosaurus: quadrupedal, slow deliberate movement, neck sway

## Dependencies for Agent #11 (NPC Behavior)
- AnimZone actors are in place — NPC behavior trees can reference these zones
- IK target actors define terrain interaction points for NPC pathfinding
- Animation states (crouch/sneak) should trigger stealth detection changes in NPC AI
