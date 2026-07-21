# Lighting Palette: Bright Midday — Cycle PROD_CYCLE_AUTO_20260703_001
## Agent #08 — Lighting & Atmosphere Agent

### Palette: CRETACEOUS MIDDAY DAYLIGHT

**Emotional Intent:** Oppressive tropical heat. The world feels alive and dangerous in full light.
No hiding in shadows — the predators hunt in daylight too. Roger Deakins principle:
*light reveals danger, not just beauty.*

---

## Sun Configuration
| Property | Value | Rationale |
|----------|-------|-----------|
| Actor Label | `Sun_Midday_Main` | Single authoritative sun |
| Pitch | -60° | High noon, near-zenith tropical sun |
| Yaw | 45° | NE→SW diagonal for dramatic shadow angles |
| Intensity | 75,000 lux | Bright tropical midday (UE5 physical units) |
| Color | RGB(1.0, 0.97, 0.88) | Warm white — slightly yellow at zenith |
| atmosphere_sun_light | true | Drives SkyAtmosphere scattering |
| cast_shadows | true | Sharp midday shadows |
| indirect_lighting_intensity | 1.2 | Boosted bounce for dense canopy |

## Sky & Atmosphere
| Component | Setting | Value |
|-----------|---------|-------|
| SkyAtmosphere | Single instance enforced | CAP dedup |
| SkyLight | real_time_capture | true |
| SkyLight | intensity | 1.5 |
| ExponentialHeightFog | fog_density | 0.02 (light daytime haze) |
| ExponentialHeightFog | inscattering_color | RGB(0.6, 0.75, 1.0) — sky blue |
| ExponentialHeightFog | volumetric_fog | true |
| ExponentialHeightFog | extinction_scale | 0.5 (light, airy) |
| ExponentialHeightFog | height_falloff | 0.2 |

## Hub Fill Lights (X=2100, Y=2400)
| Label | Type | Intensity | Color | Purpose |
|-------|------|-----------|-------|---------|
| `Light_HubFill_North` | PointLight | 8,000 lm | Warm white (1.0, 0.95, 0.8) | Key fill from NE |
| `Light_HubFill_South` | PointLight | 5,000 lm | Cool sky (0.85, 0.92, 1.0) | Shadow fill from SW |
| `Light_HubFill_Ambient` | PointLight | 3,000 lm | Sky blue (0.7, 0.85, 1.0) | Overhead ambient dome |

## Lumen Settings
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.Lumen.DiffuseIndirect.Allow 1
r.RayTracing.Shadows 0
r.SkyAtmosphere.FastSkyLUT 1
r.VolumetricFog 1
```

---

## Lighting Palette History (this cycle chain)
| Cycle | Palette | Sun Pitch | Intensity |
|-------|---------|-----------|-----------|
| AUTO_20260702_011 | Stormy Night | -5° | Low |
| AUTO_20260702_012 | Dusk Golden Hour | -8° | Medium-warm |
| AUTO_20260702_013 | Sunrise Morning | -45° | Rising |
| **AUTO_20260703_001** | **Bright Midday** | **-60°** | **75,000 lux** |

---

## CAP Enforcement Rules (enforced every cycle)
1. **ONE DirectionalLight** — extras destroyed on sight
2. **ONE SkyAtmosphere** — extras destroyed on sight
3. **ONE SkyLight** — extras destroyed on sight
4. **ONE ExponentialHeightFog** — extras destroyed on sight
5. **Sun intensity FLOOR: 10,000 lux** — never below (server guard)
6. **Hub (2100, 2400) MUST read as bright daytime** — fill lights ensure this

---

## Handoff to Agent #09 (Character Artist)
The lighting environment is set for bright midday. Character artists should note:
- **Skin tones** will be lit with warm top-light (pitch -60°)
- **Subsurface scattering** on skin will be visible — configure SSS radius accordingly
- **Eye reflections** will catch the warm sun from above-right
- **Costume/hide materials** should have low-medium roughness to catch midday highlights
- Hub area (2100, 2400) has 3 fill lights — characters placed here will be well-lit for hero screenshots
