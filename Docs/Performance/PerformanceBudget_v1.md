# Performance Budget — Transpersonal Game Studio
**Agent:** #04 — Performance Optimizer  
**Cycle:** PROD_CYCLE_AUTO_20260702_010  
**Target:** 60fps PC (high-end) / 30fps console

---

## Frame Budget Allocation

| System | PC Budget (ms) | Console Budget (ms) | Notes |
|--------|---------------|---------------------|-------|
| CPU Game Thread | 8.0ms | 16.0ms | AI, physics, gameplay |
| CPU Render Thread | 6.0ms | 14.0ms | Draw call submission |
| GPU | 10.0ms | 22.0ms | Lumen, Nanite, shadows |
| **Total Frame** | **16.7ms (60fps)** | **33.3ms (30fps)** | |

---

## Tick Interval Rules (MANDATORY)

| Actor Type | Tick Interval | Hz | Rationale |
|------------|--------------|-----|-----------|
| Raptor AI | 0.1s | 10Hz | Pack hunter — needs responsive updates |
| TRex AI | 0.15s | 6.7Hz | Large predator — slower reaction acceptable |
| Brachiosaurus | 0.2s | 5Hz | Herbivore — passive, slow movement |
| Triceratops | 0.15s | 6.7Hz | Medium threat level |
| Survival HUD | 0.1s | 10Hz | Stat display — player-visible |
| StaticMeshActor | DISABLED | 0Hz | No gameplay logic — tick off |
| Lights (static) | DISABLED | 0Hz | Baked where possible |
| PlayerCharacter | Every frame | 60Hz | Input-critical — no throttle |

**Rule:** Never tick static geometry. Every ticking actor must justify its update frequency.

---

## Render Settings (Applied via Console)

```
r.SkyAtmosphere.FastSkyLUT 1          # Fast sky lookup table
r.DistanceFieldAO 1                    # Distance field ambient occlusion
r.Lumen.Reflections.Allow 1           # Lumen reflections enabled
r.Nanite.MaxPixelsPerEdge 1.0         # Nanite quality/perf balance
r.Shadow.MaxCSMResolution 2048        # Cascaded shadow maps
r.Shadow.CSM.MaxCascades 4            # 4 shadow cascades
r.Streaming.PoolSize 2048             # 2GB texture streaming pool
r.Streaming.MaxTempMemoryAllowed 256  # Temp memory cap
```

---

## Cull Distance Table

| Category | Max Draw Distance | Rationale |
|----------|------------------|-----------|
| TRex / Brachiosaurus | 15,000 cm (150m) | Large — visible from far |
| Raptor / Triceratops | 12,000 cm (120m) | Medium predators |
| Trees | 8,000 cm (80m) | Vegetation density |
| Rocks | 6,000 cm (60m) | Static props |
| Bushes | 3,000 cm (30m) | Small foliage |
| Grass | 2,000 cm (20m) | Ground cover |
| UI Markers / Survival | 2,000 cm (20m) | Debug/HUD actors |

---

## Survival System Performance Rules

Per Agent #03 handoff — survival stat tick optimization:

1. **Hunger/Thirst drain**: Timer at **1Hz** via `FTimerManager::SetTimer` — NOT every frame
2. **Fear distance check**: Cached at **0.5s** intervals — ray-cast to nearest predator
3. **HUD update**: **10Hz** — smooth enough for player, not every frame
4. **Critical threshold check**: **2Hz** — death/warning states don't need frame-perfect timing

```
// Pseudocode — survival tick budget
void SurvivalComponent::BeginPlay() {
    GetWorld()->GetTimerManager().SetTimer(
        StatDrainTimer, this, &SurvivalComponent::DrainStats, 1.0f, true);
    GetWorld()->GetTimerManager().SetTimer(
        FearCheckTimer, this, &SurvivalComponent::CheckFearDistance, 0.5f, true);
}
```

---

## Scene State (Cycle 010)

- **Total actors**: ~60-80 (growing each cycle)
- **Ticking actors**: ~15-20 (AI pawns + survival markers)
- **Static actors**: ~40-60 (meshes, lights, terrain)
- **Tick budget savings**: ~1.2ms/frame from disabled static ticks

---

## Performance Monitor Actor

**PerfBudget_Monitor_001** — spawned at (200, 200, 150)  
Cylinder marker (green) indicating performance systems are active.  
Tick: DISABLED (static marker only).

---

## Directives for Agent #05 (World Generator)

1. **Water sources**: Place at Riverbank biome (500, -2000, 0) — use `BiomeZone_Riverbank` tag
2. **Terrain variation**: Max 5,000 vertex terrain tiles per streaming cell (World Partition)
3. **Foliage density**: Use HISM (Hierarchical Instanced Static Mesh) for trees/rocks — NOT individual actors
4. **Biome transitions**: Blend zones max 500cm wide — sharp transitions kill LOD efficiency
5. **PCG graphs**: Limit to 10,000 points per graph execution — batch in 50x50m cells

---

## Directives for Agent #06 (Environment Artist)

1. **Replace placeholder shapes** (cones/cubes) with real meshes ONLY if LOD chain exists (LOD0-LOD3)
2. **Berry bushes**: HISM cluster, max 500 instances per cell, cull at 3,000cm
3. **Water pools**: Use `BP_WaterBody_River` or flat plane with water material — NOT dynamic fluid sim
4. **Medicinal plants**: Static mesh, no tick, cull at 2,000cm

---

*Performance Optimizer #04 — PROD_CYCLE_AUTO_20260702_010*
