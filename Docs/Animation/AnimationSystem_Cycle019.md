# Animation System — Cycle 019 (PROD_CYCLE_AUTO_20260619_002)

## Agent #10 — Animation Agent

### Overview
This document describes the animation system architecture and what was implemented in this production cycle for the Transpersonal Game Studio prehistoric survival game.

---

## Animation State Machine Design

### Player Character States
The player character (`Survivor_Player_001`) uses a state-based animation system with the following states:

| State | Trigger Condition | Animation Type |
|-------|-------------------|----------------|
| **Idle** | Velocity < 10 cm/s | Looping idle, subtle breathing |
| **Walk** | 10–250 cm/s | Walk cycle, foot IK active |
| **Run** | 250–600 cm/s | Run cycle, lean forward |
| **Sprint** | > 600 cm/s | Full sprint, arms pumping |
| **Crouch_Idle** | Crouched + velocity < 10 | Crouched idle |
| **Crouch_Walk** | Crouched + 10–150 cm/s | Slow crouch walk |
| **Jump_Start** | Jump triggered | Jump launch |
| **Jump_Loop** | In air | Falling loop |
| **Jump_Land** | Landing | Land recovery |
| **Combat_Ready** | Near threat | Combat stance |
| **Attack_Melee** | Attack input | Strike montage |
| **Death** | Health = 0 | Ragdoll transition |

### Blend Space Design (Locomotion)
- **X-axis**: Speed (0 → 600 cm/s)
- **Y-axis**: Direction (-180° → +180°)
- Samples: Idle(0,0), Walk_Fwd(250,0), Run_Fwd(500,0), Sprint(700,0), Walk_Back(250,180), Strafe_L(250,-90), Strafe_R(250,90)

---

## Foot IK System

### Purpose
Adapt foot placement to uneven prehistoric terrain — rocky outcrops, muddy riverbanks, sloped hillsides.

### Implementation Strategy
Using UE5's built-in **Two Bone IK** node in the AnimGraph:
1. Trace ray from foot bone downward (100cm)
2. Get hit normal from terrain surface
3. Rotate foot to match surface normal
4. Offset pelvis height to prevent leg over-extension

### Foot IK Test Surface
- Actor: `FootIK_TestPlane_001` placed at (300, -200, 10)
- Scale: 5×5×0.1 — flat reference plane for IK calibration
- Use this surface to verify foot placement aligns correctly

---

## Animation Zones in MinPlayableMap

| Actor Label | Location | Purpose |
|-------------|----------|---------|
| `AnimZone_Safe_001` | (0, 0, 50) | Safe zone — idle/walk animations |
| `AnimZone_Combat_001` | (2000, 1500, 50) | Combat zone — alert/attack animations |
| `AnimMarker_Idle_Zone` | (200, 0, 80) | Visual marker — idle state |
| `AnimMarker_Walk_Path` | (600, 200, 80) | Visual marker — walk path |
| `AnimMarker_Run_Zone` | (1200, 400, 80) | Visual marker — run zone |
| `AnimMarker_Crouch_Zone` | (-300, 500, 80) | Visual marker — stealth/crouch zone |
| `FootIK_TestPlane_001` | (300, -200, 10) | Foot IK calibration surface |

---

## NPC Animation Targets

Three NPC actors placed by Agent #9 are ready for animation:
- `NPC_Hunter_001` at (500, 300, 100) — aggressive patrol animations
- `NPC_Gatherer_001` at (-400, 600, 100) — foraging/crouching animations  
- `NPC_Scout_001` at (800, -200, 100) — alert/scanning animations

Each NPC should have a distinct animation personality:
- **Hunter**: Heavy footfalls, weapon-ready stance, aggressive idle
- **Gatherer**: Bent-over foraging, cautious movement, light footsteps
- **Scout**: Upright alert posture, head-scanning, quick directional changes

---

## Motion Matching Notes

UE5 5.x Motion Matching system (`UE5_MotionMatching` plugin) is the target for fluid locomotion:
- Requires a **PoseSearch Database** asset with tagged animation sequences
- Trajectory matching for responsive direction changes
- Works best with 20+ animation clips per character

### Priority Clips Needed (for Asset Team)
1. Idle_Breathing_01 (4s loop)
2. Walk_Forward_01 (2s loop)
3. Walk_Forward_Carry (2s loop — carrying resources)
4. Run_Forward_01 (1.5s loop)
5. Sprint_Forward_01 (1s loop)
6. Crouch_Idle_01 (3s loop)
7. Crouch_Walk_01 (2s loop)
8. Jump_Start_01 (0.3s)
9. Jump_Loop_01 (1s loop)
10. Jump_Land_01 (0.5s)
11. Combat_Idle_01 (3s loop)
12. Attack_Spear_01 (0.8s)
13. Attack_Club_01 (0.6s)
14. Death_Forward_01 (1.5s)
15. Climb_Ledge_01 (1.2s)

---

## Dinosaur Animation Notes

Dinosaur pawns (`TRex_Savana_001`, `Raptor_*`, `Brachio_*`) require separate animation sets:
- **T-Rex**: Bipedal locomotion, tail counterbalance, head bob
- **Raptor**: Agile quadruped-to-biped transitions, pack coordination
- **Brachiosaurus**: Slow quadruped, neck sweep, ground feeding

Dinosaur animations should use **Root Motion** to ensure accurate world-space movement matching AI navigation.

---

## Technical Decisions

1. **Animation Zones over C++ triggers** — Used TriggerBox actors in-world to define animation state zones. This is Blueprint-compatible and works without recompilation.
2. **Sphere markers for visual debugging** — Small sphere actors mark animation zone boundaries for level designers.
3. **Foot IK test plane** — Flat reference surface at known height for IK system calibration.
4. **SkeletalMeshActor animation_mode** — Set to ANIMATION_BLUEPRINT on all skeletal actors for future AnimBP assignment.

---

## Next Steps for Agent #11 (NPC Behavior)

- Use `AnimZone_Safe_001` and `AnimZone_Combat_001` as behavior state triggers
- NPC Behavior Trees should query animation zone overlaps to select appropriate animation states
- `NPC_Hunter_001`, `NPC_Gatherer_001`, `NPC_Scout_001` are ready for BT assignment
- Foot IK should be enabled in NPC AnimBPs when terrain navigation is active

---

*Generated by Animation Agent #10 — PROD_CYCLE_AUTO_20260619_002*
