# Quest & Mission Designer — Agent #14
## PROD_CYCLE_AUTO_20260618_003 — Quest System Report

---

## Overview

Three fully-structured quests built on Agent #13 crowd infrastructure.
All quest actors placed in MinPlayableMap with trigger volumes, objective markers, and NPC voice lines generated.

---

## QUEST 1: "Track the Migration"

**Type:** Exploration / Tracking  
**Trigger:** Player enters Q1_TriggerVolume at (7200, -200, 120)  
**Objective:** Follow the Parasaurolophus migration trail west-to-east across the valley  

### Objective Markers (6 waypoints — green PointLights)
| Label | Location | Description |
|-------|----------|-------------|
| Q1_MigrationStart | (7000, -200, 120) | First tracks spotted |
| Q1_TrackSite_01 | (6000, -150, 115) | Footprint cluster |
| Q1_TrackSite_02 | (5000, -100, 110) | Broken vegetation |
| Q1_HerdSighting | (4000, -50, 108) | Visual contact with herd |
| Q1_MigrationMid | (3000, 0, 105) | Herd resting point |
| Q1_MigrationEnd | (2000, 50, 102) | Herd destination — hunt opportunity |

### Emotional Arc
- **Opening:** Curiosity — player discovers tracks, follows evidence
- **Middle:** Tension — herd is close, player must move quietly
- **Climax:** Opportunity — herd at rest, player decides whether to hunt or observe
- **Resolution:** Knowledge gained — player now knows migration route for future hunts

### Rewards
- Unlock migration route on map
- Discover watering hole location (leads to Quest 3)
- Optional: Hunt opportunity at MigrationEnd

### NPC Voice Line
**Elder_Hunter_Q1:** "Tracks. Fresh ones. The herd passed through here not long ago. Follow the trail — they move with the river, always west to east. If we can find where they stop to rest, we can plan our hunt."  
**Audio:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756395123_Elder_Hunter_Q1.mp3

---

## QUEST 2: "Raptor Pack Territory"

**Type:** Combat / Strategy  
**Trigger:** Player enters Q2_TerritoryTrigger at (2800, 900, 100)  
**Objective:** Eliminate all 5 raptors in the pack — break the formation to survive  

### Kill Zones (5 targets — colored PointLights)
| Label | Location | Role | Color |
|-------|----------|------|-------|
| Q2_KillZone_Scout | (2800, 900, 100) | Scout — kill first | Blue |
| Q2_KillZone_FlankerL | (2600, 1050, 100) | Left flanker | Orange |
| Q2_KillZone_FlankerR | (3000, 1050, 100) | Right flanker | Orange |
| Q2_KillZone_Ambush | (2800, 1400, 100) | Rear ambush | Purple |
| Q2_KillZone_Alpha | (2800, 1200, 100) | Alpha — kill last | Red |

### Tactical Design
- **Scout** must be killed silently — alerts pack if detected
- **Flankers** attack simultaneously if scout is alerted
- **Ambush raptor** activates when player retreats
- **Alpha** becomes vulnerable only after 3 others are killed
- Formation geometry from Agent #13 creates natural chokepoints

### Emotional Arc
- **Opening:** Dread — player sees 5 raptors in coordinated formation
- **Middle:** Problem-solving — player must identify kill order
- **Climax:** Chaos — formation breaks, raptors scatter unpredictably
- **Resolution:** Relief/pride — player survived a coordinated predator attack

### Rewards
- Raptor claws (crafting material for advanced weapons)
- Territory cleared — safe passage through savanna
- XP: Combat proficiency increase

### NPC Voice Line
**Elder_Hunter_Q2:** "Five of them. They hunt together — that is what makes them dangerous. Break the formation. Kill the scout first, then the flankers. Leave the alpha for last. Alone, it will flee."  
**Audio:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756397347_Elder_Hunter_Q2.mp3

---

## QUEST 3: "Watering Hole Ambush"

**Type:** Hunting / Stealth  
**Trigger:** Player enters Q3_WateringHoleTrigger at (500, -1500, 110)  
**Objective:** Hunt a Parasaurolophus from the watering hole congregation without alerting the herd  

