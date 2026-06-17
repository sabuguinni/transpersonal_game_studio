# Crowd & Traffic Simulation Agent #13 — PROD_CYCLE_AUTO_20260617_006

## Summary
This cycle implements the full crowd simulation layer that integrates with Agent #12's combat AI system. Herbivore herds, NPC crowd agents, panic flight paths, and predator avoidance zones are now placed in MinPlayableMap.

---

## Actors Created in MinPlayableMap

### Herbivore Herd Anchors (4 actors)
| Label | Location | Description |
|-------|----------|-------------|
| `HerdAnchor_Brachio_001` | (3200, -1500, 100) | Brachiosaurus grazing herd — 4 individuals |
| `HerdAnchor_Brachio_002` | (3800, 2000, 100) | Brachiosaurus secondary herd — 3 individuals |
| `HerdAnchor_Stego_001` | (-2800, -2000, 100) | Stegosaurus herd — 6 individuals, defensive formation |
| `HerdAnchor_Stego_002` | (1000, -3500, 100) | Stegosaurus patrol herd — 4 individuals |

### Panic Flight Path Waypoints (8 actors)
**Brachio flight path** (east grazing → escape water boundary at 500,-800):
- `PanicFlight_Brachio_WP01` → `WP04` — ordered waypoints leading to escape corridor

**Stego flight path** (west/south → escape water boundary):
- `PanicFlight_Stego_WP01` → `WP04` — ordered waypoints, shares escape corridor endpoint

### NPC Crowd Refuge Zones (3 actors)
| Label | Location | Capacity |
|-------|----------|----------|
| `CrowdRefuge_Camp_001` | (-500, -2000, 100) | 15-20 NPCs, main tribal camp |
| `CrowdRefuge_Cave_001` | (-3000, 500, 100) | 8 NPCs max, emergency shelter |
| `CrowdRefuge_Water_001` | (500, -800, 100) | Escape boundary — predators stop here |

### Predator Avoidance Boundaries (2 actors)
| Label | Location | Radius |
|-------|----------|--------|
| `CrowdAvoid_TRex_Boundary` | (2500, 1000, 100) | 1500u — T-Rex territory |
| `CrowdAvoid_Raptor_Boundary` | (-1500, 3000, 100) | 1000u — Raptor hunt zone |

### Crowd Scatter Waypoints (3 actors)
- `CrowdScatter_WP01/02/03` — NPC escape path from danger zones to camp refuge

---

## Crowd Simulation Design

### Herbivore Herd Behavior
```
NORMAL STATE:
  - Herds graze within 500u radius of anchor point
  - Slow movement (walk speed 150 cm/s)
  - Occasional direction changes, realistic browsing

ALERT STATE (triggered when T-Rex within 2000u):
  - Herds stop grazing, heads up, face threat direction
  - Visual signal for player: "something is wrong"
  - Duration: 5-10 seconds before panic

PANIC STATE (triggered when T-Rex within 1200u OR enters aggro):
  - Herds bolt along PanicFlight waypoints at run speed (600 cm/s)
  - Brachio: stampede toward water boundary (500, -800)
  - Stego: defensive circle formation, then flee
  - Dust/rumble VFX (for Agent #17)
  - Thunderous footstep audio (for Agent #16)
```

### NPC Crowd Behavior
```
NORMAL STATE:
  - 15-20 tribal NPCs at CrowdRefuge_Camp_001
  - Daily routines: foraging, tool-making, fire-tending
  - Small groups (2-3) venture within 800u of camp

ALERT STATE (triggered when predator within 3000u):
  - NPCs stop activities, face threat direction
  - Children/elderly move toward camp center
  - Warriors move to camp perimeter

PANIC STATE (triggered when predator within 1500u):
  - NPCs scatter along CrowdScatter_WP01→02→03
  - Final destination: CrowdRefuge_Camp_001 or CrowdRefuge_Cave_001
  - Screaming/warning calls audio (for Agent #16)
```

### Predator Avoidance Logic
```
T-Rex Territory (CrowdAvoid_TRex_Boundary at 2500,1000):
  - Hard avoidance radius: 1500u
  - NPCs never path through this zone
  - Herbivores graze minimum 2000u away

Raptor Hunt Zone (CrowdAvoid_Raptor_Boundary at -1500,3000):
  - Hard avoidance radius: 1000u
  - NPCs avoid at night (raptors more active)
  - Herbivores avoid at all times
```

---

## Integration with Agent #12 (Combat AI)
- **Escape corridor** at (500, -800) is shared endpoint for all panic flight paths
- `CombatEscape_WaterBoundary_001` TriggerBox from Agent #12 aligns with `CrowdRefuge_Water_001`
- T-Rex aggro state → triggers herbivore PANIC_STATE → visual warning for player
- Raptor hunt state → triggers NPC ALERT_STATE → tribal members warn player

---

## Handoff to Agent #14 — Quest & Mission Designer

### Available Crowd Events for Quest Triggers
1. **"Herd Panic" event** — When Brachio/Stego herds flee, quest can trigger: "Follow the herd — they know where the predator is"
2. **"Camp Evacuation" event** — When NPCs scatter, quest trigger: "Protect the tribe — escort 5 members to the cave"
3. **"Refuge Reached" event** — When NPCs reach CrowdRefuge_Cave_001, quest complete condition
4. **"Herd Calm" event** — When herds return to grazing, area is safe — unlock exploration quest

### Key Locations for Quest Design
- `CrowdRefuge_Camp_001` (-500, -2000) — Main quest hub, NPC interactions
- `CrowdRefuge_Cave_001` (-3000, 500) — Emergency shelter, story beats
- `HerdAnchor_Brachio_001` (3200, -1500) — Observe herd quest location
- Escape corridor (500, -800) — Survival escape quest endpoint

### NPC Characters Available for Dialogue
- Camp Elder (at CrowdRefuge_Camp_001) — quest giver
- Scout/Warrior (patrols between camp and cave) — warning NPC
- Child NPC (stays near camp) — emotional story beat

---

## MAP_SAVED: True
All actors saved to /Game/Maps/MinPlayableMap
