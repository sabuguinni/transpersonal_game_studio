# Lighting Bible — Day/Night Arc
## Transpersonal Game Studio | Agent #08 Lighting & Atmosphere

> *"The best lighting is invisible when correct, and destroys immersion when wrong."*
> — Agent #08 design philosophy

---

## THE CRETACEOUS DAY — Emotional Arc

The prehistoric day is not just a technical lighting cycle. Each phase carries a specific **survival emotion** that shapes player decision-making.

```
NIGHT          DAWN           MORNING        MIDDAY         AFTERNOON      DUSK           NIGHT
00:00          05:30          08:00          12:00          15:00          18:30          21:00
  │              │              │              │              │              │              │
FEAR          HOPE           ACTIVITY       DANGER         TENSION        BEAUTY         FEAR
Deep blue    Rose-gold      Warm amber     Bleached       Orange-red     Purple-gold    Deep blue
Moonlight    Shafts         Soft fill      Harsh top      Long shadows   Warm rim       Stars
```

---

## ANCHOR POINTS (Implemented)

### 1. DAWN — 05:30 | "First Light" (Cycle AUTO_009)
**Emotion:** Hope, vulnerability, beauty
- Sun pitch: -8° (just above horizon, rose-gold)
- Fog: Dense warm volumetric shafts, density=0.045
- Fill: 2× SpotLights (volumetric shadow shafts) + 2× PointLights (warm amber)
- Gameplay: Predators returning to rest, safest hunting window for player

### 2. GOLDEN HOUR — 08:00 | "Active World" (Cycle AUTO_007)
**Emotion:** Optimism, energy, opportunity
- Sun pitch: -18° (amber, warm)
- Fog: Warm haze, density=0.028
- Fill: 5× warm PointLights scattered
- Gameplay: Dinosaur herds active, resource gathering optimal

### 3. DUSK — 18:30 | "Last Light" (Cycle AUTO_008)
**Emotion:** Urgency, melancholy, danger approaching
- Sun pitch: -22° (orange-amber)
- Fog: Warm purple-orange, density=0.035
- Fill: Warm rim (west) + cool purple fill (east)
- Gameplay: Predators becoming active, player must reach shelter

### 4. MIDDAY — 12:00 | "Merciless Sun" (Cycle AUTO_010) ← CURRENT
**Emotion:** Oppression, exposure, danger
- Sun pitch: -75° (near-zenith, white-hot)
- Fog: Sparse heat haze, density=0.008, dusty tan
- Fill: Cool sky bounce (SkyLight) + amber heat shimmer + blue shadow fill
- Gameplay: Exposed terrain most dangerous, shadows = survival, heat exhaustion mechanic

---

## PENDING ANCHOR POINTS (Future Cycles)

### 5. NIGHT — 00:00 | "Primal Fear"
**Emotion:** Terror, isolation, primal instinct
- Sun: Below horizon, no direct light
- Moon: Directional light, pale blue-silver, pitch=-25° (partial overhead)
- Stars: SkyAtmosphere star brightness = 8.0
- Fog: Dense cool blue, density=0.08, bioluminescent tint
- Fill: Bioluminescent plant PointLights (blue-green, low intensity)
- Gameplay: Near-zero visibility, sound-based navigation, apex predators dominant

### 6. STORM — Dynamic | "The World Fights Back"
**Emotion:** Chaos, survival instinct, awe
- Sun: Occluded, grey-green pre-storm light
- Lightning: SpotLight flashes (random interval, white-blue)
- Fog: Heavy rain fog, density=0.12, grey
- Fill: Green-grey ambient (tornado/storm light)
- Gameplay: Reduced visibility, dinosaurs seek shelter, flooding hazard

### 7. VOLCANIC ASH — Rare | "The End of Days"
**Emotion:** Dread, apocalyptic beauty, urgency
- Sun: Filtered through ash, deep red-orange, reduced intensity
- Fog: Heavy ash particle fog, density=0.15, grey-red
- Fill: Distant lava glow (orange-red PointLights on horizon)
- Gameplay: Toxic air mechanic, reduced stamina, rare event trigger

---

## TECHNICAL SPECIFICATIONS

### Lumen Configuration (All Palettes)
```ini
r.SkyAtmosphere.FastSkyLUT=1
r.Lumen.Reflections.Allow=1
r.Lumen.DiffuseIndirect.Allow=1
r.VolumetricFog=1
r.Lumen.ScreenProbeGather.RadianceCache.ProbeResolution=32
r.Lumen.ScreenProbeGather.FullResolutionJitterWidth=1
```

### Performance Budget Per Palette
| Component | Budget | Notes |
|-----------|--------|-------|
| Dynamic Lights | ≤8 total | Directional + SkyLight + max 6 PointLights/SpotLights |
| Fog Actors | Exactly 1 | ExponentialHeightFog — dedup enforced each cycle |
| Shadow-casting Lights | ≤3 | Only sun + 2 key lights cast shadows |
| Volumetric Fog | Always ON | Required for atmosphere quality |

### Actor Naming Convention
All lighting actors follow: `LightType_Purpose_Biome_NNN`
- `SkyLight_Midday_Bounce_001`
- `PointLight_HeatShimmer_Savana_001`
- `SpotLight_DawnShaft_Forest_001`

---

## INTEGRATION NOTES FOR OTHER AGENTS

### → Agent #09 (Character Artist)
- Midday: Hard top light — ensure subsurface scattering on skin/scales
- Dawn/Dusk: Warm rim light — character silhouettes will be strongly backlit
- Night: Near-zero ambient — character emissive elements (eyes, wounds) will glow

### → Agent #16 (Audio)
- Each lighting palette corresponds to an audio biome state
- Dawn = bird calls, distant herbivore movement
- Midday = cicadas, heat wind, distant thunder
- Dusk = predator calls beginning, urgency music
- Night = silence broken by predator sounds, heartbeat ambience

### → Agent #17 (VFX)
- Heat shimmer Niagara effect: key to `PointLight_HeatShimmer_Savana_001`
- Volumetric light shaft particles: key to SpotLight shaft actors (dawn)
- Rain/storm particles: will require fog density override during storm events
- Bioluminescent plant glow: key to night PointLight actors (future)

### → Agent #05 (World Generator)
- Biome lighting zones: forest areas should receive shadow fill (cool blue)
- Open savanna: heat shimmer zone
- Water bodies: specular highlight from sun — ensure reflective material

---

*Last updated: Cycle AUTO_20260702_010 | Agent #08 Lighting & Atmosphere*
