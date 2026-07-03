# Lighting & Atmosphere — Cycle 013: Sunrise Morning Palette
## Agent #08 | PROD_CYCLE_AUTO_20260702_013

---

## Lighting Palette: Sunrise Morning

### Director of Photography Intent
Sunrise is the most emotionally charged moment of the prehistoric day. The player wakes in a dangerous world — the light should feel like hope laced with threat. Warm rose-gold from the east, cool blue-purple shadows still clinging to the west, amber mist hugging the ground. The jungle breathes. The dinosaurs are already moving.

### Technical Parameters Applied

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Sun Pitch | -30° | Low horizon angle — long dramatic shadows |
| Sun Yaw | -60° | East-facing — morning direction |
| Sun Intensity | 38,000 lux | Above CAP floor (10,000), below blinding noon |
| Sun Color | RGB(1.0, 0.72, 0.45) | Warm rose-gold — classic sunrise warmth |
| Fog Density | 0.018 | Light morning mist — not obscuring |
| Fog Color | RGB(0.72, 0.55, 0.38) | Amber mist — matches sun color temperature |
| Fog Falloff | 0.18 | Gradual height dissipation |
| Fog Start | 500 cm | Starts at mid-distance |
| Volumetric Fog | Enabled | God rays through canopy |
| SkyLight Intensity | 1.8 | Moderate bounce — dawn sky not yet bright |
| SkyLight Mode | Real-time capture | Accurate sky color reflection |

### Hub Fill Lights (X=2100, Y=2400)

| Actor Label | Type | Position | Color | Intensity | Purpose |
|-------------|------|----------|-------|-----------|---------|
| Light_Hub_SunriseFill_East | PointLight | (2900, 2400, 250) | Warm orange | 4,500 lm | Simulates direct sunrise from east |
| Light_Hub_ShadowFill_West | PointLight | (1500, 2400, 200) | Cool blue-purple | 1,800 lm | Shadow fill — cool contrast |
| Light_Hub_GroundBounce | PointLight | (2100, 2400, 50) | Warm amber | 1,200 lm | Ground bounce from warm soil |

### Lumen Settings
- `r.Lumen.Reflections.Allow 1`
- `r.Lumen.GlobalIllumination.Allow 1`
- `r.VolumetricFog 1`
- `r.VolumetricFog.GridSizeZ 64`
- `r.FastSkyLUT 0` (high quality sky LUT)
- `r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget 300`

---

## Audio References — Sunrise Atmosphere

### Selected Freesound Assets

| ID | Name | Duration | Tags | Use Case |
|----|------|----------|------|----------|
| 644989 | Jungle / Forest Ambience | 39.5s | dawn, morning, birds, jungle, forest | Primary hub ambient loop |
| 749737 | denseforestwithbirds | 101.6s | birdsong, dense, field-recording | Secondary layered ambient |
| 813495 | AMBForst_Summer.Forest.Strong Wind | 288.4s | wind, forest, atmosphere | Wind layer for canopy movement |
| 335889 | Wind Through Trees | 284.9s | forest, leaves, birds, ambient | Gentle wind through prehistoric ferns |
| 800712 | AMBForst_Forest Birds Wind 01 | 30s | Birds, Breeze, Forest, Natural | Short loop for close-range bird calls |

### Audio Layering Strategy for Sunrise
```
Layer 1 (Base):    Freesound #644989 — Jungle/Forest Ambience (looped, -12dB)
Layer 2 (Wind):    Freesound #335889 — Wind Through Trees (looped, -18dB, randomized pitch ±5%)
Layer 3 (Birds):   Freesound #800712 — Forest Birds Wind (looped, -8dB, positional 3D)
Layer 4 (Detail):  Freesound #749737 — Dense Forest Birds (triggered randomly, 3D positioned)
```

### MetaSounds Blueprint Recommendation (for Audio Agent #16)
- Create `MS_Sunrise_Ambient` MetaSound source
- Blend layers based on `TimeOfDay` float (0.0=midnight, 0.25=sunrise, 0.5=noon, 0.75=sunset)
- At sunrise (0.2-0.3): bird layer at full volume, wind at 60%, base at 80%
- Spatial audio: birds positioned at tree actor locations in hub area

---

## Cinematic Lighting Notes

### Roger Deakins Reference — Sunrise Principles Applied
1. **Motivated light source**: Every fill light has a logical source (sun from east, sky from above, ground bounce)
2. **Color temperature contrast**: Warm key (3200K equivalent) vs cool fill (6500K equivalent) creates depth
3. **Volumetric atmosphere**: Morning mist is not decoration — it defines distance and scale
4. **Shadow length**: -30° pitch creates shadows 1.7x the height of objects — dinosaurs cast dramatic long shadows
5. **The invisible rule**: Player should feel "it's morning" without consciously noticing the lighting setup

### Emotional Intent
- **Mood**: Cautious optimism — a new day, but the world is still dangerous
- **Color story**: Gold and amber (warmth, life) vs blue-purple shadows (danger still lurking)
- **Atmosphere**: The mist says "this world is ancient and alive"

---

## Day/Night Cycle Palette Progression (All 4 Cycles)

| Cycle | Palette | Sun Pitch | Sun Color | Intensity | Fog |
|-------|---------|-----------|-----------|-----------|-----|
| 010 | Midday | -75° | White-hot | 75,000 lux | Heat haze |
| 011 | Stormy Night | -5° | Cold blue-grey | 12,000 lux | Heavy volumetric |
| 012 | Dusk Golden Hour | -8° | Golden-orange | 28,000 lux | Warm amber |
| **013** | **Sunrise Morning** | **-30°** | **Rose-gold** | **38,000 lux** | **Amber mist** |

---

## Files Created This Cycle
- `Docs/Lighting/Cycle013_SunrisePalette_AudioRefs.md` (this file)
- `Docs/Lighting/DayNightCycle_Spec.md` (day/night cycle complete specification)

## Next Agent (#09 — Character Artist)
The sunrise palette is now active. Character models will be lit with:
- Warm rose-gold rim light from east (dramatic silhouetting)
- Cool blue-purple fill from west (depth on character faces)
- Ground bounce amber (warm underlight on lower body)
This creates cinematic character lighting without any additional setup needed.
