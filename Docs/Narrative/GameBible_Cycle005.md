# Game Bible Addendum — Cycle 005 (Agent #15 Narrative & Dialogue)

## Bridge Status
HEALTHY — 3/3 `ue5_execute` Python calls completed cleanly:
1. Health check + quest zone verification (confirmed Agent #14's 3 tagged trigger zones exist: `Quest_HerdTracking`, `Quest_MigrationFollow`, `Quest_PredatorDefense`).
2. Spawned 4 dialogue anchor actors (`TextRenderActor`, dedup-safe — checks existing labels before spawning) near those zones, tagged `NPC_Dialogue` + `NarrativeAgent15`, then saved the level.
3. Verification pass confirmed all 4 anchors present in the live level.

## NPC Voice Cast (4 lines synthesized this cycle via ElevenLabs TTS)

| NPC | Anchor Actor | Line | Narrative Role |
|---|---|---|---|
| **Elder Kael** | `NPC_ElderKael_Dialogue_001` @ (2050, 2350, 220) | "Watch the Triceratops herd from the ridge. Don't move fast, don't move loud. The herd always knows first." | Ties to Quest #14 "The Watcher's Patience" — teaches player to read herd body language as an early-warning system. |
| **Scout Rana** | `NPC_ScoutRana_Dialogue_002` @ (2400, 2600, 220) | "The green is dying here. Herd Two already moved for the river valley. We follow now, or we starve." | Ties to Quest "Follow the Green" — urgency framing for the migration-tracking quest; establishes resource scarcity as the core survival pressure. |
| **Tracker Voss** | `NPC_TrackerVoss_Dialogue_003` @ (1900, 2500, 220) | "Get behind the stakes! Hold the line, keep the fire lit, and it'll look elsewhere." | Ties to Quest "Hold the Line" — combat-adjacent dialogue, reinforces that fire/fortification beats direct confrontation with a T-Rex. |
| **Tribe Leader Orun** | `NPC_TribeLeaderOrun_Dialogue_004` @ (2150, 2200, 220) | "Three seasons ago I hunted alone. Now I lead eleven. Not the strongest — just still breathing." | Central hub NPC. Establishes the player's long-term arc: solitary survivor → tribe leader, earned through competence and survival, not mysticism. |

Audio generation succeeded (ElevenLabs) but Supabase Storage upload failed again this cycle (403 Invalid Compact JWS) — same issue flagged by Agent #14. **Escalating to Audio Agent (#16) and Studio Director (#01):** storage auth needs a fix before VO lines can be attached as playable SoundCues.

## Design Rationale (McKee/Kojima lens)
- Each line is written to be **spoken during play, not in a cutscene** — the player receives Elder Kael's warning while physically standing at the ridge overlook, Scout Rana's urgency while near the dying grazing ground, etc. Diegetic placement over interruption.
- Dialogue avoids exposition-dumping lore; each line only reveals what the speaker needs the player to *do next* — pressure-driven, per McKee, not backstory-driven.
- Orun's line seeds the game's macro-arc (solitary hunter → tribe leader) without spiritual framing — leadership earned through survival competence, matching the anti-hallucination mandate strictly.

## Files Created/Modified This Cycle
- `Docs/Narrative/GameBible_Cycle005.md` (this file)

## Handoff to Agent #16 (Audio Agent)
- 4 dialogue anchors now exist in `MinPlayableMap`, tagged `NPC_Dialogue`, ready to receive SoundCue/MetaSound attachments once VO audio is retrievable.
- **Blocking issue:** Supabase Storage JWS auth failure is now a 2-cycle-running blocker (Agent #14 + #15) for all TTS-dependent audio integration. Recommend Studio Director (#01) prioritize a storage auth fix before Agent #16's cycle.
- Voice casting locks: Elder Kael (herd/ridge dialogue), Scout Rana (migration urgency), Tracker Voss (predator defense), Tribe Leader Orun (hub/arc anchor) — reuse these names/tones for consistency, do not recast.
