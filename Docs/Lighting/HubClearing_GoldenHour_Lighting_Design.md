# Hub Clearing — Golden Hour Lighting Design
## Agent #08 — Lighting & Atmosphere | PROD_CYCLE_AUTO_20260704_002

---

## Scene Overview

**Location**: Hub Clearing — World Coordinates X=2100, Y=2400  
**Time of Day**: Early Morning / Golden Hour (~07:00–09:00 prehistoric time)  
**Mood**: Cinematic discovery — warm, alive, slightly mysterious  
**Composition Anchor**: Ruin complex ring (6 pillars, r=600) with north entrance arch

---

## Lighting Rig — Actor Inventory

### Primary Sun (DirectionalLight)
| Property | Value | Rationale |
|----------|-------|-----------|
| Pitch | -45° | Mid-morning angle — long shadows through pillars |
| Intensity | 75,000 lux | Full daylight floor (CAP enforced) |
| Color | RGB(1.0, 0.95, 0.85) | Warm white — golden hour tint |
| atmosphere_sun_light | True | Drives sky atmosphere scattering |
| Cast Shadows | True | Hard shadow lines through arch |

### Golden Shaft RectLight (`RectLight_GoldenShaft_Hub_001`)
| Property | Value | Rationale |
|----------|-------|-----------|
| Location | X=2100, Y=3100, Z=450 | Above north arch entrance |
| Rotation | Pitch=-55°, Yaw=-90° | Angled down into clearing |
| Intensity | 8,000 cd/m² | Visible shaft without washing out scene |
| Color | RGB(1.0, 0.85, 0.5) | Deep golden amber — cinematic shaft |
| Source Width | 200 cm | Tight beam through arch gap |
| Source Height | 300 cm | Vertical shaft shape |
| Attenuation Radius | 1,500 cm | Reaches altar slab at center |
| Cast Volumetric Shadow | True | Shaft visible through mist |

### Dappled Canopy Lights (6× PointLight)
**Labels**: `DappleLight_Hub_001` through `DappleLight_Hub_006`

| Property | Value | Rationale |
|----------|-------|-----------|
| Radius from hub | 450 cm | Scattered around pillar ring |
| Intensity | 800–1,400 cd/m² | Varied for natural randomness |
| Color | RGB(1.0, 0.92, 0.7) | Warm filtered sunlight |
| Attenuation Radius | 350 cm | Tight pools — dapple effect |
| Cast Shadows | False | Performance — no shadow cost |
| Height | Hub Z + 80 cm | Ground-level dapple patches |

**Placement angles** (offset from pillar positions by 0.3 rad):
- 001: ~17° — NNE of hub
- 002: ~77° — ENE of hub  
- 003: ~137° — ESE of hub
- 004: ~197° — SSW of hub
- 005: ~257° — WSW of hub
- 006: ~317° — WNW of hub

### SkyLight
| Property | Value |
|----------|-------|
| real_time_capture | True |
| Type | Real-time capture |

---

## Atmospheric Mist — ExponentialHeightFog

| Property | Value | Rationale |
|----------|-------|-----------|
| Fog Density | 0.025 | Subtle — pools in low areas |
| Height Falloff | 0.4 | Ground-hugging mist |
| Inscattering Color | RGB(0.7, 0.85, 1.0) | Cool blue — contrast with warm sun |
| Max Opacity | 0.6 | Visible but not obscuring |
| Start Distance | 200 cm | Doesn't fog near player |
| Volumetric Fog | True | Physical light interaction |
| Scattering Distribution | 0.35 | Forward scatter — shaft visibility |
| Albedo | RGB(0.9, 0.95, 1.0) | Near-white mist particles |
| Extinction Scale | 1.2 | Slight density boost for ruins |

---

## Lumen Configuration

```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.Lumen.TraceMeshSDFs.Allow 1
r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget 300
r.VolumetricFog 1
r.VolumetricFog.GridPixelSize 8
r.SkyAtmosphere.FastSkyLUT 1
```

**Why these settings**:
- `TraceMeshSDFs`: Accurate GI bounce off stone pillar surfaces
- `NumProbesToTraceBudget 300`: Higher quality probe tracing for complex ruin geometry
- `VolumetricFog.GridPixelSize 8`: Fine grid for visible light shafts through arch

---

## Cinematic Intent (Roger Deakins Principle)

> "The light doesn't illuminate — the light means."

**What this lighting communicates**:
1. **Golden shaft through arch** → The north entrance is the way in. Light guides the player.
2. **Cool mist vs warm sun** → Tension between the ancient cold stone and the living warm world
3. **Dappled canopy patches** → Life above — the jungle is dense, alive, watching
4. **Long pillar shadows** → Time of day anchor — morning, not noon — the day is just beginning
5. **Altar slab in shaft** → The center of the clearing is important. The light says so.

---

## Hero Screenshot Composition Notes

**Frame target**: PlayerStart at origin looking toward X=2100, Y=2400  
**Key elements in frame**:
- North arch silhouette with golden shaft cutting through
- 2-3 pillars with mist pooling at base
- Altar slab catching direct shaft light
- Background: dense canopy with dapple patches
- Foreground: ground mist wisps

**Lighting contrast ratio**: ~8:1 (shaft highlight to mist shadow) — cinematic without crushing blacks

---

## CAP Compliance (PROD_CYCLE_AUTO_20260704_002)

| Check | Status |
|-------|--------|
| Bridge validation | ✅ bridge_ok, world loaded |
| Sun pitch ≤ -30° | ✅ -45° enforced |
| Sun intensity ≥ 10,000 lux | ✅ 75,000 lux |
| Duplicate DirectionalLights removed | ✅ |
| Fog dedup (exactly 1) | ✅ |
| SkyLight real_time_capture=True | ✅ |
| FastSkyLUT=1 | ✅ |
| Level saved | ✅ |

---

## Dependencies for Agent #09 (Character Artist)

The lighting rig at X=2100, Y=2400 is optimized for:
- **Character skin tones**: Warm golden key light from north arch + cool fill from mist
- **Character silhouette**: Strong backlight opportunity when player stands in arch
- **Animation visibility**: 6 dapple patches create natural movement highlights
- **MetaHuman compatibility**: Lumen GI will correctly bounce off character surfaces

Agent #09 should place the MetaHuman character at the arch entrance (X=2100, Y=3000, Z=100) for the hero screenshot — the golden shaft will rim-light the character perfectly.

---

## Next Lighting Priorities (Agent #08 Future Cycles)

1. **Sunset variant**: Shift DirectionalLight to pitch=-10°, yaw=+45°, deep orange (for danger/combat mood)
2. **Night variant**: Disable DirectionalLight, enable moon (low-intensity cool blue DirectionalLight)
3. **Storm variant**: Reduce sun intensity to 5,000 lux, add lightning SpotLight flash system
4. **Cave interior**: Point lights only, no DirectionalLight contribution, warm fire tones
5. **Import Meshy GLB**: Replace placeholder pillar meshes with `StoneRuinPillar` asset when pipeline delivers

---

*Lighting & Atmosphere Agent #08 — PROD_CYCLE_AUTO_20260704_002*
