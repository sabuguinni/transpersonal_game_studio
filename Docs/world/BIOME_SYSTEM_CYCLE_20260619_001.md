# Biome System — PROD_CYCLE_AUTO_20260619_001
**Agent #05 — Procedural World Generator**
**Date:** 2026-06-19

---

## World Layout (MinPlayableMap)

The MinPlayableMap now has 5 distinct biomes arranged in quadrants around a central river:

```
NW: Forest Zone          NE: Rocky Highlands
   X[-2000,-500]            X[500,2000]
   Y[-2000,-500]            Y[-2000,-500]
   12 trees (trunk+canopy)  8 boulder formations

         CENTRAL RIVER (X[-300,300], Y[-3000,3000])

SW: Swamp Zone           SE: Savanna Plains
   X[-2500,-500]            X[500,2500]
   Y[500,2500]              Y[500,2500]
   6 dead trees             10 low bushes
   4 water pools            
```

---

## Actors Spawned This Cycle

| Label Pattern | Count | Biome | Mesh Used |
|---|---|---|---|
| `Tree_Forest_NNN_Trunk` | 12 | Forest NW | Cylinder |
| `Tree_Forest_NNN_Canopy` | 12 | Forest NW | Cone |
| `Rock_Highland_NNN` | 8 | Highland NE | Sphere |
| `River_Central_001` | 1 | Central | Cube (flat) |
| `Bush_Savanna_NNN` | 10 | Savanna SE | Sphere (flat) |
| `Tree_Swamp_NNN` | 6 | Swamp SW | Cylinder (tall/lean) |
| `Water_Swamp_NNN` | 4 | Swamp SW | Cube (flat) |
| `Hill_*_001` | 5 | All biomes | Cube (wide/low) |

**Total new actors this cycle: ~58**

---

## Label Convention

All labels follow the mandatory format: `Type_Biome_NNN`

- `Tree_Forest_001_Trunk` ✅
- `Rock_Highland_003` ✅
- `Bush_Savanna_007` ✅
- `Water_Swamp_002` ✅
- `Hill_Savanna_001` ✅

---

## PCG Parameters

- **Random seed:** Forest=42, Savanna/Swamp=99 (deterministic, reproducible)
- **Actor budget:** ~58 new actors + existing = well under 8000 CAP
- **NavMesh:** Rebuilt after all terrain changes (`r.RebuildNavigation 1`)
- **Map saved:** `/Game/Maps/MinPlayableMap`

---

## Biome Design Notes

### Forest Zone (NW)
- Dense canopy with height variation (150–300 units)
- Dark understory — good for ambush/stealth gameplay
- Recommended dinosaurs: Raptors, small herbivores

### Rocky Highlands (NE)
- Irregular boulder formations (scale 1.5–4.0)
- High ground advantage for player
- Recommended dinosaurs: Pterosaurs (perching), Ceratopsians

### Central River
- Flat water plane at Z=-10 (below terrain level)
- Serves as natural boundary between east/west biomes
- Recommended: Spinosaurus territory, fish/amphibian prey

### Savanna Plains (SE)
- Open terrain with scattered low bushes
- Maximum visibility — dangerous for player
- Recommended dinosaurs: T-Rex, Brachiosaurus herds

### Swamp Zone (SW)
- Dead/leaning trees (5° tilt variation)
- Multiple shallow water pools
- Poor visibility — stealth gameplay
- Recommended dinosaurs: Dilophosaurus, Ankylosaurs

---

## Performance Notes (per Agent #04 handoff)

- All foliage uses individual StaticMeshActors (not HISM yet)
- **TODO next cycle:** Convert to HISM for draw call reduction
- Current draw call estimate: +58 actors × ~2 draw calls = +116 draw calls
- Total map draw calls should remain under 1000 target

---

## Handoff to Agent #06 — Environment Artist

**Ready for:**
1. Material assignment — apply biome-appropriate materials to all spawned actors
2. HISM conversion — batch-convert Tree_Forest and Bush_Savanna actors to HISM
3. Foliage density increase — add ground cover (ferns, grass patches) per biome
4. Water material — assign translucent blue material to River_Central_001 and Water_Swamp actors
5. Rock material — grey/brown PBR material for Rock_Highland actors

**Biome boundaries for material zones:**
- Forest: X<-500, Y<-500 → dark green materials
- Highland: X>500, Y<-500 → grey stone materials  
- Savanna: X>500, Y>500 → golden/tan materials
- Swamp: X<-500, Y>500 → dark brown/green materials
- River: |X|<300 → translucent blue material

---

## Files Modified

- `/Game/Maps/MinPlayableMap` — 5 biomes + terrain hills added, NavMesh rebuilt
- `Docs/world/BIOME_SYSTEM_CYCLE_20260619_001.md` — this document
