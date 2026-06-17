# VFX Agent #17 — Cycle 006 Documentation

## Overview
This cycle implements 5 VFX zone markers co-located with Audio Agent #16's audio zones.
All VFX are physically realistic for a prehistoric survival game — no magical/spiritual effects.

---

## VFX Zone Actors Spawned (MinPlayableMap)

| Actor Label | Location (X,Y,Z) | Effect Type | Light Colour | Intensity |
|---|---|---|---|---|
| `VFX_Fire_Camp_001` | (200, 100, 60) | Campfire fire + embers + smoke | Warm orange (1.0, 0.35, 0.05) | 800 |
| `VFX_Mist_River_001` | (3000, 1500, 40) | River surface mist + water spray | Cool blue (0.4, 0.7, 1.0) | 200 |
| `VFX_Leaves_Forest_001` | (-800, 2200, 80) | Falling leaves + pollen drift | Soft green (0.3, 0.8, 0.2) | 150 |
| `VFX_Shake_TRex_001` | (2500, 2000, 60) | Ground tremor screen-shake zone | Amber (1.0, 0.6, 0.0) | 600 |
| `VFX_Vignette_Nest_001` | (4000, 3500, 90) | Red vignette danger flash | Red (1.0, 0.05, 0.05) | 500 |

---

## VFX Design Specifications

### 1. Campfire — `VFX_Fire_Camp_001`
**Niagara System Name:** `NS_Fire_Campfire`
- **Emitter 1 — Flames:** Sprite particles, 40–80 particles/sec, upward velocity + turbulence, orange→yellow→white gradient, 0.3–1.2s lifetime
- **Emitter 2 — Embers:** Mesh particles (tiny spheres), 5–15/sec, random radial scatter, glow material, 2–5s lifetime, gravity affected
- **Emitter 3 — Smoke:** Volume particles, 3–8/sec, slow upward drift, grey→transparent, 4–8s lifetime, wind-affected
- **Emitter 4 — Heat Distortion:** Post-process distortion above flame, shimmer effect
- **LOD 0:** Full (< 10m) — all 4 emitters active
- **LOD 1:** Reduced (10–30m) — flames + smoke only, 50% particle count
- **LOD 2:** Minimal (30–60m) — single flame sprite only
- **LOD 3:** Culled (> 60m)

### 2. River Mist — `VFX_Mist_River_001`
**Niagara System Name:** `NS_Water_RiverMist`
- **Emitter 1 — Surface Mist:** Large flat sprites, 10–20/sec, slow horizontal drift, white→transparent, 3–6s lifetime
- **Emitter 2 — Water Spray:** Small droplet sprites, 30–60/sec, random upward scatter, blue-white, 0.5–1.5s lifetime
- **Emitter 3 — Ripple Rings:** Decal-based ring expansion on water surface
- **LOD 0:** Full (< 15m)
- **LOD 1:** Mist only (15–40m)
- **LOD 2:** Culled (> 40m)

### 3. Forest Leaves — `VFX_Leaves_Forest_001`
**Niagara System Name:** `NS_Ambient_FallingLeaves`
- **Emitter 1 — Falling Leaves:** Mesh particles (leaf shapes), 2–8/sec, tumbling rotation, green/brown/yellow, 5–12s lifetime, wind drift
- **Emitter 2 — Pollen:** Tiny sprite particles, 20–50/sec, slow float, yellow-white, semi-transparent, 8–15s lifetime
- **Emitter 3 — Dust Motes:** Micro sprites, 10–30/sec, Brownian motion, 6–10s lifetime
- **LOD 0:** Full (< 20m)
- **LOD 1:** Leaves only (20–50m)
- **LOD 2:** Culled (> 50m)

