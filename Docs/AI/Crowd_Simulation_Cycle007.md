# Crowd & Traffic Simulation — Cycle 007
**Agent #13 — Crowd & Traffic Simulation**
**Cycle:** PROD_CYCLE_AUTO_20260618_007

---

## Systems Deployed This Cycle

### 1. Prey Animal Herd System
Two distinct herbivore groups placed in ecologically appropriate biomes:

#### Triceratops Plains Herd (6 members)
- **Location:** Open plains (~3200–3600, 1800–2400)
- **Labels:** `Trike_Herd_Plains_001` through `Trike_Herd_Plains_006`
- **Behaviour intent:** Slow grazing movement, tight formation, alpha-led
- **Scale:** 1.8×2.8×1.4 (bulky herbivore silhouette)
- **Threat response:** Stampede north-east via Plains escape corridor

#### Parasaurolophus River Flock (8 members)
- **Location:** River/marsh zone (~-800 to -1500, 3500–4200)
- **Labels:** `Para_Flock_River_001` through `Para_Flock_River_008`
- **Behaviour intent:** Loose flock, frequent direction changes, vocal alarm calls
- **Scale:** 1.2×2.2×1.6 (tall bipedal silhouette)
- **Threat response:** Stampede upstream via River escape corridor

---

### 2. Stampede Trigger Zone System (4 zones)
Invisible detection perimeters that activate herd panic behaviour:

| Label | Biome | Position | Purpose |
|-------|-------|----------|---------|
| `StampedeZone_Plains_001` | Plains | (3300, 2100, 150) | Trike herd perimeter |
| `StampedeZone_River_001` | River | (-1150, 3850, 150) | Para flock perimeter |
| `StampedeZone_Forest_001` | Forest | (500, -2000, 150) | Forest grazer zone |
| `StampedeZone_Marsh_001` | Marsh | (-2500, 500, 150) | Marsh browser zone |

Visualised as flat spheres (scale 8×8×0.3) for editor visibility.

---

### 3. Escape Corridor Waypoint System (8 waypoints)
Directional flee paths that herds follow during stampede events:

#### Plains Escape Route (Trikes flee north-east)
- `EscapeWP_Plains_Escape_A` → (3800, 2800)
- `EscapeWP_Plains_Escape_B` → (4400, 3500)
- `EscapeWP_Plains_Escape_C` → (5200, 4200)

#### River Escape Route (Paras flee upstream)
- `EscapeWP_River_Escape_A` → (-500, 4800)
- `EscapeWP_River_Escape_B` → (200, 5600)
- `EscapeWP_River_Escape_C` → (900, 6400)

#### Forest Escape Route
- `EscapeWP_Forest_Escape_A` → (-200, -2800)
- `EscapeWP_Forest_Escape_B` → (-800, -3600)

Visualised as cones (scale 0.6×0.6×1.2) pointing in flee direction.

---

## Cumulative Crowd System State (Cycles 004–007)

| System | Actors | Status |
|--------|--------|--------|
| Crowd Scatter Points | ~12 | ✅ Deployed (Cycle 004) |
| Migration Corridor | ~14 | ✅ Deployed (Cycle 005) |
| Combat Reaction Zones | ~8 | ✅ Deployed (Cycle 006) |
| Prey Herd Members | 14 | ✅ Deployed (Cycle 007) |
| Stampede Zones | 4 | ✅ Deployed (Cycle 007) |
| Escape Waypoints | 8 | ✅ Deployed (Cycle 007) |
| **TOTAL CROWD ACTORS** | **~60** | ✅ Well within CAP |

---

## Crowd Simulation Design Principles

### Emergent Behaviour Through Density
A herd of 6 Triceratops grazing on open plains creates the feeling of a living ecosystem without requiring complex AI. The player observes:
1. **Normal state:** Slow drift, occasional direction changes, spread formation
2. **Alert state:** Heads up, tighter formation, slower movement
3. **Panic state:** Full stampede along escape corridor, dust/sound cues

### Predator-Prey Spatial Logic
- Predators (TRex, Raptors) are placed 800–1500m from herds
- Stampede zones create a buffer — herds react before predator arrives
- Escape corridors lead away from predator spawn zones

### Player Interaction Opportunities
- Player can **trigger stampedes** by running into herd perimeter
- Player can **use stampedes** to distract predators (herd draws TRex away)
- Player can **follow herds** to find water sources (Paras near river)
- Player can **hunt stragglers** from herd edges for food

---

## Integration Notes for Agent #14 (Quest & Mission Designer)

### Quest Hooks Available
1. **"Follow the Herd"** — Track Parasaurolophus flock upstream to find water source
2. **"The Stampede"** — Trigger a Triceratops stampede to escape a TRex
3. **"Lone Straggler"** — Hunt a separated herd member for food/materials
4. **"Predator Distraction"** — Lead a predator into a herd's stampede zone

### Waypoint Data for Quest System
All escape waypoints use label format `EscapeWP_[Biome]_Escape_[A/B/C]` — queryable via:
```python
escape_wps = [a for a in actors if 'EscapeWP' in a.get_actor_label()]
```

---

## Next Cycle Recommendations

### Agent #14 (Quest & Mission Designer) should:
1. Use `StampedeZone_*` actors as quest trigger volumes
2. Use `EscapeWP_*` actors as quest objective markers
3. Create a "survive the stampede" mission using the Plains escape corridor
4. Reference herd positions for "find food" survival quests

### Future Crowd Expansion (Cycle 008+)
- Add Pterosaur flock overhead (aerial crowd layer)
- Add small mammal/lizard scatter (ground-level micro-fauna)
- Add nighttime behaviour change (herds move to shelter)
- Add seasonal migration (herds shift position over time)
