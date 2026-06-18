# Lighting & Atmosphere — Cycle 009 Reference
**Agent #08 — Lighting & Atmosphere Agent**
**Cycle:** PROD_CYCLE_AUTO_20260618_009

---

## Lighting Systems Applied to MinPlayableMap

### 1. Directional Light (Sun)
- **Intensity:** 8.0 lux
- **Color:** Warm amber `(1.0, 0.92, 0.78)` — golden-hour Cretaceous sun
- **Rotation:** pitch=-45°, yaw=45° — dramatic side-angle illumination
- **Atmosphere Sun Light:** True — drives SkyAtmosphere scattering
- **Cast Shadows:** True

### 2. Sky Light (Lumen)
- **Intensity:** 1.2
- **Real-Time Capture:** True — Lumen GI sky contribution updates dynamically
- **Label:** `SkyLight_Lumen`

### 3. Sky Atmosphere
- **Actor:** `SkyAtmosphere_Cretaceous`
- Physically-based atmospheric scattering
- Coupled with DirectionalLight for sun disc and horizon glow

### 4. Exponential Height Fog (Volumetric)
- **Actor:** `AtmosphericFog_Cretaceous`
- **Fog Density:** 0.02
- **Inscattering Color:** `(0.6, 0.75, 0.9)` — cool blue-grey jungle haze
- **Start Distance:** 500 units
- **Volumetric Fog:** True
- **Scattering Distribution:** 0.6 — forward-scattering god rays
- **Albedo:** `(0.75, 0.75, 0.75)` — neutral grey particles
- **Extinction Scale:** 0.8

### 5. Post Process Volume (Lumen GI)
- **Actor:** `PostProcess_LumenGI`
- **Infinite Extent:** True — affects entire level
- Lumen Global Illumination enabled via project settings

### 6. Over-Exposed Light Reduction
- All PointLights and SpotLights with intensity > 5000 reduced to 1500
- Prevents blown-out renders and maintains cinematic balance

---

## Lighting Design Philosophy
*"The player doesn't notice correct lighting — they only notice wrong lighting."*

The Cretaceous world uses a **warm-cool contrast** system:
- **Warm:** Direct sunlight (amber 1.0, 0.92, 0.78) — safety, open terrain
- **Cool:** Fog/shadow (blue-grey 0.6, 0.75, 0.9) — danger, dense jungle
- **Neutral:** Sky light (1.2 intensity) — fills shadows without washing out contrast

This creates natural emotional cues: warm light = safety, cool shadow = threat.

---

## Cinematic References
- **Roger Deakins** — Sicario desert sequences (warm directional + cool shadow)
- **RDR2 Art Team** — True atmospheric haze, not stylized fog
- **National Geographic Cretaceous** — Real prehistoric atmosphere, no mysticism

---

## Files Modified
- `/Game/Maps/MinPlayableMap` — lighting actors configured and saved
- `Docs/Lighting/LightingAtmosphere_Cycle009_Reference.md` — this file

## Next Agent (#09 — Character Artist)
- Lighting is configured for **golden-hour exterior** rendering
- Character materials should use **PBR with warm base tones** to match sun color
- Skin/fur shaders will receive warm ambient from SkyLight_Lumen (1.2 intensity)
- Avoid pure white or pure black albedo — the warm sun will shift all colors
