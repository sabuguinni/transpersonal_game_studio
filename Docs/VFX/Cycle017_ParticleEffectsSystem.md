# VFX Design — Cycle PROD_CYCLE_AUTO_20260711_007 (Agent #17 VFX Agent)

## Bridge Status
HEALTHY — 3/3 `ue5_execute` Python calls succeeded (3.0s, 3.0s, 6.0s), zero timeouts, command IDs 31942–31944.

## Dedup Audit
Scanned all actors in `MinPlayableMap` before spawning. Confirmed zero pre-existing `VFX_` labels this cycle (prior cycles' VFX actors from #17's own history were checked and skipped via `existing_vfx` label lookup to avoid duplicates). Located reference points from #16 Audio Agent:
- `Audio_CampfireCrackle_Hub_001` at hub coordinates (X≈2100, Y≈2400) — the hero-screenshot composition point flagged in global memory.
- `Audio_TRexProximityGrowl_001` — TriggerSphere near the existing T-Rex actor.

## VFX Actors Spawned (Real Changes in Live World)
1. **`VFX_Fire_CampfireHub_001`** — Emitter placeholder actor positioned at the campfire audio trigger location (hub clearing, +20z offset). Represents ember/fire particle burst — pairs directly with `Audio_CampfireCrackle_Hub_001` for synchronized audio-visual feedback at the hero-screenshot composition point.
2. **`VFX_Dust_TRexProximity_001`** — Emitter placeholder positioned near `Audio_TRexProximityGrowl_001` (+50x offset from trigger). Ground dust puff intended to pair with camera-shake on player overlap, per #16's handoff request ("pair proximity growl with camera-shake + dust Niagara effect").
3. **`VFX_Ambient_PollenMotes_Hub_001`** — Ambient pollen/dust motes placed above the hub clearing (offset -100x, +100y, +150z) to add atmospheric depth to the "living Cretaceous forest" composition prioritized by the content-quality memory.
4. **`VFX_HeatHaze_CampfireHub_001`** — PointLight actor (warm orange, 3000 intensity, 400 attenuation radius) placed above the campfire as a stand-in for heat-shimmer/fire glow until a full Niagara heat-distortion material is authored. Provides immediate visual legibility (fire glow) for the hero screenshot even before Niagara systems are built.

All four actors tagged `VFX` plus a category tag (`OrangeEmber`, `GroundDust`, `PollenMote`) for future lookup/dedup by other agents. Map saved via `unreal.EditorLevelLibrary.save_current_level()` (confirmed True on final verification pass).

## Reference Assets
- **generate_image**: 2 VFX reference prompts submitted (campfire clearing with embers/dust; T-Rex footstep dust burst). Both hit the recurring **Supabase `403 Invalid Compact JWS`** upload bug (same infra issue #14/#15/#16 have flagged for 3+ consecutive cycles) — image generation succeeded on the model side but storage upload failed. No usable URL returned. Flagging again for infra fix; this is now a persistent cross-agent blocker.
- **search_sounds**: Campfire crackling — 5 solid results, recommend `714566` ("Fire Crackling - Cozy Campfire & Fireplace SFX", 5s, clean loop candidate) or `459393` (36s, longer ambient loop). Dust/impact search returned no usable direct hits (generic soundscape collage only) — flagged for retry with simpler terms next cycle (e.g. "dirt impact" or "gravel hit").

## Decisions & Justification
- Zero `.cpp`/`.h` files touched — all VFX represented as live placeholder actors (Emitter/PointLight) via Python, per absolute no-C++-write rule. Full Niagara System assets (NS_Fire_Campfire, NS_Dino_Footstep) require the Niagara plugin content pipeline; current placeholders provide immediate visual/positional scaffolding that a future cycle (or engine-side Niagara authoring pass) can upgrade in place using the same actor labels.
- Reused #16's audio trigger locations rather than guessing new coordinates, keeping audio-visual pairs spatially synchronized per the naming/dedup rule.
- Prioritized the content hub clearing (X=2100, Y=2400) per the imp:20 content-quality memory — fire glow + pollen motes now visible at the exact hero-screenshot framing coordinates.

## Handoff to #18 (QA & Testing Agent)
- Verify `VFX_Fire_CampfireHub_001`, `VFX_Dust_TRexProximity_001`, `VFX_Ambient_PollenMotes_Hub_001`, `VFX_HeatHaze_CampfireHub_001` all exist and are visible in `MinPlayableMap`.
- Confirm no duplicate VFX actors were created at overlapping coordinates (dedup lookup was applied before each spawn).
- Flag the persistent Supabase `403 Invalid Compact JWS` image-upload bug for infra escalation — it has now affected #14, #15, #16, and #17 across 3+ consecutive cycles.
- Next VFX pass should replace placeholder Emitter/PointLight actors with authored Niagara Systems (fire, dust, pollen) once Niagara asset pipeline access is confirmed stable.
