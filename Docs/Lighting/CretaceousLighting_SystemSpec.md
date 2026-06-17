# Cretaceous Lighting System — Technical Specification
**Agent #8 — Lighting & Atmosphere | Cycle PROD_CYCLE_AUTO_20260617_011**

---

## Overview

The Cretaceous lighting system is designed around a single cinematic truth: **the player must feel the danger and beauty of the prehistoric world before they see a single dinosaur.** Every lighting decision serves emotional immersion, not technical showcase.

**Director of Photography Reference:** Roger Deakins (Blade Runner 2049, 1917) — light that *means*, not just illuminates.
**Game Reference:** Red Dead Redemption 2 — lighting so truthful the player never notices it.

---

## Active Lighting Actors (MinPlayableMap)

| Actor Label | Type | Purpose |
|---|---|---|
| `Sun_GoldenHour_Cretaceous` | DirectionalLight | Primary sun — golden hour, low angle (-22° pitch, 45° yaw) |
| `SkyAtmosphere_Cretaceous` | SkyAtmosphere | Physical atmosphere scattering |
| `SkyLight_Cretaceous` | SkyLight | Lumen real-time sky capture, cool blue fill |
| `Fog_Volumetric_Cretaceous` | ExponentialHeightFog | Volumetric fog with valley pooling |
| `RuinFill_Light_001-003` | PointLight | Warm amber fill at ruin pillar cluster (50000,50000) |
| `RuinShaft_Light_001` | SpotLight | Dramatic god-ray shaft through ruin pillars |
| `PostProcess_Cinematic_Cretaceous` | PostProcessVolume | Global colour grading, bloom, vignette, AO |

---

## Sun Configuration (DirectionalLight)

```
Rotation:       Pitch -22°, Yaw 45° (golden hour, low western sun)
Intensity:      8.5 lux
Color:          RGB(255, 210, 140) — warm amber
Cast Shadows:   True
Volumetric Shadow: True
Atmosphere Sun: True
Light Shaft Occlusion: True
Light Shaft Bloom: True
Bloom Scale:    0.4
Bloom Threshold: -1.0 (always bloom)
Occlusion Darkness: 0.05 (subtle shaft definition)
```

**Emotional intent:** Late afternoon, the most dangerous hunting hour. Predators are active, shadows are long, visibility is reduced. The warm colour creates false comfort — the danger is real.

---

## Volumetric Fog Configuration

```
Fog Density:        0.018
Height Falloff:     0.22 (fog pools in valleys, thins on ridges)
Inscattering Color: LinearColor(0.55, 0.48, 0.38) — dusty amber haze
Max Opacity:        0.85
Start Distance:     500 units
Cutoff Distance:    200,000 units
Volumetric Fog:     ENABLED
Scattering Dist:    0.35 (forward scattering — god rays visible)
Albedo:             LinearColor(0.9, 0.85, 0.75) — warm dust
Extinction Scale:   0.85
View Distance:      6,000 units
```

**Emotional intent:** The fog is not decorative. It hides what is beyond 60m. The player cannot see what is hunting them. This is survival design through lighting.

---

## Sky Light Configuration (Lumen)

```
Intensity:          1.8
Color:              RGB(180, 200, 230) — cool blue sky fill
Cast Shadows:       True
Real-Time Capture:  True (Lumen dynamic sky)
```

**Emotional intent:** The cool blue sky fill creates contrast with the warm sun — the classic cinematographic split between warm key light and cool fill. This is what makes the scene feel *real* rather than flat.

---

## Ruin Pillar Lighting (50000, 50000)

### Fill Lights (3× PointLight)
```
Intensity:          800 lux
Color:              RGB(255, 180, 100) — warm campfire amber
Attenuation Radius: 1,500 units
Volumetric Shadow:  True
```

### Shaft Light (SpotLight)
```
Position:           (50000, 50000, 8000) — high above pillar cluster
Rotation:           Pitch -85° (near-vertical shaft)
Intensity:          15,000 lux
Color:              RGB(255, 230, 180) — bright warm shaft
Inner Cone:         8° (tight beam)
Outer Cone:         25° (soft penumbra)
Volumetric Shadow:  True
Light Shaft Bloom:  True
```

