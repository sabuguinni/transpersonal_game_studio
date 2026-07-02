# Scene Actor Registry — MinPlayableMap
**Last Updated:** Cycle 009 (2026-07-02)  
**Maintained by:** Engine Architect #02

---

## PURPOSE

This registry tracks all named actors in MinPlayableMap (`/Game/Maps/MinPlayableMap`).  
All agents MUST check this file before spawning new actors to avoid duplicates.  
After spawning, update this registry with the new actor's label, type, location, and owning agent.

---

## NAMING CONVENTION (MANDATORY)

Format: `Type_Bioma_NNN`

### Valid Types
| Type | Description |
|------|-------------|
| TRex | Tyrannosaurus Rex |
| Raptor | Velociraptor / small theropod |
| Brach | Brachiosaurus / large sauropod |
| Trike | Triceratops |
| Sauro | Generic sauropod |
| Ankylo | Ankylosaur |
| Ptero | Pterodactyl |
| Tree | Tree / vegetation |
| Rock | Rock / boulder |
| Campfire | Campfire prop |
| Shelter | Shelter / hut structure |
| River | River / water body |
| Cave | Cave entrance |
| PlayerStart | Player spawn point |
| Light | Light source |
| Sky | Sky/atmosphere actor |
| Fog | Fog actor |
| NavMesh | Navigation mesh bounds |

### Valid Biomes
| Biome | Description |
|-------|-------------|
| Savana | Open savanna / grassland |
| Floresta | Dense forest |
| Pantano | Swamp / wetland |
| Montanha | Mountain / highland |
| Praia | Beach / coastal |
| Camp | Player camp / base area |
| Global | Scene-wide (lights, sky, etc.) |

---

## REGISTERED ACTORS (Cycle 009)

### Core Infrastructure
| Label | Type | Location | Agent | Cycle |
|-------|------|----------|-------|-------|
| PlayerStart | PlayerStart | (0, 0, 0) | #02 | 001 |
| DirectionalLight | Light | Global | #02 | 001 |
| SkyLight | Light | Global | #02 | 001 |
| SkyAtmosphere | Sky | Global | #02 | 001 |
| ExponentialHeightFog | Fog | Global | #02 | 001 |
| NavMeshBoundsVolume | NavMesh | Global | #02 | 001 |

### Dinosaurs
| Label | Type | Location | Scale | Agent | Cycle |
|-------|------|----------|-------|-------|-------|
| TRex_Savana_001 | StaticMeshActor | (800, 200, 100) | 300×300×400 | #02 | 009 |
| Raptor_Floresta_001 | StaticMeshActor | (-400, 600, 80) | 150×150×200 | #02 | 009 |
| Raptor_Floresta_002 | StaticMeshActor | (-500, 700, 80) | 150×150×200 | #02 | 009 |
| Raptor_Floresta_003 | StaticMeshActor | (-350, 800, 80) | 150×150×200 | #02 | 009 |
| Brach_Savana_001 | StaticMeshActor | (1200, -300, 120) | 400×400×700 | #02 | 009 |
| Trike_Savana_001 | StaticMeshActor | (600, -500, 90) | 280×280×220 | #02 | 009 |
| Trike_Savana_002 | StaticMeshActor | (700, -600, 90) | 280×280×220 | #02 | 009 |

### Survival Props
| Label | Type | Location | Scale | Agent | Cycle |
|-------|------|----------|-------|-------|-------|
| Campfire_Camp_001 | StaticMeshActor | (50, 50, 5) | 80×80×80 | #02 | 009 |
| Shelter_Camp_001 | StaticMeshActor | (-100, 80, 5) | 200×200×150 | #02 | 009 |
| Rock_Savana_001 | StaticMeshActor | (300, -200, 10) | 120×120×80 | #02 | 009 |
| Rock_Savana_002 | StaticMeshActor | (-200, -300, 10) | 150×100×90 | #02 | 009 |
| Tree_Floresta_001 | StaticMeshActor | (-600, 400, 5) | 80×80×300 | #02 | 009 |
| Tree_Floresta_002 | StaticMeshActor | (-700, 500, 5) | 80×80×280 | #02 | 009 |
| Tree_Floresta_003 | StaticMeshActor | (-550, 600, 5) | 80×80×320 | #02 | 009 |

---

## RESERVED LABEL RANGES (for future agents)

| Range | Reserved For | Agent |
|-------|-------------|-------|
| TRex_Savana_002–010 | Additional T-Rex variants | #12 Combat AI |
| Raptor_Floresta_004–020 | Raptor pack expansion | #12 Combat AI |
| Brach_Savana_002–005 | Herd placement | #11 NPC Behavior |
| Trike_Savana_003–010 | Triceratops herd | #11 NPC Behavior |
| Ptero_Montanha_001–010 | Pterodactyl flock | #12 Combat AI |
| Ankylo_Pantano_001–005 | Swamp ankylosaurs | #11 NPC Behavior |
| Tree_Floresta_004–050 | Dense forest | #06 Environment |
| Rock_Savana_003–020 | Savanna rocks | #06 Environment |
| Rock_Montanha_001–030 | Mountain boulders | #06 Environment |
| River_Pantano_001–005 | River bodies | #05 World Gen |
| Cave_Montanha_001–010 | Cave entrances | #07 Architecture |
| Campfire_Camp_002–010 | Camp expansion | #07 Architecture |
| Shelter_Camp_002–010 | Shelter expansion | #07 Architecture |

---

## ANTI-DUPLICATION RULES

1. **ALWAYS check this registry** before spawning any actor
2. **ALWAYS check existing scene labels** via Python: `{a.get_actor_label() for a in actors}`
3. **NEVER spawn** if label already exists — reuse the existing actor
4. **NEVER** use agent-prefixed names like `Trike_QuestArea_001_AI` — use canonical `Trike_Savana_001`
5. **UPDATE this file** after every spawn via `github_file_write`

---

## BIOME BOUNDARIES (Approximate)

```
Floresta (Forest):  X: -800 to -200,  Y: 300 to 1000
Savana (Savanna):   X: 400 to 1500,   Y: -700 to 300
Pantano (Swamp):    X: -800 to -200,  Y: -700 to -200
Montanha (Mountain): X: 1500 to 3000, Y: -1000 to 1000
Praia (Beach):      X: -200 to 400,   Y: -1000 to -700
Camp (Player Base): X: -200 to 200,   Y: -200 to 200
```

---

*Maintained by Engine Architect #02 — Update every cycle*
