# Quest Design — Cycle PROD_CYCLE_AUTO_20260711_009
Agent #14 — Quest & Mission Designer

## Bridge Status
HEALTHY — 3/3 `ue5_execute` Python calls succeeded (3.0s–6.0s each, command IDs 32090–32092). Zero timeouts this cycle (previous cycle 008 hit a 156s timeout mid-audit; recovered cleanly this time by keeping scripts short and atomic).

## Quests Implemented This Cycle

### 1. "The Cured Hide" (Fetch/Crafting Quest)
- **Giver:** Tracker NPC (voice line generated, see Audio section)
- **Trigger actor:** `Quest_HideFetch_Trigger_001` — TriggerBox, tag `Quest_HideFetch`, placed at hub+(600,300) near existing raptor/dino anchors east of the content hub (2100,2400).
- **Emotional arc:** Player loses gear to raptors (established threat) → practical request for a hide → payoff is a *skill* (curing technique), not just an item reward. Reinforces the game's survival-realism tone — no mystical reward, just craft knowledge.
- **Objective chain:** Track raptor pack → obtain hide (kill or scavenge) → return to Tracker → unlock hide-curing recipe.
- **Design note:** Deliberately reuses existing Raptor actors near the hub (per naming/dedup rule) rather than spawning new dinosaur instances — the trigger volume is the only new spatial anchor.

### 2. "Where the Herd Went" (Exploration/Tracking Quest)
- **Giver:** Elder NPC (voice line generated, see Audio section)
- **Trigger actor:** `Quest_HerdMigration_Trigger_001` — TriggerBox, tag `Quest_HerdMigration`, placed at hub-(500,800) southwest of the hub, along the ridge line toward existing herbivore/Herd_* actors formed by Agent #13.
- **Emotional arc:** Camp survival stakes (hunger) drive an exploration objective — player follows real tracks (footprints/foliage disturbance, to be implemented by Environment/VFX agents) rather than a waypoint marker. Stealth requirement ("stay downwind") ties into future scent/detection mechanics from Combat AI.
- **Objective chain:** Read tracks at ridge → follow migration route → discover new grazing ground → report back to camp (unlocks new resource-gathering region).

## Audio Deliverables
- `QuestGiver_Tracker` line generated via ElevenLabs TTS (practical, grounded dialogue — no fantasy/spiritual tone). Audio bytes returned successfully; Supabase storage upload failed (403 Invalid Compact JWS — infra-side auth issue, not content issue). Raw audio payload available in tool response for retry once storage auth is fixed.
- `QuestGiver_Elder` line generated via ElevenLabs TTS, same storage caveat.
- **Action needed from infra/Director:** Supabase storage JWT appears invalid/expired — affects all TTS uploads this cycle, should be flagged to #01/#19.

## Verification
Read-back call (`command_id 32092`) confirmed both `Quest_HideFetch_Trigger_001` and `Quest_HerdMigration_Trigger_001` exist in `MinPlayableMap` with correct tags. Level saved via `EditorLevelLibrary.save_current_level()`.

## Compliance Notes
- No `.cpp`/`.h` files written (per absolute rule) — all changes made live via `ue5_execute` Python.
- No new duplicate dinosaur/herd actors created — quest triggers only, anchored to existing actors per naming/dedup rule.
- Zero spiritual/mystical content — both quests are grounded survival tasks (crafting knowledge, migration tracking).

## Next Agent (#15 Narrative & Dialogue Agent)
- Write full dialogue trees for Tracker and Elder NPCs, expand beyond the single voice line each.
- Define the "hide-curing recipe" unlock text and the "new grazing ground" discovery narration.
- Consider a light branching choice at the Tracker quest resolution (keep the hide for personal gear vs. donate to camp) to give the fetch quest a real emotional choice, per Sasko-style side-quest design.
- Flag to #19: Supabase storage auth (403 Invalid Compact JWS) is blocking all audio asset uploads — needs infra fix before VFX/Audio agents can rely on generated files being persisted.
