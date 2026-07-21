# Physics & Collision Setup — Core Systems Programmer #03
## Cycle: PROD_CYCLE_AUTO_20260703_005

---

## Overview

This document describes the physics and collision configuration applied to the
Transpersonal Game (Prehistoric Survival) world during Cycle 005. All changes
were applied via UE5 Python Remote Control — no C++ recompilation required.

---

## Collision Profiles Applied

### Dinosaur Actors (Hub Clearing — X=2100, Y=2400, radius=3000u)

| Actor Pattern | Collision Profile | Simulate Physics | Notes |
|---|---|---|---|
| TRex_* | BlockAll | False (kinematic) | Blocks player + projectiles |
| Raptor_* | BlockAll | False (kinematic) | Blocks player + projectiles |
| Trike_* | BlockAll | False (kinematic) | Blocks player + projectiles |
| Brachio_* | BlockAll | False (kinematic) | Blocks player + projectiles |
| Stego_* | BlockAll | False (kinematic) | Blocks player + projectiles |
| Ankylo_* | BlockAll | False (kinematic) | Blocks player + projectiles |

**Rationale**: Kinematic (non-simulating) collision is correct for static
dinosaur display meshes. Physics simulation is reserved for ragdoll-on-death
events, which require a separate PhysicsAsset and will be implemented in a
dedicated ragdoll pass.

### Terrain Actors

| Actor Pattern | Collision Profile | Notes |
|---|---|---|
| Ground_* | BlockAll | Player walks on surface |
| Terrain_* | BlockAll | Player walks on surface |
| Floor_* | BlockAll | Player walks on surface |

### Physics Test Actor

- **Label**: `PhysicsTest_Cube_001`
- **Location**: (2100, 2400, 200) — hub center, elevated
- **Profile**: `PhysicsActor`
- **Simulate Physics**: True
- **Purpose**: Runtime verification that UE5 physics simulation is active

---

## Biome Physics Parameters (Design Spec)

Per the BiomeSystemArchitecture.md from Agent #02, each biome has distinct
physical properties that affect movement and gameplay:

### Dense Forest Biome
```
friction_coefficient: 0.85
mud_density: 0.0 (dry leaf litter)
water_depth_max: 0.0
movement_speed_modifier: 0.90 (undergrowth slows player)
jump_height_modifier: 1.0
```

### River/Wetland Biome
```
friction_coefficient: 0.40 (wet mud)
mud_density: 0.65
water_depth_max: 150.0 cm
movement_speed_modifier: 0.60 (wading through water)
jump_height_modifier: 0.80
```

### Open Savanna Biome
```
friction_coefficient: 0.95
mud_density: 0.0
water_depth_max: 0.0
movement_speed_modifier: 1.10 (flat ground, easy running)
jump_height_modifier: 1.0
```

### Rocky Highland Biome
```
friction_coefficient: 0.70 (loose rock)
mud_density: 0.0
water_depth_max: 0.0
movement_speed_modifier: 0.75 (climbing slows player)
jump_height_modifier: 0.90
```

### Volcanic Ash Biome
```
friction_coefficient: 0.50 (loose ash)
mud_density: 0.30 (ash behaves like fine mud)
water_depth_max: 0.0
movement_speed_modifier: 0.70
jump_height_modifier: 0.95
```

### Coastal/Beach Biome
```
friction_coefficient: 0.60 (wet sand)
mud_density: 0.10
water_depth_max: 80.0 cm (shallow surf)
movement_speed_modifier: 0.85
jump_height_modifier: 1.0
```

---

## Integration with TranspersonalCharacter

The `TranspersonalCharacter` (pre-built binary) exposes these movement
properties via `UCharacterMovementComponent`:

```
GroundFriction           → maps to friction_coefficient * 8.0
MaxWalkSpeed             → maps to movement_speed_modifier * 600.0 cm/s
JumpZVelocity            → maps to jump_height_modifier * 600.0 cm/s
BrakingDecelerationWalking → inverse of friction_coefficient * 2048.0
```

These can be set at runtime via Blueprint or Python without recompilation.

---

## Ragdoll System (Future Pass)

When a dinosaur dies, the following sequence should execute:

1. Disable kinematic (set `simulate_physics = True`)
2. Apply death impulse vector (direction of killing blow × force_magnitude)
3. Enable `PhysicsAsset` on the SkeletalMeshComponent
4. After 8 seconds: disable physics, hide actor (or trigger decay VFX)

**Physics Asset requirements per species**:
- TRex: 32 bones, spine chain, tail chain, jaw
- Raptor: 28 bones, spine chain, tail chain, claw bones
- Triceratops: 30 bones, frill bones, horn bones, tail chain
- Brachiosaurus: 40 bones, long neck chain (12 bones), tail chain

---

## Survival Physics Integration

The `SurvivalComponent` (integrated into `TranspersonalCharacter`) uses physics
queries for:

- **Fall damage**: `FallVelocity > 800 cm/s` → apply damage = (velocity - 800) * 0.05
- **Water detection**: Overlap with `WaterVolume` → trigger swim mode
- **Mud slowdown**: Overlap with `MudVolume` → apply movement modifier
- **Temperature zones**: Overlap with `TemperatureVolume` → modify body temp stat

---

## Validation Checklist

- [x] Bridge alive (command_id 27579)
- [x] CAP enforcement applied (command_id 27580)
- [x] Hub dinosaur audit completed (command_id 27581)
- [x] BlockAll collision applied to all hub dinos (command_id 27582)
- [x] Terrain collision verified (command_id 27582)
- [x] PhysicsTest_Cube_001 spawned with simulate_physics=True (command_id 27582)
- [x] Level saved after all changes

---

## Next Steps for Agent #04 (Performance Optimizer)

1. **LOD on dinosaur meshes**: All hub dinos should have LOD0/LOD1/LOD2 chain
   - LOD0: full mesh (within 1500u)
   - LOD1: 50% triangles (1500-4000u)
   - LOD2: 25% triangles (4000-8000u)
   - Cull: beyond 8000u

2. **Collision complexity**: Switch dinosaur collision from `BlockAll` (complex)
   to `BlockAll_Simple` using simplified convex hulls — reduces CPU cost by ~40%

3. **Physics budget**: Limit simultaneous simulating physics actors to 32
   (use `p.MaxPhysicsSubsteps 2` and `p.PhysicsHistoryCount 1`)

4. **Character movement**: Verify `MaxWalkSpeed=600`, `MaxRunSpeed=900`,
   `JumpZVelocity=600` are set on the pre-built `TranspersonalCharacter` CDO

---

*Generated by Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260703_005*
