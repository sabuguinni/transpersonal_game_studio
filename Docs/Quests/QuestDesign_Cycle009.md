# Quest & Mission Designer Agent #14 — Cycle 009 Report

## Production Summary
**Cycle:** PROD_CYCLE_AUTO_20260617_009
**Agent:** #14 — Quest & Mission Designer
**Tools Used:** 6 (4× ue5_execute, 2× text_to_speech, 1× github_file_write)

---

## Voice Lines Produced

### CampElder_QuestGiver (Quest 1 — "Warn the Camp")
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781713400498_CampElder_QuestGiver.mp3
- **Text:** "The scouts have returned. The T-Rex has crossed into our valley. We need three volunteers to warn the river camp before the herd scatters. Move fast — and stay low."
- **Duration:** ~11s
- **Usage:** Quest 1 trigger dialogue, plays when player approaches Quest_WarnCamp_Giver_001

### Tracker_NPC (Quest 3 — "Witness the Scatter")
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781713412296_Tracker_NPC.mp3
- **Text:** "You found the tracks. Three toes, deep in the mud — a raptor, and it was running. Something scared it. Follow the trail north and find out what."
- **Duration:** ~10s
- **Usage:** Quest 3 intro dialogue, plays when player finds raptor tracks near Quest_WitnessScatter_Obs_001

---

## Quest Actors Spawned in MinPlayableMap

### Quest 1: "Warn the Camp"
**Premise:** The camp elder has received word that a T-Rex has entered the valley. The player must warn the river camp before the herd scatters and cuts off the path.
**Emotional Arc:** Urgency → Danger → Relief → Belonging (player becomes trusted member of the tribe)

| Actor Label | Location (X,Y,Z) | Role |
|---|---|---|
| Quest_WarnCamp_Giver_001 | (0, 0, 120) | Camp Elder NPC — quest giver, plays CampElder voice line |
| Quest_WarnCamp_Obj1_001 | (-400, 200, 120) | Objective 1: Reach the scout post |
| Quest_WarnCamp_Obj2_001 | (-800, 600, 120) | Objective 2: Warn the river camp |
| Quest_WarnCamp_Obj3_001 | (-1200, 1000, 120) | Objective 3: Return to main camp |

**Mechanics:**
- Time pressure: herd scatter begins 3 minutes after quest start (linked to HerdScatter_Brachio_001)
- If player is too slow, river camp NPC is found injured (alternate outcome)
- Reward: Crafting recipe unlocked — Bone Spear (requires 2 bones + 1 stick)

---

### Quest 2: "Survive the T-Rex Encounter"
**Premise:** The player strays too close to the T-Rex exclusion zone. They must reach safety without being detected.
**Emotional Arc:** Shock → Fear → Cunning → Triumph (player outsmarts apex predator through stealth)

| Actor Label | Location (X,Y,Z) | Role |
|---|---|---|
| Quest_TRexSurvive_Start_001 | (800, 1800, 120) | Quest trigger boundary (2500u from T-Rex zone center) |
| Quest_TRexSurvive_Shelter1_001 | (200, 2400, 120) | Shelter point 1 — behind rock formation |
| Quest_TRexSurvive_Shelter2_001 | (-400, 2000, 120) | Shelter point 2 — dense tree cover |
| Quest_TRexSurvive_Exit_001 | (-800, 1400, 120) | Safe exit point — quest completion trigger |

**Mechanics:**
- Stealth mechanic: player must crouch and move slowly (stamina drain reduced while crouching)
- T-Rex detection radius: 1800u visual, 2200u audio (footsteps trigger detection)
- Wind direction matters: upwind = detected faster, downwind = detection range halved
- Reward: Fear stat permanently reduced by 10% (player learns T-Rex behavior patterns)

**Integration with Crowd System:**
- CrowdExclusion_TRex_Zone_001 defines the hard boundary (2000u radius)
- CrowdPanic_Raptor_Zone_001 adds secondary danger — raptors flee T-Rex into player's path
- CrowdFlow_SafePath_001–003 are the intended escape routes (player discovers them organically)

---

### Quest 3: "Witness the Scatter"
**Premise:** A tracker NPC has found raptor tracks suggesting something large scared the herd. Player must reach high ground to observe the scatter event.
**Emotional Arc:** Curiosity → Wonder → Understanding (player gains ecological knowledge of the world)

