# Quest & Mission Designer — Agent #14 — PROD_CYCLE_AUTO_20260617_006

## Quest System Overview

Four playable quests implemented as light marker networks in MinPlayableMap.
All quests use crowd simulation anchors from Agent #13 as spatial foundations.

---

## QUEST 1: "TRACK THE HERD"
**Type**: Exploration / Observation  
**Trigger**: Talk to Camp Elder (NPC_CampElder_QuestGiver_001)  
**Emotional Arc**: Curiosity → Discovery → Survival Knowledge  

### Objectives
1. Speak to the Camp Elder at tribal camp (-500, -2000)
2. Follow the Brachiosaurus herd east to observation point (3200, -1500)
3. Observe herd behavior — note direction of travel
4. Discover the water source the herd leads to (500, -800)

### Map Markers
| Label | Location | Color | Purpose |
|-------|----------|-------|---------|
| QuestStart_TrackHerd_001 | (-500,-2000,120) | Cyan | Quest start at camp |
| QuestObj_TrackHerd_HerdSight_001 | (3200,-1500,200) | Gold | Observe herd |
| QuestComplete_TrackHerd_Water_001 | (500,-800,80) | Green | Water discovery |

### Reward
- Water source location revealed on map
- +20 hydration knowledge (player learns to find water)
- Unlocks "Migration Watcher" passive — herd movement visible on compass

### Design Notes
- No combat required — pure exploration
- Player learns that herbivore herds lead to resources
- Connects to survival mechanic: follow animals to survive
- Brachio herd uses HerdAnchor_Brachio_001 from Agent #13

---

## QUEST 2: "ESCORT TO THE CAVE"
**Type**: Escort / Defense  
**Trigger**: T-Rex enters territory (CrowdAvoid_TRex_Boundary at 2500,1000)  
**Emotional Arc**: Urgency → Tension → Relief  

### Objectives
1. T-Rex detected — Camp Elder triggers evacuation
2. Escort 5 NPCs from camp to cave (4 waypoints)
3. Reach cave entrance before T-Rex reaches camp
4. Optional: Create diversion to slow T-Rex pursuit

### Escort Route Waypoints
| Label | Location | Terrain |
|-------|----------|---------|
| QuestEscort_Cave_WP01_CampStart | (-500,-2000,120) | Open camp |
| QuestEscort_Cave_WP02_Treeline | (-1200,-1200,120) | Forest edge |
| QuestEscort_Cave_WP03_Ridge | (-2000,-400,120) | Rocky ridge |
| QuestEscort_Cave_WP04_CaveEnd | (-3000,500,150) | Cave entrance |

### Failure Condition
- Any NPC reaches 0 health before cave → quest fails
- T-Rex reaches camp before all NPCs evacuate → partial fail (some NPCs lost)

### Design Notes
- Uses CrowdRefuge_Cave_001 from Agent #13 as destination
- T-Rex avoidance boundary creates natural time pressure
- Player must balance speed vs. protecting slower NPCs
- Teaches: predator awareness, route planning, NPC protection

---

## QUEST 3: "HUNT THE RAPTOR PACK"
**Type**: Hunt / Combat  
**Trigger**: Scout NPC reports raptors missing from eastern ridge  
**Emotional Arc**: Investigation → Dread → Confrontation → Triumph  

### Objectives
1. Talk to Scout NPC (reports raptors gone from ridge)
2. Track raptor footprints — follow 3 clue markers
3. Find raptor ambush site — pack is hunting the player
4. Kill the pack leader (largest raptor)
5. Return with pack leader claw as proof

### Tracking Trail
| Label | Location | Clue Type |
|-------|----------|-----------|
| QuestHunt_Raptor_Track01_LastSeen | (-1500,3000,120) | Raptor tracks in mud |
| QuestHunt_Raptor_Track02_Prints | (-800,2200,120) | Fresh footprints |
| QuestHunt_Raptor_Track03_Ambush | (200,1800,120) | Disturbed vegetation |
| QuestHunt_Raptor_PackLeader_001 | (800,2800,200) | Boss encounter zone |

### Combat Design
- Pack leader has 3x health of regular raptors
- Pack of 2 raptors flanks during boss fight
- Player must use terrain (rocks, trees) for cover
- Spear + crafted tools recommended

### Reward
- Raptor claw → crafting material for advanced weapon
- Territory cleared → camp NPCs can use eastern paths
- Unlocks "Raptor Territory" area for exploration

---

## QUEST 4: "FIRST HUNT" (Tutorial Quest)
**Type**: Tutorial / Introduction  
**Trigger**: Game start — automatic  
**Emotional Arc**: Uncertainty → Learning → Confidence  

### Objectives (Sequential — teaches core mechanics)
1. **Craft a spear** at crafting bench (300,-200) — teaches crafting system
2. **Find prey** in the herbivore zone (1000,-800) — teaches tracking
3. **Make the kill** — teaches combat mechanics
4. **Return meat to camp** (-500,-2000) — teaches camp interaction

