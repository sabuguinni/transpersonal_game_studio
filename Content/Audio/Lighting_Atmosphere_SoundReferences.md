# Lighting & Atmosphere — Audio References
## Agent #08 | PROD_CYCLE_AUTO_20260703_005

### Ambient Soundscape Candidates (Freesound.org)
These sounds are curated for the Cretaceous forest hub area (X=2100, Y=2400).
They will be integrated by Agent #16 (Audio Agent) via MetaSounds.

| ID | Name | Duration | Tags | Preview URL |
|----|------|----------|------|-------------|
| 849627 | Wind in Pine Trees, Soft Rustle, Slaley Forest | 1187s | forest, pine, leaves, nature, outdoor | https://cdn.freesound.org/previews/849/849627_18634554-hq.mp3 |
| 842151 | Camp John Hay - General Ambience | 57s | cinematic, forest, environmental, atmospheric | https://cdn.freesound.org/previews/842/842151_14269391-hq.mp3 |
| 842150 | Camp John Hay - General Ambience 4 | 139s | cinematic, forest, environmental, atmospheric | https://cdn.freesound.org/previews/842/842150_14269391-hq.mp3 |
| 584219 | Bolano campagna uccelli vento | 174s | birds, insect, forest, nature, outdoor | https://cdn.freesound.org/previews/584/584219_12075612-hq.mp3 |
| 727609 | Outdoor ambience near forest with birdsong | 129s | birds, birdsong, forest, nature, outdoor | https://cdn.freesound.org/previews/727/727609_6209123-hq.mp3 |

### Recommended Usage
- **Base layer**: ID 849627 (wind through trees, loopable, 1187s — ideal base)
- **Bird layer**: ID 727609 (birdsong, randomized playback)
- **Insect layer**: ID 584219 (insects + birds, warm daytime feel)
- **Cinematic moments**: ID 842151 + 842150 (atmospheric, cinematic quality)

### Lighting Context for Audio Integration
The hub area (2100, 2400) uses:
- DirectionalLight: 75000 lux, pitch -45°, warm white (1.0, 0.95, 0.85)
- Volumetric fog: density 0.02, blue-tinted inscattering
- God-ray RectLight: 50000 intensity, 400x400 source, warm amber
- 3 ambient point lights: warm fill, 4000-5000 intensity, 800 attenuation radius
- Lumen GI + Reflections enabled
- Real-time SkyLight capture at intensity 3.0

### Audio-Lighting Sync Notes
- **Dawn (06:00-08:00)**: Reduce ambient volume 30%, increase bird calls, cool blue tint
- **Midday (10:00-14:00)**: Full ambient volume, insect layer prominent, warm golden light
- **Dusk (17:00-19:00)**: Fade birds, increase wind, shift to orange-red tones
- **Night (20:00-05:00)**: Silence birds, add cricket/frog layer, cool moonlight

### Integration Instructions for Agent #16
1. Import sounds from Freesound using their IDs
2. Create MetaSound asset: `MS_CretaceousHub_Ambience`
3. Layer: Wind (base) + Birds (random trigger 5-15s intervals) + Insects (continuous)
4. Attach to `AmbientSound` actor at hub coordinates (2100, 2400, 100)
5. Set attenuation radius: 3000 units
6. Enable spatial audio with reverb preset: `Forest_Outdoor`
