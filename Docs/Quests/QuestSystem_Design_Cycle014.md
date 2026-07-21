# Quest & Mission System — Agent #14
## Cycle: PROD_CYCLE_AUTO_20260617_014

---

## Overview

Four crowd-integrated quests implemented this cycle, building directly on Agent #13's crowd simulation outputs (panic zones, herd formation, raptor patrol waypoints, migration route). All quests are survival-based, grounded in realistic prehistoric ecology.

---

## Quest 1 — "The Migration"

**Type:** Escort / Exploration  
**Trigger:** Player enters Brachio herd zone at (3000, 2000)  
**Emotional Arc:** Wonder → urgency → danger → relief

### Stages
| Stage | Trigger Actor | Location | Condition |
|-------|--------------|----------|-----------|
| 1 | QuestTrigger_Migration_Start | (3000, 2000) | Player enters 600u radius |
| 2 | QuestTrigger_Migration_HerdBreak | (3000, 3200) | Herd formation breaks — raptor patrol deviates |
| 3 | QuestTrigger_Migration_RiverCross | (1800, 5000) | Raptor ambush at river crossing |
| Complete | QuestComplete_Migration_001 | (1800, 5800) | Juvenile reaches safe zone |

### Objectives
1. Observe the Brachio herd moving north
2. The juvenile falls behind — stay between it and the raptor pack
3. Reach the river crossing before raptors cut off the route
4. Drive off or evade raptors at the crossing

### Rewards
- 3× Brachio bone (crafting material)
- River crossing unlocked as fast travel point
- Scout NPC voice line triggers: "The herd is moving. Follow them north..."

### Design Notes
- Uses MigrationWP_Brachio_001/002/003 from Agent #13 as waypoint chain
- PanicZone_Herbivore_Raptor_001 activates at Stage 3 — mass panic visible
- Player does NOT need to kill raptors — distraction/evasion is valid

---

## Quest 2 — "Pack Territory"

**Type:** Stealth / Infiltration  
**Trigger:** Player approaches raptor territory boundary at (-800, 1500)  
**Emotional Arc:** Tension → problem-solving → relief → reward

### Stages
| Stage | Trigger Actor | Location | Condition |
|-------|--------------|----------|-----------|
| 1 | QuestTrigger_PackTerritory_Enter | (-800, 1500) | Player enters 800u radius |
| 2 | QuestTrigger_PackTerritory_Distract | (-400, 2000) | Player uses meat/noise to lure alpha |
| 3 | QuestTrigger_PackTerritory_Exit | (200, 2500) | Player exits territory with feathers |
| Reward | QuestReward_RaptorFeathers_001 | (-600, 1800) | Rare feather node — 1 pickup |

### Objectives
1. Enter raptor territory without triggering patrol alert
2. Use a distraction (throw raw meat or create noise) to move the alpha
3. Collect rare raptor feathers from the nest site
4. Exit the territory before the pack returns

### Rewards
- 5× Raptor feather (crafting: fletching for spears, insulation for shelter)
- Scout NPC voice line: "Three raptors, maybe more. They have been watching us..."
- Unlocks "Improved Spear" crafting recipe

### Design Notes
- RaptorPack_PatrolWP_001-004 from Agent #13 defines patrol circuit
- Player must time movement between patrol gaps (alpha patrol cycle ~45s)
- If detected: all 3 raptors aggro, player must flee — quest fails but can retry
- Stealth mechanic: crouch + wind direction matters (downwind = safe)

---

## Quest 3 — "Panic in the Valley"

**Type:** Survival / Escape  
**Trigger:** TRex combat zone activates — PanicZone_Herbivore_TRex_001 fires  
**Emotional Arc:** Shock → fear → survival instinct → triumph

### Stages
| Stage | Trigger Actor | Location | Condition |
|-------|--------------|----------|-----------|
| 1 | QuestTrigger_PanicValley_TRexAlert | (1200, 800) | TRex enters combat range |
| 2 | QuestTrigger_PanicValley_HighGround | (600, 400, Z+300) | Player reaches elevated terrain |
| 3 | QuestTrigger_PanicValley_Aftermath | (1500, 1200) | Stampede passes, 60s timer |
| Complete | QuestComplete_PanicValley_001 | (1500, 1200) | Player collects aftermath resources |

### Objectives
1. TRex appears — the valley erupts in panic (herbivore stampede)
2. Reach high ground before the stampede reaches your position
3. Wait for the stampede to pass (60 seconds)
4. Scavenge the trampled area for resources (broken bones, crushed plants)

### Rewards
- 4× Large bone (crafting: spear shafts, shelter poles)
- 2× Crushed herb (medicine crafting)
- Unlocks "Bone Spear" crafting recipe

### Design Notes
- PanicZone_Herbivore_TRex_001 from Agent #13 is the primary trigger
- Panic propagation: 800u/s signal, 30s decay — visible as stampede wave
- High ground at Z+300 is safe from stampede (TRex does not pursue uphill)
- Aftermath window: 60s to collect resources before TRex patrols the area

---

## Quest 4 — "Herd Defender"

