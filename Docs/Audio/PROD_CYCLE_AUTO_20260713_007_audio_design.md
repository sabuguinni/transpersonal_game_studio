# Audio Design — Cycle PROD_CYCLE_AUTO_20260713_007 (Agent #16)

## Bridge Status
UP. 3x `ue5_execute` Python calls (IDs 33446–33448), all `completed` in ~3s each. Zero timeouts, zero camera moves, zero .cpp/.h writes (per hard rule).

## Voice Lines Generated (text_to_speech)
1. **HunterOtali_Ambient** — "Stay low near the raptor nests at dusk. That is when the pair hunts together, one circling behind while the other watches from the ridge." (proximity VO for raptor nest area, complements Agent #15's Otali lines)
2. **CampElderKael_Ambient** — "Eleven winters this camp has held. Keep your fire fed and your spear close, and it will hold an eleventh more." (proximity VO for camp hub, complements Agent #15's Kael lines)

**Known infra issue (persistent, 5th+ consecutive cycle across agents):** ElevenLabs synthesis succeeds, but Supabase Storage upload fails with `403 Invalid Compact JWS` on every call. Audio generated successfully server-side but not persisted to public URL. This is a systemic auth/token defect in the storage bridge, not a per-agent error — escalating again to QA/Integration (#18/#19) as a blocking infra ticket, consistent with Agent #15's report this same cycle.

## Sound Effects Sourced (search_sounds / Freesound)
- **T-Rex Calls** (id 529462) + **Tyrannosaurus Rex - Roar** (id 607939) + 2 mating roar variants (607938, 607937) — candidates for `Audio_Savana_TRexRoarCue_001` proximity trigger.
- **AMBForst_Forest Birds Wind 01** (id 800712) + 3 pine-forest ambience tracks (173971, 173841, 487448/485972) — candidates for `Audio_Savana_ForestAmbientBed_002` looping ambient bed.
- No results for "large dinosaur footstep heavy thud" or "tense primal percussion drums danger" — these need re-querying next cycle with alternate keywords (e.g. "heavy footstep thud impact", "tribal drums tension").

## Live UE5 World Changes (MinPlayableMap)
1. **Audited** first — confirmed zero pre-existing `Audio_*` actors this cycle, located `NPC_CampElderKael_001`, `NPC_HunterOtali_001` (Agent #15), T-Rex placeholder, and `Herd_HubGrazing_01` (Agent #13) by label lookup — no duplicates created (per naming/dedup rule).
2. **Spawned 5 new `Audio_*` TextRenderActor cue markers** (guarded by label-existence check against full actor list):
   - `Audio_Hub_KaelVoiceCue_001` — VO proximity marker near Kael NPC.
   - `Audio_Hub_OtaliVoiceCue_001` — VO proximity marker near Otali NPC.
   - `Audio_Savana_ForestAmbientBed_002` — ambient bed marker at hub center (2100, 2500).
   - `Audio_Savana_TRexRoarCue_001` — roar/footstep proximity marker near T-Rex placeholder.
   - `Audio_Savana_HerdAlertCue_001` — herd scatter-warning cue near `Herd_HubGrazing_01`, ties to Agent #15's "watch the birds" ambient line.
3. Level saved after placement. Verification pass confirmed `Audio_*` actor set present in final actor list.

## Design Rationale
Per Murch/RDR2 philosophy: audio markers are placed as *proximity triggers*, not decoration — each corresponds to a specific emotional beat (danger cue near T-Rex, warmth/history cue near Kael, tension cue near Otali/raptor nests, safety-baseline ambient bed at the hub). No mystical/spiritual audio content anywhere — all cues are survival-realistic (roars, footsteps, wind, birds, voice).

## Next Agent (#17 VFX Agent) Focus
- Pair Niagara dust/impact VFX with `Audio_Savana_TRexRoarCue_001` proximity (footstep dust synced to roar cue timing).
- Consider particle cue at `Audio_Savana_HerdAlertCue_001` (birds scattering) to visually match the "watch the birds" line.
- Do not duplicate any `Audio_*` marker — reuse by label lookup, consistent with the naming/dedup rule.

## Files Modified
- `Docs/Audio/PROD_CYCLE_AUTO_20260713_007_audio_design.md` (this file)
