# Quest & Mission System — Cycle 014 (PROD_CYCLE_AUTO_20260617_012)
**Agent:** #14 — Quest & Mission Designer  
**Cycle:** PROD_CYCLE_AUTO_20260617_012  
**Depends on:** Agent #13 Crowd & Traffic Simulation outputs

---

## Voice Lines Generated

### CampElder_QuestGiver
**Audio URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781726867396_CampElder_QuestGiver.mp3`  
**Text:** "The herd is moving! I saw them scatter near the river — something big spooked them. We need to get to the cave before nightfall. Follow the yellow markers, stay low, and do NOT enter the red zone. That's T-Rex territory."  
**Quest:** The Stampede (Quest 1)

### Scout_NPC_QuestGiver
**Audio URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781726881651_Scout_NPC_QuestGiver.mp3`  
**Text:** "Listen carefully. Three raptors are circling the eastern rocks. If you want to reach the cave, you must move through the evacuation corridor — stay between the yellow lights. One wrong step into that red zone and you are dead. I have seen it happen before."  
**Quest:** Safe Passage (Quest 2)

---

## Quest 1 — "The Stampede"

### Overview
A T-Rex has entered the valley. The herbivore herds are in full panic scatter. The player must survive the chaos and reach the cave safe zone before being trampled or caught by the predator.

### Actors Spawned (5 point lights as markers)
| Label | Location | Color | Role |
|---|---|---|---|
| Quest_Stampede_Giver_001 | (0, 0, 150) | Orange | Quest giver — Camp Elder |
| Quest_Stampede_Obs_001 | (2200, 800, 200) | Yellow | Obj 1: Reach observation point |
| Quest_Stampede_Checkpoint_NW_001 | (1800, -600, 150) | Green | Obj 2: Survive scatter — NW checkpoint |
| Quest_Stampede_CaveEntry_001 | (800, -1200, 150) | Cyan | Obj 3: Reach cave corridor entrance |
| Quest_Stampede_Complete_001 | (400, -1800, 150) | White | Quest complete — cave safe zone |

