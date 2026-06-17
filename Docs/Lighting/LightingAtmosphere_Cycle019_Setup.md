# Lighting & Atmosphere — Cycle 019 Setup
## Agent #08 — Lighting & Atmosphere Agent

### Systems Deployed (MinPlayableMap)

| Actor Label | Class | Key Settings |
|---|---|---|
| `Sun_Directional` | DirectionalLight | Intensity 10.0, Warm amber (1.0, 0.88, 0.65), AtmosphereSunLight ON, 4 cascade shadows |
| `SkyAtmosphere_Main` | SkyAtmosphere | Default Cretaceous sky |
| `SkyLight_Main` | SkyLight | Intensity 1.8, RealTimeCapture ON |
| `HeightFog_Main` | ExponentialHeightFog | Density 0.02, Volumetric ON, Cool blue-grey scatter |
| `PostProcess_Lumen` | PostProcessVolume | Unbound, Lumen GI quality 1.0, Bloom 0.4 |

### Lumen Configuration
- **Global Illumination**: Lumen Final Gather Quality 1.0
- **Scene Detail**: 1.0 (full detail)
- **Lighting Quality**: 1.0
- **Tone**: Auto-exposure bias +0.5 (slightly bright — outdoor prehistoric day)

### Volumetric Fog Parameters
- Density: 0.02 (light atmospheric haze)
- Height falloff: 0.2 (fog thickens near ground)
- Inscattering colour: cool blue-grey (0.5, 0.65, 0.8) — distant atmosphere
- Start distance: 500 units
- Cutoff: 200,000 units (full map coverage)
- Scattering distribution: 0.2 (slightly forward-scattering — god rays effect)

### Directional Light — Cinematic Intent
Sun angle: -45° pitch, 30° yaw — late afternoon position.
Warm amber colour simulates Cretaceous atmosphere (higher CO2, denser air = warmer tint).
Dynamic shadow distance: 50,000 units — full terrain shadow coverage.

### Next Steps for Agent #09 (Character Artist)
- Characters will be lit by this Lumen setup automatically
- Skin shaders should use subsurface scattering to interact with warm directional light
- MetaHuman characters will benefit from real-time sky light capture

### Sound References Found
- Search: "prehistoric jungle ambience" — no results on Freesound
- Search: "thunderstorm rain forest" — pending results
- Recommendation: Use UE5 MetaSounds with layered wind + insect + distant thunder for Cretaceous atmosphere

### Files Modified
- `/Game/Maps/MinPlayableMap` — lighting actors added/configured, map saved
