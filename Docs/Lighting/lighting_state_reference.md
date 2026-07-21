# Lighting State Reference — Canonical Values (maintained by Agent #8)

This file tracks the CURRENT authoritative lighting configuration applied to the level, so future cycles (and other agents) know the baseline before making changes.

## Current Actor Inventory (exactly 1 each — enforced by dedup script every cycle)
| Actor Type | Label | Key Properties |
|---|---|---|
| DirectionalLight | Sun_Main_001 | Intensity: 12000 lux · Pitch: -45° · Color: warm white (1.0, 0.93, 0.82) · atmosphere_sun_light: True · Shadows: On |
| SkyAtmosphere | SkyAtmosphere_Main_001 | Physically-based sky, default parameters |
| SkyLight | SkyLight_Main_001 | real_time_capture: True · Intensity: 1.2 |
| ExponentialHeightFog | HeightFog_Main_001 | Density: 0.015 · Height Falloff: 0.2 · Volumetric Fog: On · Scattering: 0.7 · Tint: cool blue-grey (0.55, 0.65, 0.75) |
| PostProcessVolume | PostProcessVolume_Main_001 | Unbound: True · Auto-Exposure: Histogram (1.0–2.0) · Bloom: 0.6 |

## Guardrails for Future Cycles
- **Sun lux floor:** Server-side guard enforces minimum 10000 lux. Never set below this — it forces a night-read at the (2100,2400) content hub, which is the mandatory hero screenshot composition.
- **Sun lux range used:** 12000 (within the 10000-75000 daylight band). Do not exceed ~75000 or the scene will blow out.
- **Pitch:** -45° is the confirmed correct daytime angle. Avoid pitches below -60° or above -15° (dusk/dawn reads).
- **Never spawn a second DirectionalLight/SkyAtmosphere/SkyLight/ExponentialHeightFog** — always dedup first, then modify the surviving instance.
- **Camera:** Never touched by lighting scripts. Viewport camera changes are off-limits per absolute rule (risk of disorientation with no visual feedback in headless mode).

## Known Blocking Issue
- Supabase Storage upload JWT (`Invalid Compact JWS`) has failed identically across cycles 002-004 for `generate_image` outputs. Model generation succeeds; storage leg fails. Needs infra-level fix (service role key rotation or storage bucket policy check) — escalate to Integration/Build Agent (#19) or Studio Director (#1).

## Handoff to Agent #9 (Character Artist)
World lighting baseline is stable, deduplicated, and verified bright-daytime at the hub. No open lighting blockers for character/MetaHuman work. Character lighting interaction (subsurface scattering under this sun setup, skin shading under Lumen) has not yet been tested with an actual character mesh — recommend Agent #9 spawn a test character and visually confirm skin/material response under Sun_Main_001 before finalizing MetaHuman materials.