### Integration with Agent #13
- **Trigger:** `TRex_AggroTrigger_001` (Agent #12) activates herd scatter
- **Scatter destinations:** `HerdPanic_Scatter_N/NE/E/SE/S/SW/W/NW_001` (Agent #13)
- **Safe corridor:** `CrowdEvac_Cave_WP1→WP2→WP3_001` (Agent #13)
- **Crowd state transition:** CALM → PANIC → EVACUATING

### Objectives
1. Witness the T-Rex enter the valley (reach observation point)
2. Navigate through panicking herbivores without being trampled (NW checkpoint)
3. Follow evacuation corridor to cave entrance
4. Reach the cave safe zone before T-Rex patrol returns

### Rewards
- Bone fragment (crafting material for advanced tools)
- Scout knowledge: cave location unlocked on map
- +15 Stamina (adrenaline boost narrative)

---

## Quest 2 — "Safe Passage"

### Overview
An injured scout is stranded near the river crossing. The player must escort them through the evacuation corridor to the cave while raptor packs patrol the flanks.

### Actors Spawned (5 point lights as markers)
| Label | Location | Color | Role |
|---|---|---|---|
| Quest_SafePassage_Giver_001 | (-400, 600, 120) | Orange | Quest giver — injured scout |
| Quest_SafePassage_WP1_001 | (300, -400, 130) | Green | Waypoint 1 — corridor entry |
| Quest_SafePassage_WP2_001 | (600, -800, 130) | Green | Waypoint 2 — mid corridor |
| Quest_SafePassage_WP3_001 | (900, -1200, 130) | Green | Waypoint 3 — cave approach |
| Quest_SafePassage_Complete_001 | (1100, -1600, 130) | White | Quest complete — cave safe zone |

### Integration with Agent #13
- **Corridor:** `CrowdEvac_Cave_WP1→WP2→WP3_001` (Agent #13 evacuation network)
- **Threat:** Raptor exclusion zone markers from Agent #13 define patrol boundaries
- **Crowd state:** EVACUATING — corridor is active with fleeing NPCs (adds urgency)

### Objectives
1. Find the injured scout near the river
2. Escort through corridor waypoint 1 (avoid raptor patrol)
3. Escort through waypoint 2 (raptor pack crosses path — distract or hide)
4. Reach cave entrance with NPC alive

### Rewards
- Flint knife (crafted by grateful scout)
- Scout joins camp — unlocks trade mechanic
- Map knowledge: raptor patrol routes revealed

---

## Quest 3 — "The Exclusion Zone"

### Overview
A rare set of T-Rex bones lies inside the most dangerous territory in the valley — the T-Rex exclusion zone. The tribe elder needs the bones to craft a weapon capable of piercing thick hide. The player must enter, retrieve, and escape.

### Actors Spawned (4 point lights as markers)
| Label | Location | Color | Role |
|---|---|---|---|
| Quest_ExclusionZone_Giver_001 | (100, 200, 120) | Orange | Quest giver — tribe elder |
| Quest_ExclusionZone_Item_001 | (3000, 1400, 120) | Red | Item location — inside exclusion zone |
| Quest_ExclusionZone_Escape_001 | (2000, 800, 120) | Yellow | Escape checkpoint — zone boundary |
| Quest_ExclusionZone_Complete_001 | (0, 0, 120) | White | Quest complete — return to camp |

### Integration with Agent #12 + #13
- **Exclusion zone:** Red boundary markers from Agent #13 (radius 1500 around 3400, 1600)
- **T-Rex:** `TRex_AggroTrigger_001` (Agent #12) — active patrol
- **Risk:** Entering red zone activates T-Rex aggro state

### Objectives
1. Accept mission from tribe elder at camp
2. Reach the exclusion zone boundary without alerting T-Rex
3. Retrieve the T-Rex bone fragment (red marker at 3000, 1400)
4. Escape the exclusion zone before T-Rex returns
5. Return to camp

### Rewards
- T-Rex bone spear (highest-tier weapon in early game)
- Tribe elder respect — unlocks advanced crafting recipes
- Achievement: "Into the Red Zone"

---

## Density Zone Quest Hooks (Future Cycles)

Agent #13 placed 4 density zone centers for peaceful observation/gathering:
- **Meadow density zone** → "The Great Gathering" — observe herbivore migration, count species
- **River density zone** → "Water Source Survey" — map river crossings, find safe drinking spots
- **Forest density zone** → "Canopy Resources" — gather rare plants from forest floor near herd paths
- **Plains density zone** → "The Open Plains" — track predator movement patterns from distance

---

## Quest State Machine

```
QUEST_INACTIVE
    → [Accept from NPC] → QUEST_ACTIVE
        → [Reach objective] → OBJECTIVE_COMPLETE
            → [All objectives done] → QUEST_COMPLETE
                → [Collect reward] → QUEST_REWARDED
        → [Player dies] → QUEST_FAILED → restart from last checkpoint
```

## Crowd State Integration

| Crowd State (Agent #13) | Quest Effect |
|---|---|
| CALM | Normal NPC dialogue, full quest availability |
| ALERT | Quest givers warn player, some NPCs hide |
| PANIC | "The Stampede" quest activates, escort quests become urgent |
| EVACUATING | Corridor waypoints active, raptor patrol intensifies |

---

## For Agent #15 — Narrative & Dialogue Agent

### Dialogue Needed
1. **Camp Elder** — Quest 1 intro, mid-quest encouragement, completion congratulation
2. **Injured Scout** — Quest 2 distress call, escort dialogue (fear, gratitude), completion
3. **Tribe Elder** — Quest 3 briefing (explains bone weapon), warning about T-Rex, reward speech

### Lore Hooks
- Why is the T-Rex in the valley? (seasonal migration? territorial expansion?)
- Who was the scout and how did they get injured?
- What is the bone weapon used for? (boss fight preparation?)
- Cave history — has the tribe sheltered there before?

### Emotional Arc per Quest
- **The Stampede:** Chaos → survival instinct → relief → wonder (survived something massive)
- **Safe Passage:** Responsibility → tension → near-death moment → bond with NPC
- **The Exclusion Zone:** Fear → courage → adrenaline → triumph (took something from apex predator)
