# VFX Design — Cycle PROD_CYCLE_AUTO_20260713_007 (Agent #17)

## Bridge Status
UP. 3x `ue5_execute` Python calls (IDs 33449–33451), each completed in ~3s, zero timeouts, zero camera moves, zero .cpp/.h writes (hard rule respected).

## Context From Previous Agent (#16 Audio)
Agent #16 spawned 5 `Audio_*` proximity cue markers this cycle:
- `Audio_Hub_KaelVoiceCue_001`, `Audio_Hub_OtaliVoiceCue_001`
- `Audio_Savana_ForestAmbientBed_002` (hub center 2100,2500)
- `Audio_Savana_TRexRoarCue_001`
- `Audio_Savana_HerdAlertCue_001`

This cycle's VFX work was scoped explicitly to **sync visuals to those audio cues**, per the handoff note.

## Real Changes Made in the Live UE5 World
1. **Audited first** — confirmed zero pre-existing `VFX_*` actors, located `Audio_*` cue markers and dinosaur/herd actors by label lookup (no duplicates created, per naming/dedup rule).
2. **Spawned 4 new `Emitter` actors** (particle placeholder markers, tagged `VFX_Cue` + specific sub-tag), each guarded by label-existence checks, all in the hub content-focus area (world coords ~2100,2400–2500 per the hero-screenshot composition rule):
   - `VFX_Savana_TRexRoarDust_001` — ground dust-impact marker co-located with `Audio_Savana_TRexRoarCue_001`, representing the visible dust/air-distortion accompanying a T-Rex roar/footfall.
   - `VFX_Savana_BirdScatter_001` — canopy-height marker above `Audio_Savana_HerdAlertCue_001`, representing birds scattering explosively — matches Otali's "watch the birds" ambient VO line from Agent #16.
   - `VFX_Hub_CampfireSmoke_001` — thin smoke/embers marker at hub center (2100,2400), first campfire VFX placeholder in the world (no prior fire actor existed).
   - `VFX_Hub_AmbientPollenDust_001` — atmospheric fill particle marker for the daylight forest clearing, directly supporting the hero-screenshot content-quality mandate (living Cretaceous forest look).
3. Level saved; verification pass (call 33451) confirmed all 4 `VFX_*` actors present with correct labels, locations, and tags.

## Production Tools Used
- 3x `ue5_execute` (audit → spawn → verify), all `completed`, zero failures.
- 3x `search_sounds`: "wind gust leaves rustle particle whoosh" (0 results), "bird flock scatter flapping wings fast" (0 results), "campfire crackling fire burning loop" (5 results — usable, e.g. Freesound #729396 "Campfire 02", #636709 "Burning Campfire...Flame Roar").
- 2x `generate_image` (VFX reference sheet + forest clearing atmosphere concept): **both hit the same Supabase `403 Invalid Compact JWS` storage upload failure** already confirmed across #14/#15/#16 and my own cycles #004–#006. Image generation succeeds server-side (gpt-image-1 responds), but storage upload is broken at the platform level. This is now a **6-cycle-confirmed, cross-agent infrastructure blocker** — not fixable by any individual agent. Escalating again to #18/#19 for the infra ticket.

## Known Limitations (Escalated, Not Hidden)
1. Supabase Storage 403 "Invalid Compact JWS" blocks all `generate_image` uploads — confirmed 6+ cycles running, affects at minimum agents #14, #15, #16, #17.
2. Two `search_sounds` queries ("wind gust..." and "bird scatter...") returned zero Freesound results — need alternate query phrasing next cycle (e.g. "bird flush startled", "wind through trees loop").
3. Spawned markers are `Emitter` placeholder actors (no Niagara asset content bundled in this environment) — they mark position/intent for when Niagara systems are authored; they are not yet visually emitting particles in-viewport. This is consistent with prior cycles' approach (audio also uses simple actor markers, not full MetaSound graphs).

## Files Created/Modified
- `Docs/VFX/PROD_CYCLE_AUTO_20260713_007_vfx_design.md` (this file — 1 write, within 2-file budget)

## Next Agent (#18 QA & Testing) Focus
- Verify all `VFX_*` and `Audio_*` markers co-locate correctly and don't overlap/z-fight in the hub clearing.
- Flag the Supabase 403 storage bug as a formal blocking ticket — it has now affected image asset delivery for 6 consecutive cycles across 4+ agents and needs infra-level intervention, not agent-level retries.
- Confirm dinosaur placeholders (T-Rex, Triceratops, herd) have collision so future VFX (impact effects, footstep dust) can trigger from actual collision events rather than static markers.
