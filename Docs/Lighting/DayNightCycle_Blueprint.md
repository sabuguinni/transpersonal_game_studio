# Day/Night Cycle — Blueprint Reference
**Agent:** #08 — Lighting & Atmosphere Agent  
**Status:** Design Complete — Implementation via Blueprint recommended

## Overview
The Cretaceous day/night cycle drives the emotional rhythm of the survival experience.
Dawn is hopeful and dangerous (predators active). Midday is harsh and bright (heat/thirst pressure).
Dusk is golden and tense (predators hunting). Night is terrifying (near-zero visibility, sound-driven).

## Time Periods & Lighting Intent

| Period | Time | Sun Pitch | Sun Intensity | Fog Color | Emotional Tone |
|--------|------|-----------|---------------|-----------|----------------|
| Pre-Dawn | 04:00-06:00 | -5° | 0.5 | Deep blue-purple | Dread, anticipation |
| Dawn | 06:00-08:00 | -15° | 3.0 | Warm orange-pink | Hope, danger |
| Morning | 08:00-11:00 | -35° | 8.0 | Light blue | Alert, active |
| Midday | 11:00-14:00 | -75° | 15.0 | White-yellow | Harsh, oppressive |
| Afternoon | 14:00-17:00 | -45° | 10.0 | Warm amber | Golden, beautiful |
| Dusk | 17:00-19:00 | -10° | 2.0 | Deep orange-red | Tense, hunting |
| Twilight | 19:00-20:30 | -2° | 0.3 | Purple-grey | Transition, fear |
| Night | 20:30-04:00 | +5° (below horizon) | 0.0 | Dark blue-black | Terror, survival |

## Blueprint Implementation (UE5)

### BP_DayNightManager (Actor Blueprint)
```
Variables:
- CurrentTimeOfDay: Float (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
- DayDurationSeconds: Float (default: 1200.0 = 20 real minutes per day)
- SunActor: DirectionalLight reference
- FogActor: ExponentialHeightFog reference
- SkyLightActor: SkyLight reference

Tick Event:
  CurrentTimeOfDay += DeltaTime / DayDurationSeconds
  If CurrentTimeOfDay >= 1.0: CurrentTimeOfDay = 0.0
  Call UpdateLighting(CurrentTimeOfDay)

UpdateLighting(T: Float):
  SunPitch = Lerp curve based on T
  SunIntensity = Lerp curve based on T
  FogColor = Lerp curve based on T
  Apply to SunActor, FogActor, SkyLightActor
```

### Curve Assets Needed
- `CRV_SunPitch` — Float curve, 0-1 time, -90 to +5 pitch values
- `CRV_SunIntensity` — Float curve, 0-1 time, 0.0 to 15.0 intensity
- `CRV_FogR/G/B` — Three float curves for fog color channels
- `CRV_SkyLightIntensity` — Float curve, 0.1 to 2.0

## Weather Integration
Day/night cycle interacts with weather system:
- Storm clouds reduce sun intensity by 60-80%
- Rain increases fog density (0.02 → 0.08)
- Lightning flashes: SpotLight with random interval, intensity spike 50000 → 0
- Overcast: SkyLight color shifts to grey, intensity drops to 0.8

## Performance Notes
- SkyLight recapture: every 30 seconds during day, every 60 at night
- Lumen GI updates automatically with sun movement
- VolumetricFog: reduce GridSizeZ to 32 at night (less variation needed)
- PostProcess exposure: auto-adjust min/max based on time of day

## Audio Triggers (for Agent #16)
- Dawn: bird calls, distant dinosaur roars begin
- Midday: insect chorus, heat shimmer audio
- Dusk: hunting calls, urgency music layer
- Night: silence broken by predator sounds, heartbeat ambient

## Survival Mechanics Integration (for Agent #03)
- Temperature: peaks at midday (heat damage risk), drops at night (cold damage risk)
- Visibility: night reduces player sight range by 80%
- Predator activity: nocturnal species active at night, diurnal during day
- Plant growth: some resources only harvestable at specific times
