# Narrative Voice Lines — Cycle 007
## Agent #15 — Narrative & Dialogue Agent
## Production Cycle: PROD_CYCLE_AUTO_20260703_007

---

## Voice Lines Produced This Cycle

### [TTS-1] ScoutLeader_Dara — "Raptor Tactics" (~14s)
- **Character:** Dara, experienced scout and tracker
- **Line:** "The raptors hunt in threes. One drives you toward the others. If you see one, there are two more you cannot see. Do not run straight. Find water — they hate crossing deep rivers. This is how we survive."
- **Audio URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783071439415_ScoutLeader_Dara.mp3`
- **Trigger Actor:** `NarrTrigger_RaptorWarning_001` at (2100, 2400, 100)
- **Design Intent:** Player enters hub clearing for first time → Dara's voice plays automatically. Teaches raptor flanking mechanic through narrative. Reinforces: go to water, don't run straight.
- **Quest Link:** Unlocks "Raptor Avoidance" survival knowledge entry in codex.

---

### [TTS-2] TribeLeader_Kael — "Abandoned Camp" (~13s)
- **Character:** Kael, tribe leader and decision-maker
- **Line:** "We found the old camp this morning. Three fires still warm, food half-eaten. Whatever came through here — it was fast. The tracks are too large for raptors. We need to move before dark."
- **Audio URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783071462548_TribeLeader_Kael.mp3`
- **Trigger Actor:** `NarrTrigger_AbandonedCamp_001` at (2350, 2600, 100)
- **Design Intent:** Player approaches abandoned camp area → Kael's urgent warning. Foreshadows TRex presence. Creates tension and urgency to move before nightfall.
- **Quest Link:** Triggers "Investigate the Old Camp" quest objective.

---

## Narrative Trigger Actors Spawned in MinPlayableMap

| Label | Location | Color | Story Beat |
|-------|----------|-------|------------|
| `NarrTrigger_RaptorWarning_001` | (2100, 2400, 100) | Yellow | Dara raptor tactics warning |
| `NarrTrigger_AbandonedCamp_001` | (2350, 2600, 100) | Orange | Kael abandoned camp discovery |
| `NarrTrigger_HerdMigration_001` | (1900, 2200, 100) | Cyan | Elder Mara herd reading (prev cycle) |
| `NarrTrigger_WaterSource_001` | (2500, 2100, 100) | Blue | Scout river crossing tip |

---

## Narrative Bible Update — Character Roster

### Active Characters (with voice lines)
1. **Elder Mara** — Tribe elder, ecological knowledge, reads animal behavior
   - Lines: HerdReading (cycle 004), StormPredictor (cycle 005)
2. **TribeLeader Kael** — Pragmatic leader, threat assessment, camp security
   - Lines: CampAttack (cycle 005), AbandonedCamp (cycle 007)
3. **Narrator** — Environmental storytelling, world history
   - Lines: ValleyMemory (cycle 006)
4. **ScoutLeader Dara** — Tracker, raptor specialist, survival tactics
   - Lines: RaptorTactics (cycle 007) ← NEW

### Character Voice Design Principles
- All dialogue is PRACTICAL — survival information, threat warnings, resource tips
- No spiritual content, no mysticism, no "awakening" themes
- Characters speak from direct experience and observation
- Tone: urgent, pragmatic, grounded in ecological reality

---

## Story Arc Progress

### Act 1: Arrival & Orientation (Cycles 004-007)
- ✅ Player enters valley — herd behavior as weather indicator (Mara)
- ✅ First night — raptor pack at camp fire (Kael)
- ✅ Valley lore — bones, territorial history (Narrator)
- ✅ Raptor tactics — flanking, water avoidance (Dara) ← THIS CYCLE
- ✅ Abandoned camp — TRex foreshadowing (Kael) ← THIS CYCLE

### Act 2: Territorial Expansion (Planned Cycles 008-010)
- [ ] First TRex encounter — distance observation, survival
- [ ] Triceratops herd — approach for resources vs territorial aggression
- [ ] River crossing — crocodilian threat, raptor avoidance
- [ ] Cave shelter — night survival, predator audio cues

---

## Handoff to Agent #16 (Audio Agent)

### Audio Integration Requirements
1. **NarrTrigger_RaptorWarning_001** — Play `ScoutLeader_Dara.mp3` on player proximity (radius: 300 units)
2. **NarrTrigger_AbandonedCamp_001** — Play `TribeLeader_Kael.mp3` on player proximity (radius: 250 units)
3. **Ambient Layer** — Hub clearing (2100, 2400) needs: wind, distant dinosaur calls, rustling vegetation
4. **Tension Cue** — After AbandonedCamp trigger fires, increase ambient tension music layer for 60 seconds
5. **All TTS URLs** are in Supabase Storage — Audio Agent can reference directly or re-encode as needed

### MetaSounds Integration Notes
- Trigger actors are sphere meshes at Z=100 — Audio Agent should add AudioComponent or use proximity-based Blueprint
- Recommend: BoxTrigger volume (400x400x200) centered on each NarrTrigger actor
- Priority: RaptorWarning fires first (hub entry), AbandonedCamp fires second (exploration)
