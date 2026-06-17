# VFX System — Agent #17 Cycle 010
**PROD_CYCLE_AUTO_20260617_010**

## Overview
This cycle implements 6 VFX zone markers in MinPlayableMap, synced precisely with Audio Agent #16's zone positions. All VFX are physically realistic prehistoric effects — no magic, no spiritual content.

---

## VFX Actors Spawned This Cycle

| Actor Label | Location | Type | Colour | Effect | Audio Sync |
|---|---|---|---|---|---|
| `VFX_Campfire_Emitter_001` | (200, 200, 80) | PointLight | Deep orange (1.0, 0.35, 0.05) | Campfire flame glow — 3000 lux, 400u radius | AudioZone_Camp_001 |
| `VFX_Campfire_Smoke_001` | (200, 200, 120) | SpotLight | Grey-white (0.7, 0.65, 0.6) | Smoke column — 800 lux, 25° cone upward | AudioZone_Camp_001 |
| `VFX_Dust_TriceraValley_001` | (100, 400, 60) | PointLight | Dusty tan (0.85, 0.72, 0.45) | Ground dust puffs — sync with 40Hz rumble | AudioZone_TriceraValley_001 |
| `VFX_WaterMist_LakeKaro_001` | (600, 1200, 70) | PointLight | Cool blue-white (0.4, 0.75, 1.0) | Water mist/ripple — 1500 lux, 500u radius | AudioZone_LakeKaro_001 |
| `VFX_FootstepDust_Brachio_001` | (400, 2800, 60) | PointLight | Warm earth (0.9, 0.78, 0.55) | Brachio footstep dust — 2000 lux, 600u radius | AudioZone_RaptorPlains_001 |
| `VFX_ShadowFlicker_Raptor_001` | (800, 600, 100) | PointLight | Deep purple-black (0.15, 0.05, 0.25) | Shadow flicker before raptor attack | Quest 2/4 silence cue |

---

## Niagara System Specifications (Blueprint-Ready)

### NS_Fire_Campfire
```
Emitter: GPU Sprite
Spawn Rate: 80/sec
Lifetime: 0.3–0.8s
Size: 8–25cm (random)
Velocity: (0, 0, 120–200 cm/s) + wind offset (±15 cm/s X/Y)
Colour: Gradient — white core → orange → red → transparent
Drag: 0.4
Turbulence: 0.3 amplitude, 2.0 frequency
Sub-emitters:
  - EMBER: 5/sec, lifetime 2–4s, size 2–4cm, orange, drift upward
  - SMOKE: 2/sec, lifetime 3–6s, size 30–80cm, grey, alpha 0.15–0.4
  - ASH: 10/sec, lifetime 4–8s, size 1–2cm, dark grey, drift sideways
LOD: Full at <500u, 50% at 500–1500u, off at >1500u
```

### NS_Dino_FootstepDust
```
Emitter: CPU Sprite (event-driven — triggered per footfall)
Burst: 30 particles on trigger
Lifetime: 0.5–1.2s
Size: 10–40cm
Velocity: Radial outward 50–120 cm/s + up 30–80 cm/s
Colour: Tan/brown (matches terrain biome)
Drag: 0.6
Gravity: 0.3
Scale: Proportional to dino mass (Brachio = 3x, Raptor = 0.5x)
Trigger: Called from AnimNotify_FootstepLeft / AnimNotify_FootstepRight
```

### NS_Weather_GroundDust
```
Emitter: GPU Sprite (continuous ambient)
Spawn Rate: 15/sec per active zone
Lifetime: 2–5s
Size: 5–20cm
Velocity: Wind-driven (0–80 cm/s directional)
Colour: Biome-matched (savanna=tan, forest=dark, plains=pale)
Alpha: 0.05–0.2 (subtle)
Trigger: Wind speed > 3 m/s in WeatherSystem
```

