# Performance Optimizer — Cycle PROD_CYCLE_AUTO_20260617_010

## Agent #4 — Performance Optimizer

### UE5 Commands Executed (5/5 ✅)

| # | Command | Result |
|---|---------|--------|
| 1 | Bridge validation | `bridge_ok` ✅ |
| 2 | CAP enforcement audit | Actor/dino/survival/patrol/perf counts verified ✅ |
| 3 | Performance profiling | Actor type breakdown + frame budget estimate ✅ |
| 4 | Cull distance optimization | Applied LOD cull distances to static mesh actors ✅ |
| 5 | Performance zone audit | Patrol dinos + survival zones + perf health verified ✅ |

---

## Performance Budgets (60fps PC / 30fps Console)

| Category | Budget (ms) | Estimated Cost |
|----------|-------------|----------------|
| Static Meshes | ~3ms | ~0.05ms × count |
| Skeletal Meshes | ~4ms | ~0.3ms × count |
| Lights | ~2ms | ~0.5ms × count |
| AI / Tick | ~3ms | ~0.1ms × dino |
| Render overhead | ~2ms | Fixed |
| **Total budget** | **16.67ms** | Varies |

---

## Cull Distance Rules Applied

| Actor Type | Cull Distance |
|------------|---------------|
| Trees / Rocks / Props | 8,000 units |
| Waypoints / Markers / Zones | 3,000 units |
| Dinosaur actors | 12,000 units |
| Lights / Sky | No cull (always visible) |

---

## CAP Limits (Enforced)

| Resource | Limit | Status |
|----------|-------|--------|
| Total actors | 8,000 | ✅ |
| Dinosaur actors | 150 | ✅ |
| Skeletal meshes ticking | 50 | Monitor |
| Dynamic lights | 20 | Monitor |

---

## Directives for Next Agents

### Agent #5 (Procedural World Generator)
- Keep terrain actor count under 500 static mesh actors for terrain chunks
- Use instanced static meshes (ISM) for foliage — NOT individual actors
- Each biome zone should be a single ISM component, not 100 separate actors

### Agent #6 (Environment Artist)
- Use Hierarchical ISM (HISM) for trees and rocks
- Max 50 unique static mesh types in the scene (GPU memory budget)
- Rocks: cull at 6,000 units; Trees: cull at 10,000 units

### Agent #9 (Character Artist)
- Skeletal mesh actors: max 20 simultaneously ticking in viewport
- Use LOD 0 within 3,000 units, LOD 1 within 8,000, LOD 2 beyond
- Disable animation ticking on dinos > 15,000 units from player

### Agent #12 (Combat & Enemy AI)
- Behavior Tree tick interval: 0.2s minimum (5 ticks/second max per dino)
- Perception radius: 5,000 units max (beyond = too expensive)
- Max 8 dinos in active combat state simultaneously

---

## Performance Health Summary

- **Frame budget**: Healthy — well within 16.67ms target
- **Actor count**: Within CAP limits
- **Cull distances**: Applied to static mesh actors
- **Patrol system**: 5 patrol dinos with waypoints verified
- **Survival zones**: 5 zones (Water/Food×2/Shelter/DangerZone) verified
- **Map saved**: ✅

---

## NEXT CYCLE PRIORITIES

1. **Instanced Static Mesh audit** — verify foliage uses HISM not individual actors
2. **Tick rate profiling** — measure actual ms cost of patrol AI tick
3. **Memory budget** — audit GPU texture memory usage
4. **Streaming volume** — add World Partition streaming volumes for distant zones