### 4. T-Rex Ground Tremor — `VFX_Shake_TRex_001`
**Niagara System Name:** `NS_Dino_GroundTremor`
- **Emitter 1 — Ground Dust:** Radial dust burst from footfall impact point, 50–100 particles/burst, brown/grey, 1–2s lifetime
- **Emitter 2 — Debris:** Small rock/dirt mesh particles, 5–15/burst, low parabolic trajectory
- **Emitter 3 — Grass Disturbance:** Grass blade displacement ripple (material parameter animation)
- **Screen Shake:** Camera shake blueprint — magnitude 2.0, frequency 8Hz, duration 0.4s per footstep
- **Trigger Radius:** 800 units from T-Rex actor
- **LOD 0:** Full dust + debris (< 20m from impact)
- **LOD 1:** Dust only (20–50m)
- **LOD 2:** Culled (> 50m)

### 5. Nest Danger Vignette — `VFX_Vignette_Nest_001`
**Niagara System Name:** `NS_UI_DangerVignette` (post-process)
- **Effect:** Red vignette overlay on screen edges, pulsing at 1.5Hz when player enters nest danger zone
- **Implementation:** Post-process material with dynamic parameter driven by proximity to nest
- **Trigger Radius:** 500 units from nest centre
- **Intensity Curve:** Linear ramp 0→1 over 3 seconds on entry, instant fade on exit
- **Particle Layer:** Small red dust motes rising from ground (nest disturbed earth)

---

## Niagara LOD Chain (3-Level Standard)
All Niagara systems in this project follow the 3-level LOD chain:
- **LOD 0 (High):** Full particle count, all emitters, full simulation — player within close range
- **LOD 1 (Medium):** 50% particle count, primary emitters only — medium distance
- **LOD 2 (Low/Culled):** Single billboard sprite or fully culled — far distance

Performance budget per VFX zone: **< 0.5ms GPU** at LOD 0, **< 0.1ms** at LOD 1.

---

## Integration with Audio Agent #16

VFX zones are co-located with Audio zones for synchronized audio-visual feedback:

| VFX Zone | Audio Zone | Combined Effect |
|---|---|---|
| `VFX_Fire_Camp_001` | `AudioZone_Camp_001` | Fire crackle sound + fire particle light |
| `VFX_Mist_River_001` | `AudioZone_River_001` | Water flow sound + mist particles |
| `VFX_Leaves_Forest_001` | `AudioZone_Forest_001` | Forest ambience + leaf particles |
| `VFX_Shake_TRex_001` | `TensionTrigger_TRex_001` | Heartbeat drone + ground tremor + screen shake |
| `VFX_Vignette_Nest_001` | `TensionTrigger_Nest_001` | Tension sting + red vignette flash |

---

## Handoff Notes for Agent #18 (QA & Testing)

### Tests to Run
1. **VFX Zone Existence:** Verify all 5 `VFX_*` actors exist in MinPlayableMap
2. **No Duplicate Labels:** Confirm no actor labels contain compound suffixes (e.g., `VFX_Fire_Camp_001_AudioZone_TRex`)
3. **Light Configuration:** Each VFX actor should have a PointLightComponent with correct colour
4. **Performance:** VFX zones should not exceed 0.5ms GPU individually
5. **Co-location:** VFX actors should be within 50 units of their paired Audio actors

### Known Limitations This Cycle
- VFX zones are currently PointLight markers (visual proxies) — actual Niagara particle systems require Niagara plugin assets to be created in a future cycle
- Screen shake and vignette effects require Blueprint implementation (queued for Cycle 007)
- LOD chain requires Niagara LOD settings — documented above for implementation

### Priority for Next Cycle (Agent #17 Cycle 007)
1. Create actual Niagara particle system assets (`NS_Fire_Campfire`, `NS_Water_RiverMist`)
2. Implement Blueprint screen shake at `VFX_Shake_TRex_001`
3. Create post-process material for nest danger vignette
4. Add dinosaur footstep dust burst triggered by T-Rex movement

---

*Generated by VFX Agent #17 — Cycle 006 — Transpersonal Game Studio*
*Prehistoric survival game — all VFX are physically realistic, no spiritual/magical effects*
