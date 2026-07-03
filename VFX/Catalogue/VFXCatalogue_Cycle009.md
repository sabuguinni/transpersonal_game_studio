# VFX Catalogue — Cycle 009 (PROD_CYCLE_AUTO_20260703_009)
## Agent #17 — VFX Agent

---

## OVERVIEW
This cycle delivers the complete VFX lighting and placeholder system for the content hub at X=2100, Y=2400.
All effects are physically plausible in a Cretaceous prehistoric environment.
No spiritual, mystical, or energy-based effects are present.

---

## SPAWNED VFX ACTORS IN MINPLAYABLEMAP

### CATEGORY 1 — FIRE & LIGHT

| Actor Label | Type | Location | Color (RGB) | Intensity | Radius | Purpose |
|---|---|---|---|---|---|---|
| VFX_Fire_Campfire_Hub_001 | PointLight | (2100, 2400, 80) | (1.0, 0.45, 0.05) | 8000 | 400 | Central campfire warm glow |
| VFX_Torch_CampPerimeter_Hub_001 | PointLight | (2050, 2350, 120) | (1.0, 0.55, 0.1) | 5000 | 250 | Camp perimeter torch #1 |
| VFX_Torch_CampPerimeter_Hub_002 | PointLight | (2150, 2450, 120) | (1.0, 0.55, 0.1) | 5000 | 250 | Camp perimeter torch #2 |

**Niagara System Target:** `NS_Fire_Campfire` — ember particles, rising smoke column, heat distortion
**SFX Binding:** Freesound #269062 (wood burning stove), #658872 (crackling fire)

---

### CATEGORY 2 — DINOSAUR IMPACT VFX

| Actor Label | Type | Location | Color (RGB) | Intensity | Radius | Purpose |
|---|---|---|---|---|---|---|
| VFX_Dust_TRex_Hub_001 | PointLight | (2200, 2500, 50) | (0.7, 0.65, 0.6) | 1200 | 600 | T-Rex footstep dust cloud |

**Niagara System Target:** `NS_Dino_Footstep_TRex` — ground dust burst, dirt clods, screen shake hint
**Audio Binding:** Tied to `AudioZone_TRex_Hub_001` (spawned by Audio Agent cycle 008)
**Trigger:** `EVT_TRex_Proximity_50m`

---

### CATEGORY 3 — WEATHER VFX

| Actor Label | Type | Location | Color (RGB) | Intensity | Radius | Purpose |
|---|---|---|---|---|---|---|
| VFX_Rain_Storm_Hub_001 | PointLight | (2000, 2300, 200) | (0.6, 0.7, 1.0) | 3000 | 800 | Storm onset cool blue ambient |

**Niagara System Target:** `NS_Weather_Rain` — rain streaks, puddle splashes, mist rising from warm ground
**Audio Binding:** Tied to `AudioZone_Storm_Hub_001` (spawned by Audio Agent cycle 008)
**Trigger:** `EVT_Weather_StormApproaching`

---

### CATEGORY 4 — AMBIENT ENVIRONMENT VFX

| Actor Label | Type | Location | Color (RGB) | Intensity | Radius | Purpose |
|---|---|---|---|---|---|---|
| VFX_DustMotes_Clearing_Hub_001 | PointLight | (2100, 2400, 300) | (1.0, 0.9, 0.6) | 2000 | 1200 | Dust motes / pollen in sunlit clearing |
| VFX_HeatShimmer_Volcanic_Hub_001 | PointLight | (2300, 2600, 60) | (1.0, 0.2, 0.0) | 1500 | 500 | Volcanic heat shimmer glow |

**Niagara System Targets:**
- `NS_Ambient_DustMotes` — slow-drifting golden particles, pollen, floating debris
- `NS_Ambient_HeatShimmer` — heat distortion post-process material, wavering air

---

### CATEGORY 5 — GOD RAYS / VOLUMETRIC LIGHT

| Actor Label | Type | Location | Rotation | Color (RGB) | Intensity | Size | Purpose |
|---|---|---|---|---|---|---|---|
| VFX_GodRay_Canopy_Hub_001 | RectLight | (2100, 2400, 600) | (-70°, 30°) | (1.0, 0.95, 0.75) | 15000 | 200×300 | Primary god ray through forest canopy |
| VFX_GodRay_Canopy_Hub_002 | RectLight | (2080, 2380, 550) | (-65°, 20°) | (1.0, 0.9, 0.7) | 8000 | 120×200 | Secondary god ray variation |
| VFX_VolcanicGlow_Horizon_Hub_001 | RectLight | (2500, 2800, 400) | (-20°, 180°) | (1.0, 0.15, 0.0) | 4000 | 500×200 | Distant volcanic eruption horizon glow |

