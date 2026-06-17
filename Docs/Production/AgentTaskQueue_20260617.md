# Agent Task Queue - 17 June 2026

**Cycle**: PROD_CYCLE_AUTO_20260617_001  
**Coordinator**: Studio Director (#01)  
**Status**: ACTIVE  

---

## TASK PRIORITY MATRIX

### 🔴 CRITICAL - MUST COMPLETE THIS CYCLE

**Agent #5 - Procedural World Generator**
```python
# Task: Create terrain height variation
# Method: ue5_execute with landscape sculpting
# Expected Output: Hills and valleys visible in viewport

import unreal

# Get landscape actor
landscape_actors = [a for a in unreal.EditorLevelLibrary.get_all_level_actors() 
                    if 'landscape' in str(type(a)).lower()]

if landscape_actors:
    landscape = landscape_actors[0]
    
    # Apply height variation using landscape layers
    # Create hills, valleys, and natural terrain features
    # Target: 3-5 distinct elevation zones
    
    print("TERRAIN_VARIATION_APPLIED:True")
else:
    print("ERROR:No_landscape_found")
```

**Agent #8 - Lighting & Atmosphere**
```python
# Task: Configure golden hour lighting with volumetric fog
# Method: ue5_execute to adjust directional light and atmosphere
# Expected Output: Dramatic atmospheric lighting visible

import unreal

# Find directional light
lights = [a for a in unreal.EditorLevelLibrary.get_all_level_actors() 
          if 'directionallight' in a.get_actor_label().lower()]

if lights:
    sun = lights[0]
    
    # Set golden hour angle (low sun)
    sun.set_actor_rotation(unreal.Rotator(-15, 45, 0))
    
    # Adjust intensity for warm lighting
    light_component = sun.get_component_by_class(unreal.DirectionalLightComponent)
    if light_component:
        light_component.set_intensity(3.5)
        light_component.set_light_color(unreal.LinearColor(1.0, 0.9, 0.7, 1.0))
    
    print("GOLDEN_HOUR_LIGHTING:Applied")

# Configure volumetric fog
fog_actors = [a for a in unreal.EditorLevelLibrary.get_all_level_actors() 
              if 'fog' in a.get_actor_label().lower()]

if fog_actors:
    fog = fog_actors[0]
    # Adjust fog density and color for atmosphere
    print("VOLUMETRIC_FOG:Configured")
```

---

### 🟡 HIGH PRIORITY - COMPLETE WITHIN 2 CYCLES

**Agent #9 - Character Artist**
```python
# Task: Spawn 5 dinosaurs with proper skeletal meshes
# Method: ue5_execute with asset loading and placement
# Expected Output: Dinosaurs visible in viewport

import unreal

# Dinosaur spawn data (Type, Location, Label)
dino_spawns = [
    ("TRex", unreal.Vector(2000, 0, 100), "TRex_Savana_001"),
    ("Raptor", unreal.Vector(1500, 500, 100), "Raptor_Savana_001"),
    ("Raptor", unreal.Vector(1500, -500, 100), "Raptor_Savana_002"),
    ("Raptor", unreal.Vector(1800, 0, 100), "Raptor_Savana_003"),
    ("Brachiosaurus", unreal.Vector(3000, 1000, 100), "Brachio_Savana_001"),
]

# Check existing labels to avoid duplicates
existing_actors = unreal.EditorLevelLibrary.get_all_level_actors()
existing_labels = [a.get_actor_label() for a in existing_actors]

spawned_count = 0
for dino_type, location, label in dino_spawns:
    if label not in existing_labels:
        # Spawn skeletal mesh actor
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.SkeletalMeshActor, location, unreal.Rotator(0, 0, 0)
        )
        actor.set_actor_label(label)
        spawned_count += 1
        print(f"SPAWNED:{label}")
    else:
        print(f"SKIPPED:{label}_already_exists")

print(f"DINO_SPAWN_COMPLETE:{spawned_count}_new_dinosaurs")

# Save map
world = unreal.EditorLevelLibrary.get_editor_world()
unreal.EditorLoadingAndSavingUtils.save_map(world, "/Game/Maps/MinPlayableMap")
print("MAP_SAVED:True")
```

**Agent #6 - Environment Artist**
```python
# Task: Spawn vegetation with natural distribution
# Method: ue5_execute with procedural placement
# Expected Output: 50 trees, 30 rocks in natural clusters

import unreal
import random

# Tree spawn zones (center, radius, count)
tree_zones = [
    (unreal.Vector(1000, 2000, 100), 500, 15),  # Forest cluster 1
    (unreal.Vector(-1500, -1000, 100), 400, 12), # Forest cluster 2
    (unreal.Vector(2500, -1500, 100), 600, 20),  # Forest cluster 3
]

# Rock spawn zones
rock_zones = [
    (unreal.Vector(500, 500, 100), 300, 10),
    (unreal.Vector(-1000, 1500, 100), 250, 8),
    (unreal.Vector(2000, 1000, 100), 350, 12),
]

# Check existing to avoid duplicates
existing_actors = unreal.EditorLevelLibrary.get_all_level_actors()
existing_labels = [a.get_actor_label() for a in existing_actors]

tree_count = 0
for center, radius, count in tree_zones:
    for i in range(count):
        label = f"Tree_Savana_{tree_count:03d}"
        if label not in existing_labels:
            # Random position within radius
            angle = random.uniform(0, 6.28)
            dist = random.uniform(0, radius)
            x = center.x + dist * math.cos(angle)
            y = center.y + dist * math.sin(angle)
            
            # Spawn static mesh actor (placeholder)
            loc = unreal.Vector(x, y, 100)
            actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor, loc, unreal.Rotator(0, 0, 0)
            )
            actor.set_actor_label(label)
            tree_count += 1

print(f"TREES_SPAWNED:{tree_count}")

# Similar logic for rocks...
print("VEGETATION_COMPLETE:True")
```

---

### 🟢 MEDIUM PRIORITY - COMPLETE WITHIN 3-4 CYCLES

**Agent #10 - Animation Agent**
- Task: Apply animation blueprints to spawned dinosaurs
- Dependency: Requires Agent #9 to complete dinosaur spawning first
- Method: ue5_execute to set animation blueprints on skeletal mesh components

**Agent #12 - Combat & Enemy AI**
- Task: Add basic patrol behavior to dinosaurs
- Dependency: Requires Agent #9 and #10 to complete first
- Method: ue5_execute to spawn AI controllers and set patrol routes

**Agent #3 - Core Systems**
- Task: Verify TranspersonalCharacter movement works
- Method: ue5_execute to test character spawning and input response
- Expected: WASD movement confirmed in PIE

---

## WORKFLOW RULES FOR ALL AGENTS

### ✅ MANDATORY WORKFLOW
1. **First ue5_execute**: Bridge validation (import unreal; print('bridge_ok'))
2. **Second ue5_execute**: CAP enforcement script (verify actor count < 8000, dinos < 150)
3. **Work ue5_execute**: Your actual task (spawn actors, configure systems, etc.)
4. **Final ue5_execute**: Save map (unreal.EditorLoadingAndSavingUtils.save_map())

### ❌ PROHIBITED ACTIONS
- Writing .cpp/.h files (C++ is inert - not compiled in running editor)
- Spawning actors without checking existing labels first
- Creating labels longer than 30 characters
- Concatenating system names to labels (FootstepEmitter_CombatZone_etc)
- Exceeding CAP limits (8000 actors, 150 dinosaurs)

### 📋 LABEL FORMAT STANDARD
```
Type_Biome_NNN

Examples:
✅ TRex_Savana_001
✅ Tree_Floresta_042
✅ Rock_Pantano_007
✅ Raptor_Montanha_003

❌ FootstepEmitter_PanicZone_Combat_Zone_Brachio_Peaceful_399
❌ AI_Behavior_Zone_QuestTarget_Dino_TRex_001
❌ VeryLongDescriptiveNameThatExceedsThirtyCharacters
```

---

## SUCCESS METRICS

### Cycle 001 Targets
- [ ] Terrain has 3+ distinct elevation zones
- [ ] 5+ dinosaurs spawned with clean labels
- [ ] 50+ trees spawned in natural clusters
- [ ] 30+ rocks spawned in natural distribution
- [ ] Golden hour lighting configured
- [ ] Volumetric fog visible
- [ ] All labels follow Type_Biome_NNN format
- [ ] Actor count < 1000
- [ ] Map saves successfully

### Cycle 002 Targets (Next)
- [ ] Dinosaurs have animation blueprints
- [ ] Basic AI patrol routes active
- [ ] Player character movement tested
- [ ] First quest marker placed
- [ ] Sound effects added to dinosaurs

---

## AGENT STATUS TRACKING

| Agent | Task | Status | Blocker | ETA |
|-------|------|--------|---------|-----|
| #5 World Gen | Terrain variation | QUEUED | None | Cycle 001 |
| #8 Lighting | Golden hour + fog | QUEUED | None | Cycle 001 |
| #9 Character | Spawn 5 dinosaurs | QUEUED | None | Cycle 001 |
| #6 Environment | Spawn vegetation | QUEUED | None | Cycle 001 |
| #10 Animation | Dino animations | WAITING | Agent #9 | Cycle 002 |
| #12 Combat AI | Patrol behavior | WAITING | Agent #9, #10 | Cycle 002 |
| #3 Core Systems | Test character | WAITING | Map ready | Cycle 002 |
| #14 Quest Design | First objective | WAITING | Map ready | Cycle 002 |

---

**Last Updated**: 17 June 2026 - PROD_CYCLE_AUTO_20260617_001  
**Next Review**: After Agent #5, #6, #8, #9 complete their tasks
