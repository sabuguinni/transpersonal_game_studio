# Lighting & Atmosphere — Agent #08 System Notes
## Cycle: PROD_CYCLE_AUTO_20260620_004

---

## CRITICAL RULES (NEVER BREAK)

### 1. Sun Pitch MUST Be Negative
```python
a.set_actor_rotation(unreal.Rotator(roll=0.0, pitch=-45.0, yaw=45.0), False)
```
- `pitch=-45` → sun vector points DOWN → terrain illuminated ✅
- `pitch=+45` → sun vector points UP → terrain BLACK ❌
- Confirmed by SceneCapture2D screenshot test (17 Jun 2026)

### 2. PostProcess Exposure = AEM_MANUAL
```python
settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_MANUAL)
settings.set_editor_property("auto_exposure_bias", 1.0)
```
- AEM_HISTOGRAM or AEM_BASIC can over-darken → black screen
- ALWAYS audit PostProcessVolumes BEFORE rebuilding lighting stack

### 3. Fog = Exactly 1 ExponentialHeightFog
- Multiple fogs = visual artifacts
- Zero fogs = flat, lifeless world
- Sanity guard enforces this every cycle

### 4. Lumen GI Console Commands
```
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.MaxTraceDistance 20000
r.SkyAtmosphere.FastSkyLUT 1
```

---

## BIOME LIGHTING MAP

### BiomeA — Open Plains (center ~0,0)
- Primary: DirectionalLight Sun_Main (pitch=-45, intensity=10)
- Fill: SkyLight real-time (intensity=1.5)
- Fog: Volumetric (density=0.02, distance=6000)
- Mood: Bright, exposed, dangerous — player visible to dinosaurs

### BiomeB — Stone Ruins (X=50000, Y=50000)
- Added: 3× PointLight amber-orange (intensity=2000, radius=800)
  - PillarLight_A at (50000, 50000, 200)
  - PillarLight_B at (50300, 50000, 200)
  - PillarLight_C at (50150, 50260, 200)
- Mood: Amber warmth vs cool Lumen bounce — navigational beacons
- Gameplay: Pillar shadows = ambush zones for AI and stealth

### BiomeC — Dense Jungle (TBD)
- Planned: Filtered dappled light through canopy
- Technique: SpotLights with IES profiles + green-tinted fog layer
- Mood: Claustrophobic, tense, limited visibility

### BiomeD — Cave System (TBD)
- Planned: Near-total darkness with player-carried torch as primary light
- Technique: Dynamic PointLight attached to character + bioluminescent PointLights
- Mood: Maximum tension, survival horror

---

## POINT LIGHT BUDGET
| Zone | Count | Avg Intensity | Purpose |
|------|-------|---------------|---------|
| BiomeB Ruins | 3 | 2000 lm | Pillar base glow |
| Pending: Cave | 6 | 500 lm | Bioluminescent accents |
| Pending: Camp | 2 | 3000 lm | Campfire simulation |
| **Total Active** | **3** | — | — |
| **Budget Limit** | **20** | — | Performance cap |

---

## CINEMATOGRAPHY INTENT (Roger Deakins Principle)
> "Light doesn't illuminate — light means."

Each light source in this game has a **narrative purpose**:
- **Sun** = time of day, danger exposure, warmth of safety
- **Pillar lights** = ancient human presence, navigational beacon, warmth vs cold wild
- **Cave bioluminescence** = alien beauty, false safety, mystery
- **Campfire** = survival achieved, temporary safety, community

**Color palette established:**
- Cool blue-teal: Lumen sky bounce (ambient, neutral)
- Warm amber-orange: Human/fire presence (safety, civilization)
- Deep blue-purple: Shadow fill via volumetric fog (danger, unknown)
- Green-filtered: Dense jungle canopy (claustrophobia, predator territory)

---

## NEXT CYCLE PRIORITIES
1. Add SpotLight at cave entrance arch (pending asset placement from A#07)
2. Add RectLight rim lighting on stone wall slab
3. Begin BiomeC jungle canopy light filtering system
4. Day/night cycle blueprint — sun rotation over 24-minute real-time cycle
