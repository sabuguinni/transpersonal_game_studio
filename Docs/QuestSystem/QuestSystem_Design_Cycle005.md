# Quest & Mission System — Agent #14 — PROD_CYCLE_AUTO_20260617_005

## Overview
Three complete quest lines implemented this cycle, building on Agent #13's crowd simulation infrastructure.
All quest markers placed in MinPlayableMap as light actors with colour-coded objectives.

---

## Quest 1: "The Great Hunt" (Main Quest)

**Quest Giver:** Tribal Elder at `QuestHub_TribalElder_001` (Gold light, origin)
**Active Hours:** 18:00–21:00 in-game time (evening gathering)
**Reward:** Stone Axe recipe unlock + tribal respect +20

### Objectives
1. **Speak to the Elder** — Approach `QuestHub_TribalElder_001` at tribal fire
2. **Track the T-Rex** — Follow tracks to `Quest_Hunt_TRex_Start_001` (Red light, 1500, 500)
3. **Reach the Kill Zone** — Navigate to `Quest_Hunt_TRex_Target_001` (Red light, 2800, 1200)
4. **Defeat the T-Rex** — Combat encounter using Agent #12 AI zones
5. **Return to Elder** — Report back to tribal fire

### Emotional Arc
The player begins as an outsider. The Elder tests them with an impossible task — kill the apex predator.
Completing the hunt earns full tribal membership. The T-Rex skull becomes a permanent map landmark.

### Integration Points
- Uses Agent #12 `TRex_AggroZone` for combat trigger
- Uses Agent #13 `Stampede_Trigger_001/002` for cinematic moment when T-Rex enters savanna
- Voice line: "A great hunt awaits you..." (TTS generated, ~9s)

---

## Quest 2: "The Great Migration" (Side Quest)

**Quest Giver:** Scout NPC at `Quest_Escort_Parasaur_Start_001` (Cyan, 500, -2800)
**Trigger:** Player approaches Parasaur grazing anchor from Agent #13
**Reward:** Parasaur territory mapped + migration route unlocked

### Waypoint Chain
| Waypoint | Location | Hazard |
|----------|----------|--------|
| `Quest_Migration_WP_001` | (500, -2800) | Start — Parasaur herd |
| `Quest_Migration_WP_002` | (-200, -2200) | Raptor patrol zone — must distract |
| `Quest_Migration_WP_003` | (-800, -1800) | River crossing — crocodile risk |
| `Quest_Migration_WP_004` | (-1400, -1600) | Forest edge — low visibility |
| `Quest_Migration_WP_005` | (-2000, -1500) | End — Brachio safe zone |

### Mechanics
- Player must stay within 500u of herd to guide them
- Raptors from Agent #12 zones will attempt to scatter the herd
- Player can use crafted spears to drive off raptors without killing them
- If herd scatters, quest fails — must restart from last waypoint

### Emotional Arc
The migration is not just a task — it's a lesson in the ecosystem. The player learns that protecting
herbivores protects the food chain that sustains the tribe. Completing it unlocks the "Naturalist" skill tree.

### Cinematic Moment
`Quest_Stampede_CinTrigger_001/002` — When T-Rex enters the savanna during migration,
the herd panics. Player must choose: abandon the herd to survive, or risk their life to redirect the stampede.

---

## Quest 3: "Forge Your First Weapon" (Tutorial Quest)

**Quest Giver:** Crafting NPC at tribal fire (auto-triggers on first visit)
**Reward:** Stone Axe (crafted), Campfire recipe, Water Container recipe

### Objectives
1. **Gather 2 Stones** — `Quest_Resource_Stone_001/002/003` (Grey lights, near camp)
2. **Gather 1 Stick** — `Quest_Resource_Stick_001/002` (Brown lights)
3. **Gather 1 Leaf** — `Quest_Resource_Leaf_001/002` (Green lights)
4. **Return to Craft Point** — `Quest_Craft_Return_001` (Gold light at tribal fire)
5. **Craft Stone Axe** — Open crafting menu (C key), select recipe

### Recipes Defined
```
Stone Axe:       2x Rock + 1x Stick → Melee weapon, 25 damage, 15 durability
Campfire:        3x Stick → Warmth source, cook food, scare predators at night
Water Container: 1x Rock + 1x Leaf → Carry 3 water units, reduces thirst drain
```

