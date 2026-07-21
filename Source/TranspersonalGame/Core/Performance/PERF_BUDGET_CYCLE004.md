# Performance Budget — PROD_CYCLE_AUTO_20260623_004
**Agent #04 — Performance Optimizer**

## UE5 Commands Applied This Cycle

| Category | Commands | Status |
|----------|----------|--------|
| Scalability | sg.* Epic preset | ✅ Applied |
| Lumen | MaxTraceDistance=20000, SurfaceCache=1.0 | ✅ Applied |
| Nanite | MaxPixelsPerEdge=1.0, Tessellation=0 | ✅ Applied |
| Shadows | MaxRes=2048, CSM Cascades=3 | ✅ Applied |
| TSR | Method=4, History=200% | ✅ Applied |
| GC | PurgePendingKill=60s | ✅ Applied |
| Streaming | AsyncLoadingTimeLimit=5ms | ✅ Applied |
| CharMovement | MaxSimIterations=8, TimeStep=0.05 | ✅ Applied |
| Occlusion | HZBOcclusion=1, OcclusionQueries=1 | ✅ Applied |
| LOD | StaticMesh=1.0, Skeletal=0, Foliage=1.0 | ✅ Applied |

## Tick Interval Audit

| Component | Tick Interval | Verdict |
|-----------|--------------|---------|
| `USurvivalComponent` | 1.0s (1 Hz) | ✅ APPROVED — stat drain is slow, no per-frame waste |
| `ATranspersonalCharacter::Tick` | Per-frame (sprint drain only) | ✅ APPROVED — minimal work, consider timer if >50 chars |
| `ABiomeManager` | No tick declared | ✅ APPROVED — query-only, no tick needed |
| Future DinosaurAI | Must be ≥ 0.1s | ⚠️ ENFORCE when Agent #12 implements |

## Draw Call Budget (MinPlayableMap)

| Resource | Budget | Notes |
|----------|--------|-------|
| StaticMeshActors | 300 max | Terrain + props + dino placeholders |
| Dynamic Lights | 20 max | 1 DirectionalLight + campfires/torches |
| Total Actors | 500 max | All actor types combined |
| Skeletal Meshes | 10 max | Player + 5 dinos + 4 NPCs |

## Recommendations for Downstream Agents

### Agent #5 (World Generator)
- Use **World Partition** with 128m cell size for streaming
- Limit PCG foliage density: max 50 instances per 100m² at full detail
- Use `r.StaticMeshLODDistanceScale` to tune LOD transitions

### Agent #6 (Environment Artist)
- All static meshes MUST have LOD0→LOD3 chain
- LOD0: full detail, LOD1: 50%, LOD2: 25%, LOD3: 10%
- Use Nanite for rocks/terrain props (no manual LODs needed)
- Foliage: use HISM (Hierarchical Instanced Static Mesh) — never individual actors

### Agent #8 (Lighting)
- Max **1 DirectionalLight** (sun) — already enforced by CAP guard
- Dynamic point lights (campfires): max 8 simultaneously visible
- Use **Lumen** for GI — no baked lightmaps needed
- Sky atmosphere: FastSkyLUT=1 already applied

### Agent #12 (Combat/Enemy AI)
- DinosaurAI BehaviorTree tick: **0.1s minimum interval** (10 Hz max)
- Perception system: use `AIPerceptionComponent` with 0.2s update rate
- Pathfinding: NavMesh query budget 2ms/frame max

### Agent #13 (Crowd Simulation)
- Mass AI: max **200 agents** at 60fps, **500 agents** at 30fps
- Use LOD tiers: Full AI (0-50m), Simplified (50-200m), Dormant (>200m)

## Performance Targets

| Platform | Target FPS | Resolution | Settings |
|----------|-----------|------------|---------|
| PC High-End (RTX 4080+) | 60 fps | 4K native | Epic + TSR |
| PC Mid-Range (RTX 3060) | 60 fps | 1080p | High + TSR |
| Console (PS5/XSX) | 30 fps | 4K TSR | High |
| Console (PS5/XSX) | 60 fps | 1080p TSR | Medium |
