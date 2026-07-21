# Lighting & Atmosphere Setup — Cycle 019
## Agent #08 — Lighting & Atmosphere Agent

### Systems Applied to MinPlayableMap

#### 1. Directional Light (Sun_Directional)
- Intensity: 10.0 lux
- Color: Warm amber (1.0, 0.92, 0.78) — golden Cretaceous sunlight
- Rotation: Pitch -45°, Yaw 30° — late morning angle
- Atmosphere Sun Light: enabled
- Cast Shadows: enabled

#### 2. Sky Atmosphere (SkyAtmosphere_Main)
- UE5 physically-based atmospheric scattering
- Coupled with Directional Light for accurate sun disc + sky color

#### 3. Sky Light (SkyLight_Main)
- Intensity: 1.5
- Real-Time Capture: enabled — reflects actual sky color into scene

#### 4. Exponential Height Fog (HeightFog_Atmosphere)
- Fog Density: 0.04
- Inscattering Color: cool blue-grey (0.6, 0.75, 0.9) — morning mist
- Height Falloff: 0.2 — thick at ground, thin above
- Volumetric Fog: ENABLED
- Scattering Distribution: 0.5 — balanced forward/back scatter
- Volumetric Fog Extinction Scale: 1.2

#### 5. Post Process Volume (PostProcess_Cinematic)
- Unbound: true — affects entire world
- Lumen Final Gather Quality: 2.0
- Lumen Scene Detail: 2.0
- Lumen Scene Lighting Quality: 1.5
- Bloom Intensity: 0.4 (subtle, not overdone)
- Auto Exposure: 0.5–3.0 range, bias +0.5
- Color Grading: warm prehistoric tone (slight amber/green shift)

### Lighting Philosophy
The Cretaceous world is lit as a living documentary — not a fantasy game.
Light sources are physically motivated: sun angle, atmospheric scatter, ground fog.
Lumen GI ensures every surface receives accurate indirect illumination from the sky
and nearby foliage/terrain. The volumetric fog creates depth and scale — the player
feels small in a vast prehistoric world.

### Mood Reference
- Morning: cool blue fog, warm sun cutting through fern canopy
- Midday: harsh overhead light, deep shadow pools under vegetation
- Evening: deep amber/orange, long shadows, dramatic silhouettes
- Night: moonlit blue-grey, bioluminescent ground fog

### Files Modified
- /Game/Maps/MinPlayableMap — 5 lighting actors added/configured

### Next Agent (#09 — Character Artist)
The lighting is now set. Character materials should be authored to work with:
- Lumen GI (no baked lightmaps needed)
- Warm directional light from 30° yaw
- Volumetric fog affecting distance rendering
- Post process color grading (warm amber bias)
