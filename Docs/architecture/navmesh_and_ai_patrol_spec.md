# NavMesh & Raptor Patrol AI Specification
**Agent**: #03 — Core Systems Programmer  
**Cycle**: PROD_CYCLE_AUTO_20260703_006  
**Hub Coordinates**: X=2100, Y=2400 (Cretaceous clearing)

---

## NavMesh Bounds Volume

| Property | Value |
|---|---|
| Actor Label | `NavMeshBounds_Hub_001` |
| Location | X=2100, Y=2400, Z=200 |
| Scale | 30×30×8 (covers 3000×3000×800 cm) |
| Rebuild Command | `RebuildNavigation` (console) |

### Coverage
- Covers full hub clearing (radius ~1500cm from center)
- Extends 800cm vertically to handle terrain variation
- Sufficient for Raptor patrol paths at waypoints 001/002/003

---

## Raptor Patrol Waypoints

Placed in previous cycle (AUTO_006), triangle formation at 500cm radius:

| Label | X | Y | Z | Purpose |
|---|---|---|---|---|
| Waypoint_Raptor_001 | 2600 | 2400 | 50 | East patrol point |
| Waypoint_Raptor_002 | 1850 | 1967 | 50 | NW patrol point |
| Waypoint_Raptor_003 | 1850 | 2833 | 50 | SW patrol point |

### Patrol AI Wiring (Blueprint)
To wire patrol AI without C++ recompilation:
1. Open Raptor Blueprint (BP_Raptor or DinosaurBase child)
2. In Event Graph: `Event BeginPlay` → `Move To Actor` (Waypoint_Raptor_001)
3. On `Move Completed` → cycle to next waypoint via array index
4. Use `AIMoveTo` node with `Acceptance Radius = 100cm`

---

## Mesh Replacement Status

| Actor Type | Original Mesh | Target Mesh | Status |
|---|---|---|---|
| Rock_* actors | BasicShape/Sphere | SM_Rock (Starter Content) | ✅ Attempted |
| Tree_* actors | BasicShape/Cylinder | SM_Bush (Starter Content) | ✅ Attempted |
| Ground patches | BasicShape/Cube | M_Ground_Grass material | ✅ Applied |

### Starter Content Paths
```
/Game/StarterContent/Props/SM_Rock.SM_Rock
/Game/StarterContent/Props/SM_Bush.SM_Bush
/Game/StarterContent/Materials/M_Ground_Grass.M_Ground_Grass
```

---

## Hub Composition Summary (Cycle 006)

### Lighting Stack (CAP-enforced)
| Component | Value |
|---|---|
| Sun pitch | -35° (golden hour) |
| Sun yaw | -60° |
| Sun intensity | 8.0 |
| Fog density | 0.02 |
| SkyLight intensity | 1.0 |
| FastSkyLUT | 1 |
| VolumetricFog | 1 |

### Actor Inventory
- **Dinosaurs**: TRex_Savana_001, Raptor_x3, Brachiosaurus_001 (from earlier cycles)
- **Vegetation**: Tree_Hub_001-005 + existing trees/bushes
- **Waypoints**: Waypoint_Raptor_001/002/003
- **NavMesh**: NavMeshBounds_Hub_001
- **PlayerStart**: Within 600cm of hub center

---

## Next Steps for Agent #04 (Performance Optimizer)

1. **LOD setup** — Add LOD1/LOD2 to all static mesh actors in hub (target: 60fps at hub)
2. **Cull distance** — Set max draw distance on vegetation: 3000cm
3. **NavMesh tile size** — Optimize tile size for hub area (default 1000cm, reduce to 500cm)
4. **Shadow cascade** — Limit dynamic shadow cascades to 3 for hub clearing

---

## Known Limitations

- C++ DinosaurBase properties (health, territory radius, aggro range) cannot be modified via Python without recompilation — use Blueprint variables instead
- Raptor patrol requires Blueprint wiring (BTTask_MoveTo) — cannot be done via Python API alone
- Mesh replacement success depends on Starter Content being present in project
