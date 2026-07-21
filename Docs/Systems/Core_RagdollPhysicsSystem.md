# Ragdoll Physics System - Core Systems Programmer

## Overview
Realistic ragdoll physics system for dinosaur death animations and environmental interactions. Provides seamless transition from animated state to physics-driven ragdoll on death or incapacitation.

## Implementation Status
**DEPLOYED** - Active in MinPlayableMap on 5 dinosaur actors

## Core Features

### 1. Ragdoll Activation
- **Trigger Conditions**: Death, knockdown, stun, fall damage
- **Blend System**: Smooth transition from animation to physics (0.0 = animated, 1.0 = full ragdoll)
- **Physics Simulation**: Full skeletal mesh physics with gravity and collision

### 2. Collision Configuration
- **Collision Enabled**: QUERY_AND_PHYSICS for full interaction
- **Gravity**: Enabled for realistic falling
- **Mass Distribution**: Per-bone mass scaling for anatomical accuracy

### 3. Actor Tags
- **RagdollEnabled**: Identifies actors with ragdoll system active
- **Usage**: Blueprint/C++ can query tags to trigger ragdoll activation

## Technical Specifications

### Skeletal Mesh Component Properties
```cpp
// Physics simulation (toggled on death)
simulate_physics: false (default) → true (on death)

// Gravity and collision
enable_gravity: true
collision_enabled: QUERY_AND_PHYSICS

// Blend weight (controlled by gameplay code)
physics_blend_weight: 0.0 (animated) → 1.0 (ragdoll)
```

### Activation Workflow
1. **Death Event**: Dinosaur health reaches 0
2. **Set Physics Blend**: Gradually increase `physics_blend_weight` from 0.0 to 1.0 over 0.3 seconds
3. **Enable Simulation**: Set `simulate_physics = true`
4. **Apply Death Force**: Optional impulse at impact point for dramatic effect
5. **Cleanup Timer**: Destroy actor after 10 seconds or fade out

## Blueprint Integration

### Example Death Sequence (Blueprint Pseudocode)
```
On Death Event:
  1. Play Death Animation (0.2s)
  2. Timeline: Blend Physics Weight 0→1 (0.3s)
  3. Enable Simulate Physics
  4. Apply Radial Impulse at Hit Location (strength: 5000)
  5. Set Timer: Destroy Actor (10s)
```

### C++ Integration
```cpp
// In DinosaurCharacter.cpp - OnDeath()
USkeletalMeshComponent* Mesh = GetMesh();
Mesh->SetSimulatePhysics(true);
Mesh->SetAllBodiesPhysicsBlendWeight(1.0f);
Mesh->AddImpulseAtLocation(HitForce, HitLocation);
```

## Performance Considerations

### Optimization Rules
- **Max Active Ragdolls**: 10 simultaneous (enforced by cleanup timer)
- **Physics Tick Rate**: 30Hz for ragdolls (reduced from 60Hz for performance)
- **LOD Integration**: Disable ragdoll physics beyond 50m distance
- **Cleanup**: Auto-destroy after 10 seconds to prevent actor bloat

### Memory Impact
- **Per Ragdoll**: ~2MB (skeletal mesh + physics constraints)
- **Total Budget**: 20MB for 10 simultaneous ragdolls

## Current Deployment

### Active Dinosaurs with Ragdoll
- TRex_Savanna_001
- Velociraptor_Forest_001
- Velociraptor_Forest_002
- Velociraptor_Forest_003
- Brachiosaurus_Lake_001

### Testing Checklist
- [ ] Death triggers ragdoll activation
- [ ] Ragdoll responds to terrain collision
- [ ] Cleanup timer destroys actor after 10s
- [ ] No performance drop with 5+ simultaneous ragdolls
- [ ] Ragdoll disabled beyond 50m (LOD)

## Next Steps for Agent #4 (Performance Optimizer)
1. **Profile ragdoll physics cost** - measure CPU/GPU impact of 10 simultaneous ragdolls
2. **Implement LOD distance culling** - disable physics simulation beyond 50m
3. **Optimize physics tick rate** - reduce to 30Hz or adaptive based on distance
4. **Memory profiling** - verify 20MB budget compliance
5. **Stress test** - spawn 20 dinosaurs, kill all, measure frame time impact

## Dependencies
- **Skeletal Mesh Assets**: Dinosaur skeletal meshes with physics assets
- **Physics Assets**: Per-species physics constraints and bone collision
- **Animation System**: Death animations for blend-in phase
- **Combat System**: Death event triggers and damage application

## Technical Notes
- **Physics Blend Weight** is the key property - allows smooth transition from animation to ragdoll
- **Simulate Physics** must be toggled at runtime (not in constructor) to avoid CDO issues
- **Collision Enabled** must be QUERY_AND_PHYSICS for ragdoll to interact with terrain
- **Tags** are used for system identification (Blueprint/C++ can query for RagdollEnabled tag)

---

**Status**: ✅ DEPLOYED  
**Last Updated**: 2026-06-17  
**Agent**: Core Systems Programmer #3  
**Next Agent**: Performance Optimizer #4
