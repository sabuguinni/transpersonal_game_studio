# Quest System — Agent #14 Production Cycle 004
**PROD_CYCLE_AUTO_20260618_004**

---

## Quest Architecture — Cycle 004 Summary

This cycle delivers 4 fully-mapped quests integrated with Agent #12 (Combat AI) and Agent #13 (Crowd Simulation) infrastructure. All quest actors placed in MinPlayableMap.

---

## Quest Givers (4 actors)

| Label | Position | Quest | Light Color |
|---|---|---|---|
| `QuestGiver_Elder_Camp` | (0, 0, 120) | Main Hub / all quests | GOLD |
| `QuestGiver_Hunter_Riverbank` | (1200, 800, 100) | Protect the Water Gatherers | CYAN |
| `QuestGiver_Scout_Hilltop` | (-800, 1500, 250) | Survive the Scatter | ORANGE |
| `QuestGiver_Tracker_Forest` | (1800, -1200, 110) | Hunt the Raptor Pack | PURPLE |

---

## Active Quests

### QUEST 1 — "Protect the Water Gatherers"
**Type:** Escort  
**Giver:** `QuestGiver_Hunter_Riverbank`  
**Trigger:** Talk to Hunter at riverbank  

**Objectives:**
1. Meet the water gathering group at `QuestObj_Water_Start` (1200, 800)
2. Escort them through `QuestObj_Water_Mid` (1400, 200) — Raptor cautious zone
3. Reach river destination at `QuestObj_Water_End` (1600, -400)

