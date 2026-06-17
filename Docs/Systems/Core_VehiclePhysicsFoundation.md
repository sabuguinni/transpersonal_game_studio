# Vehicle Physics Foundation - Core Systems Programmer

## Overview
Foundation system for physics-based vehicle transportation in the prehistoric world. Enables player to construct and operate primitive carts, rafts, and sleds for resource transport and exploration.

## Implementation Status
**PROTOTYPE DEPLOYED** - Placeholder vehicle active in MinPlayableMap at Savanna location

## Core Concept

### Primitive Vehicles (Stone Age Technology)
- **Wooden Cart**: 4-wheel cart for land transport (pulled by player or tamed dinosaur)
- **Raft**: Floating platform for river/lake navigation
- **Sled**: Snow/ice transport for tundra biome
- **Travois**: Drag-sled for heavy loads (2 poles + hide)

### Physics Requirements
- **Realistic Weight**: Vehicle mass affects acceleration and terrain traversal
- **Terrain Response**: Mud slows wheels, ice reduces friction, rocks cause bumps
- **Load Capacity**: Max cargo weight before vehicle becomes immobile
- **Durability**: Damage from collisions, wear from use

## Technical Specifications

### Vehicle Actor Properties
```cpp
// Base vehicle class (extends WheeledVehiclePawn or custom)
UCLASS()
class ACore_PrimitiveVehicle : public APawn
{
    UPROPERTY(EditAnywhere, Category = "Physics")
    float MaxCargoWeight = 500.0f;  // kg
    
    UPROPERTY(EditAnywhere, Category = "Physics")
    float CurrentCargoWeight = 0.0f;
    
    UPROPERTY(EditAnywhere, Category = "Physics")
    float WheelFriction = 1.0f;  // Affected by terrain
    
    UPROPERTY(EditAnywhere, Category = "Physics")
    float Durability = 100.0f;  // Health of vehicle
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxSpeed = 800.0f;  // cm/s (8 m/s = 28.8 km/h)
};
```

### Terrain Interaction System
```cpp
// Terrain type affects vehicle performance
enum class ETerrainType : uint8
{
    Grass,      // Friction: 1.0, Speed: 100%
    Mud,        // Friction: 2.5, Speed: 40%
    Sand,       // Friction: 1.8, Speed: 60%
    Rock,       // Friction: 0.8, Speed: 120% (bumpy)
    Ice,        // Friction: 0.3, Speed: 150% (slippery)
    Water       // Raft only, Speed: 50%
};
```

### Physics Simulation
- **Wheel Colliders**: 4 sphere colliders for cart wheels
- **Suspension**: Spring-damper system for terrain absorption
- **Center of Mass**: Low and centered for stability
- **Cargo Physics**: Dynamic mass added to vehicle on load

## Current Deployment

### Prototype Vehicle
- **Label**: PrimitiveCart_Placeholder_001
- **Location**: Savanna (5000, 3000, 200)
- **Scale**: 3.0 x 1.5 x 1.0 (placeholder dimensions)
- **Tags**: VehiclePhysics, PlaceholderVehicle

### Placeholder Status
Currently using StaticMeshActor as placeholder. Full WheeledVehiclePawn implementation requires:
1. Custom physics asset for wheels
2. Suspension tuning
3. Input binding for player control
4. Cargo attachment system

## Gameplay Integration

### Crafting Requirements (Wooden Cart)
- **Materials**: 20x Wood, 4x Stone Wheel, 10x Rope
- **Crafting Time**: 5 minutes
- **Crafting Location**: Workbench or Campfire

### Player Interaction
1. **Approach Vehicle**: Prompt "Press E to Mount Cart"
2. **Mount**: Player enters vehicle, camera switches to third-person follow
3. **Control**: WASD for movement, Space for brake
4. **Cargo Management**: Open inventory UI, drag items to cart slots
5. **Dismount**: Press E again to exit

### Tamed Dinosaur Integration
- **Attach to Dinosaur**: Rope connects cart to tamed Parasaurolophus or Triceratops
- **Auto-Follow**: Dinosaur pulls cart, player rides on dinosaur
- **Speed Boost**: Dinosaur-pulled cart moves 2x faster than player-pushed

## Performance Considerations

### Optimization Rules
- **Max Active Vehicles**: 5 simultaneous in world
- **Physics Tick Rate**: 60Hz for player vehicle, 30Hz for distant vehicles
- **LOD System**: Disable wheel physics beyond 100m, use simplified collision
- **Cargo Limit**: Max 50 items per vehicle to prevent inventory bloat

### Memory Budget
- **Per Vehicle**: ~5MB (mesh + physics + cargo inventory)
- **Total Budget**: 25MB for 5 vehicles

## Next Steps for Agent #4 (Performance Optimizer)

### Immediate Tasks
1. **Profile vehicle physics cost** - measure CPU impact of wheel simulation
2. **Implement distance-based LOD** - disable wheel physics beyond 100m
3. **Optimize cargo system** - batch inventory updates to reduce overhead
4. **Test terrain interaction** - verify mud/sand/ice friction modifiers work

### Future Enhancements
1. **Raft physics** - buoyancy system for water navigation
2. **Sled physics** - low-friction system for ice/snow
3. **Damage system** - collision detection and durability reduction
4. **Wheel breakage** - individual wheel health and replacement

## Dependencies

### Required Systems
- **Crafting System**: Vehicle construction recipes
- **Inventory System**: Cargo storage and management
- **Dinosaur AI**: Taming and follow behavior for cart-pulling
- **Terrain System**: Surface type detection for friction modifiers

### Required Assets
- **Wooden Cart Mesh**: Primitive cart with 4 wheels
- **Raft Mesh**: Log platform with rope bindings
- **Sled Mesh**: Wooden runners with hide covering
- **Wheel Mesh**: Stone or wooden wheel

## Technical Notes

### Why Placeholder?
WheeledVehiclePawn requires extensive setup:
- Physics asset with wheel constraints
- Suspension spring configuration
- Input axis mapping
- Camera boom setup

Current placeholder allows:
- Position testing in world
- Tag-based system identification
- Scale and placement validation
- Integration planning with other systems

### Upgrade Path
1. **Replace StaticMeshActor** with WheeledVehiclePawn
2. **Add wheel colliders** (4x sphere colliders)
3. **Configure suspension** (spring stiffness, damping)
4. **Bind input** (WASD movement, Space brake)
5. **Implement cargo UI** (inventory panel for vehicle storage)

---

**Status**: 🟡 PROTOTYPE (Placeholder Active)  
**Last Updated**: 2026-06-17  
**Agent**: Core Systems Programmer #3  
**Next Agent**: Performance Optimizer #4  
**Priority**: Medium (P3 - Character System dependency)
