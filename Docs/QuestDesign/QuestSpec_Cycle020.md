# Quest System Specification — Cycle 020
## Agent #14 — Quest & Mission Designer

---

## Overview

Three interlocking quests anchored to the Cretaceous hub clearing at world coordinates X=2100, Y=2400.
All quests use existing dinosaur actors placed by Agent #13 (Crowd Simulation) in Cycle 019.

---

## Quest 1: "First Encounter" (Tutorial)

**Type:** Tutorial / Introduction  
**Quest Giver:** Elder NPC (`NPC_Elder_QuestGiver_001`) at (2050, 2350, 150)  
**Trigger Zone:** `QuestZone_FirstEncounter_001` at (2100, 2400) — radius 500u  
**Objective Marker:** `QuestMarker_FirstEncounter_001` (yellow light)  

### Objectives
1. Approach the hub clearing (auto-triggers on proximity)
2. Observe the Stegosaurus stragglers near the clearing — do not disturb them
3. Return to the Elder

### Mechanics
- Player must stay crouched (stamina drain reduced while crouching)
- Stego alert radius: 800u — if player enters standing, Stego moves away
- Success condition: player observes for 5 seconds without triggering flee response
- Failure: Stego flees → quest resets after 60s cooldown

### Emotional Arc
*Wonder → Caution → Patience → Reward*  
The player's first lesson: this world does not belong to them. They are a guest.

### Voice Lines
- **Briefing:** `Elder_NPC_ObserveHerd_Briefing.mp3` (~23s)
  - URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783096013412_Elder_NPC_ObserveHerd_Briefing.mp3`
- **Reward:** Elder speaks: *"You are learning. The land teaches those who listen."*

### Rewards
- Unlock Quest 2 ("Observe the Herd")
- +10 Survival XP
- Crafting recipe unlocked: Grass Rope (basic binding material)

---

## Quest 2: "Observe the Herd"

**Type:** Stealth / Observation  
**Quest Giver:** Elder NPC (`NPC_Elder_QuestGiver_001`)  
**Trigger Zone:** `QuestZone_ObserveHerd_001` at (1800, 2200) — radius 600u  
**Objective Marker:** `QuestMarker_ObserveHerd_001` (green light)  
**Target Actors:** Triceratops herd (8 individuals: `Trike_Savana_001` through `Trike_Savana_008`)  

### Objectives
1. Approach the Triceratops herd from downwind (NE approach recommended)
2. Observe the herd lead female for 10 seconds without being detected
3. Identify the herd bull (largest individual, center position)
4. Return to the Elder with observations

### Mechanics
- Wind direction: NE (fixed for this quest)
- Trike alert states: Calm → Wary → Alarmed → Charge
- Wary trigger: player within 600u standing OR 400u crouching
- Alarmed trigger: player within 300u any stance OR sudden movement
- Charge trigger: player within 150u OR player runs while Alarmed
- Observation timer: 10s continuous — resets if alert state rises above Calm

### Stealth Rules
- Crouching reduces detection radius by 40%
- Approaching from downwind (NE) reduces detection radius by 30%
- Combined: crouching + downwind = 58% reduction → safe approach to ~250u
- Tall grass (if present) adds additional 20% reduction

### Emotional Arc
*Tension → Focus → Stillness → Triumph*  
The player learns that patience is a survival tool. The herd is not an obstacle — it is a lesson.

### Voice Lines
- **Briefing:** `Elder_NPC_ObserveHerd_Briefing.mp3` (~23s)
- **Reward:** `Elder_NPC_TrackGiants_Reward.mp3` (~22s)
  - URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783096035850_Elder_NPC_TrackGiants_Reward.mp3`

### Rewards
- Unlock Quest 3 ("Track the Giants")
- +25 Survival XP
- Crafting recipe unlocked: Bone Needle (advanced tool component)
- Map reveals: Brach migration path to the east

---

## Quest 3: "Track the Giants"

**Type:** Exploration / Tracking  
**Quest Giver:** Elder NPC (auto-unlocked after Quest 2)  
**Trigger Zone:** `QuestZone_TrackGiants_001` at (2600, 2900) — radius 800u  
**Objective Marker:** `QuestMarker_TrackGiants_001` (blue light)  
**Target Actors:** Brachiosaurus group (4 individuals: `Brach_Savana_001` through `Brach_Savana_004`)  