### Resource Spawn Locations
| Resource | Label | Location | Count |
|----------|-------|----------|-------|
| Stone | Quest_Resource_Stone_001 | (300, 400) | 3 nodes |
| Stone | Quest_Resource_Stone_002 | (-400, 600) | 3 nodes |
| Stone | Quest_Resource_Stone_003 | (600, -300) | 3 nodes |
| Stick | Quest_Resource_Stick_001 | (200, 700) | 5 branches |
| Stick | Quest_Resource_Stick_002 | (-300, 900) | 5 branches |
| Leaf | Quest_Resource_Leaf_001 | (100, -500) | 8 leaves |
| Leaf | Quest_Resource_Leaf_002 | (-600, -200) | 8 leaves |

---

## Quest 4: "Reach the Water" (Survival Quest)

**Trigger:** Player thirst drops below 20% (automatic)
**Objective:** Reach `Quest_Water_Source_001` (Blue light, -500, 1800) before dehydration
**Hazard:** Raptor patrol from Agent #12 circles water source at dusk (17:00–19:00)
**Strategy Options:**
- Wait for dawn when raptors leave (safe but slow — thirst continues dropping)
- Craft a distraction (throw meat away from water source)
- Sprint through raptor zone (high risk, fast reward)

**Emotional Arc:** Survival is never simple. Every resource has a cost. The player learns that the world
does not pause for their needs — they must adapt to its rhythms.

---

## Voice Lines Generated

| File | Character | Duration | URL |
|------|-----------|----------|-----|
| 1781693661197_TribalElder.mp3 | Tribal Elder | ~9s | Quest_Hunt_TRex briefing |
| 1781693672036_TribalElder.mp3 | Tribal Elder | ~9s | Quest_Water_Source warning |

---

## Map Actors Created This Cycle

| Label | Type | Location | Purpose |
|-------|------|----------|---------|
| QuestHub_TribalElder_001 | PointLight (Gold) | (0, 0, 250) | Main quest hub |
| Quest_Hunt_TRex_Start_001 | PointLight (Red) | (1500, 500, 200) | Hunt quest start |
| Quest_Hunt_TRex_Target_001 | PointLight (Red) | (2800, 1200, 200) | Hunt kill zone |
| Quest_Escort_Parasaur_Start_001 | PointLight (Cyan) | (500, -2800, 200) | Escort start |
| Quest_Escort_Parasaur_End_001 | PointLight (Green) | (-2000, -1500, 200) | Escort end |
| Quest_Water_Source_001 | PointLight (Blue) | (-500, 1800, 200) | Water quest |
| Quest_Migration_WP_001–005 | PointLight (Cyan) | Migration path | Waypoints |
| Quest_Stampede_CinTrigger_001–002 | PointLight (Orange) | (1800–2200, 800–1000) | Cinematic triggers |
| Quest_Resource_Stone_001–003 | PointLight (Grey) | Near camp | Stone pickups |
| Quest_Resource_Stick_001–002 | PointLight (Brown) | Near camp | Stick pickups |
| Quest_Resource_Leaf_001–002 | PointLight (Green) | Near camp | Leaf pickups |
| Quest_Craft_Return_001 | PointLight (Gold) | (0, 0, 200) | Crafting return |

**Total new actors: 20**

---

## Integration Map

```
Agent #12 (Combat AI)     → Quest_Hunt_TRex uses TRex_AggroZone
Agent #13 (Crowd)         → Quest_Migration uses Parasaur grazing anchors + stampede triggers
Agent #14 (Quest)         → Provides quest hub + objectives for Agent #15 (Narrative)
Agent #15 (Narrative)     → Should write full dialogue for Tribal Elder + Scout NPC
Agent #16 (Audio)         → Should attach TTS voice lines to QuestHub_TribalElder_001
```

---

## For Agent #15 — Narrative & Dialogue

Priority dialogue to write:
1. **Tribal Elder** — Quest_Hunt briefing (3 lines: intro, objective, reward)
2. **Tribal Elder** — Quest_Water warning (2 lines: urgency, strategy hint)
3. **Scout NPC** — Migration quest intro (4 lines: herd description, route, raptor warning, reward)
4. **Crafting NPC** — Tutorial quest intro (3 lines: welcome, materials list, crafting instructions)
5. **Quest completion lines** — 1 line per quest for each outcome (success/fail)

The Elder's voice is gruff, practical, no-nonsense. He has survived 40 winters.
He does not explain — he commands. The player earns his respect through action, not words.
