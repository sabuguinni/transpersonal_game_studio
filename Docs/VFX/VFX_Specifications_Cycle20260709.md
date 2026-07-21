# VFX Agent #17 — Niagara Effect Specifications
Cycle: PROD_CYCLE_AUTO_20260709_001
Bridge status: HEALTHY (3 ue5_execute calls, 6-33s each, zero timeouts)

> NOTE per `hugo_no_cpp_h_v2`: No .cpp/.h files were written this cycle. All VFX
> specs below are implemented in-engine as `Note` actor anchors (queryable via
> `VFX` tag) in `MinPlayableMap`, NOT as compiled C++ classes. When a real
> Niagara/UE5 C++ VFX subsystem is greenlit, these specs are the authoritative
> source for `NS_*` system parameters.

## Anchors spawned this cycle (4 total, all tagged `VFX`)

| Actor Label | Location basis | Purpose |
|---|---|---|
| `VFX_Fire_HubCampfire_001` | Hub clearing (2100,2400,150) | Campfire flame/ember/smoke, LOD chain 3 levels |
| `VFX_Atmosphere_HubDustMotes_001` | Hub clearing (2400,2400,350) | Pollen/dust in god-rays for bright daylight hero shot |
| `VFX_Dino_TRexFootstepDust_001` | 150u offset from existing TRex actor (reused, not duplicated) | Footstep dust burst + breath vapor |
| `VFX_Atmosphere_DuskGodRays_001` | 300u above existing DirectionalLight (reused, not duplicated) | Volumetric dusk light shafts + color grade sync |

All anchors reuse existing scene actors (TRex, DirectionalLight, hub coordinates)
per `hugo_naming_dedup_v2` — no duplicate geometry was spawned.

## NS_Fire_Campfire
- Flame core (additive, 200 particles LOD0)
- Ember sparks rising with gravity/wind force
- Gray smoke column, wind-bent (curl noise)
- LOD0: full sim within 2000u | LOD1: sprite billboard 50 particles 2000-5000u | LOD2: static smoke decal beyond 5000u
- Synced with Audio Agent's `Audio_HubCampfireAmbient_001` crackle loop

## NS_Atmosphere_DustMotes
- Slow-drifting pollen/dust in volumetric god-rays through canopy
- Additive blend, LOD0 200 / LOD1 60 / LOD2 disabled beyond 3000u
- Directly supports the mandated hero screenshot composition (X=2100,Y=2400):
  bright daylight Cretaceous forest clearing per `hugo_hub_quality_v2_fix`

## NS_Dino_Footstep + NS_Dino_BreathVapor
- Radial dust burst + dirt debris on foot-bone socket impact event
- LOD0 80 particles / LOD1 20 sprite / LOD2 none beyond 1500u
- Breath vapor: additive fog card puff from snout in cold morning air
- Trigger radius synced to Audio Agent's `Audio_TRexProximityGrowl_001` (2000u)

## NS_Atmosphere_DuskGodRays
- Volumetric light shaft intensity ramps as DirectionalLight pitch crosses -10deg
- Paired with subtle warm-to-cool color grade LUT blend
- Synced threshold with Audio Agent's `Audio_DuskTransitionCue_001`

## Reference imagery
Attempted 2x `generate_image` calls (campfire/god-rays concept, TRex footstep
dust concept) — both failed upstream with `403 Invalid Compact JWS` (Supabase
storage auth bug, same recurring issue reported by Agents #14/#15/#16 in prior
cycles). Image generation itself likely succeeded server-side; only the
storage upload step failed. Not fixable from this agent — flagged for
Integration/Build Agent (#19) or platform owner.

## Sound reference search
- "campfire crackle fire burning loop" → 5 usable Freesound results (IDs 729395,
  729396, 813328, 626277, 636709) suitable for `NS_Fire_Campfire` companion audio
  layer, to hand off to Audio Agent for final loop selection.
- "wind through jungle leaves rustle" → 0 results (rate limit or no match this cycle)

## Dependencies for next agent (#18 QA & Testing)
- Verify all 4 `VFX`-tagged Note anchors persist after level save/reload
- Confirm no duplicate anchors were created by other agents at same coordinates
  (dedup check performed this cycle: only expected actors found near hub coords)
- When real Niagara systems are authored, these Note specs should be treated
  as acceptance criteria for LOD behavior and audio-sync thresholds
