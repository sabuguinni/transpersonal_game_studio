# Lighting & Atmosphere — Audio References
## Agent #08 | PROD_CYCLE_AUTO_20260703_009

These sound references are curated for the Cretaceous prehistoric world ambient audio layer.
They complement the Lumen lighting system by providing acoustic atmosphere that matches
the visual mood of each lighting state (daylight, dusk, night).

---

## DAYLIGHT JUNGLE AMBIENCE (Primary Hub Zone — X=2100, Y=2400)

| ID | Name | Duration | Preview URL | Use Case |
|----|------|----------|-------------|----------|
| 804838 | Forest Ambience, Monk's Trail, Chiang Mai, Thailand | 95.96s | https://cdn.freesound.org/previews/804/804838_5417386-hq.mp3 | Primary daytime jungle loop — birds + insects + rustling leaves |
| 804709 | Forest Ambience, Chiang Mai, Thailand | 379.32s | https://cdn.freesound.org/previews/804/804709_5417386-hq.mp3 | Long-form background jungle loop (6+ min, ideal for seamless looping) |
| 653743 | Peruvian Amazon birds frogs daytime | 58.06s | https://cdn.freesound.org/previews/653/653743_8323061-hq.mp3 | Exotic bird layer — Amazon soundscape for Cretaceous feel |
| 749737 | denseforestwithbirds | 101.63s | https://cdn.freesound.org/previews/749/749737_16219462-hq.mp3 | Dense canopy bird chorus — layered over primary loop |

## INSECT / CICADA LAYER (Heat of Day)

| ID | Name | Duration | Preview URL | Use Case |
|----|------|----------|-------------|----------|
| 805467 | Crickets - Jungle & Summer day | 85.50s | https://cdn.freesound.org/previews/805/805467_13973196-hq.mp3 | Insect drone layer — adds prehistoric heat and scale |

## WIND THROUGH CANOPY (Atmospheric Depth)

| ID | Name | Duration | Preview URL | Use Case |
|----|------|----------|-------------|----------|
| 800712 | AMBForst_Forest Birds Wind 01 | 30s | https://cdn.freesound.org/previews/800/800712_12846320-hq.mp3 | Gentle wind through canopy — pairs with god-ray lighting moments |

## WATER / STREAM (Secondary Zones)

| ID | Name | Duration | Preview URL | Use Case |
|----|------|----------|-------------|----------|
| 468242 | Thailand Phangan Jungle Waterfall 1 | 25s | https://cdn.freesound.org/previews/468/468242_652422-hq.mp3 | River/stream proximity audio — for water features near hub |
| 468241 | Thailand Phangan Jungle Waterfall 2 | 37.48s | https://cdn.freesound.org/previews/468/468241_652422-hq.mp3 | Alternate stream loop |

---

## LIGHTING-AUDIO SYNC RECOMMENDATIONS

### Bright Daylight (Sun pitch -45° to -60°, intensity 50k-75k lux)
- Primary: `804709` (long forest loop) + `805467` (insect layer)
- Wind layer: `800712` at 30% volume
- Mood: Active, alive, dangerous — dinosaurs are hunting

### Dusk Transition (Sun pitch -20° to -30°, intensity 10k-25k lux)
- Primary: `653743` (Amazon birds) — bird chorus intensifies at dusk
- Reduce insect layer, add distant rumble (dinosaur calls from Audio Agent)
- Mood: Tension rising — predators become more active

### Night (Sun pitch 0° to +15°, moon light)
- Primary: `669357` (Amazon night insects) — nocturnal soundscape
- Mood: High danger — player must seek shelter

---

## IMPLEMENTATION NOTES FOR AUDIO AGENT (#16)

1. **MetaSounds Integration**: These loops should be imported as MetaSound Sources with
   `bIsLooping=true` and crossfade blend time of 3-5 seconds for day/night transitions.

2. **Spatial Audio**: Place ambient sound actors at hub center (2100, 2400, 0) with
   attenuation radius 5000 units. Use `USoundAttenuation` with logarithmic falloff.

3. **Lumen Sync**: The god-ray RectLight at hub (2100, 2400, 800) fires at intensity 8000.
   When this light is at peak, boost the `800712` wind sound to 60% — the visual and
   acoustic god-ray should feel simultaneous.

4. **Layering Priority**:
   - Layer 1 (always on): `804709` at 40% volume
   - Layer 2 (time-of-day): insect or bird layer at 60%
   - Layer 3 (event): wind gust triggered by large dinosaur movement nearby

---

## LIGHTING STATE MACHINE (for Audio Agent reference)

```
DAYLIGHT_BRIGHT  → intensity >= 50000 lux, pitch <= -40°
DAYLIGHT_NORMAL  → intensity 25000-50000 lux, pitch -30° to -40°
DUSK             → intensity 10000-25000 lux, pitch -15° to -30°
NIGHT            → intensity < 10000 lux (moon only), pitch > -15°
```

Each state drives both visual (Lumen) and acoustic (MetaSounds) parameters simultaneously
via the DayNightCycleManager blueprint.
