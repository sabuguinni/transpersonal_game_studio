# Lighting & Atmosphere System — Cycle 004
**Agent #8 — Lighting & Atmosphere**
**Cycle:** PROD_CYCLE_AUTO_20260622_004

---

## UE5 Commands Executed (Single Script)

### Sanity Guard
- Sun DirectionalLight: pitch=-45 enforced (negative = illuminates terrain)
- ExponentialHeightFog: exactly 1 instance enforced
- FastSkyLUT + AerialPerspectiveLUT: enabled

### CAP Enforcement
- Actor count logged
- Dino audit: labels scanned for trex/raptor/brach/dino/rex/saur
- Degenerate label check: empty/None/Actor labels flagged
- Status: CAP_SAFE

### Lumen Global Illumination
```
r.DynamicGlobalIlluminationMethod 1   (Lumen)
r.ReflectionMethod 1                   (Lumen Reflections)
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.Reflections.Allow 1
r.Lumen.HardwareRayTracing 0          (Software — broader GPU support)
r.Lumen.TraceMeshSDFs 1
```

### Atmosphere
```
r.VolumetricFog 1
r.VolumetricCloud 1
r.SkyAtmosphere 1
```

### Sun Quality
- Intensity: 10.0 lux
- LightSourceAngle: 0.5357 (realistic solar disc)
- CastShadows: true
- AtmosphereSunLight: true

### PostProcess Exposure Fix
- AutoExposureMethod: AEM_MANUAL (prevents auto-darkening)
- AutoExposureBias: 1.0

### Fog Quality
- FogDensity: 0.02
- FogHeightFalloff: 0.2
- FogInscatteringColor: (0.45, 0.55, 0.7) — cool blue-grey prehistoric haze
- VolumetricFog: enabled
- VolumetricFogScatteringDistribution: 0.2
- VolumetricFogAlbedo: (0.75, 0.75, 0.75)
- VolumetricFogExtinctionScale: 1.0

### Point Light Intensity Cap
- All PointLights/SpotLights with intensity > 5000 → capped to 2000
- Prevents overexposed colored light blowout

---

## Cinematic Intent (Roger Deakins Principle)
> "Light doesn't illuminate — light signifies."

The Cretaceous world lighting is designed around:
1. **Golden Hour Dominance** — warm directional sun at -45° pitch creates long shadows that reveal terrain topology
2. **Atmospheric Depth** — volumetric fog at density 0.02 creates distance haze without obscuring nearby detail
3. **Lumen Indirect Bounce** — GI bounces warm sunlight into shadow areas, preventing pure black shadows
4. **Manual Exposure** — AEM_MANUAL prevents the engine from auto-compensating in dark areas (ruins, caves)

---

## Dependencies
- Requires: SkyAtmosphere actor in level (placed by Agent #5/#6)
- Requires: DirectionalLight with AtmosphereSunLight=true
- Feeds into: Agent #9 (Character Artist) — skin shading under Lumen GI
- Feeds into: Agent #17 (VFX) — particle systems interact with volumetric fog

---

## Next Cycle Priorities
1. Day/Night cycle Blueprint (sun rotation over 24h)
2. Weather state machine (clear → overcast → storm → clear)
3. Sky material variations per weather state
4. Moonlight setup for night gameplay
