# Quest & Mission System — Agent #14 Cycle 010
**PROD_CYCLE_AUTO_20260617_010**

---

## Overview
4 survival quests designed around Agent #13's crowd simulation actors and Agent #12's combat AI placements. All quests are grounded in realistic prehistoric survival — no spiritual/mystical content.

---

## Quest 1: "The Herd Moves"
**Type:** Migration / Exploration  
**Trigger:** Proximity to `Brachio_Herd_001` at (400, 2800)  
**Objective:** Follow the Brachiosaurus herd north to the water source

### Markers (Yellow PointLights)
| Label | Position | Role |
|-------|----------|------|
| `Quest_HerdMoves_Start` | (400, 2800, 200) | Quest activation trigger |
| `Quest_HerdMoves_WP1` | (420, 3500, 200) | Waypoint 1 — mid-corridor |
| `Quest_HerdMoves_WP2` | (450, 4200, 200) | Waypoint 2 — approaching water |
| `Quest_HerdMoves_End` | (480, 4900, 200) | Quest complete — water source |

### Narrative Beat
The player observes the herd moving with purpose. Camp Elder NPC explains: "They know where the water is." Player must stay downwind (stealth mechanic) or the herd scatters.

### Mechanics
- **Stealth proximity check:** Stay >300u downwind of herd lead
- **Reward:** Water source location revealed on map, 2x water containers filled
- **Failure state:** Herd scatters if player gets too close upwind — quest fails, herd resets after 5 minutes

### Voice Line
Camp Elder: *"The herd is moving north. Follow them — they know where the water is. But stay downwind, or the big ones will scatter."*  
Audio: `tts/1781717819901_CampElder_QuestGiver.mp3`

---

## Quest 2: "Silence in the Plains"
**Type:** Investigation / Scouting  
**Trigger:** Proximity to `Paras_Grazing_001` at (1750, -700) when herd is in alert state  
**Objective:** Investigate why the Parasaurolophus herd has gone silent

### Markers (Orange PointLights)
| Label | Position | Role |
|-------|----------|------|
| `Quest_SilencePlains_Start` | (1750, -700, 200) | Quest activation — Paras alert |
| `Quest_SilencePlains_Invest` | (2000, 1500, 200) | T-Rex territory boundary |
| `Quest_SilencePlains_Clue` | (2300, 1200, 200) | Evidence — tracks/bones |
| `Quest_SilencePlains_End` | (0, 0, 200) | Return to camp with intel |

### Narrative Beat
The plains are unnaturally quiet. The Parasaurolophus herd has stopped grazing and is alert. Player must scout the T-Rex territory boundary to find evidence of recent kill.

### Mechanics
- **Evidence system:** Find 3 clue markers (tracks, bones, drag marks)
- **Danger zone:** T-Rex patrols within 1500u of (2000, 1500) — `FleeZone_TRex` active
- **Reward:** T-Rex patrol route revealed, safe passage windows identified
- **Failure state:** Player enters T-Rex aggro range without escape route

### Voice Line
Scout NPC: *"Three raptors, circling from the east. They are not hunting yet — but they will be. Get back to camp before dark, or you will not get back at all."*  
Audio: `tts/1781717833569_Scout_NPC_Warning.mp3`

---

## Quest 3: "The Horned Wall"
**Type:** Strategy / Distraction  
**Trigger:** Player needs to cross T-Rex territory — activates near `Trike_Grazing_001` at (-2200, 1200)  
**Objective:** Use the Triceratops defensive formation to distract the T-Rex and create a safe passage

### Markers (Blue PointLights)
| Label | Position | Role |
|-------|----------|------|
| `Quest_HornedWall_Start` | (-2200, 1200, 200) | Quest activation — near Trike herd |
| `Quest_HornedWall_TRex` | (2000, 1500, 250) | T-Rex current position |
| `Quest_HornedWall_Passage` | (1500, 800, 200) | Safe corridor waypoint |
| `Quest_HornedWall_End` | (3000, 500, 200) | Destination beyond T-Rex territory |

### Mechanics
- **Distraction trigger:** Player throws meat/bait near Trike herd to provoke defensive formation
- **Trike behavior:** Triceratops form defensive circle, T-Rex investigates (drawn away from passage)
- **Time window:** 90 seconds to cross while T-Rex is distracted
- **Reward:** Access to new biome beyond T-Rex territory, rare crafting materials

---

## Quest 4: "Pack Hunters"
**Type:** Survival / Escape  
**Trigger:** Proximity to `Raptor_Pack_Alpha` at (-1200, 800) — raptor chase state activates  
**Objective:** Survive the raptor pack chase and reach camp safely

### Markers (Purple PointLights)
| Label | Position | Role |
|-------|----------|------|
| `Quest_PackHunters_Start` | (-1200, 800, 200) | Chase trigger — raptor aggro |
| `Quest_PackHunters_Escape1` | (-800, 400, 200) | Escape route WP1 |
| `Quest_PackHunters_Escape2` | (-400, 200, 200) | Escape route WP2 — near camp |
| `Quest_PackHunters_End` | (0, 0, 200) | Camp safety zone — quest complete |

### Mechanics
- **Chase AI:** Raptor_Pack_Alpha + Beta1/Beta2/Beta3 pursue player in flanking formation
- **Escape options:** 
  1. Sprint to camp (stamina check)
  2. Climb elevated terrain (raptors can't follow)
  3. Use `FleeZone_Raptor` red light zone as distraction (throw meat)
- **Reward:** Raptor feathers (crafting material), combat XP, camp fortification unlock

---

## Integration Map

```
Agent #12 (Combat AI)          Agent #13 (Crowd Sim)         Agent #14 (Quest)
─────────────────────          ─────────────────────         ─────────────────
TRex_Savana_001 ──────────────→ FleeZone_TRex ──────────────→ Quest_SilencePlains_Invest
                                                              Quest_HornedWall_TRex
CombatAI_Brachio_Stampede ────→ Brachio_Herd_001–008 ────────→ Quest_HerdMoves_Start
Raptor_Pack_Alpha ────────────→ FleeZone_Raptor ─────────────→ Quest_PackHunters_Start
                                Trike_Grazing_001–005 ────────→ Quest_HornedWall_Start
                                Paras_Grazing_001–006 ────────→ Quest_SilencePlains_Start
```

---

## Voice Assets
| Character | File | Quest |
|-----------|------|-------|
| Camp Elder | `CampElder_QuestGiver.mp3` | Quest 1 — The Herd Moves |
| Scout NPC | `Scout_NPC_Warning.mp3` | Quest 2 — Silence in the Plains |

---

## Actor Count This Cycle
- Quest markers spawned: **16** (4 per quest × 4 quests)
- Total quest actors in map: ~28 (including previous cycles)
- CAP status: SAFE (well under 8000 actor limit)

---

## For Agent #15 — Narrative & Dialogue Agent
These 4 quests need full dialogue trees:
1. **Camp Elder** — Quest 1 giver, 3-line intro + 1-line completion
2. **Scout NPC** — Quest 2 giver, warning dialogue + debrief
3. **Tribe Elder** — Quest 3 context (why crossing T-Rex territory matters)
4. **Survivor NPC** — Quest 4 witness (saw raptor pack attack, warns player)

Each NPC needs:
- Opening greeting (1 line)
- Quest offer (2-3 lines)
- Quest active reminder (1 line)
- Quest completion (2 lines)
- Failure/retry (1 line)
