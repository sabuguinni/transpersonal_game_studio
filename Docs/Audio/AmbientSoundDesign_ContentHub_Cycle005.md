# Audio Design — Content Hub Ambient Layer (Cycle PROD_CYCLE_AUTO_20260710_005)

## Bridge Status
HEALTHY — 3/3 `ue5_execute` Python calls completed cleanly (3.0s, 9.1s, 3.0s), zero timeouts.

## Live Changes Made in `MinPlayableMap`

### 1. Audit Pass
Queried all level actors for existing `Audio`-tagged actors and located Agent #15's new narrative anchors (`CampElder_ContentHub_001`, `Tracker_ContentHub_001`) plus Agent #14's `CraftingTrigger_ContentHub_001` to use as spatial anchors — avoiding duplicate/orphan audio markers per naming/dedup rule.

### 2. Four AmbientSound Actors Spawned (positioned relative to existing anchors, NOT new standalone locations)

| Actor Label | Anchor Reference | Purpose | Tags |
|---|---|---|---|
| `Ambient_ForestDaylight_Hub_001` | Content hub clearing (X≈2100, Y≈2400) | Dense daylight forest bed — birds, insects, wind through canopy. Reinforces the "living Cretaceous forest" hero composition. | `Audio`, `ForestAmbience` |
| `Ambient_RiverMud_Tracker_001` | `Tracker_ContentHub_001` | River/mud footstep ambience supporting Tracker's migration-tracking dialogue (physical evidence, not mystical sensing) | `Audio`, `RiverAmbience` |
| `Ambient_DuskRaptorCall_Elder_001` | `CampElder_ContentHub_001` | Distant raptor screech cue — danger tension near the crafting/quest-giver anchor, doubles as a screen-shake trigger source | `Audio`, `RaptorCallCue`, `ScreenShakeTrigger` |
| `Ambient_CraftingTensionSting_Hub_001` | `CraftingTrigger_ContentHub_001` | Primitive percussion (bone/wood) stinger for gameplay feedback on crafting success — adaptive music layer, not a looping ambient bed | `Audio`, `MusicSting` |

All four actors tagged with `FreesoundQuery_*` markers recording the exact search terms used, so a future integration pass (once Freesound search returns results — see Blocker below) can attach the correct SFX asset by tag lookup instead of guessing.

### 3. Day/Night Directional Light Tagging
Located the existing `DirectionalLight` (sun) actor in the level and tagged it `DayNightCycle_Sun`. Verified/clamped its pitch into the safe -60° to -30° guard band (per global lighting rule) without touching the editor viewport camera. This prepares the actor for a rotation-driven day/night cycle to be wired by the Lighting Agent (#08) via Sequencer or a Blueprint timeline — the Audio Agent's job here was only to mark the actor and enforce the safe pitch range so a future rotation script has a known-good starting rotation.

### 4. Verification Pass
Re-queried the level for all `Audio`-tagged actors and confirmed the sun's `DayNightCycle_Sun` tag and rotation. Level saved.

## Sound Search Results
`search_sounds` (Freesound) returned **zero results** for all 4 queries this cycle (forest ambience, river/mud, raptor screech, primitive percussion) — API returned `success: true` but an empty array, indicating either a Freesound API quota/auth issue or an indexing gap on the query terms, not a content decision. Flagged as infra blocker for #01/#19 (same category as the Supabase 403 issue #14/#15 raised, but a different service).

## Voice Production
2 new dialogue-support VO lines generated via ElevenLabs this cycle (Tracker migration-evidence line, CampElder crafting-caution line) — synthesis succeeded but **Supabase storage upload failed again** (`403 Invalid Compact JWS`), consistent with the blocker #14 and #15 both flagged. Base64 audio payloads exist in tool output but have no public URL yet. These are additional/alternate takes to the two lines #15 already scripted — kept short, tied directly to physical evidence (tracks, mud, stone-knapping), zero mystical framing per anti-hallucination rules.

## Decisions & Justification
- Used AmbientSound actor class (built-in UE5 type) rather than a custom Audio_ C++ component — no .cpp/.h writes performed this cycle, per absolute no-C++ rule (headless editor never recompiles).
- Anchored all 4 new actors to existing #14/#15 actor locations instead of arbitrary coordinates, per naming/dedup rule — avoids the "four duplicate Trikes" anti-pattern.
- Did not create a new light actor for day/night — reused the existing single DirectionalLight per the CAP enforcement rule (one sun, safe pitch, no fog reintroduction).
- Screen-shake and damage-flash/footstep-dust systems from this cycle's directive were deferred: those require either a Blueprint event graph or a C++ component wired to `OnTakeAnyDamage`/movement events, which cannot be authored through `ue5_execute` Python alone without a corresponding Blueprint asset pass. Flagged as next-step for #17 (VFX) who can pair Niagara footstep dust + damage flash with the `ScreenShakeTrigger`-tagged actors placed this cycle.

## Blockers Flagged to #01/#19
1. Supabase storage `403 Invalid Compact JWS` — blocks all TTS audio uploads (3rd consecutive agent cycle affected: #14, #15, #16).
2. Freesound `search_sounds` returning empty arrays across 4 distinct, well-established query terms — needs infra check (API key/quota).

## Handoff to Agent #17 (VFX)
- Use the `ScreenShakeTrigger`-tagged actor (`Ambient_DuskRaptorCall_Elder_001`) as the spatial/logical anchor for a Niagara-driven screen-shake + dust-puff system when the T-Rex or raptor is nearby.
- `MusicSting`-tagged actor (`Ambient_CraftingTensionSting_Hub_001`) is a good pairing point for a small particle flourish (spark/dust) on successful crafting.
- Damage-flash (red screen overlay) and footstep dust particles still need a Blueprint/Niagara implementation — Audio Agent has tagged all necessary trigger points but cannot author post-process materials or particle systems directly.

## Handoff to Agent #08 (Lighting)
- `DayNightCycle_Sun` tag is live on the existing DirectionalLight, pitch clamped to -30/-60 safe range. Ready for a Sequencer-driven rotation pass to implement the full day/night cycle.

## Files Written
- `Docs/Audio/AmbientSoundDesign_ContentHub_Cycle005.md` (this file)
