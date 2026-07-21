# Audio Design — Cycle PROD_CYCLE_AUTO_20260712_006

## Bridge Status
UP. 2/2 `ue5_execute` Python calls succeeded (command IDs 32621, 32622), zero timeouts, zero camera manipulation, zero .cpp/.h writes (compliant with hugo_no_cpp_h_v2 / hugo_no_camera_v2 rules).

## Real Changes Made in the Live World (MinPlayableMap)
1. **Audit pass** — Enumerated all level actors, confirmed zero pre-existing `Audio_` actors, logged exact world coordinates of dinosaur actors (TRex/Raptor/Trike/Brachiosaurus) and #15's `Narr_*` dialogue TargetPoints.
2. **Spawned 3 `TargetPoint` ambient audio anchors** (native UE5 class, zero-cost, no C++ needed):
   - `Audio_TRexRoarZone_HubForest_001` — anchored 300u offset from the live TRex actor position (fallback: content-hub coords 2100,2400 if no TRex found). Tags: `Audio_ProximityRoar`, `SFX_TRexRoar`, `Radius_1500`.
   - `Audio_ForestAmbienceLoop_HubForest_001` — placed directly at the content-hub clearing (X=2100, Y=2400, Z=150), matching the hero-screenshot composition zone per brain memory `hugo_hub_quality_v2_fix`. Tags: `Audio_AmbientLoop`, `SFX_ForestBirdsWind`, `Radius_3000`.
   - `Audio_RiverStreamLoop_HubForest_001` — anchored 200u offset from #15's Gather-quest NPC marker (fallback: 1800,2600). Tags: `Audio_AmbientLoop`, `SFX_RiverRocks`, `Radius_1200`.
3. Level saved.

## Naming Compliance
Followed `Type_Bioma_NNN` convention per `hugo_naming_dedup_v2` (e.g. `Audio_TRexRoarZone_HubForest_001`). Audited for duplicates before spawning — none existed.

## Sourced Freesound Assets (search_sounds, 6 queries)
| Purpose | Asset | Freesound ID | Duration |
|---|---|---|---|
| Forest ambience (hub loop) | AMBForst_Forest Birds Wind 01 | 800712 | 30s |
| Forest ambience (alt/long) | pineforest 0-2bft winter LONG | 173971 | 1788s |
| River/stream loop | Forest river flows over rocks | 442478 | 116s |
| River/stream (alt) | rocky-stream calm | 579427 | 281s |
| T-Rex roar (proximity trigger) | Tyrannosaurus Rex - Roar | 607939 | 3.1s |
| T-Rex roar (variant) | T-Rex Mating roar 01/02 | 607938 / 607937 | ~3s each |

No results found for "dinosaur heavy footsteps low growl" or "primitive percussion bone drum tribal" or "wood stone crafting hit percussion impact" — flagged for next cycle retry with alternate query terms (e.g. "heavy footstep thud", "hand drum skin", "flint knapping stone").

## Voice Lines (text_to_speech, 2x)
- Hunt Elder herd-shift line (~11s) — generated OK, hit the known Supabase 403 "Invalid Compact JWS" storage failure (systemic infra issue flagged by #14/#15 last cycle, not per-agent). Audio payload exists in response but no persisted public URL.
- Hub-intro Narrator line (~12s) — same Supabase storage failure.

## MetaSounds Implementation Notes (for next cycle / #17 VFX handoff)
- `Audio_TRexRoarZone_HubForest_001` should drive a MetaSound Source with distance-based volume attenuation (Radius_1500 tag = trigger falloff distance) plus a **screen-shake** trigger (per Agent #16 directive: T-Rex proximity screen shake) — recommend a Blueprint `ActorComponent` listening for `OnAudioComponentFinished`/proximity overlap on this TargetPoint to call `PlayerCameraManager.StartCameraShake` at low intensity (avoid heavy footstep-only shake; combine with roar SFX for game feel).
- `Audio_ForestAmbienceLoop_HubForest_001` and `Audio_RiverStreamLoop_HubForest_001` are looping ambient beds — recommend UE5 `AmbientSound` actor conversion once MetaSounds patch is authored (currently TargetPoint markers only, since C++ ambient actor classes cannot compile in this headless editor).

## Known Limitation (Systemic, Not Per-Agent)
Supabase JWT signing (403 Invalid Compact JWS) continues to block audio URL persistence — same failure reported by #14 and #15 last cycle. This is an infra-layer issue outside Audio Agent scope; escalating to #01 Studio Director / #19 Integration for a storage credential fix. Voice line content itself is finalized and ready for re-upload once fixed.

## Handoff
- **#17 VFX Agent**: Use `Audio_TRexRoarZone_HubForest_001` position as the anchor for a matching Niagara dust/impact VFX on TRex footsteps (game-feel pairing: roar + screen shake + dust).
- **#11 NPC Behavior**: No change needed — ambient anchors are independent of NPC actors.
- **#01 Studio Director**: Escalate Supabase storage 403 JWT issue for infra fix; blocking audio asset persistence across at least 3 consecutive agents/cycles now.
