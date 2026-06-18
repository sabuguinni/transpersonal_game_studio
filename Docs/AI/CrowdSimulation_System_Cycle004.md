# Crowd & Traffic Simulation System — Agent #13 Production Cycle 004

## Overview
This document describes the crowd simulation infrastructure placed in MinPlayableMap during cycle 004.
All actors are TargetPoints and PointLights — visible, functional, and ready for Blueprint/Mass AI integration.

---

## Actors Placed This Cycle

### 1. Crowd Scatter System (12 actors)
**Trigger**: `CombatZone_Camp_DinoThreat` (Agent #12) activates scatter behavior

| Label | Type | Purpose |
|-------|------|---------|
| `CrowdScatter_Escape_01–08` | TargetPoint | 8 radial escape waypoints (1800u radius from camp) |
| `CrowdAvoid_TRex_01–04` | TargetPoint | Boundary markers for TRex forbidden zone (2500u radius) |

**Behavior Logic**:
- When threat detected: crowd agents pick nearest `CrowdScatter_Escape_*` waypoint NOT in TRex zone
- Agents move at 150% normal speed during scatter
- Scatter lasts 45 seconds, then agents return to nearest safe zone

---

### 2. Behavioral Zone Lights (12 actors)

| Label | Color | Meaning | Speed Modifier |
|-------|-------|---------|----------------|
| `CrowdSafe_Riverbank_001` | 🟢 GREEN | Safe gathering zone | 100% speed |
| `CrowdSafe_Forest_Edge_001` | 🟢 GREEN | Safe gathering zone | 100% speed |
| `CrowdSafe_Hilltop_001` | 🟢 GREEN | Safe gathering zone | 100% speed |
| `CrowdCautious_RaptorZone_001–003` | 🟠 ORANGE | Raptor territory — cautious | 50% speed |
| `CrowdClear_Ankyl_001–003` | 🔵 CYAN | Ankylosaurus clearance (500u buffer) | Detour required |
| `CrowdEscape_River_Corridor_001–003` | 💙 BLUE | Emergency escape corridor to river | 200% speed |

---

### 3. Crowd Flow Network (17 actors)

**Morning Migration Route** (Camp → Foraging, 5 waypoints):
```
CrowdFlow_Morning_001 (0,0) → 002 (400,-200) → 003 (700,-400) → 004 (1000,-300) → 005 (1200,0)
```

**Evening Return Route** (Foraging → Camp, 5 waypoints):
```
CrowdFlow_Evening_001 (1200,0) → 002 (900,300) → 003 (600,200) → 004 (300,100) → 005 (0,0)
```

**Water Gathering Route** (Camp → River, 4 waypoints):
```
CrowdFlow_Water_001 (-100,200) → 002 (-300,500) → 003 (-500,900) → 004 (-700,1300)
```

**Density Markers**:
- `CrowdDensity_Camp_High` — 🟡 YELLOW, 800 intensity (most humans gather here at night)
- `CrowdDensity_Hunting_Medium` — 🟡 dim YELLOW (daytime hunting parties)
- `CrowdDensity_River_Low` — 💙 BLUE (small water-gathering groups)

---

## Crowd Simulation Design Rules

### Daily Schedule (24-hour cycle)
| Time | Behavior | Route Used |
|------|----------|-----------|
| Dawn (05:00–08:00) | Morning migration begins | `CrowdFlow_Morning_*` |
| Day (08:00–17:00) | Foraging + hunting | Density points at hunting ground |
| Dusk (17:00–20:00) | Evening return | `CrowdFlow_Evening_*` |
| Night (20:00–05:00) | Camp gathering | `CrowdDensity_Camp_High` |
| Any time | Water gathering (small groups) | `CrowdFlow_Water_*` |

### Threat Response Protocol
1. **T-Rex detected** (enters `CombatZone_TRex_AggroDetect`):
   - ALL crowd agents immediately scatter via `CrowdScatter_Escape_*`
   - Avoid `CrowdAvoid_TRex_*` boundary — no crowd agent enters this zone
   - Emergency river escape via `CrowdEscape_River_Corridor_*`

2. **Raptor pack detected** (enters `CombatZone_RaptorPack_*`):
   - Crowd agents in `CrowdCautious_RaptorZone_*` reduce speed to 50%
   - Agents form defensive clusters (groups of 3–5)
   - No solo movement in orange zone

3. **Ankylosaurus patrol**:
   - All crowd agents maintain 500-unit clearance from `Patrol_Ankyl_*`
   - Reroute via `CrowdClear_Ankyl_*` boundary markers
   - Tail-swing radius = 400 units (lethal to crowd agents)

---

## Integration with Other Agents

### From Agent #12 (Combat AI)
- Uses: `CombatZone_Camp_DinoThreat`, `CombatZone_TRex_AggroDetect`, `CombatZone_TRex_ChargeTrigger`
- Uses: `Raptor_Zone_Marker_*`, `TRex_Territory_Marker_*`, `Patrol_Ankyl_*`
- Uses: `CombatRetreat_River_*` as emergency escape waypoints

### From Agent #11 (NPC Behavior)
- Camp NPCs (hunters, gatherers) follow crowd flow routes
- NPC daily routines align with crowd schedule above

### For Agent #14 (Quest Designer)
- Quest triggers can use crowd density to measure "social" state
- `CrowdDensity_Camp_High` = safe zone for quest handoffs
- Crowd scatter = environmental storytelling (player sees humans fleeing)
- Water route = patrol quest opportunity (protect water gatherers)

---

## Performance Notes
- All actors are lightweight TargetPoints or PointLights
- No Mass AI entities spawned yet (requires UE5 Mass framework setup)
- Current implementation: waypoint network ready for Blueprint crowd controller
- Estimated crowd agent count at runtime: 15–40 agents (performance budget)
- LOD: agents beyond 3000 units switch to billboard representation

---

## Next Steps for Agent #14 (Quest Designer)
1. Use `CrowdDensity_Camp_High` as quest hub location
2. Create "Protect the Water Gatherers" quest using `CrowdFlow_Water_*` route
3. "Escort to Safety" quest triggers when crowd scatter activates
4. NPC quest givers should be placed at `CrowdSafe_*` green zone locations
5. Quest failure condition: crowd agent killed by dinosaur in `CrowdCautious_RaptorZone_*`
