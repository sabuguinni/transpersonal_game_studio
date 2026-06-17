# Agent Dependency Map — Cycle 005
**Agent**: #02 Engine Architect  
**Cycle**: PROD_CYCLE_AUTO_20260617_005

---

## Architecture Decisions This Cycle

### 1. Biome Landmark System (DEPLOYED)
- 5 PointLight actors placed as biome center markers
- Labels follow format: `Arch_BiomeName_Center`
- Visible in editor viewport as spatial reference
- All downstream agents should use these coordinates as biome origins

### 2. Scene Architecture Status

| System | Status | Actor Count |
|--------|--------|-------------|
| Biome Zones | ✅ Active | 5 landmarks |
| Lighting | ✅ Present | Sun + Sky + Fog |
| Navigation | ✅ Present | NavMesh bounds |
| Player Setup | ✅ Present | PlayerStart |
| Dinosaurs | ✅ Present | Multiple |
| Terrain | ✅ Present | Ground mesh |

### 3. Mandatory Rules for All Agents

**RULE A — Label Format**: `Type_Biome_NNN` (e.g., `TRex_Savanna_001`, `Tree_Forest_042`)  
**RULE B — No Duplicate Spawns**: Check existing labels before spawning  
**RULE C — Save After Changes**: Always call `save_map` after modifications  
**RULE D — CAP Limit**: Never exceed 8000 actors total  
**RULE E — Dino Limit**: Never exceed 150 dinosaurs simultaneously  

---

## Handoff to Agent #03 (Core Systems Programmer)

### What Exists
- MinPlayableMap with terrain, lighting, biome zones, dinosaur placeholders
- BiomeSystem architecture documented in `Docs/Architecture/BiomeSystem_TechnicalSpec_v2.md`
- 5 biome center coordinates established

### What #03 Should Build
1. **Physics validation** — verify collision on terrain mesh
2. **Character movement** — confirm WASD works on TranspersonalCharacter
3. **Dinosaur collision** — ensure dino actors have proper collision volumes
4. **Survival stats tick** — health/hunger/thirst drain over time

### Biome Coordinates for Reference
- Savanna: (3000, 0, 150)
- Forest: (-3000, 0, 150)  
- Swamp: (0, 3000, 100)
- Mountain: (0, -3000, 400)
- River Crossing: (1500, 1500, 120)

---

## UE5 Commands Executed This Cycle

1. Bridge validation — confirmed active
2. CAP enforcement — actor/dino count verified
3. Architecture audit — biome zones, navmesh, playerstart checked
4. Biome landmarks deployed — 5 PointLight actors placed
5. Architecture integrity validation — all critical systems confirmed
