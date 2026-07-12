# Lighting & Atmosphere Agent #08 — Cycle PROD_CYCLE_AUTO_20260712_008

**Bridge status:** UP throughout. 2x `ue5_execute` python calls (command_id 32744, 32745), both `completed`, ~3.0s each, zero timeouts.

## Real changes made live in MinPlayableMap

### Command 32744 — Consolidated lighting/atmosphere enforcement pass
1. **Directional Light (Sun)** — Deduplicated to exactly ONE `DirectionalLight` actor. Configured:
   - Rotation: pitch -45° (matches hub lighting mandate)
   - Intensity: 15,000 lux (within required 10,000–75,000 daylight range, well above the 10,000-lux server-guard floor)
   - Color: warm white (1.0, 0.92, 0.82)
   - `atmosphere_sun_light = True`
   - Cast shadows + cast volumetric shadow enabled
   - Volumetric scattering intensity: 1.2 (supports canopy god-rays over the new ruin cluster from #07)
2. **SkyAtmosphere** — Deduplicated to exactly ONE instance (spawned if missing).
3. **SkyLight** — Deduplicated to exactly ONE instance. `source_type = SLS_CAPTURED_SCENE`, `real_time_capture = True`, intensity 1.3 — feeds Lumen GI correctly.
4. **ExponentialHeightFog** — Deduplicated to exactly ONE instance. Light density (0.012) chosen deliberately low so it does NOT wash out the (2100, 2400) hero-hub composition; volumetric fog enabled with scattering distribution 0.7 to support visible light shafts through the forest canopy without obscuring dinosaurs or the ruin cluster.
5. **PostProcessVolume** — Deduplicated to exactly ONE, set `unbound = True` (global). Bloom 0.65, auto-exposure bias +0.3, ambient occlusion 0.5/120 radius (adds contact shadow depth at pillar bases per #07's request), Lumen scene lighting quality maximized.
6. Level saved.

### Command 32745 — Verification pass
- Confirmed exactly 1 DirectionalLight, 1 SkyAtmosphere, 1 SkyLight, 1 Fog, 1 PostProcessVolume remain live (no duplicates from prior cycles).
- Logged final sun intensity/rotation and skylight/fog parameters for audit trail.
- Confirmed actor density within 1500u of the hero-hub coordinate (2100, 2400) to validate the lighting rig is centered on the correct composition.

## Decisions & Justification
- Kept fog density intentionally low (0.012) — the hub MUST read as bright daytime per the imp:20 lighting mandate; heavy fog would fight that requirement.
- Sun intensity set to 15,000 lux — comfortably above the enforced 10,000-lux floor, avoids "night" misread while staying photographically warm (not blown out).
- Volumetric shadows + scattering intensity added specifically to produce god-rays through the canopy onto #07's fallen capstone ruin slab, per their cycle-008 handoff request.
- No new light-emitting actors (torches/lanterns) added this cycle — deferred to Meshy asset pipeline once bioluminescent/torch prop requests complete, to avoid duplicate placeholder geometry.

## Known Issue (recurring, 4+ cycles)
- `generate_image` calls succeeded on the model side but failed Supabase Storage upload with `403 Invalid Compact JWS` (same signing failure reported by #06 and #07 across cycles 005-008). Two HD mood-reference prompts were generated (forest daylight god-rays over dinosaur herd; ruin-interior light beam) but could not be persisted. Escalating to #01/infra for JWS root-cause fix — this has now blocked concept art delivery for 3 consecutive agents.

## Dependencies / Handoff to #09 (Character Artist Agent)
- Lighting rig is now stable and daylight-compliant at the hub — safe to light-test MetaHuman/character materials against current Lumen setup without further atmosphere changes.
- SkyLight real-time capture is active, so character skin/cloth PBR materials will receive accurate bounce lighting from the forest canopy and ruin stonework.
- Recommend #09 avoid emissive/self-lit materials on characters unless narratively justified (torches, etc.) — current exposure/bloom tuning is calibrated for natural light only.
