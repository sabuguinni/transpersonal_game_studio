# Crowd & Traffic Simulation System — Agent #13
## PROD_CYCLE_AUTO_20260618_012

---

## Overview
This document defines the crowd simulation design for the prehistoric survival game. All crowd behavior is driven by predator proximity, territory ownership, and migration patterns. No spiritual or mystical content.

---

## Crowd Zones Deployed in MinPlayableMap

### Herbivore Herds (Safe Zones)
| Label | Location | Species | Notes |
|-------|----------|---------|-------|
| Herd_Parasaur_West_001/002/003 | (-3500,-1000) | Parasaurolophus | Western grassland — safe from TRex |
| Herd_Stego_South_001/002 | (0,-3500) | Stegosaurus | Southern plains — far from all predators |
| Herd_Trike_North_001/002 | (1500,3500) | Triceratops | Northern safe zone — above TRex patrol |
| Herd_Brachio_River_001 | (-1500,2000) | Brachiosaurus | Near river — avoids both predator territories |

**Visual:** Green point lights (intensity 600-1000) mark herd positions.

---

## Predator Exclusion Zones

### TRex Patrol Corridor
- **Bounds:** X 3000–4200, Y -500 to 1600
- **Effect:** Zero herbivore herds placed inside this corridor
- **Panic radius:** 2000u — herbivores within this range scatter on TRex charge
- **Visual:** `CrowdPanic_TRex_Zone_001` — deep red light (2000 intensity, 7500 attenuation)

### Raptor Territory
- **Center:** (-2800, 1500)
- **Radius:** 1500u
- **Effect:** Reduced herbivore density — migration routes avoid this zone
- **Visual:** `CrowdDensity_Raptor_Exclusion_001` — purple light (1500 intensity, 4500 attenuation)

---

## Crowd Panic System

### TRex Charge Panic
When TRex enters charge state (detected by proximity or audio cue):
1. All herbivores within 2000u enter PANIC state
2. Each herbivore selects nearest `Panic_Scatter_WP_*` that is:
   - Away from TRex (dot product check)
   - Not inside Raptor territory
   - Not blocked by terrain
3. Herbivores sprint at 1.5× normal speed for 8 seconds
4. After 8s, transition to ALERT state (elevated vigilance, no grazing)
5. After 30s with no TRex detection, return to GRAZE state

### Scatter Waypoints
8 waypoints placed at 2000u radius from TRex patrol center (3600, 550):
- `Panic_Scatter_WP_001` through `Panic_Scatter_WP_008`
- Angles: 0°, 45°, 90°, 135°, 180°, 225°, 270°, 315°
- Raptor-zone waypoints automatically skipped during placement
- **Visual:** Yellow-orange lights (400 intensity)

---

## Migration Corridors

### Western Herd → Northern Safe Zone
Route avoids both TRex corridor and Raptor territory:
```
(-3500,-500) → (-2000,500) → (-1500,1500) → (500,2800) → (1000,3200)
```
Waypoints: `Migration_WP_West_001`, `Migration_WP_CenterW_001/002`, `Migration_WP_North_001/002`
**Visual:** Blue lights (500 intensity, 1200 attenuation)

**Migration Trigger:** Seasonal (every 20 min real-time) OR when herd density exceeds threshold in current zone.

---

## Crowd Density Rules

| Zone | Density | Reason |
|------|---------|--------|
| TRex Corridor (3000-4200, -500 to 1600) | 0% | Active apex predator patrol |
| Raptor Territory (-2800, 1500 ± 1500u) | 10% | Pack predator exclusion |
| Western Grassland (-4000 to -1000) | 80% | Safe, open terrain |
| Southern Plains (Y < -2000) | 70% | Distance from predators |
| Northern Zone (Y > 2500) | 60% | Moderate safety |
| Near Water/River | 90% | Drinking requirement |

---

## Behavioral States (Per Herbivore)

```
GRAZE → ALERT → PANIC → FLEE → RECOVER → GRAZE
         ↑                        ↓
         └────────────────────────┘
```

| State | Speed | Duration | Trigger |
|-------|-------|----------|---------|
| GRAZE | 0 (stationary) | Until threat | Default state |
| ALERT | 0.5× | Until threat confirmed/cleared | Predator within 3000u |
| PANIC | 1.5× | 8s | TRex charge / Raptor sprint |
| FLEE | 1.2× | Until safe zone reached | Sustained threat |
| RECOVER | 0.3× | 30s | Threat cleared |

---

## Integration with Combat AI (Agent #12)

### Query Labels for Runtime Logic
```python
# Get all combat zones
combat_zones = [a for a in actors if a.get_actor_label().startswith('CombatZone_')]
# Get escape routes
escape_routes = [a for a in actors if a.get_actor_label().startswith('CombatEscape_')]
# Get TRex patrol waypoints
patrol_wps = [a for a in actors if a.get_actor_label().startswith('TRex_Patrol_WP_')]
```

### Crowd Response to Combat Events
- `CombatZone_TRex_ChargeRadius_001` active → all `Herd_*` actors within 4000u enter PANIC
- `CombatZone_RaptorAmbush_Jungle_001` active → `Herd_*` actors avoid Y 0-3000, X -4300 to -1300
- `CombatEscape_TRex_Route_A/B` → valid flee destinations for herbivores (green = safe)

---

## Actor Color Legend (MinPlayableMap)

| Color | Meaning |
|-------|---------|
| 🟢 Green | Herbivore herd / safe zone |
| 🔵 Blue | Migration waypoint |
| 🟡 Yellow-Orange | Panic scatter waypoint |
| 🔴 Deep Red | TRex danger zone / panic zone |
| 🟣 Purple | Raptor exclusion / low density zone |

---

## Handoff to Agent #14 — Quest & Mission Designer

### Quest Hooks Available
1. **"Follow the Migration"** — Player follows `Migration_WP_*` chain to discover new biome
2. **"Protect the Herd"** — TRex enters `Herd_Parasaur_West_001` zone → player must distract it
3. **"Raptor Territory Crossing"** — Player must cross `CrowdDensity_Raptor_Exclusion_001` zone safely
4. **"Panic Response"** — When `CrowdPanic_TRex_Zone_001` activates, player has 8s to reach safety

### Key Actor Labels for Quest Triggers
- `CrowdPanic_TRex_Zone_001` — TRex danger zone (trigger quest on enter)
- `Herd_Parasaur_West_001` — Primary herd (protect quest anchor)
- `Migration_WP_North_001` — Migration destination (discovery quest)
- `CrowdDensity_Raptor_Exclusion_001` — Raptor zone (stealth quest)

---

*Generated by Agent #13 — Crowd & Traffic Simulation*  
*Cycle: PROD_CYCLE_AUTO_20260618_012*
