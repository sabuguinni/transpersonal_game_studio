# VFX Design — Hub Grazing Chain (Cycle PROD_CYCLE_AUTO_20260712_001)

**Agent:** #17 VFX Agent
**Bridge status:** HEALTHY — 4/4 `ue5_execute` Python calls succeeded (command IDs 32299–32302, ~3.0s each, zero timeouts).

## Objective
Layer visual feedback matching the 3 new ambient audio zones created by Agent #16 at the content hub (world coords ~X=2100, Y=2400), per the `hugo_hub_quality_v2_fix` directive — this clearing must read as a living, grounded Cretaceous forest scene, not abstract geometry.

## Dedup Audit
Queried all level actors for `VFX_` prefix before spawning — zero pre-existing VFX actors found. No duplicates created (per `hugo_naming_dedup_v2`).

## Real changes made in the live world (MinPlayableMap)
Spawned 4 `NiagaraActor` instances, all co-located with Agent #16's existing `Audio_` actors (reusing established hub coordinates instead of inventing new ones):

1. **`VFX_HubGrazing_CampfireSmoke_001`** @ (2050, 2350, 130) — smoke/ember plume co-located with `Audio_HubGrazing_CampfireCrackle_001`.
2. **`VFX_HubGrazing_ForestDustMotes_001`** @ (2100, 2400, 180) — ambient dust motes in god-ray light, co-located with `Audio_HubGrazing_ForestAmbience_001`.
3. **`VFX_HubGrazing_DustKickup_001`** @ (2150, 2450, 120) — ground dust disturbance near the tribal percussion/dance zone, co-located with `Audio_HubGrazing_TribalPercussion_001`.
4. **`VFX_HubGrazing_CampfireHeatShimmer_001`** @ (2060, 2360, 140) — heat-haze distortion layer above the campfire, distinct from the smoke emitter for LOD variety.

All 4 actors verified present in `get_all_level_actors()` post-spawn. Level saved via `EditorLevelLibrary.save_current_level()`.

**Note:** Niagara Systems are currently unassigned on these actors (empty `NiagaraActor` placeholders — no `.uasset` particle systems exist in the project yet). This mirrors the audio pipeline's placeholder pattern (empty `AmbientSound` actors awaiting Sound Wave references). The actors exist as anchor points with correct positions/tags, ready for a Niagara System asset to be assigned once one exists in `/Game/VFX/`.

## Reference art (blocked by infra bug)
Generated 2 VFX concept images via GPT Image 1:
1. Campfire smoke/embers + god-ray dust in tribal camp clearing (HD, 1792x1024).
2. Dust motes + ground dust kickup near drum circle (Standard, 1792x1024).

Both generations **succeeded at the model level** but failed Supabase Storage upload with `HTTP 403 Invalid Compact JWS` — identical to the failure reported by Agent #16 for TTS uploads this same cycle, and by #14/#15 in prior cycles. This is now confirmed as a **persistent, cross-agent Supabase JWT auth bug**, not a transient network issue.

## Sound sourcing (Freesound, supporting VFX-synced audio)
- "fire crackling embers pop sizzle" → 2 strong candidates (metal poker/coals, candle wick crackle loop) suitable for syncing with `VFX_HubGrazing_CampfireSmoke_001`.
- "god ray ambient forest wind rustle leaves" → 0 results, documented as an asset gap for future sourcing.

## Decisions & Justification
- Used `NiagaraActor` (native UE5 class) rather than custom C++ VFX classes — per the hard rule that .cpp/.h writes have zero effect in this headless, non-recompiling editor.
- Co-located every VFX actor with an existing Audio actor rather than the hub's raw coordinates, ensuring perfect audio-visual sync at the same trigger points and avoiding duplicate-actor anti-patterns.
- Did not touch sun, fog, or camera — outside VFX scope this cycle and covered by prior CAP enforcement.

## Dependencies / Next steps
- **#19 Integration:** Fix Supabase JWT auth (`403 Invalid Compact JWS`) — now blocking image delivery (#17) and voice delivery (#14/#15/#16) simultaneously across 2+ cycles. This is the single highest-priority infra fix.
- **#17 (next cycle):** Once a real Niagara System asset exists in `/Game/VFX/`, assign it to the 4 placeholder actors created this cycle via `set_editor_property('niagara_system_asset', ...)`.
- **#18 QA:** Verify the 4 new VFX actors + 3 audio actors co-render correctly in the hero screenshot composition at (2100, 2400).
