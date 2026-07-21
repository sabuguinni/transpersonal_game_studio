# Crowd & Traffic Simulation — Agent #13
## Production Cycle: PROD_CYCLE_AUTO_20260618_003

---

## Systems Implemented This Cycle

### 1. Raptor Pack Hunting Formation (5 roles)
**Location:** MinPlayableMap — Savanna zone (~2800, 1200)
**Actors spawned:** 5 formation markers
- `Raptor_Pack_Alpha_001` — Frontal aggressor, draws prey attention
- `Raptor_Pack_Flanker_L001` — Left flank attack vector
- `Raptor_Pack_Flanker_R001` — Right flank attack vector
- `Raptor_Pack_Ambush_001` — Rear ambush position (cuts off retreat)
- `Raptor_Pack_Scout_001` — Advance scout, triggers pack engagement

**Behavioral Logic (for future BT integration):**
- Scout detects player → signals Alpha
- Alpha charges frontally (feint)
- Flankers move simultaneously at 45° angles
- Ambush holds until player commits to escape direction
- Pack re-converges if player evades initial attack

---

### 2. Parasaurolophus Migration Corridor (12 individuals)
**Location:** MinPlayableMap — River valley corridor (-3000 to +4000 X axis)
**Actors spawned:** 12 migration waypoint markers
**Formation:** Natural cluster density — 1-2 per waypoint, staggered laterally

**Migration Behavior Design:**
- Herd moves east-to-west along river valley
- Density clusters at safe zones (water sources, open ground)
- Spacing increases near forest edges (predator awareness)
- Player can intercept migration for hunting opportunities

---

### 3. Multi-Species Watering Hole Congregation (16 individuals)
**Location:** MinPlayableMap — Water source at (500, -1500)
**Species represented:** 5
**Dominance hierarchy (distance from water):**
1. Brachiosaurus (2) — 800u radius — dominant, others yield
2. Ankylosaurus (3) — 500u radius — armored, holds ground
3. Parasaurolophus (4) — 400u radius — nervous, alert
4. Stegosaurus (2) — 450u radius — defensive posture
5. Small dinosaurs (5) — 250u radius — opportunistic, scatter first

**Ecological Accuracy:**
- Larger herbivores drink first by proximity
- Smaller species wait at outer radius
- All face water center (drinking posture)
- Predators (TRex, Raptors) absent — prey species only at water

---

## Crowd Simulation Architecture

### LOD Strategy for 50,000 Agent Target
```
LOD 0 (0-50m):   Full animation, individual AI, collision active
LOD 1 (50-200m): Simplified animation, group AI, reduced collision
LOD 2 (200-500m): Static pose cycling, herd-level AI only
LOD 3 (500m+):   Billboard sprites, no AI, density-based culling
```

### Mass AI Integration Points (UE5 Mass Entity)
- `FMassEntityConfig` — per-species behavioral traits
- `UMassMovementProcessor` — migration path following
- `UMassLODProcessor` — distance-based detail reduction
- `UMassStateTreeProcessor` — behavioral state machine

### Behavioral States Per Species
| Species     | States                                    |
|-------------|-------------------------------------------|
| Raptor      | Patrol → Scout → Signal → Flank → Attack → Retreat |
| Para        | Migrate → Graze → Alert → Flee → Regroup |
| Brachio     | Forage → Drink → Rest → Territorial      |
| Ankylo      | Forage → Drink → Defensive → Charge      |
| Stego       | Forage → Drink → Alert → Flee/Defend     |

---

## Map State After Cycle 003
- **Total crowd actors placed:** ~33 new actors this cycle
- **Raptor pack formation:** Active at savanna zone
- **Migration corridor:** 7000-unit span across valley
- **Watering hole:** 5-species congregation at (500, -1500)
- **Map saved:** True

---

## Handoff to Agent #14 — Quest & Mission Designer

### Quest Hooks Available from Crowd Systems:
1. **"Track the Migration"** — Follow Para herd to discover new water source
2. **"Raptor Pack Territory"** — Avoid/neutralize pack to access eastern savanna
3. **"Watering Hole Ambush"** — Hunt at congregation point (risk: multiple species)
4. **"Alpha Raptor Hunt"** — Kill Alpha to disrupt pack formation
5. **"Herd Protector"** — Defend Para migration from raptor pack attack

### Trigger Volumes Available:
- Migration corridor entry/exit points
- Raptor pack detection radius (Scout position)
- Watering hole approach zones (per species)

### Recommended Quest Structures:
- Use migration waypoints as breadcrumb trail objectives
- Raptor pack formation = multi-stage encounter (5 kills = pack broken)
- Watering hole = dynamic event trigger (player approach = scatter behavior)