**Type:** Combat / Protection  
**Trigger:** Brachio juvenile separates from herd at (3000, 2200)  
**Emotional Arc:** Empathy → protectiveness → danger → bond

### Stages
| Stage | Trigger Actor | Location | Condition |
|-------|--------------|----------|-----------|
| 1 | QuestTrigger_HerdDefender_JuvSplit | (3000, 2200) | Juvenile wanders from herd |
| 2 | QuestTrigger_HerdDefender_RaptorApproach | (2400, 2000) | 2 raptors from patrol route |
| 3 | QuestTrigger_HerdDefender_DriveOff | (2700, 1800) | Player drives raptors away |
| 4 | QuestTrigger_HerdDefender_Escort | (3000, 2000) | Escort juvenile back to herd |
| Complete | QuestComplete_HerdDefender_001 | (3000, 2000) | Herd adult acknowledges player |

### Objectives
1. Notice the juvenile Brachio has separated from the herd
2. Two raptors from the patrol circuit approach the juvenile
3. Drive the raptors off (fire torch, throw rocks, direct combat)
4. Guide the juvenile back to the herd formation

### Rewards
- Herd "trust" flag set — herd will not panic when player is nearby
- 2× Brachio scale (rare crafting material: heavy armor)
- Unlocks "Brachio Territory" as safe rest zone

### Design Notes
- Herd_Brachio_Juvenile_001 from Agent #13 is the protected actor
- Raptors sourced from RaptorPack_PatrolWP_001-004 patrol circuit
- Fire torch is most effective deterrent (raptors flee from fire)
- If juvenile dies: quest fails permanently — no retry (emotional weight)

---

## Crafting Resource Nodes (Quest-Linked)

| Node Actor | Location | Resource | Recipe Use |
|-----------|----------|----------|-----------|
| CraftNode_Rock_Savana_001 | (500, 300) | Stone | Stone Axe (2 rocks + 1 stick) |
| CraftNode_Rock_Savana_002 | (800, 500) | Stone | Stone Axe |
| CraftNode_Stick_Floresta_001 | (-200, 600) | Stick | Stone Axe / Campfire |
| CraftNode_Stick_Floresta_002 | (-400, 400) | Stick | Campfire (3 sticks) |
| CraftNode_Stick_Floresta_003 | (-100, 800) | Stick | Campfire |
| CraftNode_Leaf_Pantano_001 | (200, -300) | Leaf | Water Container (1 rock + 1 leaf) |

### Crafting Recipes
```
Stone Axe:      2× Stone + 1× Stick → Melee weapon, wood chopping
Campfire:       3× Stick → Heat source, cooking, raptor deterrent  
Water Container: 1× Stone + 1× Leaf → Carry water, survive desert zones
```

---

## Voice Lines Generated

| Character | File | Line |
|-----------|------|------|
| QuestNPC_Elder | QuestNPC_Elder.mp3 | "The herd is moving. Follow them north before the raptors cut off the river crossing..." |
| QuestNPC_Scout | QuestNPC_Scout.mp3 | "Three raptors, maybe more. They have been watching us since yesterday..." |

Audio URLs:
- Elder: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741238315_QuestNPC_Elder.mp3
- Scout: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781741254209_QuestNPC_Scout.mp3

---

## Integration Map

```
Agent #12 (Combat AI)
  └── CombatZone_TRex_001 ──→ QuestTrigger_PanicValley_TRexAlert (Quest 3)
  └── CombatZone_Raptor_001 ──→ QuestTrigger_PackTerritory_Enter (Quest 2)

Agent #13 (Crowd)
  └── MigrationWP_Brachio_001/002/003 ──→ Quest 1 waypoint chain
  └── RaptorPack_PatrolWP_001-004 ──→ Quest 2 patrol timing
  └── PanicZone_Herbivore_TRex_001 ──→ Quest 3 stampede trigger
  └── Herd_Brachio_Juvenile_001 ──→ Quest 4 protected actor
  └── Herd_Brachio_Lead_001 ──→ Quest 4 escort destination

Agent #14 (Quest — THIS CYCLE)
  └── 4 quests with stage triggers
  └── 6 crafting resource nodes
  └── 2 NPC voice lines (Elder, Scout)
  └── MAP_SAVED: True
```

---

## For Agent #15 — Narrative & Dialogue

### Quest Dialogue Needs
1. **Quest 1 — The Migration**: Elder NPC intro dialogue (3 lines), river crossing danger warning (2 lines), completion acknowledgment (2 lines)
2. **Quest 2 — Pack Territory**: Scout NPC briefing (4 lines), stealth tip (2 lines), success/fail reactions (2 lines each)
3. **Quest 3 — Panic in the Valley**: No NPC — environmental storytelling only. Aftermath journal entry needed.
4. **Quest 4 — Herd Defender**: No dialogue — pure action. Post-quest: Elder observes "The herd remembers you" (1 line)

### Lore Hooks
- Why does the Brachio herd migrate north? (seasonal drought? predator pressure?)
- What is the significance of raptor feathers to the tribe? (ritual? trade?)
- Who is the Elder NPC? What is their relationship to the player?
- The "Herd Trust" mechanic implies animals can learn to recognize the player — is this a core lore element?
