# Agent #08 — Lighting & Atmosphere — Cycle PROD_CYCLE_AUTO_20260705_004

## Status: DEGRADED MODE (Bridge DOWN + generate_image API 401)

### Bridge Status
- ue5_execute: TIMEOUT (bridge not responding — 4 consecutive cycles)
- generate_image: FAIL (401 Unauthorized — OpenAI API key invalid/expired)
- All visual production tools unavailable this cycle

---

## Lighting Design Specification (Ready for when bridge resumes)

### PRIMARY SUN — DirectionalLight "Sun_Directional_001"
```
Intensity:        75,000 lux (bright tropical midday)
Color:            RGB(255, 247, 224) — warm white daylight
Rotation:         Pitch=-45, Yaw=30, Roll=0
atmosphere_sun_light: True
cast_shadows:     True
shadow_distance:  50,000 units (movable primitives)
shadow_amount:    0.8
```

### SKY ATMOSPHERE — "SkyAtmosphere_001"
```
Type:             Physical sky simulation
Rayleigh scattering: Default (blue sky)
Mie scattering:   Slight haze for tropical atmosphere
```

### SKY LIGHT — "SkyLight_001"
```
real_time_capture: True
intensity:         1.5
Source type:       SLS_CapturedScene
```

### EXPONENTIAL HEIGHT FOG — "HeightFog_001"
```
fog_density:          0.008 (very subtle — does NOT obscure scene)
fog_height_falloff:   0.2
inscattering_color:   RGB(153, 191, 229) — slight blue-green atmospheric
start_distance:       2,000 units
cutoff_distance:      80,000 units
```

### LUMEN GLOBAL ILLUMINATION
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
r.Lumen.TraceMeshSDFs 1
r.Lumen.ScreenProbeGather.RadianceCache.NumProbeTracesBudget 200
```

### POST PROCESS VOLUME — "PostProcess_Global_001"
```
Location:   (2100, 2400, 300) — centered on hub clearing
Unbound:    True (affects entire world)
```

---

## Hub Clearing Lighting Intent (X=2100, Y=2400)

The hero composition at (2100, 2400) must read as:
- **BRIGHT TROPICAL MIDDAY** — not dusk, not dawn, not overcast
- **Hard directional shadows** from sun at -45 pitch
- **Warm golden-white light** on dinosaur skin and vegetation
- **Subtle atmospheric depth** — distant trees slightly hazy, foreground crisp
- **Lumen GI bounce** — underside of leaves lit by ground-reflected light
- **No fog obscuring** the clearing — fog starts at 2000 units distance

### Emotional Intent (per Roger Deakins principle)
The light at the hub clearing communicates: **dangerous beauty**. 
The sun is bright and warm — this world is alive and lush. But the hard shadows 
hide predators. The player feels exposed in the clearing, drawn to the beauty 
but aware of the danger. Light serves the survival game tension.

---

## Anti-Pattern Guards (from Brain Memory imp=20)

1. **NEVER spawn duplicate DirectionalLights** — audit existing before spawning
2. **NEVER set intensity < 10,000 lux** — server guard enforces 10,000 floor
3. **NEVER modify viewport camera** — SceneCapture2D only for screenshots
4. **NEVER create .cpp/.h files** — binary is pre-built, UHT has 218 errors
5. **ALWAYS set atmosphere_sun_light=True** on the DirectionalLight

---

## Degraded Mode Protocol Applied

Per Brain Memory (imp=10, reflection_agent_auto):
- Bridge DOWN confirmed (4 consecutive cycles: 001, 002, 003, 004)
- generate_image API 401 confirmed (OpenAI key invalid)
- DEGRADED MODE: GitHub documentation only
- ZERO ue5_execute retries (bridge process is dead)
- Hugo intervention required to restart bridge

### What Hugo Needs to Do
1. Restart UE5 Editor bridge process on server
2. Renew/replace OpenAI API key in orchestrator environment
3. Verify Remote Control Plugin is enabled on port 30010

---

## Script Ready for Execution (when bridge resumes)

The full lighting Python script is queued as command_id=28981.
When bridge restarts, this command should execute automatically.

Script summary:
- Audits existing lights (prevents duplicates)
- Configures/spawns DirectionalLight at 75,000 lux
- Configures/spawns SkyAtmosphere
- Configures/spawns SkyLight with real_time_capture
- Spawns subtle ExponentialHeightFog
- Enables Lumen via console commands
- Spawns global PostProcessVolume (unbound)
- Saves level

---

## Deliverables This Cycle
- [DOC] This specification file — complete lighting blueprint
- [QUEUED] ue5_execute command_id=28981 — full lighting setup (pending bridge restart)
- [BLOCKED] generate_image — OpenAI API key expired (Hugo action required)

## Next Agent (#09 — Character Artist)
- Bridge is DOWN — apply DEGRADED MODE (GitHub only)
- Do NOT attempt ue5_execute or generate_image
- Document character/MetaHuman specifications in GitHub
- When bridge resumes, lighting will be configured per this spec
