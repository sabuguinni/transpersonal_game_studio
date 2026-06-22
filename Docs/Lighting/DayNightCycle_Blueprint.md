# Day/Night Cycle — Blueprint Implementation Guide
## Agent #8 — Lighting & Atmosphere | Transpersonal Game Studio

---

## Overview
The day/night cycle drives the emotional and gameplay rhythm of the prehistoric survival experience.
A full 24-hour cycle should complete in **20 real-time minutes** (configurable).

---

## Core Architecture

### Blueprint: BP_DayNightCycle (Actor)
Place one instance in MinPlayableMap at origin (0,0,0).

```
Variables:
  - TimeOfDay (float, 0.0–24.0) — current hour
  - DayDuration (float, default=1200.0) — seconds per full day
  - SunActor (DirectionalLight ref)
  - MoonActor (DirectionalLight ref, optional)
  - SkyLightActor (SkyLight ref)
  - FogActor (ExponentialHeightFog ref)

Event Tick:
  1. TimeOfDay += (DeltaTime / DayDuration) * 24.0
  2. If TimeOfDay >= 24.0: TimeOfDay = 0.0
  3. Call UpdateSunPosition(TimeOfDay)
  4. Call UpdateFogDensity(TimeOfDay)
  5. Call UpdateSkyLightIntensity(TimeOfDay)
```

### Sun Position Curve
```
Hour  | Pitch  | Yaw   | Intensity | Color
------|--------|-------|-----------|------------------
0.0   | +10°   | 180°  | 0.0       | Night (no sun)
5.0   | -2°    | 90°   | 0.5       | Pre-dawn grey
6.0   | -5°    | 80°   | 2.0       | Dawn orange-pink
8.0   | -25°   | 60°   | 6.0       | Morning warm gold
12.0  | -75°   | 45°   | 10.0      | Noon white-yellow
16.0  | -30°   | 20°   | 7.0       | Afternoon amber
18.0  | -5°    | -10°  | 2.5       | Dusk orange-red
19.0  | +5°    | -20°  | 0.3       | Twilight purple
20.0  | +10°   | -30°  | 0.0       | Night
```

### Fog Density by Time
```
Dawn (5-7h):   density=0.04, inscatter=(0.9, 0.6, 0.4) — warm morning mist
Day (8-16h):   density=0.02, inscatter=(0.6, 0.7, 0.9) — clear blue-grey
Dusk (17-19h): density=0.05, inscatter=(0.8, 0.4, 0.2) — dusty orange
Night (20-4h): density=0.08, inscatter=(0.1, 0.1, 0.3) — dark blue night fog
```

### SkyLight Intensity by Time
```
Day (6-18h):   intensity=1.5, real_time_capture=True
Dusk/Dawn:     intensity=0.8
Night (20-4h): intensity=0.3 (moonlight simulation)
```

---

## Weather System Integration

### Weather States
| State | Fog Density | Sun Intensity | Volumetric Fog Extinction |
|---|---|---|---|
| Clear | 0.02 | 10.0 | 0.15 |
| Overcast | 0.06 | 3.0 | 0.4 |
| Light Rain | 0.08 | 1.5 | 0.6 |
| Heavy Rain | 0.15 | 0.5 | 1.2 |
| Storm | 0.20 | 0.2 | 2.0 |
| Fog | 0.25 | 4.0 | 3.0 |

### Weather Transition
- Blend duration: 60–300 seconds
- Use Timeline or Lerp in Blueprint
- Trigger weather changes via gameplay events (e.g., player enters new biome)

---

## UE5 Python Implementation (Quick Setup)

```python
import unreal

# Find existing DirectionalLight (Sun)
actors = unreal.EditorLevelLibrary.get_all_level_actors()
sun = None
for a in actors:
    if a.get_class().get_name() == "DirectionalLight":
        sun = a
        break

if sun:
    # Set to golden hour (16:00)
    sun.set_actor_rotation(unreal.Rotator(roll=0.0, pitch=-30.0, yaw=20.0), False)
    comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    if comp:
        comp.set_editor_property("intensity", 7.0)
        # Warm afternoon color temperature
        comp.set_editor_property("light_color", unreal.LinearColor(1.0, 0.85, 0.6, 1.0))
    print("SUN_GOLDEN_HOUR_SET")
```

---

## Gameplay Impact of Lighting

### Survival Mechanics Tied to Time
- **Dawn (5-7h)**: Herbivores most active (feeding), predators returning to rest
- **Day (8-16h)**: Safe exploration window, best visibility
- **Dusk (17-19h)**: Predators becoming active, player should seek shelter
- **Night (20-4h)**: Maximum danger, reduced visibility, temperature drops

### Player Visibility Thresholds
- Day: Full visibility, no penalties
- Dusk/Dawn: -10% detection range for player
- Night: -40% visibility, require fire/torch for safety
- Storm: -30% visibility, movement speed reduced

---

## Performance Notes
- Lumen GI: Software tracing (no RT hardware required)
- Volumetric Fog: GridPixelSize=8, GridSizeZ=64 (balanced quality/perf)
- SkyLight: Real-time capture (updates every frame — can be throttled to every 0.5s)
- Day/Night tick: Only update sun every 1.0s (not every frame) for performance

---

*Agent #8 — Lighting & Atmosphere Agent | Day/Night Cycle Reference*
*For Agent #9 (Character Artist): Characters will be lit by this dynamic system — ensure materials use Lumen-compatible shading models*
