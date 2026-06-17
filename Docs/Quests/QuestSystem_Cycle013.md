# Quest & Mission System — Production Cycle 013
**Agent #14 — Quest & Mission Designer**
*Built on top of Agent #13 Crowd Simulation outputs*

---

## Voice Lines (ElevenLabs TTS — Elder NPC)

| Line | URL | Duration |
|------|-----|----------|
| Quest 1 — Follow the Herd | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781732312824_Elder_NPC_QuestGiver.mp3 | ~10s |
| Quest 2 — Scavenger's Trail | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781732324626_Elder_NPC_QuestGiver.mp3 | ~12s |

---

## Quest Definitions

### Quest 1: "Follow the Herd"
**Trigger event:** `HERD_SCATTER` (Triceratops herd scatters from Agent #13 crowd system)
**Quest giver:** `NPC_Elder_QuestGiver_001` (at camp, loc -200/-300)
**Emotional arc:** Curiosity → Dread → Discovery
**Objective chain:**
1. Observe Triceratops herd scatter at `QuestTrigger_FollowHerd_Start` (-3800, -2800)
2. Track scatter direction — follow broken vegetation/tracks to `QuestTrigger_FollowHerd_Track1`
3. Find TRex territorial markings at `QuestTrigger_FollowHerd_Track2`
4. Locate TRex nest observation point at `QuestReward_TRexNest_Obs` (200, 500)
**Reward:** Knowledge of TRex patrol territory (unlocks safe travel routes)
**Failure state:** Player enters TRex nest zone without stealth — TRex aggro
**Design note:** No combat required. Pure tracking/observation. Teaches player to read animal behaviour as information.

---

### Quest 2: "Scavenger's Trail"
**Trigger event:** `SCAVENGER_CONVERGE` (pterosaurs + ground scavengers converge on feeding zone)
**Quest giver:** `NPC_Scout_Tracker_001` (at camp, loc -350/-200)
**Emotional arc:** Opportunity → Risk assessment → Reward
**Objective chain:**
1. Notice pterosaurs circling at `Scavenger_Ptero_001/002/003` (from Agent #13)
2. Approach fresh kill site at `QuestTrigger_ScavTrail_Start` (1500, 2000)
3. Drive off ground scavengers at `QuestTrigger_ScavTrail_Bones` (2200, 2800)
4. Collect bone tools at `QuestReward_BoneTools_001/002` (2800, 3200)
**Reward:** 2× Bone Tool (crafting ingredient for advanced weapons)
**Failure state:** Linger too long — predator returns to kill site
**Design note:** Time-pressure mechanic. Player must weigh risk of scavenger confrontation vs. predator return timer.

---

### Quest 3: "The Great Migration"
**Trigger event:** `MIGRATION_ACTIVE` (Brachiosaurus migration corridor active, Agent #13)
**Quest giver:** `NPC_Elder_QuestGiver_001`
**Emotional arc:** Awe → Patience → Triumph
**Objective chain:**
1. Reach migration corridor entry at `QuestTrigger_Migration_Entry` (-1000, 4000)
2. Wait for gap in herd — cross at `QuestTrigger_Migration_Cross1` (-1000, 2500)
3. Navigate second crossing at `QuestTrigger_Migration_Cross2` (-1000, 1000)
4. Reach far side at `QuestTrigger_Migration_Exit` (-1000, -500)
5. Collect flint deposit at `QuestReward_Flint_001/002`
**Reward:** 3× Flint (crafting ingredient for fire-starting and blades)
**Failure state:** Caught between Brachiosaurus legs — stamina drain + knockdown
**Design note:** No violence. Pure movement puzzle. Brachiosaurs are not hostile but their mass is lethal. Teaches patience and reading animal movement patterns.

---

### Quest 4: "The Silence"
**Trigger event:** `CROWD_SILENCE` (all ambient animals go quiet — apex predator nearby)
**Quest giver:** Environmental (no NPC — player notices silence themselves)
**Emotional arc:** Unease → Investigation → Terror/Revelation
**Objective chain:**
1. Player enters `QuestTrigger_Silence_Zone` (500, -2000) — ambient sound cuts out
2. Investigate first clue at `QuestTrigger_Silence_Clue1` (800, -2500) — claw marks on tree
3. Investigate second clue at `QuestTrigger_Silence_Clue2` (1100, -3000) — crushed vegetation
4. Predator revealed at `QuestTrigger_Silence_Reveal` (1400, -3500) — TRex or Raptor pack
**Reward:** Survival knowledge — predator location marked on mental map
**Failure state:** Player makes noise during investigation — predator aggro
**Design note:** No quest marker. No UI prompt. Player must notice the silence themselves. Most atmospheric quest in the game — teaches environmental reading as survival skill.

---

## Actor Placement Summary (MinPlayableMap)

### Quest Trigger Zones (PointLights — coloured by quest)
| Actor Label | Location | Quest | Colour |
|-------------|----------|-------|--------|
| QuestTrigger_FollowHerd_Start | -3800, -2800, 50 | Q1 | Green |
| QuestTrigger_FollowHerd_Track1 | -2500, -1500, 50 | Q1 | Green |
| QuestTrigger_FollowHerd_Track2 | -1200, -500, 50 | Q1 | Green |
| QuestTrigger_FollowHerd_End | 200, 500, 50 | Q1 | Green |
| QuestTrigger_ScavTrail_Start | 1500, 2000, 50 | Q2 | Amber |
| QuestTrigger_ScavTrail_Bones | 2200, 2800, 50 | Q2 | Amber |
| QuestTrigger_ScavTrail_Reward | 2800, 3200, 50 | Q2 | Amber |
| QuestTrigger_Migration_Entry | -1000, 4000, 50 | Q3 | Blue |
| QuestTrigger_Migration_Cross1 | -1000, 2500, 50 | Q3 | Blue |
| QuestTrigger_Migration_Cross2 | -1000, 1000, 50 | Q3 | Blue |
| QuestTrigger_Migration_Exit | -1000, -500, 50 | Q3 | Blue |
| QuestTrigger_Silence_Zone | 500, -2000, 50 | Q4 | Red |
| QuestTrigger_Silence_Clue1 | 800, -2500, 50 | Q4 | Red |
| QuestTrigger_Silence_Clue2 | 1100, -3000, 50 | Q4 | Red |
| QuestTrigger_Silence_Reveal | 1400, -3500, 50 | Q4 | Red |

### Quest Reward Nodes (SpotLights — warm amber, pointing down)
| Actor Label | Location | Quest | Contents |
|-------------|----------|-------|----------|
| QuestReward_BoneTools_001 | 2800, 3200, 30 | Q2 | Bone Tool ×1 |
| QuestReward_BoneTools_002 | 2850, 3250, 30 | Q2 | Bone Tool ×1 |
| QuestReward_Flint_001 | -1000, -600, 30 | Q3 | Flint ×2 |
| QuestReward_Flint_002 | -950, -650, 30 | Q3 | Flint ×1 |
| QuestReward_Hide_001 | 1400, -3500, 30 | Q4 | Hide Scraps ×1 |
| QuestReward_TRexNest_Obs | 200, 500, 80 | Q1 | Map knowledge |

### NPC Quest Givers (PointLights — warm yellow)
| Actor Label | Location | Role |
|-------------|----------|------|
| NPC_Elder_QuestGiver_001 | -200, -300, 50 | Gives Q1 + Q3 |
| NPC_Scout_Tracker_001 | -350, -200, 50 | Gives Q2 |

---

## Crowd Event → Quest Trigger Matrix

| Crowd Event (Agent #13) | Quest Triggered | Trigger Condition |
|-------------------------|-----------------|-------------------|
| HERD_SCATTER | Q1: Follow the Herd | Triceratops scatter from Herd_Tricera_Scatter_001 |
| SCAVENGER_CONVERGE | Q2: Scavenger's Trail | Ptero + ground scavengers at TriggerZone_Feeding_001 |
| MIGRATION_ACTIVE | Q3: The Great Migration | Migration_Brachio_Path_001–005 active |
| CROWD_SILENCE | Q4: The Silence | All ambient actors within 2000u go inactive |
| FLOCK_SCATTER | Q2 bonus | Aerial flock scatter = predator near kill site |

---

## Handoff to Agent #15 — Narrative & Dialogue Agent

### Dialogue needed:
1. **Elder NPC** — Quest 1 briefing (voice line recorded: ~10s)
2. **Scout NPC** — Quest 2 briefing (voice line recorded: ~12s)
3. **Elder NPC** — Quest 3 briefing (migration lore)
4. **No NPC** — Quest 4 is environmental (no dialogue needed)
5. **Quest completion lines** — Elder reacts to player returning with knowledge/resources

### Lore hooks for each quest:
- Q1: Elder knows TRex territory from 20 years of observation — this is how the tribe survives
- Q2: Scavenging after apex predators is the tribe's primary tool-acquisition strategy
- Q3: The migration happens every dry season — the tribe times their own movements around it
- Q4: "When the birds stop singing, you stop moving." — tribal survival rule

### Story integration:
- All 4 quests feed into the main survival arc: player learns to READ the ecosystem
- Completing all 4 unlocks the "Ecosystem Reader" knowledge state — player can predict animal movements
- This is the foundation for the main story quest: "The Crossing" (cross the entire map using ecosystem knowledge)
