# Atmospheric Audio References — Agent #8 Lighting & Atmosphere

## Cycle: PROD_CYCLE_AUTO_20260618_001

### Freesound References for Prehistoric Atmosphere

| ID | Name | Duration | Use Case | Preview |
|----|------|----------|----------|---------|
| 681516 | Thunder Claps, Constant, Strikes, Storm, Birds, Rumble Distant | 413s | Distant storm ambience layer — play at low volume during overcast weather state | https://cdn.freesound.org/previews/681/681516_11791687-hq.mp3 |
| 317481 | rainstorm with distant thunder | 245s | Heavy rain weather state — main rain loop | https://cdn.freesound.org/previews/317/317481_1510029-hq.mp3 |
| 641055 | thunder and gentle rain | 69s | Transitional weather — clearing storm, calm after rain | https://cdn.freesound.org/previews/641/641055_13582756-hq.mp3 |
| 584946 | Distant rumbles | 120s | Active storm background — low rumble layer under rain | https://cdn.freesound.org/previews/584/584946_1481531-hq.mp3 |
| 249508 | distant thunder | 12s | Single thunder crack event — trigger on lightning flash | https://cdn.freesound.org/previews/249/249508_197070-hq.mp3 |

---

## Lighting Setup Applied This Cycle

### MinPlayableMap — Cinematic Lumen Configuration

#### Sun_Directional
- Intensity: 10.0 lux
- Color: RGB(255, 224, 165) — warm amber (golden hour)
- Rotation: Pitch -45°, Yaw 30° — low angle for dramatic shadows
- Atmosphere Sun Light: True
- Shadow Cascades: 4
- Dynamic Shadow Distance: 50,000 units

#### Sky_Light
- Intensity: 1.2
- Color: RGB(180, 210, 255) — cool blue sky bounce
- Real Time Capture: True (reflects environment changes)

#### Sky_Atmosphere
- Standard UE5 SkyAtmosphere component
- Coupled with Sun_Directional for physically accurate scattering

#### Height_Fog (Volumetric)
- Fog Density: 0.015
- Height Falloff: 0.2
- Inscattering Color: Linear(0.6, 0.75, 1.0) — blue-grey atmospheric haze
- Start Distance: 500 units
- Max Opacity: 0.85
- Volumetric Fog: ENABLED
- Scattering Distribution: 0.5 (balanced forward/back scatter)
- Grid Pixel Size: 8 (quality)
- Grid Size Z: 64 (vertical resolution)

#### Fill_Light_Ground
- Type: RectLight (simulates ground bounce)
- Intensity: 800 lux
- Color: RGB(200, 220, 180) — warm earth-green bounce
- Source: 2000×2000 units
- Attenuation: 8000 units
- Cast Shadows: False (fill only)

#### Lumen Global Illumination
- r.DynamicGlobalIlluminationMethod 1 (Lumen)
- r.ReflectionMethod 1 (Lumen Reflections)
- r.Lumen.HardwareRayTracing 0 (software Lumen for compatibility)
- r.Lumen.Reflections.Allow 1
- r.Lumen.GlobalIllumination.Allow 1

#### Volumetric Fog Console
- r.VolumetricFog 1
- r.VolumetricFog.GridPixelSize 8
- r.VolumetricFog.GridSizeZ 64

---

## Emotional Intent (Director of Photography Notes)

The lighting is designed to evoke **danger within beauty** — the Cretaceous was stunning and lethal simultaneously.

- **Golden hour sun angle**: Long shadows make every dinosaur silhouette dramatic. The player sees threats before they see details.
- **Cool sky bounce vs warm sun**: Creates natural colour contrast that reads as "alive" — the same technique used in RDR2 and Horizon Zero Dawn.
- **Volumetric fog layers**: Near-ground haze obscures low-lying predators. The player cannot see what lurks in the fern beds. This is intentional fear design.
- **Ground bounce fill**: Prevents harsh shadow crush in the undergrowth. Ensures the player can read terrain detail even in shadow areas.

## Audio Integration Notes for Agent #16

Weather state → lighting state mapping:
- **Clear/Golden Hour**: Sun_Directional at full intensity, fog density 0.015, no thunder
- **Overcast**: Sun intensity reduced to 3.0, fog density 0.04, play Freesound 681516 (distant thunder) at 20% volume
- **Storm**: Sun intensity 1.0, fog density 0.08, play Freesound 317481 (rainstorm) + 584946 (rumbles)
- **Post-Storm**: Sun intensity 6.0 warm, fog density 0.025, play Freesound 641055 (clearing storm)
- **Night**: Sun disabled, SkyLight intensity 0.3, add point lights at campfires

## Next Agent (#9 — Character Artist)
The lighting is calibrated for **human skin tones** — the warm amber sun + cool sky bounce creates natural skin rendering without additional character-specific lights. MetaHuman characters will look correct under this setup without modification.
