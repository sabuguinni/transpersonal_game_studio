# Quest Design Log — PROD_CYCLE_AUTO_20260704_001

**Agent:** #14 — Quest & Mission Designer  
**Cycle:** PROD_CYCLE_AUTO_20260704_001  
**Hub Reference:** X=2100, Y=2400 (hero screenshot clearing)

---

## Quest Infrastructure Placed This Cycle

### Quest 1 — "Observe the Herd"
**Type:** Exploration / Observation  
**Trigger:** `QuestTrigger_ObserveHerd_001` @ (1750, 2100, 50) — scale 8×8×3  
**NPC Briefer:** `NPC_Elder_QuestGiver_001` @ (1600, 2050, 50)  
**Waypoints:** 3 observation posts ringing the Trike herd perimeter  
- `QuestWP_ObservePost_001` @ (1650, 1950, 80) — NW ridge  
- `QuestWP_ObservePost_002` @ (2000, 1850, 80) — N ridge  
- `QuestWP_ObservePost_003` @ (2200, 2050, 80) — NE ridge  

**Objective:** Reach all 3 observation posts without alerting the Triceratops herd.  
**Reward:** Herd behavior knowledge (unlocks "Juvenile Separation Hunt")  
**Voice Line:** `Elder_NPC_QuestBriefing_HerdObserve.mp3` (~14s)  
> *"The great horned ones graze to the north. Do not startle them. Watch from the ridge — count their young, note which direction the bull faces at dusk. This knowledge will keep us alive through the cold season."*

---

### Quest 2 — "Cross the Feeding Ground"
**Type:** Stealth / Survival  
**Trigger:** `QuestTrigger_CrossFeeding_001` @ (1950, 2300, 50) — scale 12×6×3  
**Objective:** Cross through the active Triceratops feeding zone without being charged.  
**Mechanic:** Player must time movement between herd members, crouch, avoid direct line-of-sight to bull.  
**Reward:** Access to the river resource zone beyond the herd.  
**Design Note:** No NPC briefer — discovered organically. The trigger fires when the player first approaches the herd perimeter.

---

### Quest 3 — "Juvenile Separation Hunt"
**Type:** Hunt / Survival  
**Trigger:** `QuestTrigger_JuvenileHunt_001` @ (1500, 2500, 50) — scale 6×6×3  
**NPC Briefer:** `NPC_Elder_QuestGiver_001` (same Elder, second dialogue)  
**Target:** `Trike_Savana_Juvenile_001` @ (1520, 2480, 50)  
**Resource Pickups (track evidence):**  
- `QuestObj_DinoTrack_001` @ (1480, 2520, 52)  
- `QuestObj_DinoTrack_002` @ (1560, 2460, 52)  
- `QuestObj_ShedHorn_001` @ (1620, 2550, 52)  
- `QuestObj_HerdDung_001` @ (1700, 2400, 52)  

**Objective:** Track the juvenile Triceratops using environmental clues, then execute a clean ambush strike.  
**Mechanic:** Collect 3 of 4 track evidence pickups → juvenile location revealed → approach from downwind → attack.  
**Reward:** Meat supply (survival stat boost), horn material (crafting unlock)  
**Voice Line:** `Elder_NPC_QuestBriefing_JuvenileHunt.mp3` (~11s)  
> *"Stay low. Move slow. If the young one breaks from the herd — that is your moment. One clean strike, no hesitation. The tribe eats tonight, or it does not eat at all."*

---

### Quest 4 — "Track the Sauropods"
**Type:** Exploration / Tracking  
**Trigger:** `QuestTrigger_TrackSauropods_001` @ (2750, 3100, 50) — scale 10×10×3  
**Target Herd:** `Brach_Savana_001..005` (5 Brachiosaurus, SE of hub)  
**Objective:** Follow the Brachiosaurus herd migration path to discover a new resource-rich area.  
**Mechanic:** Player follows herd movement direction, finds new biome edge with rare materials.  
**Reward:** Map expansion, rare plant materials for advanced crafting.  
**Design Note:** Prerequisite — complete "Observe the Herd" first. Unlocks after player demonstrates herd-reading skill.

---

## Actor Reference Table

| Actor Label | Type | Location | Quest |
|---|---|---|---|
| `QuestTrigger_ObserveHerd_001` | TriggerBox | (1750, 2100, 50) | Quest 1 |
| `QuestTrigger_CrossFeeding_001` | TriggerBox | (1950, 2300, 50) | Quest 2 |
| `QuestTrigger_JuvenileHunt_001` | TriggerBox | (1500, 2500, 50) | Quest 3 |
| `QuestTrigger_TrackSauropods_001` | TriggerBox | (2750, 3100, 50) | Quest 4 |
| `NPC_Elder_QuestGiver_001` | StaticMeshActor (placeholder) | (1600, 2050, 50) | Q1, Q3 |
| `Trike_Savana_Juvenile_001` | StaticMeshActor (placeholder) | (1520, 2480, 50) | Quest 3 |
| `QuestObj_DinoTrack_001` | StaticMeshActor (sphere) | (1480, 2520, 52) | Quest 3 |
| `QuestObj_DinoTrack_002` | StaticMeshActor (sphere) | (1560, 2460, 52) | Quest 3 |
| `QuestObj_ShedHorn_001` | StaticMeshActor (sphere) | (1620, 2550, 52) | Quest 3 |
| `QuestObj_HerdDung_001` | StaticMeshActor (sphere) | (1700, 2400, 52) | Quest 3 |
| `QuestWP_ObservePost_001` | StaticMeshActor (cube) | (1650, 1950, 80) | Quest 1 |
| `QuestWP_ObservePost_002` | StaticMeshActor (cube) | (2000, 1850, 80) | Quest 1 |
| `QuestWP_ObservePost_003` | StaticMeshActor (cube) | (2200, 2050, 80) | Quest 1 |

---

## Voice Lines Produced

| File | Character | Duration | Quest |
|---|---|---|---|
| `Elder_NPC_QuestBriefing_HerdObserve.mp3` | Elder NPC | ~14s | Quest 1 |
| `Elder_NPC_QuestBriefing_JuvenileHunt.mp3` | Elder NPC | ~11s | Quest 3 |

---

## Design Rationale

All 4 quests are **survival-grounded** — no spiritual content, no mysticism. Every objective is physically achievable by a primitive human:
- Observation (stealth, patience)
- Navigation (crossing dangerous terrain)
- Hunting (tracking, ambush, timing)
- Exploration (following animal migration)

The **Elder NPC** is a practical tribal knowledge-keeper, not a shaman. Dialogue is direct, tactical, and survival-focused.

Quest chain order: Q1 → Q2 → Q3 → Q4 (each unlocks the next via knowledge/access gates).

---

## [NEXT] Agent #15 — Narrative & Dialogue Agent

Quest infrastructure is in place. Narrative agent should:
1. Write **full dialogue trees** for `NPC_Elder_QuestGiver_001` (Q1 briefing, Q3 briefing, quest completion lines)
2. Define **lore context** for why the tribe needs herd observation data (seasonal migration? predator pressure?)
3. Write **journal entries** the player receives after completing each quest (survival knowledge unlocked)
4. Reference actor labels directly — all quest actors are stable and persistent in MinPlayableMap
5. The Elder's voice is established: direct, experienced, no-nonsense, survival-focused

Herd actors available for narrative reference:
- `Trike_Savana_001` — alpha bull (dominant, territorial)
- `Trike_Savana_Juvenile_001` — young stray (quest target)
- `Brach_Savana_001` — dominant female sauropod (migration leader)
