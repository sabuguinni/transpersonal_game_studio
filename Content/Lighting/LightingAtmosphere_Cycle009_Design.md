# Lighting & Atmosphere — Cycle 009 Design Document
**Agent #8 — Lighting & Atmosphere Agent**
**Cycle:** PROD_CYCLE_AUTO_20260617_009

---

## Concept Reference
**Mood:** Cretaceous jungle ruins at golden hour — warm amber shafts piercing dense canopy, deep green shadows, atmospheric haze, dramatic contrast between illuminated pillars and dark undergrowth.

**Cinematographic Reference:** Roger Deakins — *Skyfall* jungle sequences, *Prisoners* dappled forest light. RDR2 environment team — atmospheric truth over beauty.

---

## Lighting Setup — Active Actors in MinPlayableMap

### 1. Sun_GoldenHour (DirectionalLight)
- **Intensity:** 8.5 lux
- **Color:** RGB(255, 217, 165) — warm amber 3200K
- **Pitch:** -35° (low golden hour angle)
- **Yaw:** 45° (side-lit for dramatic pillar shadows)
- **Volumetric Scattering:** 2.5x (god rays through jungle canopy)
- **Shadow Cascades:** 4 (high quality terrain shadows)
- **Atmosphere Sun Light:** True (drives sky atmosphere)

### 2. SkyAtmosphere_Prehistoric (SkyAtmosphere)
- **Rayleigh Scale:** 0.0331 (slightly elevated for prehistoric haze)
- **Mie Scattering:** 0.003 (dust/pollen in air)
- **Mie Anisotropy:** 0.8 (forward scattering for sun halo)
- **Aerial Perspective Scale:** 1.2 (depth haze on distant terrain)

### 3. SkyLight_Ambient (SkyLight)
- **Intensity:** 1.8
- **Color:** RGB(200, 220, 255) — cool blue-sky ambient fill
- **Real Time Capture:** True (reflects dynamic sky)
- **Cast Shadows:** True

### 4. Fog_JungleAtmosphere (ExponentialHeightFog)
- **Density:** 0.035 (moderate jungle humidity)
- **Height Falloff:** 0.2 (fog pools in valleys)
- **Inscattering Color:** RGBA(0.55, 0.72, 0.58) — green-tinted jungle haze
- **Volumetric Fog:** True
- **Scattering Distribution:** 0.6 (forward-biased for god rays)
- **Albedo:** RGBA(0.75, 0.85, 0.70) — green-grey fog color
- **View Distance:** 6000 units

### 5. Light_RuinFill_Canopy (PointLight @ Ruin Cluster)
- **Location:** (50000, 50000, 800) — above ruin cluster
- **Intensity:** 3000 lux
- **Color:** RGB(255, 240, 180) — warm canopy fill
- **Attenuation Radius:** 3000 units
- **Cast Shadows:** False (fill light, no hard shadows)
- **Purpose:** Simulates filtered light through jungle canopy over ruins

### 6. Light_RuinSpot_Dramatic (SpotLight @ Ruin Cluster)
- **Location:** (50000, 50000, 1200) — high angle
- **Pitch:** -70° (steep downward angle for pillar drama)
- **Intensity:** 8000 lux
- **Color:** RGB(255, 200, 120) — golden shaft
- **Inner Cone:** 20° / **Outer Cone:** 45°
- **Volumetric Scattering:** 3.0x (visible god ray shaft)
- **Cast Shadows:** True (hard pillar shadows)
- **Purpose:** Dramatic single shaft of light through pillar gap

### 7. PostProcess_CinematicGrade (PostProcessVolume)
- **Unbound:** True (affects entire world)
- **Auto Exposure Bias:** +0.5 (slightly bright for jungle feel)
- **Bloom Intensity:** 0.4 / Threshold: 1.0
- **Vignette:** 0.35 (subtle edge darkening)
- **Chromatic Aberration:** 0.5 (slight lens imperfection)
- **Color Saturation:** (1.0, 1.0, 0.95, 1.05) — warm desaturate blues
- **Color Contrast:** (1.05, 1.0, 0.95, 1.0) — lift reds, reduce blues
- **Color Gamma:** (0.98, 0.97, 0.93, 1.0) — warm midtones
- **Color Gain:** (1.02, 0.99, 0.95, 1.0) — warm highlights

---

## Emotional Intent

The ruin cluster at (50000, 50000) should feel **discovered, not built**. The player stumbles upon these ruins in dense jungle — the light should feel like it's revealing something hidden, not illuminating something on display.

- **Pillars in shadow** = mystery, danger
- **Shaft of light on altar** = discovery, significance
- **Green-tinted fog** = isolation, ancient, alive
- **Warm sky** = beauty that contrasts with danger below

This is NOT a sacred space. It is an ancient geological/structural formation — perhaps a collapsed cliff face or early hominid shelter site. The light serves the **survival narrative**: this is a landmark the player can use for navigation and shelter.

---

## Lumen Configuration Notes

For full Lumen GI quality on this scene:
```ini
r.Lumen.DiffuseIndirect.Allow=1
r.Lumen.Reflections.Allow=1
r.Lumen.HardwareRayTracing=1
r.Lumen.TraceMeshSDFs=1
r.VolumetricFog=1
r.VolumetricFog.GridPixelSize=8
```

These should be set in DefaultEngine.ini under `[/Script/Engine.RendererSettings]`.

---

## Next Agent Handoff — Agent #9 (Character Artist)

The lighting is configured for:
- **Player character visibility**: warm fill ensures the character reads against dark jungle
- **Shadow casting**: 4-cascade directional shadows for accurate character grounding
- **Rim lighting**: sun angle at 45° yaw creates natural rim light on characters facing camera
- **Skin tone rendering**: warm 3200K key light flatters realistic skin tones from MetaHuman

Character Artist should note: the post-process color grade slightly desaturates blues — MetaHuman skin tones should lean warm (not cool-toned) to read correctly under this lighting.

---

## Files Modified
- `Content/Lighting/LightingAtmosphere_Cycle009_Design.md` (this file)
- MinPlayableMap — 7 lighting actors configured/spawned
