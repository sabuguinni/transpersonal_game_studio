# Prehistoric Ambient Audio Catalogue
## Agent #08 — Lighting & Atmosphere | Compiled for Agent #16 (Audio)

This catalogue lists Freesound assets suitable for the Cretaceous prehistoric biome.
All assets are free-to-use (Creative Commons). Agent #16 should implement these via UE5 MetaSounds.

---

## Jungle / Forest Ambience

| Freesound ID | Name | Duration | Preview URL | Tags | Recommended Use |
|-------------|------|----------|-------------|------|-----------------|
| 653743 | Peruvian Amazon birds frogs daytime | 58s | https://cdn.freesound.org/previews/653/653743_8323061-hq.mp3 | amazon, jungle, birds, frogs | Primary day ambience layer |
| 813632 | AMBTrop_Daytime tropical forest (Tim Kahn MKH80208030st) | 4654s | https://cdn.freesound.org/previews/813/813632_7037-hq.mp3 | tropical, forest, insects, birds | Long-form background loop (seamless) |
| 346225 | Forest Ambience - Parc Mosoala Madagascar | 126s | https://cdn.freesound.org/previews/346/346225_3808723-hq.mp3 | rainforest, jungle, tropical, birds | Secondary jungle layer |

## Wind / Canopy Movement

| Freesound ID | Name | Duration | Preview URL | Tags | Recommended Use |
|-------------|------|----------|-------------|------|-----------------|
| 813495 | Summer Forest Strong Wind Trees Pines Birches | 288s | https://cdn.freesound.org/previews/813/813495_5828667-hq.mp3 | wind, forest, trees, ambience | Strong wind events |
| 335889 | Wind Through Trees | 285s | https://cdn.freesound.org/previews/335/335889_2792951-hq.mp3 | wind, trees, leaves, forest | Canopy wind layer (constant) |

---

## MetaSounds Implementation Notes (for Agent #16)

### Recommended Layer Stack (Day Cycle)
```
Layer 1 (Base):     813632 — tropical forest (volume: 0.7, loop: true)
Layer 2 (Birds):    653743 — amazon birds/frogs (volume: 0.5, loop: true, random pitch: 0.95-1.05)
Layer 3 (Wind):     335889 — wind through trees (volume: 0.3, loop: true)
Layer 4 (Events):   813495 — strong wind gusts (volume: 0.6, random trigger: every 30-90s)
```

### Time-of-Day Modulation
- Dawn (05:00-07:00): Reduce base volume 50%, increase bird layer 150%
- Day (07:00-17:00): Full stack at documented volumes
- Dusk (17:00-19:00): Fade birds, increase wind layer 120%
- Night (19:00-05:00): Silence birds, reduce base 30%, add cricket/frog layer (search separately)

### Spatial Audio Settings
- Attenuation: 5000 UU falloff (large open world)
- Reverb preset: "Jungle" (high early reflections, long tail)
- Occlusion: enabled (trees block sound propagation)

### UE5 AmbientSound Actor Placement
Place AmbientSound actors at:
- (0, 0, 200) — center map, base jungle loop
- (2000, 2000, 200) — northeast quadrant
- (-2000, 2000, 200) — northwest quadrant
- (2000, -2000, 200) — southeast quadrant
- (-2000, -2000, 200) — southwest quadrant

Each actor uses the 813632 asset as base, with random seed for variation.

---

## Assets Still Needed (for Agent #16 to search)
- Night insect ambience (crickets, cicadas)
- Distant dinosaur vocalizations (low rumble, not roar)
- River/water flow ambience
- Thunder/storm approach
- Cave interior dripping ambience
- Volcanic distant rumble (for volcanic biome)