**Emotional intent:** The ruin pillars are the first sign of something *before* the player's time. The dramatic shaft light makes them feel sacred and dangerous simultaneously. The player will be drawn to investigate — and that is where the first encounter should occur.

---

## Post Process Volume (Global, Unbound)

```
Bloom Intensity:        0.35
Bloom Threshold:        -1.0 (always active)
Auto Exposure Min:      0.3 EV
Auto Exposure Max:      3.0 EV
Exposure Speed Up:      3.0 (fast adaptation to bright)
Exposure Speed Down:    1.0 (slow adaptation to dark — tension in shadows)
Vignette Intensity:     0.4 (subtle frame darkening)
Chromatic Aberration:   0.5 (slight lens imperfection — realism)
Ambient Occlusion:      0.8 intensity, 200 unit radius
```

**Emotional intent:** The slow exposure adaptation to darkness is a survival design decision. When the player enters shadow (a cave, dense forest, under a large dinosaur), the eye adapts slowly. This creates genuine tension — not artificial darkness, but the biological reality of moving from light to shadow.

---

## Lumen Configuration (Console Commands)

Apply via UE5 console for full Lumen quality:

```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.ScreenProbeGather.DownsampleFactor 1
r.Lumen.TraceMeshSDFs.Allow 1
r.VolumetricFog 1
r.VolumetricFog.GridPixelSize 8
r.VolumetricFog.GridSizeZ 64
r.Shadow.Virtual.Enable 1
r.Shadow.Virtual.SMRT.RayCountDirectional 8
```

---

## Day/Night Cycle Design (Future Implementation)

The lighting system is designed for a full 24-hour cycle. Key emotional moments:

| Time | Lighting State | Emotional Intent |
|---|---|---|
| 05:30 — Dawn | Cool blue-grey, mist heavy | Hope, vulnerability, cold |
| 07:00 — Morning | Warm gold, fog burning off | Activity, opportunity |
| 12:00 — Noon | Harsh white, hard shadows | Exposure, danger, heat |
| 16:00 — Golden Hour | Amber, long shadows | Beauty, false safety |
| 18:30 — Dusk | Red-orange, dramatic | Warning, predator hour |
| 20:00 — Night | Deep blue, moonlight | Terror, survival instinct |
| 02:00 — Deep Night | Near black, stars only | Maximum danger |

**Implementation note:** Use a Timeline Blueprint driving the DirectionalLight rotation from -90° (midnight) through 0° (noon) to 90° (midnight again) over a configurable cycle duration (default: 20 real minutes = 24 game hours).

---

## Biome Lighting Variants (Future)

| Biome | Key Light Modifier | Fog Modifier | Emotional Tone |
|---|---|---|---|
| Savanna | +15% intensity, yellower | -30% density | Open, exposed, dangerous |
| Dense Forest | -40% intensity, green tint | +50% density, green | Claustrophobic, hidden predators |
| Swamp | -20% intensity, grey-green | +80% density | Oppressive, diseased |
| Volcano Zone | Orange-red tint, flickering | Ash grey fog | Apocalyptic, extreme danger |
| Coastal | +10% intensity, blue tint | Sea spray haze | Open, resource-rich |
| Cave | Near-zero ambient, point lights only | No fog | Absolute terror |

---

## Integration Notes for Agent #9 (Character Artist)

- Character skin shaders should be calibrated against the golden hour sun (RGB 255,210,140)
- MetaHuman skin subsurface scattering will read correctly with the current sky light intensity (1.8)
- Character shadow casting is enabled — ensure character meshes have proper shadow LODs
- The vignette (0.4) will frame the character naturally in wide shots

## Integration Notes for Agent #16 (Audio)

- The volumetric fog creates a natural audio occlusion zone — sounds beyond 6,000 units should be muffled
- The ruin pillar cluster at (50000,50000) is a designated atmospheric audio anchor point
- Dawn/dusk transitions (when implemented) should trigger ambient audio state changes

---

*Lighting system designed for emotional truth, not technical demonstration.*
*The best lighting is invisible. The player feels it without seeing it.*