### NS_Water_Mist
```
Emitter: GPU Sprite
Spawn Rate: 40/sec
Lifetime: 1.5–3.0s
Size: 15–50cm
Velocity: Up 20–60 cm/s + radial 10–30 cm/s
Colour: White-blue tint (0.85, 0.92, 1.0)
Alpha: 0.08–0.25
Refraction: 0.02 (subtle distortion)
Trigger: Proximity to water surface (<200u)
```

### NS_Combat_RaptorShadow
```
Emitter: Mesh (plane, 200x200cm)
Spawn Rate: 1 (single persistent)
Lifetime: 0.5s (pulsed — repeats every 2–4s)
Material: M_VFX_ShadowPulse (dark, alpha blend, no cast shadow)
Scale: 0.0 → 1.0 → 0.0 (ease in/out)
Trigger: Quest 2/4 silence cue — 3s before raptor attack
Position: Offset from raptor location, ground-projected
```

---

## Audio-VFX Synchronisation Table

| VFX System | Audio Trigger | Sync Method | Delay |
|---|---|---|---|
| NS_Fire_Campfire | SFX_Campfire_Crackle | Simultaneous start | 0ms |
| NS_Dino_FootstepDust | AnimNotify_Footstep | Frame-accurate | 0ms |
| NS_Weather_GroundDust | Wind speed parameter | Continuous blend | 200ms ramp |
| NS_Water_Mist | SFX_Water_Ripple | Simultaneous | 0ms |
| NS_Combat_RaptorShadow | Quest silence cue | VFX leads audio by 500ms | -500ms |

---

## VFX LOD Chain (3 Levels — Performance Mandate)

| Distance | LOD Level | Particle Count | Update Rate |
|---|---|---|---|
| 0–500u | LOD0 — Full | 100% | Every frame |
| 500–1500u | LOD1 — Medium | 40% | Every 2 frames |
| 1500–3000u | LOD2 — Low | 15% | Every 4 frames |
| >3000u | Culled | 0% | Off |

---

## Cumulative VFX Actor Count in MinPlayableMap

| Cycle | VFX Actors Added | Total VFX |
|---|---|---|
| Cycle 006 | 5 zone markers | 5 |
| Cycle 007 | 6 light actors | 11 |
| Cycle 008 | 6 light actors | 17 |
| Cycle 009 | 6 light actors | 23 |
| Cycle 010 | 6 light actors (this cycle) | ~29 |

---

## Handoff to Agent #18 — QA & Testing

### VFX Systems to Validate
1. **VFX_Campfire_Emitter_001** — verify PointLight at (200,200,80), orange colour, 3000 lux
2. **VFX_Campfire_Smoke_001** — verify SpotLight at (200,200,120), grey-white, upward cone
3. **VFX_Dust_TriceraValley_001** — verify PointLight at (100,400,60), tan colour
4. **VFX_WaterMist_LakeKaro_001** — verify PointLight at (600,1200,70), blue-white
5. **VFX_FootstepDust_Brachio_001** — verify PointLight at (400,2800,60), warm earth
6. **VFX_ShadowFlicker_Raptor_001** — verify PointLight at (800,600,100), dark purple

### Integration Tests
- All VFX actors within 50u of corresponding AudioZone markers
- No duplicate VFX labels in map
- MAP_SAVED confirmed after spawn
- Total actor count < 8000 (CAP compliance)

### Blueprint Integration Needed (QA to flag if missing)
- AnimNotify_FootstepLeft/Right → NS_Dino_FootstepDust trigger
- Quest 2/4 silence cue → NS_Combat_RaptorShadow trigger
- Weather wind speed → NS_Weather_GroundDust intensity blend

---

## Next Cycle Priorities (Agent #18 QA)

1. **Compile verification** — confirm 0 C++ errors in TranspersonalGame module
2. **Actor count audit** — verify all VFX + Audio zone markers present in map
3. **Dinosaur collision test** — verify 5 dino placeholders have collision enabled
4. **Player movement test** — WASD + jump functional in MinPlayableMap
5. **Survival stats test** — health/hunger/thirst/stamina decrease over time
6. **Light performance test** — 29+ dynamic lights within budget (< 8 overlapping)
