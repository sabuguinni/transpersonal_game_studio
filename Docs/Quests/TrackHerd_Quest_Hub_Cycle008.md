
# Quest: "Reading the Herd" — Content Hub (X=2100, Y=2400)

## Design Philosophy
Per Sasko/Blow influence: this is a *low-stakes observation quest* that teaches the player
core survival literacy (herd behavior, wind direction, safe distances) before combat or
hunting quests are introduced. It has an emotional arc — curiosity → tension (getting close)
→ quiet accomplishment (returning knowledge to camp) — independent of any spiritual framing.
No mysticism, no "beast whisperers." This is field biology and woodcraft, the kind a
National-Geographic-style documentary would show a tracker doing.

## Quest Chain (3 stages, engine-native TriggerBox actors — no custom C++ required)

| Stage | Actor Label | Location (approx) | Tags | Player Goal |
|---|---|---|---|---|
| 1 | `Quest_TrackHerd_Observe_001` | (2450, 2600, 100) | `Quest_TrackHerd`, `Quest_Type_Observation`, `Quest_Stage_001` | Spot the herd from ridge distance without triggering flight/aggro |
| 2 | `Quest_TrackHerd_Approach_002` | (2250, 2100, 100) | `Quest_TrackHerd`, `Quest_Type_Observation`, `Quest_Stage_002` | Approach downwind through vegetation cover to study herd up close |
| 3 | `Quest_TrackHerd_Report_003` | (1700, 2500, 100) | `Quest_TrackHerd`, `Quest_Type_Observation`, `Quest_Stage_003` | Return toward camp marker to "report" findings (fires quest complete) |

## Target Actors (Cross-referenced from Agent #13 — Crowd Simulation)
- Quest objective actors are the existing herd tagged `Crowd_HerdID_Hub01` /
  `Crowd_HerdMember` (Triceratops/Brachiosaurus family). **No new dinosaurs spawned** —
  reused per `hugo_naming_dedup_v2` anti-duplication rule.
- Herd members also carry `Combat_NonAggressive` (Agent #12), confirming this is
  appropriate low-risk early-game content.

## Data-Driven Quest Definition
See `Docs/Quests/QuestData_TrackHerd.json` for the structured quest data (stage order,
trigger labels, completion conditions) — intended to be consumed later by a QuestManager
subsystem once C++ compilation is restored in this environment. Currently implemented as
pure Actor Tags + TriggerBox placement via `ue5_execute`, since this headless editor does
not recompile C++ (per `hugo_no_cpp_h_v2`).

## Verification (this cycle)
- 3 `TriggerBox` actors spawned via `unreal.EditorLevelLibrary.spawn_actor_from_class`
  (engine-native class `/Script/Engine.TriggerBox`, no custom Actor subclass needed).
- Re-scanned level actors post-spawn: all 3 `Quest_TrackHerd_*` labels confirmed present
  with correct tags.
- Level saved via `unreal.EditorLevelLibrary.save_current_level()`.

## Voice Lines Generated (NPC: "Tracker Elder")
1. Pre-quest briefing line — cautions player on approach/downwind technique, ties directly
   to Stage 1→2 gameplay (do not spook the herd).
2. Post-quest debrief line — reinforces the "knowledge over strength" survival theme and
   closes the emotional arc (curiosity → competence).
   NOTE: TTS generation succeeded server-side (audio synthesized, ~18-22s duration) but
   Supabase storage upload returned `403 Invalid Compact JWS` — audio bytes were produced
   but the public URL was not persisted this cycle. Flagging for Audio Agent (#16) to
   re-run storage upload once the Supabase auth token is refreshed.

## Handoff to Agent #15 (Narrative & Dialogue Agent)
- The "Tracker Elder" NPC voice lines above need a matching character bio + full dialogue
  tree in the Narrative Bible. Suggested role: veteran camp scout, practical and terse,
  no mystical framing — pure survival pragmatism.
- Quest text/UI strings (objective markers, journal entries) should be written by #15 using
  the 3-stage structure above as the mechanical skeleton.

## Handoff to Agent #16 (Audio Agent)
- Re-attempt Supabase upload for the 2 Tracker Elder lines (synthesis succeeded, storage
  failed with JWT error). Do not regenerate from scratch — audio bytes exist server-side.

## Next Quest Cycle Should Build
- Hunting/crafting quest chain building on Stage 3 completion (player has now safely
  studied the herd — next logical quest: gather stone/stick resources near hub for
  Stone Axe crafting, tying into Agent #14's next crafting-focused directive).
