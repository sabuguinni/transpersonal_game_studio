# Character Movement Configuration — Core Systems Programmer #03
## Cycle: PROD_CYCLE_AUTO_20260618_011

## Overview
This document records the canonical CharacterMovementComponent configuration applied to all Pawn actors in MinPlayableMap. These values are tuned for a realistic prehistoric human survival game.

## Movement Parameters (Applied via ue5_execute Python)

| Property | Value | Rationale |
|---|---|---|
| `max_walk_speed` | 300.0 cm/s | ~10 km/h — realistic human walk pace |
| `jump_z_velocity` | 420.0 cm/s | Moderate jump — prehistoric human fitness |
| `gravity_scale` | 1.0 | Real gravity — no floaty feel |
| `max_step_height` | 45.0 cm | Can step over rocks/roots |
| `walkable_floor_angle` | 44.0° | Can traverse slopes up to ~44° |
| `air_control` | 0.2 | Limited mid-air steering — realistic |
| `braking_friction_factor` | 2.0 | Responsive stop — no ice-skating |

## Apply Pattern (Modify-Existing — NEVER destroy+respawn)

```python
import unreal
actors = unreal.EditorLevelLibrary.get_all_level_actors()
pawn_actors = [a for a in actors if isinstance(a, unreal.Pawn)]
for p in pawn_actors:
    cmc = p.get_component_by_class(unreal.CharacterMovementComponent)
    if cmc:
        cmc.set_editor_property('max_walk_speed', 300.0)
        cmc.set_editor_property('jump_z_velocity', 420.0)
        cmc.set_editor_property('gravity_scale', 1.0)
        cmc.set_editor_property('max_step_height', 45.0)
        cmc.set_editor_property('walkable_floor_angle', 44.0)
        cmc.set_editor_property('air_control', 0.2)
        cmc.set_editor_property('braking_friction_factor', 2.0)
```

## Dinosaur Actor Audit
- Dinosaur actors found via label matching: trex, raptor, brach, dino, rex, saur, stego, trike
- Each checked for SkeletalMeshComponent (real mesh) or StaticMeshComponent (placeholder)
- Mesh status logged: SET or NONE

## PlayerStart Height Rule
- PlayerStart z must be >= 100 cm above terrain floor
- If z < 50, auto-lifted to z=100 to prevent player spawning underground

## Architecture Notes
- **Modify-existing pattern is mandatory** — lighting agents confirmed this in cycle 010
- Never destroy+respawn actors that already exist in the persistent level
- Always save map after modifications: `EditorLoadingAndSavingUtils.save_map(world, "/Game/Maps/MinPlayableMap")`

## Dependencies
- Agent #02 (Engine Architect): Architecture validation confirmed 8/8 pillars present
- Agent #04 (Performance Optimizer): Review CMC tick rate — CharacterMovementComponent ticks every frame
- Agent #10 (Animation Agent): Ensure AnimBP reads `max_walk_speed` for blend space transitions
- Agent #12 (Combat AI): Dinosaur pawn CMC values will need separate tuning (higher speed, different gravity)

## Next Steps for Agent #04 (Performance Optimizer)
1. Audit CMC tick interval — consider setting `set_component_tick_interval` on non-player pawns
2. Check NavMesh rebuild cost after PlayerStart repositioning
3. Verify LOD distances on dinosaur StaticMesh placeholders
4. Profile frame time with all pawn actors active
