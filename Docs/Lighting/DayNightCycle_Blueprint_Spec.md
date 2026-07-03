# Day/Night Cycle Blueprint Specification
## Agent #08 — Lighting & Atmosphere Agent | Cycle AUTO_20260703_001

### Overview
This document specifies the Blueprint implementation for the full 24-hour day/night cycle
for the Cretaceous survival game. This is a **Blueprint-only** implementation (no C++ required)
using UE5's Timeline and DirectionalLight rotation.

---

## Blueprint: BP_DayNightCycle

### Actor Class: Actor (AActor subclass via Blueprint)
Place ONE instance in the persistent level.

### Components
- `TimelineComponent` — drives the 24-hour cycle
- `DirectionalLightReference` (soft object reference) → Sun_Midday_Main

### Timeline Curve: DayNightCurve
- Duration: 1440 seconds (= 24 real-minutes for 1 game-day, configurable)
- Float track: `SunPitch` — maps 0→1440 to sun pitch angle

| Time (s) | Sun Pitch | Phase |
|----------|-----------|-------|
| 0 | -90° | Midnight |
| 180 | -90° | Pre-dawn |
| 300 | -5° | Sunrise |
| 360 | -20° | Early morning |
| 480 | -45° | Mid-morning |
| 720 | -60° | Noon |
| 900 | -45° | Afternoon |
| 1080 | -20° | Late afternoon |
| 1140 | -8° | Golden hour |
| 1200 | -3° | Sunset |
| 1260 | -90° | Dusk |
| 1440 | -90° | Midnight |

### Intensity Curve: SunIntensityCurve
| Time (s) | Intensity (lux) | Phase |
|----------|-----------------|-------|
| 0 | 0 | Night |
| 300 | 5000 | Sunrise |
| 480 | 30000 | Morning |
| 720 | 75000 | Noon |
| 1080 | 40000 | Afternoon |
| 1140 | 15000 | Golden hour |
| 1200 | 2000 | Sunset |
| 1260 | 0 | Night |

### Color Curve: SunColorCurve (Linear Color)
| Phase | R | G | B | Description |
|-------|---|---|---|-------------|
| Sunrise | 1.0 | 0.5 | 0.2 | Deep orange |
| Morning | 1.0 | 0.85 | 0.6 | Warm yellow |
| Noon | 1.0 | 0.97 | 0.88 | Warm white |
| Afternoon | 1.0 | 0.9 | 0.7 | Slightly warm |
| Golden Hour | 1.0 | 0.6 | 0.2 | Deep gold |
| Sunset | 1.0 | 0.3 | 0.1 | Red-orange |
| Night | 0.1 | 0.15 | 0.3 | Cold blue-black |

---

## Fog Density Curve: FogDensityCurve
| Phase | Density | Volumetric Extinction |
|-------|---------|----------------------|
| Night | 0.08 | 1.5 |
| Sunrise | 0.06 | 1.2 (morning mist) |
| Midday | 0.02 | 0.5 |
| Golden Hour | 0.04 | 0.8 |
| Night | 0.08 | 1.5 |

---

## Blueprint Event Graph Nodes

```
Event BeginPlay
  → Set Timer by Function Name (FunctionName="AdvanceCycle", Time=0.1, Looping=true)

Function AdvanceCycle:
  → Get World Delta Seconds
  → Add to CurrentTime (clamped 0→1440, wraps)
  → Sample DayNightCurve at CurrentTime → Set Sun Pitch
  → Sample SunIntensityCurve at CurrentTime → Set Sun Intensity
  → Sample SunColorCurve at CurrentTime → Set Sun Color
  → Sample FogDensityCurve at CurrentTime → Set Fog Density

Exposed Variables (EditAnywhere):
  - DayLengthSeconds (float, default=1440)
  - StartTimeOfDay (float, default=720 = noon)
  - bPauseCycle (bool, default=false)
  - TimeMultiplier (float, default=1.0)
```

---

## Integration Notes for Agent #09 (Character Artist)
- Characters spawned during MIDDAY palette will be lit at 75,000 lux warm white
- The cycle will eventually run continuously — character materials must look good at ALL phases
- Subsurface scattering on skin should be tuned for the NOON baseline (most common gameplay time)
- Night phase uses cold blue ambient — ensure characters are still readable (not too dark)

## Integration Notes for Agent #16 (Audio Agent)
- Day phase transitions should trigger ambient sound layer changes:
  - Night → Dawn: silence → bird calls begin
  - Dawn → Morning: birds peak, insects fade
  - Midday: insects/cicadas dominant
  - Dusk: birds return, then night insects
  - Night: crickets, distant predator sounds

---

## Current Cycle Status
- ✅ Sun configured: pitch=-60°, intensity=75,000 lux, warm white
- ✅ SkyLight: real_time_capture=True, intensity=1.5
- ✅ Fog: light daytime, volumetric enabled
- ✅ Hub fill lights: 3 actors at (2100, 2400)
- ✅ Lumen GI + Reflections enabled
- ✅ FastSkyLUT=1 (performance)
- 📋 Blueprint BP_DayNightCycle: SPEC READY — awaiting Blueprint creation in next cycle