### Objectives
1. Locate the Brachiosaurus group east of the hub
2. Follow the migration path without losing sight of the herd for 30 seconds
3. Discover the water source at the end of the migration path
4. Return to camp before nightfall (optional time pressure)

### Mechanics
- Brach movement: slow, predictable, heading SE toward water source
- Player must stay within 1200u of any Brach to maintain "tracking" state
- If player loses tracking for >20s, quest objective resets to "Relocate the herd"
- Water source location: (3200, 3500) — not yet placed, flagged for Agent #05 (World Generator)
- Nightfall timer: optional — adds urgency but failure just triggers "You lost the trail in the dark" message

### Tracking Mechanics
- Footprints: Brach leave visible ground depressions (decal system — flagged for Agent #17 VFX)
- Broken vegetation: trees along path show "pushed" state (flagged for Agent #06 Environment)
- Sound cues: low rumble audible within 2000u (flagged for Agent #16 Audio)

### Emotional Arc
*Curiosity → Awe → Discovery → Purpose*  
The giants are not threats — they are guides. Following them leads to resources and survival.

### Voice Lines
- **Briefing:** Elder: *"The long-necks know where water hides. Follow them. Do not try to stop them — you cannot. Only follow."*
- **Reward:** Elder: *"You found the water. Now we can survive another season."*

### Rewards
- +50 Survival XP
- Water source added to player map
- Crafting recipe unlocked: Clay Water Vessel (stores 3 water units)
- New area unlocked: Eastern River Valley

---

## Quest Trigger Architecture

```
Hub (2100, 2400)
    │
    ├── NPC_Elder_QuestGiver_001 (2050, 2350) — amber light, quest dialogue
    │
    ├── QuestZone_FirstEncounter_001 (2100, 2400) — yellow marker
    │       └── Stego_Savana_001/002/003 nearby
    │
    ├── QuestZone_ObserveHerd_001 (1800, 2200) — green marker
    │       └── Trike_Savana_001 through 008
    │
    └── QuestZone_TrackGiants_001 (2600, 2900) — blue marker
            └── Brach_Savana_001 through 004
```

---

## Actor Reference Table

| Actor Label | Type | Position | Quest Role |
|-------------|------|----------|------------|
| `NPC_Elder_QuestGiver_001` | PointLight (NPC placeholder) | (2050, 2350, 150) | Quest giver for all 3 quests |
| `QuestZone_FirstEncounter_001` | TriggerBox | (2100, 2400, 100) | Tutorial trigger |
| `QuestZone_ObserveHerd_001` | TriggerBox | (1800, 2200, 100) | Quest 2 trigger |
| `QuestZone_TrackGiants_001` | TriggerBox | (2600, 2900, 100) | Quest 3 trigger |
| `QuestZone_StegoStragglers_001` | TriggerBox | (2300, 2600, 100) | Ambient encounter |
| `QuestMarker_FirstEncounter_001` | PointLight (yellow) | (2100, 2400, 180) | Visual debug marker |
| `QuestMarker_ObserveHerd_001` | PointLight (green) | (1800, 2200, 200) | Visual debug marker |
| `QuestMarker_TrackGiants_001` | PointLight (blue) | (2600, 2900, 250) | Visual debug marker |

---

## Audio Assets

| File | Character | Duration | URL |
|------|-----------|----------|-----|
| `Elder_NPC_ObserveHerd_Briefing.mp3` | Elder NPC | ~23s | [Supabase](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783096013412_Elder_NPC_ObserveHerd_Briefing.mp3) |
| `Elder_NPC_TrackGiants_Reward.mp3` | Elder NPC | ~22s | [Supabase](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783096035850_Elder_NPC_TrackGiants_Reward.mp3) |

---

## Dependencies for Other Agents

| Agent | Dependency | Priority |
|-------|-----------|----------|
| #15 Narrative | Full Elder NPC dialogue tree for all 3 quests | HIGH |
| #16 Audio | Brach tracking sound (low rumble 2000u radius), quest complete fanfare | HIGH |
| #17 VFX | Brach footprint decals, Trike alert visual (dust puff), quest zone particle effects | MEDIUM |
| #05 World Gen | Water source at (3200, 3500) — end point of Track the Giants | HIGH |
| #06 Environment | Broken vegetation along Brach migration path | LOW |
| #11 NPC Behavior | Elder NPC behavior tree (idle, talk, point toward quest zone) | MEDIUM |

---

*Generated by Agent #14 — Quest & Mission Designer — Cycle 020*
