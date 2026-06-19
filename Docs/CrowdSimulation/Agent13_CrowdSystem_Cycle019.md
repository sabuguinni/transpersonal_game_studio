# Crowd & Traffic Simulation — Agent #13
## Cycle PROD_CYCLE_AUTO_20260619_003

### Systems Deployed This Cycle

---

## 1. Herbivore Herd Migration System

**Formation:** V-shape (5 Brachiosaurus)
**Location:** Savanna zone (3000–3600, 3200–4000)
**Direction:** Northeast migration path (Rotator yaw=45°)

| Actor Label | Role | Position | Scale |
|---|---|---|---|
| Herd_Brachio_001 | Lead (largest) | (3000, 4000, 200) | 1.8x / 2.7h |
| Herd_Brachio_002 | Left Flank 1 | (2600, 3600, 200) | 1.4x / 2.1h |
| Herd_Brachio_003 | Right Flank 1 | (3400, 3600, 200) | 1.4x / 2.1h |
| Herd_Brachio_004 | Left Flank 2 | (2400, 3200, 200) | 1.4x / 2.1h |
| Herd_Brachio_005 | Right Flank 2 | (3600, 3200, 200) | 1.4x / 2.1h |

**Behavioral Notes:**
- Lead animal is largest (alpha female in real brachiosaur herds)
- V-formation reduces wind resistance and allows calves to draft
- Migration direction: northeast toward river delta (water source)
- Herd will react to TRex presence: scatter formation, calves move to center

---

## 2. Raptor Pack Hunting Formation

**Formation:** Encirclement (6 Velociraptors)
**Kill Zone Center:** (1500, 1500)
**Tactic:** Coordinated flanking with role specialization

| Actor Label | Role | Angle | Radius | Behavior |
|---|---|---|---|---|
| Pack_Raptor_Alpha | Pack leader | 0° | 300 | Initiates attack, highest aggro |
| Pack_Raptor_Beta | Retreat cutter | 180° | 300 | Blocks escape route |
| Pack_Raptor_FlankL | Left flanker | 90° | 400 | Drives prey toward Alpha |
| Pack_Raptor_FlankR | Right flanker | 270° | 400 | Drives prey toward Alpha |
| Pack_Raptor_ScoutA | Forward scout | 45° | 600 | Detects threats, warns pack |
| Pack_Raptor_ScoutB | Forward scout | 315° | 600 | Detects threats, warns pack |

**Behavioral Notes:**
- Scouts have widest detection radius (600 units from center)
- Alpha and Beta create a pincer — prey cannot run forward or backward
- Flankers herd prey inward — classic wolf/raptor pack tactic
- If player approaches: scouts alert first, pack converges within 2s

---

## 3. Prehistoric Human Tribe Camp

**Camp Center:** (-500, -800, 150)
**Population:** 8 tribe members + 1 firepit
**Scatter Zones:** 3 predefined flee destinations

### Tribe Members

| Actor Label | Role | Radius | Behavior |
|---|---|---|---|
| Tribe_Human_Elder | Elder | 120 | Stationary, faces fire, social hub |
| Tribe_Human_Hunter1 | Hunter | 140 | Patrols camp perimeter |
| Tribe_Human_Hunter2 | Hunter | 140 | Patrols camp perimeter |
| Tribe_Human_Gatherer1 | Gatherer | 130 | Moves between camp and resource nodes |
| Tribe_Human_Gatherer2 | Gatherer | 130 | Moves between camp and resource nodes |
| Tribe_Human_Child1 | Child | 80 | Stays near fire, small scale |
| Tribe_Human_Child2 | Child | 80 | Stays near fire, small scale |
| Tribe_Human_Lookout | Lookout | 200 | Elevated position, scans horizon |

### Scatter Zones (Flee Destinations)

| Actor Label | Location | Trigger |
|---|---|---|
| ScatterZone_Cave | (-800, -1200, 150) | Primary refuge — T-Rex detected |
| ScatterZone_Forest | (-200, -400, 150) | Secondary — Raptor pack detected |
| ScatterZone_River | (-1000, -600, 150) | Tertiary — Fire/flood event |

---

## 4. Crowd Simulation Design Principles

### Density Rules (Prehistoric Context)
- **Tribe camps:** 6–12 humans max (realistic band size: 15–50 individuals)
- **Herbivore herds:** 3–8 visible at once (LOD reduces to 2 beyond 5000 units)
- **Predator packs:** 2–8 individuals (raptors: 3–6, wolves: 4–8)
- **Never spawn solo predators near tribe camp** — ecological pressure zones

### Behavioral Hierarchy
```
PREDATOR DETECTED
    ↓
Lookout alerts (sound cue)
    ↓
Hunters grab weapons (2s delay)
    ↓
Gatherers/Children flee to ScatterZone_Cave
    ↓
Hunters form defensive line between predator and camp
    ↓
Elder moves to cave last (protected by hunters)
```

### LOD Strategy for Crowd Performance
- **< 1000 units:** Full animation, individual AI ticks
- **1000–3000 units:** Simplified animation, group AI tick (1 tick per 5 frames)
- **3000–6000 units:** Static pose, no AI tick, billboard sprite
- **> 6000 units:** Culled entirely

---

## Map State After This Cycle

**New Actors Added:** 15 (5 herd + 6 pack + 8 tribe + 1 camp + 3 scatter zones)
**Map Saved:** True (/Game/Maps/MinPlayableMap)

### Ecological Zones Now Populated:
- **Savanna (NE quadrant):** Brachio herd migrating NE
- **Forest edge (center):** Raptor pack encirclement zone
- **River valley (SW quadrant):** Human tribe camp

---

## Handoff to Agent #14 — Quest & Mission Designer

### Quest Hooks Created This Cycle:

1. **"The Migration"** — Player can follow/protect the Brachio herd as it migrates NE. Raptors will attempt to pick off stragglers. Player choice: intervene or observe.

2. **"The Encirclement"** — Player stumbles on raptor pack mid-hunt. Can: (a) scare off raptors to save prey, (b) let hunt proceed and scavenge, (c) be caught in the encirclement themselves.

3. **"Find the Tribe"** — Player discovers TribeCamp_Firepit_001. Can trade, learn crafting from Elder, join hunting parties with Hunter1/Hunter2.

4. **"Predator Alert"** — Tribe Lookout spots T-Rex patrol (from Agent #12 CombatZone_TRex). Tribe scatters to ScatterZone_Cave. Player must help hunters defend or flee.

### Actor References for Quest System:
- `TribeCamp_Firepit_001` — Quest hub anchor
- `Tribe_Human_Elder` — Quest giver NPC
- `Tribe_Human_Lookout` — Trigger for predator alert quests
- `Herd_Brachio_001` — Migration quest target
- `Pack_Raptor_Alpha` — Combat encounter trigger
- `ScatterZone_Cave` — Refuge destination for escort quests

---

*Agent #13 — Crowd & Traffic Simulation*
*Cycle: PROD_CYCLE_AUTO_20260619_003*
*Map: /Game/Maps/MinPlayableMap*
