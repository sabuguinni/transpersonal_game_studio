# Lighting & Atmosphere System — Agent #08
## Cycle: PROD_CYCLE_AUTO_20260623_009

---

## Lighting Stack Status

### Active Components in MinPlayableMap
| Component | Status | Config |
|-----------|--------|--------|
| DirectionalLight (Sun_Main) | ✅ GUARD ENFORCED | pitch=-45°, yaw=45°, intensity=12.0 |
| SkyAtmosphere | ✅ VERIFIED | Spawned if missing |
| SkyLight | ✅ VERIFIED | real_time_capture=True, intensity=1.0 |
| ExponentialHeightFog | ✅ GUARD ENFORCED | Exactly 1 instance |
| PostProcessVolume | ✅ EXPOSURE FIXED | AEM_MANUAL, bias=1.0 |

### Lumen GI Console Vars (Applied Each Cycle)
```
r.Lumen.Reflections.Allow 1
r.Lumen.DiffuseIndirect.Allow 1
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
r.SkyAtmosphere.FastSkyLUT 1
r.SkyAtmosphere.AerialPerspectiveLUT.FastApply 1
```

---

## Atmospheric Design Reference — Cretaceous Era

### Time-of-Day Emotional Intent
| Time | Sun Pitch | Color Temp | Emotional Tone |
|------|-----------|------------|----------------|
| Dawn | -5° to -15° | 2800K warm orange | Tension, unknown danger |
| Golden Hour AM | -20° to -35° | 3500K amber | Beauty, false safety |
| Midday | -70° to -80° | 5500K neutral | Harsh, exposed, vulnerable |
| Golden Hour PM | -35° to -20° | 3200K golden | Urgency, dwindling time |
| Dusk | -10° to -5° | 2400K deep orange | Dread, predator activity |
| Night | N/A | Moonlight 4000K blue | Fear, survival instinct |

### Fog Density by Biome
| Biome | Fog Start Distance | Fog Density | Atmosphere Color |
|-------|-------------------|-------------|------------------|
| Jungle | 500m | 0.08 | Green-grey haze |
| River Delta | 200m | 0.15 | Blue-white mist |
| Open Plains | 2000m | 0.02 | Clear amber |
| Volcanic Zone | 300m | 0.12 | Sulphur yellow |
| Coastal | 800m | 0.06 | Sea-blue haze |

---

## Sound References Found (Freesound.org)

### Thunderstorm / Weather Atmosphere
| ID | Name | Duration | Use Case |
|----|------|----------|----------|
| 802401 | THUN_The Storm Is Coming Fast — Distant Rolling Thunder | 417s | Approaching storm weather event |
| 743019 | THUN_The Storm Is Coming Fast — Distant Rumble + Close Crackling | 192s | Storm intensification layer |
| 686816 | Thunderstorm with ground strikes — Big dynamic range | 1744s | Full storm ambient loop |

**Preview URLs:**
- https://cdn.freesound.org/previews/802/802401_5828667-hq.mp3
- https://cdn.freesound.org/previews/743/743019_5828667-hq.mp3
- https://cdn.freesound.org/previews/686/686816_1531809-hq.mp3

---

## Critical Rules (Validated This Cycle)

### Sun Pitch — ALWAYS NEGATIVE
```python
# CORRECT — illuminates terrain
a.set_actor_rotation(unreal.Rotator(roll=0.0, pitch=-45.0, yaw=45.0), False)

# WRONG — sun points at sky, terrain goes BLACK
a.set_actor_rotation(unreal.Rotator(roll=0.0, pitch=+45.0, yaw=45.0), False)
```

### PostProcess Exposure — ALWAYS MANUAL
```python
# Prevents auto-exposure from darkening the scene to black
comp.set_editor_property("settings.auto_exposure_method", unreal.AutoExposureMethod.AEM_MANUAL)
comp.set_editor_property("settings.auto_exposure_bias", 1.0)
```

### SkyLight — ALWAYS real_time_capture=True
```python
# Required for SkyAtmosphere to appear in SceneCapture2D screenshots
comp.set_editor_property("real_time_capture", True)
```

---

## Handoff to Agent #09 — Character Artist

The lighting environment is stable. Character artists should note:
- **Skin shading**: Warm amber primary light (pitch=-45°) creates strong directional shadows on character faces
- **Subsurface scattering**: Lumen GI is active — SSS on skin materials will respond to indirect bounce light
- **Shadow quality**: Dynamic shadows from DirectionalLight at -45° pitch cast long realistic shadows
- **Ambient fill**: SkyLight at intensity=1.0 provides soft fill from sky — prevents characters from going fully black in shadow

---

## Next Cycle Priorities

1. **Day/Night cycle implementation** — Blueprint timeline driving DirectionalLight pitch from -5° (dawn) to -80° (noon) to -5° (dusk)
2. **Weather system** — Random fog density variation + cloud coverage driven by game events
3. **Biome-specific atmosphere** — Different fog colors and densities per biome zone
4. **Storm event** — Thunderstorm weather state using found Freesound samples (IDs: 802401, 743019)