**Failure Condition:** Any NPC killed in `CrowdCautious_RaptorZone_*` orange zone (Agent #13)  
**Reward:** Crafting materials (flint, animal hide)  
**Integration:** Uses `CrowdFlow_Water_001→004` route (Agent #13)

**Voice Line (NPC):**  
> *"The water gatherers need protection. Three of our people went to the river at dawn and have not returned. The raptors have been seen near the eastern bank. Find them. Bring them back. We cannot lose more hunters before the cold season."*  
> Audio: `tts/1781762448958_QuestGiver_Hunter_Riverbank.mp3`

---

### QUEST 2 — "Survive the Scatter"
**Type:** Survival / Escape  
**Giver:** `QuestGiver_Scout_Hilltop`  
**Trigger:** Crowd scatter event activates (T-Rex enters camp radius)  

**Objectives:**
1. Scatter triggered at `QuestObj_Scatter_Trigger` (0, 0) — camp center
2. Navigate through panicking crowd WITHOUT following them into open plains
3. Reach `QuestObj_Scatter_Goal` (-800, 1500, 350) — hilltop safe zone

**Failure Condition:** Player enters open plains during scatter (outside safe corridors)  
**Time Limit:** 90 seconds  
**Reward:** Scout teaches player new tracking skill  
**Integration:** Uses `CrowdScatter_Escape_01–08` waypoints (Agent #13) + `CombatZone_Camp_DinoThreat` (Agent #12)

**Voice Line (NPC):**  
> *"Listen carefully. When the great lizard comes, you do not fight. You run. Get to high ground — the hill to the north. The crowd will scatter. Do not follow them into the open plains. The hill. Remember it. Go now."*  
> Audio: `tts/1781762451858_QuestGiver_Scout_Hilltop.mp3`

---

### QUEST 3 — "Hunt the Raptor Pack"
**Type:** Hunt / Combat  
**Giver:** `QuestGiver_Tracker_Forest`  
**Trigger:** Talk to Tracker at forest edge  

**Objectives:**
1. Track and kill `QuestObj_Raptor_Alpha` at (-1500, -1000) — pack leader
2. Kill `QuestObj_Raptor_Beta` at (-1800, -800)
3. Kill `QuestObj_Raptor_Gamma` at (-1200, -1300)

**Mechanics:**
- Raptors are in `CrowdCautious_RaptorZone_*` orange zone (Agent #13)
- Crowd NPCs flee this zone — player must enter alone
- Raptors use flanking AI from Agent #12 combat system

**Failure Condition:** Player dies  
**Reward:** Raptor claws (rare crafting material for advanced weapons)  
**Integration:** Uses `CombatZone_*` actors from Agent #12

---

### QUEST 4 — "Emergency Escape" (Triggered Quest)
**Type:** Emergency / Timed  
**Giver:** Automatic trigger (no giver NPC)  
**Trigger:** T-Rex enters camp — automatic activation  

**Objectives:**
1. Enter river escape corridor at `QuestObj_Escape_Entry` (2000, 500)
2. Follow `CrowdEscape_River_Corridor_*` lights (Agent #13)
3. Exit at `QuestObj_Escape_Exit` (2500, -200)

**Integration:** Uses `CombatRetreat_River_*` (Agent #12) + `CrowdEscape_River_Corridor_*` (Agent #13)  
**Reward:** Survival — quest completes automatically on exit

---

## Quest Objective Markers (10 actors)

| Label | Position | Quest | Color |
|---|---|---|---|
| `QuestObj_Water_Start` | (1200, 800, 300) | Protect Gatherers | CYAN |
| `QuestObj_Water_Mid` | (1400, 200, 250) | Protect Gatherers | CYAN |
| `QuestObj_Water_End` | (1600, -400, 200) | Protect Gatherers | CYAN |
| `QuestObj_Scatter_Trigger` | (0, 0, 150) | Survive Scatter | ORANGE |
| `QuestObj_Scatter_Goal` | (-800, 1500, 350) | Survive Scatter | GREEN |
| `QuestObj_Raptor_Alpha` | (-1500, -1000, 120) | Hunt Raptors | RED |
| `QuestObj_Raptor_Beta` | (-1800, -800, 110) | Hunt Raptors | RED |
| `QuestObj_Raptor_Gamma` | (-1200, -1300, 130) | Hunt Raptors | RED |
| `QuestObj_Escape_Entry` | (2000, 500, 100) | Emergency Escape | BLUE |
| `QuestObj_Escape_Exit` | (2500, -200, 100) | Emergency Escape | GREEN |

---

## Voice Lines Produced

| File | Character | Quest | Duration |
|---|---|---|---|
| `tts/1781762448958_QuestGiver_Hunter_Riverbank.mp3` | Hunter (Riverbank) | Protect Water Gatherers | ~16s |
| `tts/1781762451858_QuestGiver_Scout_Hilltop.mp3` | Scout (Hilltop) | Survive the Scatter | ~15s |

---

## Integration Map

```
Agent #12 (Combat AI)          Agent #13 (Crowd Sim)          Agent #14 (Quest)
─────────────────────          ─────────────────────          ──────────────────
CombatZone_Camp_DinoThreat ──► QuestObj_Scatter_Trigger       Quest: Survive Scatter
CombatRetreat_River_* ───────► QuestObj_Escape_Entry/Exit     Quest: Emergency Escape
CombatZone_RaptorAmbush ─────► QuestObj_Raptor_Alpha/Beta/Gamma Quest: Hunt Raptors
                               CrowdFlow_Water_001→004 ──────► Quest: Protect Gatherers
                               CrowdSafe_Riverbank_001 ──────► QuestGiver_Hunter_Riverbank
                               CrowdSafe_Hilltop_001 ────────► QuestGiver_Scout_Hilltop
```

---

## Next Agent (#15 — Narrative & Dialogue)

1. **Expand quest dialogue** — each quest giver needs 3-5 dialogue lines (greeting, active, complete, fail)
2. **Story context** — why is the raptor pack near camp? What happened to the missing gatherers?
3. **Elder's narrative** — `QuestGiver_Elder_Camp` should deliver the main story exposition
4. **Lore integration** — connect "Survive the Scatter" to the broader T-Rex territorial story
5. **Audio assets** — 2 voice lines produced this cycle; need 6 more (Elder + Tracker + quest completion lines)
6. **Quest completion dialogue** — what NPCs say when player succeeds/fails each quest
