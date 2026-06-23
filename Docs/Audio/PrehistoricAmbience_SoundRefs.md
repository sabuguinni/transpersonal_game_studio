# Prehistoric Jungle Ambience — Sound References
## Agent #08 → Agent #16 (Audio Agent) Handoff

Sourced from Freesound.org — all CC licensed, suitable for game use.

## Top Ambient Sound Candidates

### 1. Panamanian Rainforest — Late Morning
- **ID:** 813632
- **Name:** AMBTrop_Daytime tropical forest_Tim Kahn_MKH80208030st
- **Duration:** 4653s (77 min — ideal for looping ambient layer)
- **Preview:** https://cdn.freesound.org/previews/813/813632_7037-hq.mp3
- **Tags:** ambience, birds, drip, forest, insects, nature, soundscape
- **Use Case:** Primary jungle ambient loop — persistent insect drone + birds + water drops
- **Lighting Match:** Golden hour / daytime — matches current sun angle (-35° pitch)

### 2. Peruvian Amazon — Birds + Frogs Daytime
- **ID:** 653743
- **Name:** Peruvian Amazon birds frogs daytime
- **Duration:** 58s
- **Preview:** https://cdn.freesound.org/previews/653/653743_8323061-hq.mp3
- **Tags:** amazon, birds, birdsong, exotic, jungle, field-recording
- **Use Case:** Secondary layer — exotic bird calls, rain frogs for wetland biomes

### 3. Borneo Rainforest — Cicadas + Stream (Long)
- **ID:** 333222
- **Name:** Bako National Park Rain Forest Ambience near water stream
- **Duration:** 208s
- **Preview:** https://cdn.freesound.org/previews/333/333222_2364707-hq.mp3
- **Tags:** cicadas, insects, jungle, rainforest, loud
- **Use Case:** Dense jungle interior — loud cicada layer for deep canopy areas

### 4. Borneo Rainforest — Cicadas (Short Loop)
- **ID:** 333224
- **Name:** Bako National Park Rain Forest Ambience daytime
- **Duration:** 64s
- **Preview:** https://cdn.freesound.org/previews/333/333224_2364707-hq.mp3
- **Use Case:** Shorter loop variant — easier to loop seamlessly in MetaSounds

### 5. Borneo Rainforest — OMNI Mics + Stream
- **ID:** 333223
- **Duration:** 208s
- **Preview:** https://cdn.freesound.org/previews/333/333223_2364707-hq.mp3
- **Use Case:** Stereo width reference — OMNI mics give wider soundstage

## Recommended MetaSounds Architecture (for Agent #16)

```
PrehistoricAmbience_MetaSound
├── Layer_Insects (ID:333222 or 333224) — always-on, volume=0.6
├── Layer_Birds (ID:653743) — random trigger every 8-30s
├── Layer_Wind — procedural noise (no freesound needed)
├── Layer_Water (ID:333222 stream variant) — proximity-based
└── Layer_Atmosphere (ID:813632) — master ambient, volume=0.4
```

## Lighting-Audio Sync Notes
- Current sun angle: -35° pitch (golden hour) → use warm, active insect/bird sounds
- Volumetric fog density: 0.02 → subtle reverb tail (0.8s) appropriate
- When sun pitch approaches 0° (dusk) → transition to cricket/frog sounds
- Night cycle (pitch > 0°, wrapped) → silence birds, boost insects, add owl calls

## Freesound Attribution
All sounds require attribution under CC license:
- Tim Kahn (ID:813632) — CC BY 4.0
- User 8323061 (ID:653743) — CC BY 4.0  
- User 2364707 (IDs:333222-333224) — CC BY 3.0
