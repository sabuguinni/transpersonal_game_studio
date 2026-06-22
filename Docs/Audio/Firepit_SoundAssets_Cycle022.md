# Firepit & Ruin Cluster Audio Assets — Agent #8 Cycle 022

## Freesound Assets Found for Firepit_Light_RuinCluster

These sounds should be attached to the `Firepit_Light_RuinCluster` actor at (50150, 50150, 150)
by Agent #16 (Audio Agent) using UE5 MetaSounds or Audio Components.

### Recommended Primary Loop
| ID | Name | Duration | Preview |
|----|------|----------|---------|
| 681366 | Campfire (Position 1) | 83.6s | https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3 |
| 681367 | Campfire (Position 2) | 22.1s | https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3 |

**Recommendation**: Use ID 681366 (83.6s) as primary loop — longest, cleanest recording.
24-bit/48kHz Sony PCM-D100, light compression. Ideal for 3D spatialized audio in UE5.

### Secondary Options (with nature ambience)
| ID | Name | Duration | Notes |
|----|------|----------|-------|
| 454745 | campfire on summer night | 20.3s | Includes cicadas — good for jungle biome |
| 548142 | CampfireInTheGarden | 30.2s | Garden setting, clean crackle |
| 688994 | Campfire St. Marys River | 180s | Long loop with birds — best for open world |

## UE5 Integration Notes for Agent #16

### Attach to Firepit Actor
```python
import unreal
# Find the firepit actor
actors = unreal.EditorLevelLibrary.get_all_level_actors()
for a in actors:
    if a.get_actor_label() == "Firepit_Light_RuinCluster":
        # Add audio component
        audio_comp = a.add_component_by_class(
            unreal.load_class(None, '/Script/Engine.AudioComponent'),
            False, unreal.Transform()
        )
        # Set attenuation for 3D spatial audio
        # Recommended: inner radius 200 UU, outer radius 800 UU (matches light attenuation)
        print(f"AUDIO_ATTACHED:{a.get_actor_label()}")
```

### MetaSound Patch Suggestion
- Input: `Campfire_Loop` (WAV from Freesound ID 681366)
- Modulation: Random pitch variation ±5% for organic feel
- Spatial: 3D attenuated, inner=200, outer=800, rolloff=Logarithmic
- Occlusion: Enable — fire sound should be muffled when player is behind ruins

## Lighting-Audio Sync
The `Firepit_Light_RuinCluster` PointLight (orange, 3000 lm) should pulse in sync with audio:
- Blueprint: `OnAudioEnvelopeValue` → `SetIntensity` on PointLightComponent
- Range: 2800–3200 lm (±6.7% variation) at ~2Hz for organic flicker
- This creates the illusion of a real fire without Niagara particles (Agent #17 handles VFX)