| Actor Label | Location (X,Y,Z) | Role |
|---|---|---|
| Quest_WitnessScatter_Obs_001 | (-1600, 2200, 180) | High ground observation point (elevated terrain) |
| Quest_WitnessScatter_Obj1_001 | (-1800, 2600, 120) | Brachio scatter witness zone |
| Quest_WitnessScatter_Obj2_001 | (-1400, 2800, 120) | Trike scatter witness zone |
| Quest_WitnessScatter_Obj3_001 | (-1200, 2400, 120) | Pachy scatter witness zone |
| Quest_WitnessScatter_End_001 | (-1000, 2000, 120) | Return to camp — report findings |

**Mechanics:**
- Player must observe each species scatter (enter witness zone + wait 5 seconds)
- No combat — pure observation quest
- Plays Tracker_NPC voice line when player reaches Obs_001
- Reward: Hunting knowledge unlocked — player can now read animal behavior (UI indicator shows herd stress level)

**Integration with Crowd System:**
- HerdScatter_Brachio_001, HerdScatter_Trike_001, HerdScatter_Pachy_001 are the scatter anchors
- PanicRelay chain (Scout→Camp→Gather→River→Forest) triggers scatter sequence
- Quest completes when all 3 species scatter events are witnessed

---

## Quest Dependency Map

```
Agent #13 Outputs → Agent #14 Quest Triggers
─────────────────────────────────────────────
PanicRelay_Scout_001          → Quest_WarnCamp_Giver_001 (Quest 1 origin)
CrowdExclusion_TRex_Zone_001  → Quest_TRexSurvive_Start_001 (Quest 2 trigger)
HerdScatter_Brachio/Trike/Pachy → Quest_WitnessScatter_Obj1/2/3 (Quest 3 targets)
CrowdFlow_SafePath_001–003    → Quest_TRexSurvive_Shelter1/2 (escape routes)
Migration corridor waypoints  → Quest_WarnCamp_Obj1/2/3 (path alignment)
```

---

## Quest State Machine Design

```
QUEST_INACTIVE
    │ (player approaches quest giver / trigger zone)
    ▼
QUEST_ACTIVE
    │ (player completes objectives in sequence)
    ▼
QUEST_COMPLETE
    │ (reward granted, narrative flag set)
    ▼
QUEST_ARCHIVED (available for narrative callbacks)
```

**Failure States:**
- Quest 1: QUEST_FAILED_LATE (river camp NPC injured — alternate ending, not blocked)
- Quest 2: QUEST_FAILED_DETECTED (T-Rex detects player — quest resets, player respawns at shelter)
- Quest 3: No failure state — observation quest, player can take as long as needed

---

## Narrative Handoff Notes for Agent #15

### Story Beats Established This Cycle
1. **Camp Elder** is an established NPC with voice — needs full dialogue tree (greeting, quest active, quest complete, idle lines)
2. **Tracker NPC** is established — needs name, backstory, and follow-up dialogue after Quest 3
3. **T-Rex Encounter** is a narrative milestone — player's first close encounter with apex predator
4. **Herd Scatter** is a world event — should be referenced in lore (tribal stories about "the great scatter")

### Lore Opportunities
- Why did the T-Rex cross into the valley? (narrative hook for main story)
- What do the raptor tracks tell the tracker about the ecosystem shift?
- The river camp — who lives there? Are they a rival tribe or allies?

### Dialogue Lines Needed (Priority Order)
1. Camp Elder: Quest 1 completion line ("You made it. The river camp is warned.")
2. Tracker NPC: Quest 3 completion line ("Now you understand. The land speaks — if you know how to listen.")
3. River Camp NPC: Reaction to warning (both on-time and late variants)

---

## MAP_SAVED: True
All quest actors written to `/Game/Maps/MinPlayableMap`

---

## [NEXT] Narrative & Dialogue Agent #15 should:
1. Write full dialogue trees for **Camp Elder** (Quest 1 giver) — 3 states: idle, quest active, quest complete
2. Write full dialogue for **Tracker NPC** (Quest 3 giver) — include lore about raptor behavior
3. Establish **River Camp NPC** as a named character with personality
4. Write **lore entry** about the T-Rex valley crossing — tribal oral history format
5. Reference voice line URLs from this cycle:
   - CampElder: `tts/1781713400498_CampElder_QuestGiver.mp3`
   - Tracker: `tts/1781713412296_Tracker_NPC.mp3`
