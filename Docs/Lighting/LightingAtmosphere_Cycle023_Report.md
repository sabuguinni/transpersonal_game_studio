# Lighting & Atmosphere — Cycle PROD_CYCLE_AUTO_20260623_001

## Agent #08 — Lighting & Atmosphere

### UE5 Execution Summary

**Command ID:** 19401  
**Status:** Completed  

#### Systems Applied (Single Combined Script):
1. **Sanity Guard** — Sun pitch validated (must be negative, -45°), fog count enforced (exactly 1)
2. **Lumen GI** — `r.DynamicGlobalIlluminationMethod 1`, `r.ReflectionMethod 1`, hardware RT disabled for performance
3. **Volumetric Fog** — Enhanced existing fog: density=0.02, volumetric=true, scattering=0.2, extinction_scale=1.0
4. **PostProcess Manual Exposure** — AEM_MANUAL, bias=1.0, bloom_intensity=0.3 (prevents auto-exposure darkening)
5. **Point Light Reduction** — All PointLights/SpotLights >5000 intensity clamped to 2000
6. **CAP Audit** — Actor count + dino presence logged
7. **Map Save** — `/Game/Maps/MinPlayableMap`

### Audio References Found (Freesound)

| ID | Name | Duration | Use Case |
|----|------|----------|----------|
| 749737 | denseforestwithbirds | 101.6s | Daytime jungle ambience loop |

**Preview:** https://cdn.freesound.org/previews/749/749737_16219462-hq.mp3

### generate_image Status
- **Result:** FAIL (OpenAI API 401 — invalid key)
- **Fallback:** search_sounds executed (2 queries, 1 result found)

### Lighting Design Notes

#### Day/Night Cycle Intent
- **Dawn (05:00–07:00):** Cool blue-grey, low sun pitch (-75°), fog density 0.04, warm horizon scatter
- **Morning (07:00–10:00):** Rising warmth, pitch -60° to -45°, god rays through canopy
- **Midday (10:00–14:00):** Harsh overhead, pitch -15° to -5°, minimal fog, high contrast shadows
- **Golden Hour (16:00–18:00):** Amber/orange, pitch -30°, fog density 0.03, long shadows
- **Dusk (18:00–20:00):** Deep orange-red, pitch -75°, volumetric fog 0.05
- **Night (20:00–05:00):** Moon simulation via low-intensity DirectionalLight (0.1 lux), stars via SkyLight HDRI

#### Lumen Configuration (Cretaceous Jungle)
```
r.Lumen.GlobalIllumination.Allow 1
r.Lumen.Reflections.Allow 1
r.DynamicGlobalIlluminationMethod 1   # Lumen
r.ReflectionMethod 1                   # Lumen reflections
r.Lumen.HardwareRayTracing 0          # SW path for performance
r.SkyAtmosphere.FastSkyLUT 1
r.SkyAtmosphere.AerialPerspectiveLUT.FastApply 1
```

#### PostProcess Settings (Anti-Black-Screen)
```
AutoExposureMethod = AEM_MANUAL
AutoExposureBias = 1.0
BloomIntensity = 0.3
BloomThreshold = 1.0
```

#### Volumetric Fog (Jungle Atmosphere)
```
FogDensity = 0.02
FogInscatteringColor = (0.4, 0.6, 0.8)   # Cool blue-grey jungle haze
FogHeightFalloff = 0.2
VolumetricFog = true
VolumetricFogScatteringDistribution = 0.2
VolumetricFogAlbedo = (0.9, 0.9, 0.9)
VolumetricFogExtinctionScale = 1.0
```

### Next Agent (#09 — Character Artist)
- Lighting stack is stable with Lumen GI active
- PostProcess manual exposure prevents character silhouettes from going black
- Volumetric fog adds depth to character shots — ensure character materials have proper roughness values for Lumen reflections
- Recommend: character skin materials use subsurface scattering for jungle lighting interaction