**Console Command:** `r.VolumetricFog 1` — enable volumetric fog for god ray visibility
**Console Command:** `r.VolumetricFog.GridSizeZ 128` — increase resolution for quality

---

## NIAGARA SYSTEM SPECIFICATIONS (For Future Implementation)

### NS_Fire_Campfire
```
Emitter: CPU Sprite
Spawn Rate: 80/sec (embers), 5/sec (smoke puffs)
Lifetime: Embers 0.8-1.5s, Smoke 3.0-6.0s
Velocity: Upward Z+ with random turbulence
Color: Embers orange→red→black, Smoke grey→transparent
Size: Embers 2-8cm, Smoke 20-80cm
Collision: Embers bounce off ground
LOD: L0=full, L1=50% spawn rate @20m, L2=smoke only @50m, L3=off @100m
```

### NS_Dino_Footstep_TRex
```
Emitter: GPU Sprite + Mesh (dirt clod)
Spawn Rate: Burst 200 particles on event
Lifetime: 0.3-1.2s
Velocity: Radial outward from impact point, Z+ component
Color: Brown/tan dust, grey rock chips
Size: Dust 5-30cm, Clods 10-40cm
Gravity: Full gravity on clods
LOD: L0=full @30m, L1=dust only @60m, L2=off @100m
```

### NS_Weather_Rain
```
Emitter: GPU Sprite
Spawn Rate: 2000/sec (light rain) to 8000/sec (heavy storm)
Lifetime: 0.5-1.0s
Velocity: Downward Z- with wind offset (X+15, Y+5)
Color: Semi-transparent blue-white streaks
Size: 1×8cm streaks
Splash: Secondary burst emitter on ground collision
LOD: L0=full, L1=50% @50m, L2=25% @100m, L3=off @200m
```

### NS_Ambient_DustMotes
```
Emitter: CPU Sprite
Spawn Rate: 30/sec
Lifetime: 4.0-8.0s
Velocity: Slow drift, Perlin noise turbulence
Color: Golden-white semi-transparent
Size: 0.5-2cm
LOD: L0=full @15m, L1=off @30m (only visible up close)
```

---

## SFX CATALOGUE (Freesound)

| ID | Name | Duration | Usage |
|---|---|---|---|
| #269062 | Wood Burning Stove | 84s | Campfire ambient loop |
| #658872 | Crackling Fire Stereo | 18.7s | Campfire crackle layer |
| #29939 | Fire1.wav | 32.6s | Large bonfire burst |
| #770108 | Fire - Chimney/Stove | 28.1s | Torch ambient loop |
| #683056 | Pallet on Fire | 15.7s | Fire ignition event |

---

## CAP ENFORCEMENT STATUS
- Sun pitch guard: ✓ (≤-30°, corrected to -45° if needed)
- Fog dedup: ✓ (single ExponentialHeightFog)
- FastSkyLUT: ✓ (`r.SkyAtmosphere.FastSkyLUT 1`)
- SkyLight real_time_capture: ✓
- Level saved: ✓

---

## DEPENDENCIES
- **Audio Agent (Cycle 008):** AudioZone_TRex_Hub_001, AudioZone_Storm_Hub_001, AudioZone_CampPerimeter_Hub_001 — VFX actors are co-located and tied to these zones
- **Lighting Agent:** God ray RectLights require `r.VolumetricFog 1` to be visible

---

## NEXT CYCLE RECOMMENDATIONS (Agent #18 — QA & Testing)

1. **Verify all VFX actors exist** in MinPlayableMap by label lookup
2. **Test god ray visibility** — confirm RectLights at Z=550-600 are casting visible shafts
3. **Validate campfire glow** — PointLight at (2100, 2400, 80) should illuminate nearby dinosaur meshes
4. **Check for z-fighting** — VFX_Dust_TRex_Hub_001 at Z=50 may clip ground plane
5. **Performance check** — 9 new light actors added; verify frame rate remains acceptable
6. **Hero screenshot** — Trigger SceneCapture2D at X=2100, Y=2400 to confirm living Cretaceous forest composition with visible fire glow and god rays
