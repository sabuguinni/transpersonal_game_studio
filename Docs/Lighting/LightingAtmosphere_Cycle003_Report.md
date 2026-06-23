# Lighting & Atmosphere — Agent #08 | PROD_CYCLE_AUTO_20260623_003

## Cycle Summary

### UE5 Lighting Stack Applied (Single ue5_execute)

All operations combined into one Python script per CRITERIO 2 OBRIGATORIO:

| System | Status | Parameters |
|--------|--------|------------|
| Sanity Guard — Sun pitch | ✅ | pitch=-35°, yaw=45° (golden hour angle) |
| Sanity Guard — Fog count | ✅ | Exactly 1 ExponentialHeightFog |
| Sanity Guard — FastSkyLUT | ✅ | r.SkyAtmosphere.FastSkyLUT 1 |
| Lumen GI | ✅ | r.DynamicGlobalIlluminationMethod=1, r.ReflectionMethod=1 |
| Golden Hour Sun Tint | ✅ | Color(255,220,160) — warm amber |
| SkyAtmosphere | ✅ | Verified/created |
| SkyLight | ✅ | intensity=1.5, real_time_capture=True |
| Volumetric Fog | ✅ | density=0.02, volumetric=True, extinction=0.4 |
| PostProcess Manual Exposure | ✅ | AEM_MANUAL, bias=1.0 (prevents auto-darkening) |
| Color Grading | ✅ | saturation=1.1, contrast=1.05 (warm prehistoric tone) |
| Bloom | ✅ | intensity=0.4 |
| Ambient Occlusion | ✅ | intensity=0.6 |
| Point Light Cap | ✅ | All lights >5000 lux capped to 2000 |
| CAP Audit | ✅ | Actor count + dino audit logged |
| Map Save | ✅ | /Game/Maps/MinPlayableMap |

### Lighting Design Intent

**Golden Hour — Cretaceous Savanna**
- Sun angle: -35° pitch (low horizon, long shadows)
- Color temperature: warm amber (255, 220, 160) — late afternoon
- Volumetric fog: atmospheric haze, 500m start distance, 80km cutoff
- Fog inscattering: cool blue-grey (0.6, 0.75, 0.9) for sky-ground contrast
- PostProcess: manual exposure prevents UE5 auto-darkening the scene
- Lumen: software GI (no ray tracing hardware required, better performance)

### Key Technical Decisions

1. **AEM_MANUAL exposure** — Confirmed fix for black screen issue (memory #6009)
   - Auto exposure was causing over-darkening in previous cycles
   - Manual bias=1.0 gives consistent, predictable illumination

2. **pitch=-35° (not -45°)** — Shallower angle for golden hour aesthetics
   - Creates longer shadows across terrain
   - More dramatic lighting for dinosaur silhouettes

3. **Volumetric fog extinction=0.4** — Balanced visibility
   - Enough atmosphere for depth cues without obscuring gameplay

4. **SkyLight real_time_capture=True** — Dynamic sky reflection
   - Captures SkyAtmosphere color changes automatically
   - No baked lighting required

### API Fallback Applied
- `generate_image` → FAIL (OpenAI 401)
- Fallback: `search_sounds` for prehistoric ambient audio reference

### Sound References Found
- Tropical forest daytime ambience (search executed)
- Results forwarded to Agent #16 (Audio Agent)

## Files Modified in UE5
- `/Game/Maps/MinPlayableMap` — Lighting stack updated and saved

## Next Agent (#09 — Character Artist)
- Lighting is configured for golden hour — characters will benefit from warm rim lighting
- SkyLight real_time_capture means character materials will reflect sky color dynamically
- PostProcess AO intensity=0.6 will add depth to character models
- Recommend character materials use Lumen-compatible PBR setup (no baked AO maps needed)

## Lighting Continuity Notes
- Sun tint: warm amber (255, 220, 160) — consistent across cycles
- Fog: volumetric, density 0.02 — subtle atmospheric depth
- Exposure: MANUAL — never revert to AUTO (causes black screen)
- All settings saved to MinPlayableMap
