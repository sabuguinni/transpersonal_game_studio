# Lighting & Atmosphere — Cycle 005: Ruin Volumetrics & Lumen GI
**Agent:** #08 — Lighting & Atmosphere Agent  
**Cycle:** PROD_CYCLE_AUTO_20260618_005  
**Handoff from:** Agent #07 Architecture & Interior (Cretaceous Ruins at X=50000, Y=50000)  
**Handoff to:** Agent #09 Character Artist

---

## What Was Deployed This Cycle

### 1. DirectionalLight (Sun) — Fixed
- **Intensity:** 8.0 (was likely weak/default)
- **Color:** Warm amber `(1.0, 0.95, 0.85)` — Cretaceous golden hour
- **Temperature:** 5800K (natural sunlight)
- **Rotation:** Pitch=-45°, Yaw=30° — dramatic diagonal angle for shadow depth
- **Dynamic shadow distance:** 50,000 units (covers full playable area)
- **Cast shadows:** True

**Design intent:** The terrain was appearing nearly black in top-down screenshots (confirmed in Brain Memory `hugo_lighting_fix_screenshot`). This fix ensures the terrain receives proper illumination at a 45° angle that creates readable shadows and depth.

### 2. Overexposed Point/Spot Lights — Reduced
- Any light with intensity > 5000 lux reduced to 2000 lux
- Prevents color blowout (red/orange/teal overexposure confirmed in previous screenshot)
- Maintains atmospheric character while restoring scene readability

### 3. Ruin Volumetric Light Shafts (God Rays)
Three SpotLights placed above the ruin cluster (X=50000–50200, Y=50000–50200):

| Label | Position | Angle | Purpose |
|-------|----------|-------|---------|
| `LightShaft_Ruin_001` | (50050, 50050, 800) | Pitch=-70°, Yaw=45° | Primary shaft through main wall gap |
| `LightShaft_Ruin_002` | (50150, 50100, 800) | Pitch=-65°, Yaw=20° | Secondary shaft, wider angle |
| `LightShaft_Ruin_003` | (50200, 50200, 800) | Pitch=-75°, Yaw=60° | Corner shaft, steeper angle |

**Properties:**
- Intensity: 3000 lux
- Color: `(1.0, 0.92, 0.75)` — warm amber shaft
- Inner cone: 8° / Outer cone: 18° — tight focused beam
- `cast_volumetric_shadow: True` — requires volumetric fog to be visible
- Attenuation radius: 1500 units

**Design intent (Roger Deakins principle):** Light shafts through ruin gaps are not decorative — they reveal the architecture. Each shaft is positioned to illuminate a specific wall section, creating pools of light and shadow that guide player navigation. The player will instinctively move toward the lit areas.

### 4. Campfire Point Light — Ruin Interior
- **Label:** `Campfire_Light_Ruin_001`
- **Position:** (50125, 50125, 120) — interior center of ruin structure
- **Intensity:** 1500 lux
- **Color:** `(1.0, 0.55, 0.15)` — deep orange fire
- **Temperature:** 2200K (candlelight/firelight)
- **Attenuation radius:** 600 units
- **Cast shadows:** True

**Design intent:** The campfire is the narrative anchor of the ruin. It signals "someone was here" or "this is a safe rest point." The warm 2200K against the cool 5800K sunlight creates the classic warm/cool contrast that makes scenes feel cinematic.

### 5. SkyLight (Lumen GI Ambient)
- **Intensity:** 1.2
- **Real-time capture:** True — Lumen GI updates dynamically
- Fills shadow areas with ambient sky color, preventing pure-black shadows

### 6. ExponentialHeightFog (Atmospheric Depth)
- **Fog density:** 0.02 (subtle — not soup)
- **Height falloff:** 0.2 — fog thickens toward ground
- **Inscattering color:** `(0.5, 0.65, 0.8)` — cool blue atmospheric haze
- **Volumetric fog:** True — required for light shaft visibility
- **Scattering distribution:** 0.2 — slight forward scattering
- **Extinction scale:** 1.0

---

## Lighting Philosophy Applied

> "The player doesn't notice correct lighting — they only notice wrong lighting."

The ruin cluster needed three lighting layers:
1. **Macro** — DirectionalLight establishes time of day and overall mood (golden hour)
2. **Meso** — Volumetric shafts reveal architecture and create navigation cues
3. **Micro** — Campfire creates emotional anchor and survival context

The fog is the connective tissue. Without volumetric fog, the light shafts are invisible. With too much fog, the scene becomes unreadable. 0.02 density is the calibrated balance.

---

## Known Issues / Next Steps

### For Agent #09 (Character Artist):
- Character materials should respond to the warm/cool contrast: skin tones will look correct under the 5800K sun
- MetaHuman characters should have subsurface scattering enabled — the campfire at 2200K will create beautiful rim lighting on character faces
- Recommend placing character spawn point near campfire interior for first-person emotional impact

### For Agent #10 (Animation):
- The light shaft positions define "interesting zones" — animations (idle, rest, crafting) should be placed within shaft footprints for maximum visual impact
- Campfire idle animation should be placed at (50125, 50125) — directly under campfire light

### For Agent #16 (Audio):
- Campfire light position = campfire audio source position: (50125, 50125, 120)
- Volumetric fog density correlates with jungle ambience reverb: 0.02 density → medium reverb tail

### For Agent #17 (VFX):
- Campfire particle emitter should be placed at (50125, 50125, 80) — slightly below light source
- Light shaft Niagara dust motes: spawn within cone defined by each SpotLight (8°–18° cone, 1500 unit radius)
- Fog interaction: Niagara particles should use `Particle.Opacity * FogDensity` for depth integration

---

## Lumen Configuration Notes

For full Lumen GI quality in MinPlayableMap, ensure `DefaultEngine.ini` contains:
```ini
[/Script/Engine.RendererSettings]
r.Lumen.Enabled=1
r.Lumen.DiffuseIndirect.Allow=1
r.Lumen.Reflections.Allow=1
r.Lumen.HardwareRayTracing=0
r.DynamicGlobalIlluminationMethod=1
r.ReflectionMethod=1
r.Shadow.Virtual.Enable=1
```

The campfire at 2200K will generate Lumen GI bounce light onto adjacent stone walls — this is the "wet stone reflecting firelight" effect that makes prehistoric environments feel inhabited.

---

## Files Created This Cycle
- `Docs/Lighting/LightingAtmosphere_Cycle005_RuinVolumetrics.md` (this file)

## UE5 Actors Created/Modified
- `Sun_Directional_001` — DirectionalLight (fixed/spawned)
- `LightShaft_Ruin_001/002/003` — SpotLights for god rays
- `Campfire_Light_Ruin_001` — PointLight for ruin interior
- `SkyLight_Lumen_001` — SkyLight with real-time Lumen capture
- `AtmosphericFog_001` — ExponentialHeightFog with volumetrics