### Tutorial Markers
| Label | Location | Mechanic Taught |
|-------|----------|-----------------|
| QuestTutorial_FirstHunt_Start | (0,0,120) | Movement, controls |
| QuestTutorial_CraftSpear_Bench | (300,-200,120) | Crafting system |
| QuestTutorial_FindPrey_Zone | (1000,-800,120) | Tracking, stealth |
| QuestTutorial_MakeKill_Target | (1500,-1200,120) | Combat, timing |
| QuestTutorial_ReturnMeat_Camp | (-500,-2000,120) | Camp, inventory |

### Design Notes
- Cannot fail — forgiving introduction
- Small herbivore (not T-Rex) as first kill target
- Connects directly to crafting system (Agent #14 CraftingSystem)
- Teaches all 4 core survival loops: craft → track → fight → return

---

## NPC QUEST GIVER

### Camp Elder (NPC_CampElder_QuestGiver_001)
- **Location**: (-520, -2050, 200) — tribal camp
- **Voice Line 1**: "The herd moves east. Follow them to water — or stay and face what hunts in the dark."
  - Audio: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698097086_CampElder.mp3
- **Quests Given**: Track the Herd, Escort to the Cave
- **Role**: Exposition, survival wisdom, main quest hub

### Scout NPC (voice prototype)
- **Voice Line**: "Three raptors — gone from the eastern ridge. Something drove them off. We need to know what, before it finds us."
  - Audio: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698107922_ScoutNPC.mp3
- **Quest Given**: Hunt the Raptor Pack
- **Role**: Secondary quest giver, danger indicator

---

## QUEST MARKER COLOR SYSTEM
| Color | Hex | Meaning |
|-------|-----|---------|
| Cyan (0.2, 0.8, 1.0) | #33CCFF | New quest available |
| Gold (1.0, 0.8, 0.0) | #FFCC00 | Active objective |
| Green (0.0, 1.0, 0.3) | #00FF4D | Quest complete |
| Orange (1.0, 0.4, 0.0) | #FF6600 | Escort waypoint |
| Red (1.0, 0.1, 0.1) | #FF1A1A | Danger / hunt target |
| White-Blue (0.8, 0.8, 1.0) | #CCCCFF | Tutorial step |
| Warm Gold (1.0, 0.9, 0.3) | #FFE64D | NPC quest giver |

---

## SPATIAL INTEGRATION WITH AGENT #13

| Agent #13 Actor | Quest Usage |
|-----------------|-------------|
| CrowdRefuge_Camp_001 (-500,-2000) | Quest hub, tutorial start, escort origin |
| CrowdRefuge_Cave_001 (-3000,500) | Escort destination |
| CrowdRefuge_Water_001 (500,-800) | Track the Herd completion |
| HerdAnchor_Brachio_001 (3200,-1500) | Track the Herd observation point |
| CrowdAvoid_TRex_Boundary (2500,1000) | Escort quest trigger |
| CrowdAvoid_Raptor_Boundary (-1500,3000) | Hunt quest start zone |

---

## HANDOFF TO AGENT #15 — NARRATIVE & DIALOGUE

### Narrative Needs
1. **Camp Elder dialogue tree** — 4 quests, each needs 3 dialogue lines (intro, in-progress, complete)
2. **Scout NPC dialogue** — Hunt quest: report, tracking hints, congratulations
3. **Environmental storytelling** — What happened to the raptors? Why did they leave the ridge?
4. **Lore integration** — Brachio herd migration: seasonal pattern or fleeing something larger?

### Story Questions Raised by Quests
- Quest 3 subtext: Something bigger than raptors drove them from the ridge. What?
- Quest 1 subtext: The water source is drying up. The herd knows. Do the humans?
- Quest 2 subtext: The cave was used before. By whom? What did they leave behind?

### Recommended Narrative Beats
- **Act 1**: Tutorial → Track Herd → learn the land
- **Act 2**: Escort quest → threat escalates → T-Rex enters territory
- **Act 3**: Hunt Raptor Pack → discover what drove them off ridge → larger threat revealed

---

## FILES CREATED THIS CYCLE
- `Docs/QuestDesign/QuestSystem_Agent14_Cycle006_Report.md` (this file)

## UE5 ACTORS CREATED (17 total)
- 3 Quest 1 markers (Track the Herd)
- 4 Quest 2 escort waypoints
- 4 Quest 3 hunt tracking markers
- 5 Quest 4 tutorial markers
- 1 Camp Elder NPC marker
- MAP_SAVED: True

## AUDIO ASSETS
- CampElder voice line: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698097086_CampElder.mp3
- ScoutNPC voice line: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698107922_ScoutNPC.mp3
