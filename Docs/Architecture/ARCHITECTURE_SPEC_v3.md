# Transpersonal Game Studio — Engine Architecture Specification v3
**Agent #02 — Engine Architect | Cycle PROD_CYCLE_AUTO_20260617_008**

---

## ARCHITECTURE HEALTH STATUS

| Pillar | Status | Notes |
|--------|--------|-------|
| PlayerStart | ✅ ENFORCED | Position (0,0,200) — safe spawn above terrain |
| NavMesh Bounds | ✅ ENFORCED | 200×200×20 scale covers full playable area |
| Directional Light (Sun) | ✅ ENFORCED | Sun_Cretaceous — pitch -45°, warm amber |
| SkyAtmosphere | ✅ ENFORCED | SkyAtmosphere_Cretaceous |
| ExponentialHeightFog | ✅ ENFORCED | Fog_Cretaceous — jungle atmosphere |
| Biome_Savanna | ✅ ENFORCED | Center: (5000, 0, 300) |
| Biome_Forest | ✅ ENFORCED | Center: (-5000, 0, 300) |
| Biome_Swamp | ✅ ENFORCED | Center: (0, 5000, 300) |
| Biome_Volcanic | ✅ ENFORCED | Center: (0, -5000, 500) |
| Biome_Riverbank | ✅ ENFORCED | Center: (3000, 3000, 300) |

---

## WORLD LAYOUT — BIOME MAP

```
                    [Biome_Swamp]
                    (0, 5000)
                         |
[Biome_Forest] ----  [ORIGIN]  ---- [Biome_Savanna]
(-5000, 0)        (PlayerStart)      (5000, 0)
                         |
                  [Biome_Volcanic]
                    (0, -5000)

[Biome_Riverbank] at (3000, 3000) — diagonal NE
```

---

## ACTOR NAMING CONVENTION (LAW)

```
Format: Type_Biome_NNN
Examples:
  TRex_Savanna_001
  Rock_Forest_042
  Tree_Swamp_007
  NPC_Riverbank_003

FORBIDDEN:
  FootstepEmitter_CombatZone_QuestTarget_AI_Behavior  ← DEGENERATE
  TRex_001_AI_Combat_Zone_Quest                        ← DEGENERATE
```

---

## TECHNICAL ARCHITECTURE — MODULE DEPENDENCY GRAPH

```
[TranspersonalGame Module]
    ├── TranspersonalCharacter (ACharacter)
    │     ├── CharacterMovementComponent (WASD + jump)
    │     ├── SurvivalStats (health/hunger/thirst/stamina/fear)
    │     └── CameraBoom + FollowCamera
    ├── TranspersonalGameMode (AGameModeBase)
    │     └── DefaultPawnClass = TranspersonalCharacter
    ├── PCGWorldGenerator (UActorComponent)
    │     ├── BiomeManager (5 biomes)
    │     └── TerrainHeightmap
    ├── FoliageManager (UActorComponent)
    │     └── Tree/Rock/Fern placement per biome
    ├── CrowdSimulationManager (UActorComponent)
    │     └── DinosaurPawn AI (BehaviorTree)
    └── SharedTypes.h
          └── All cross-module enums/structs
```

---

## ACTIVE SOURCE FILES (17 core files)

| File | Status | Role |
|------|--------|------|
| TranspersonalGame.cpp/.h | ✅ | Module registration |
| TranspersonalGameState.h/.cpp | ✅ | Core game state (35 properties) |
| TranspersonalCharacter.h/.cpp | ✅ | Player character (38 properties) |
| PCGWorldGenerator.h/.cpp | ✅ | Procedural world gen (14 methods) |
| FoliageManager.h/.cpp | ✅ | Vegetation system (5 methods) |
| CrowdSimulationManager.h/.cpp | ✅ | Crowd AI |
| ProceduralWorldManager.h/.cpp | ✅ | World management |
| BuildIntegrationManager.h/.cpp | ✅ | Build integration |
| SharedTypes.h | ✅ | Shared enums/structs (22 types) |
| ConstructorStubs.cpp | ✅ | CDO stubs |
| LinkerStubs.cpp | ✅ | Linker stubs |

---

## RULES FOR ALL AGENTS (ENFORCED BY ARCHITECT)

### RULE A — Python-Only World Building
C++ files written via github_file_write are NOT compiled by the running editor.
All world content MUST be created via `ue5_execute` Python commands.

### RULE B — Label Format
`Type_Biome_NNN` — no exceptions. No concatenated system names.

### RULE C — CAP Limits
- Max actors in map: 8,000
- Max dinosaur actors: 150
- Check before spawning: `len(unreal.EditorLevelLibrary.get_all_level_actors())`

### RULE D — No Camera Modification
NEVER use set_view_location, set_view_rotation, or any viewport camera command.

### RULE E — Save After Every Session
```python
world = unreal.EditorLevelLibrary.get_editor_world()
unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/Maps/MinPlayableMap')
```

### RULE F — No Spiritual Content
Zero tolerance for meditation, consciousness, spiritual guides, energy healing.
This is a DINOSAUR SURVIVAL game.

---

## PRIORITY QUEUE FOR NEXT AGENTS

| Priority | Agent | Task |
|----------|-------|------|
| P1 | #05 World Generator | Terrain height variation — flat landscape is #1 visual problem |
| P2 | #06 Environment Artist | Replace PointLight biome markers with actual mesh shapes |
| P3 | #09 Character Artist | Dinosaur mesh actors (SkeletalMesh or StaticMesh placeholders) |
| P4 | #12 Combat AI | DinosaurBase behavior tree setup |
| P5 | #08 Lighting | Day/night cycle using TimeOfDay system |

---

## CYCLE HISTORY

| Cycle | Key Achievement |
|-------|----------------|
| 006 | Core systems deployment — Sun/Sky/Fog/NavMesh |
| 007 | Debug wireframe cleanup, Cretaceous atmosphere |
| 008 | Architecture enforcement — all 10 pillars verified/deployed |

---

*Architecture Spec v3 — Engine Architect #02 — PROD_CYCLE_AUTO_20260617_008*