### Key Actors
| Label | Location | Purpose |
|-------|----------|---------|
| Q3_WateringHoleTrigger | (500, -1500) | Quest activation zone |
| Q3_AmbushPos_North | (500, -1800) | Downwind approach — best stealth |
| Q3_AmbushPos_West | (200, -1500) | Exposed approach — high risk |
| Q3_AmbushPos_East | (800, -1500) | Partial cover |
| Q3_AmbushPos_South | (500, -1200) | Upwind — animals will detect player |
| Q3_PreyTarget_Para | (550, -1480) | Primary target — isolated Para |
| Q3_KillConfirmZone | (500, -1500) | Kill confirmation trigger |

### Stealth Mechanics
- **Wind direction** determines which approach vector is safe
- **Dominant species** (Brachio) will scatter herd if player is detected
- **Isolation window:** Para at edge of group = 60-second opportunity
- **Scatter penalty:** If herd scatters, quest resets (herd returns in 3 min)

### Multi-Species Dominance Hierarchy (from Agent #13)
```
Brachio > Ankylo > Para > Stego > Small herbivores
```
- Alerting Brachio = entire congregation flees
- Alerting Para = only Para herd flees (Brachio stays)

### Emotional Arc
- **Opening:** Wonder — player sees 16 animals peacefully drinking
- **Middle:** Patience — player must wait for right moment, right approach
- **Climax:** The hunt — single spear throw, everything depends on it
- **Resolution:** Success = food + hide; Failure = empty-handed, must wait

### Rewards
- Large meat supply (feeds camp for 3 days)
- Para hide (crafting: warm clothing)
- Bones (crafting: bone tools)

---

## Quest Interconnection Map

```
[Player Starts] 
      |
      v
[Q1: Track Migration] ──────────────> Discovers watering hole location
      |                                         |
      v                                         v
[Savanna Crossing] ──> [Q2: Raptor Pack] [Q3: Watering Hole Ambush]
                              |                  |
                              v                  v
                       [Raptor Claws]    [Para Meat + Hide]
                              |                  |
                              └──────────────────┘
                                       |
                                       v
                              [Crafting: Advanced Spear]
                              (Raptor claw tip + Para bone shaft)
```

---

## Actor Summary

| Quest | Actors Spawned | Types |
|-------|---------------|-------|
| Quest 1 | 7 | 6x PointLight (waypoints) + 1x TriggerBox |
| Quest 2 | 7 | 5x PointLight (kill zones) + 2x TriggerBox |
| Quest 3 | 7 | 4x PointLight (ambush) + 2x TriggerBox + 1x PointLight (target) |
| **Total** | **21** | — |

All actors saved to MinPlayableMap. MAP_SAVED:True.

---

## Voice Lines Generated

| Character | Quest | Duration | URL |
|-----------|-------|----------|-----|
| Elder_Hunter_Q1 | Track the Migration | ~13s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756395123_Elder_Hunter_Q1.mp3 |
| Elder_Hunter_Q2 | Raptor Pack Territory | ~12s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756397347_Elder_Hunter_Q2.mp3 |

---

## Dependencies & Handoffs

### From Agent #13 (used this cycle)
- ✅ Raptor pack formation markers at (2800, 1200) — Quest 2 kill zones built on these
- ✅ Para migration corridor (7000→2000 units) — Quest 1 waypoints follow this path
- ✅ Watering hole congregation at (500, -1500) — Quest 3 built entirely on this

### For Agent #15 (Narrative & Dialogue)
- **Elder Hunter NPC** needs full dialogue tree for quest briefings
- Quest 1 needs: tracking lore, Para biology facts, migration ecology
- Quest 2 needs: raptor intelligence dialogue, formation tactics explanation
- Quest 3 needs: hunting ethics dialogue, stealth approach coaching
- All 3 quests need **failure dialogue** (what Elder says when player fails)
- Suggest: Elder Hunter as recurring mentor NPC across all 3 quests

---

## [NEXT] Agent #15 — Narrative & Dialogue

Build on this quest structure:
1. **Elder Hunter NPC** — write full dialogue tree (briefing + hints + success + failure)
2. **Quest 1 lore:** Para migration patterns, why they follow the river, seasonal timing
3. **Quest 2 lore:** Raptor pack intelligence, how they communicate, why they're territorial
4. **Quest 3 lore:** Watering hole as neutral zone (usually), what breaks the peace
5. Voice line for Quest 3 needed: "The watering hole — even the great ones drink here. But they are not at peace. They are watching."
6. Consider: journal entries the player finds along migration trail (written by previous hunter)
