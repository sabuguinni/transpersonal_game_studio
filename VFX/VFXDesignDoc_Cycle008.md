# VFX Design Document — Cycle 008
## Agent #17 — VFX Agent | PROD_CYCLE_AUTO_20260617_008

---

## VFX ACTORS PLACED IN MinPlayableMap (Cumulative — Cycles 004–008)

### Cycle 008 — Light-Based VFX Proxies (6 actors)

| Actor Label | Location | Type | Colour | Effect |
|---|---|---|---|---|
| `VFX_FireGlow_Camp_001` | (0, 0, 130) | PointLight | Orange (1.0, 0.45, 0.05) | Campfire fire glow — matches AmbientSound_Campfire_001 |
| `VFX_DangerZone_TRex_001` | (1200, 2500, 200) | SpotLight | Blood Red (1.0, 0.05, 0.0) | T-Rex territory danger indicator |
| `VFX_WaterSpray_River_001` | (3000, 1500, 100) | PointLight | Water Blue (0.3, 0.7, 1.0) | River water spray / mist |
| `VFX_VolcanicGlow_Horizon_001` | (-5000, -4000, 800) | PointLight | Lava Red (1.0, 0.2, 0.0) | Distant volcanic eruption glow |
| `VFX_MoonlightForest_001` | (1500, -2500, 180) | PointLight | Moonlit Green (0.4, 0.9, 0.5) | Forest edge night atmosphere |
| `VFX_StampedeDust_Plains_001` | (2000, 800, 120) | PointLight | Dust Amber (0.9, 0.7, 0.3) | Dinosaur stampede dust zone |

### Cycle 006–007 — VFX Zone Markers (5 actors each cycle)

| Actor Label | Location | Effect |
|---|---|---|
| `VFX_Fire_Camp_001` | (200, 100, 60) | Campfire fire zone |
| `VFX_Dust_Raptor_001` | (800, 1200, 50) | Raptor footstep dust |
| `VFX_Blood_Combat_001` | (600, 800, 80) | Combat blood impact |
| `VFX_Mist_River_001` | (3000, 1500, 60) | River mist/spray |
| `VFX_Ash_Volcano_001` | (-4000, -3000, 300) | Volcanic ash fallout |
| `VFX_Dust_Stampede_001` | (2000, 800, 100) | Herd stampede dust |
| `VFX_Heat_TRex_001` | (1200, 2500, 150) | T-Rex breath heat distortion |
| `VFX_Rain_Storm_001` | (-1000, -1000, 500) | Storm rain zone |
| `VFX_Ember_Camp_001` | (0, 0, 80) | Campfire ember particles |
| `VFX_Fog_Swamp_001` | (-2000, 3000, 50) | Swamp ground fog |

---

## NIAGARA SYSTEM SPECIFICATIONS (Ready for NS_ Asset Creation)

### NS_Fire_Campfire
```
Emitter Type: GPU Sprite
Spawn Rate: 80 particles/sec
Lifetime: 0.8–1.5s
Initial Velocity: (0, 0, 150–300) cm/s + random XY ±30
Size: 8–25 cm, shrinks over lifetime
Colour Curve: Yellow (0.0) → Orange (0.3) → Red (0.7) → Transparent (1.0)
Drag: 0.4
Turbulence: Curl noise, amplitude 50, frequency 0.8

Sub-emitter 1 — Smoke:
  Spawn Rate: 20/sec
  Lifetime: 3–6s
  Initial Velocity: (0, 0, 80–120) + wind offset
  Size: 40–120 cm, grows over lifetime
  Colour: Dark grey → transparent
  
Sub-emitter 2 — Embers:
  Spawn Rate: 5/sec
  Lifetime: 2–4s
  Physics: Gravity + random drift
  Size: 2–5 cm
  Colour: Orange → red → dark
  Glow: Emissive 3.0
```

### NS_Dino_Footstep_Dust
```
Emitter Type: GPU Sprite
Trigger: On footstep impact event
Burst: 40 particles on impact
Lifetime: 0.5–1.2s
Initial Velocity: Radial outward from impact point, 100–300 cm/s
Size: 15–60 cm, grows then fades
Colour: Sandy brown (terrain-matched)
Opacity Curve: 0→1 in 0.05s, 1→0 in 0.8s
Gravity Scale: 0.3 (slow fall)
Drag: 0.6

Scale by Dino Mass:
  Raptor (80kg): Scale 0.6
  T-Rex (8000kg): Scale 2.5
  Brachiosaurus (50000kg): Scale 4.0
```

### NS_Weather_Rain
```
Emitter Type: GPU Sprite (high count)
Spawn Rate: 2000 particles/sec (light) → 8000 (heavy storm)
Lifetime: 0.3–0.8s
Initial Velocity: (wind_x, wind_y, -800) cm/s
Size: 1×8 cm (elongated raindrop)
Colour: Blue-white, 40% opacity
Collision: Bounce on terrain, spawn splash sub-emitter

Sub-emitter — Splash:
  Burst: 8 particles on terrain hit
  Radial velocity: 50–150 cm/s
  Lifetime: 0.2s
  Size: 5–20 cm ring
```

### NS_Combat_BloodImpact
```
Emitter Type: GPU Sprite + Mesh
Trigger: On damage event (melee/bite)
Burst: 20–60 particles (scales with damage)
Lifetime: 0.3–1.0s
Initial Velocity: From impact direction, 200–600 cm/s
Size: 3–15 cm droplets
Colour: Dark red (0.6, 0.0, 0.0)
Decal: Blood pool decal spawned on terrain below

Sub-emitter — Drip:
  Spawn: 3–8 particles from wound location
  Gravity: Full (1.0)
  Lifetime: 1.5–3.0s
```

### NS_Volcano_AshFall
```
Emitter Type: GPU Sprite (ambient)
Spawn Rate: 500/sec (background)
Lifetime: 8–15s
Initial Velocity: (wind_x, wind_y, -20) cm/s (slow drift)
Size: 2–8 cm irregular flakes
Colour: Dark grey, 60% opacity
Turbulence: Gentle curl noise
Spawn Volume: 10000×10000 cm area, 2000 cm height
```

---

## SOUND CATALOGUE — VFX-LINKED AUDIO

### Campfire SFX (Freesound.org — Cycle 008)
| ID | Name | Duration | Use |
|---|---|---|---|
| 729395 | Campfire 01 | 109s | Base camp fire loop |
| 729396 | Campfire 02 | 268s | Extended campfire ambience |
| 852107 | Fireplace | 8.5s | Loopable close-up crackle |
| 803260 | Fire Poker Coals | 32.5s | Crafting/cooking interaction |
| 856943 | Campfire Forest Birds | 60s | Peaceful camp night |

### Raptor SFX (Cycle 007)
| ID | Name | Use |
|---|---|---|
| 157187 | Raptor screech | Alert call |
| 394952 | Dinosaur roar | Pack coordination |

---

## VFX INTEGRATION MAP

```
MinPlayableMap VFX Zones:
                    
  [-5000,-4000]          [3000,1500]
  VFX_VolcanicGlow  ←   VFX_WaterSpray
  (distant horizon)      (river zone)
  
  [-2000,3000]           [2000,800]
  VFX_Fog_Swamp    →    VFX_StampedeDust
  (wetlands)             (open plains)
  
  [0,0] BASE CAMP:       [1200,2500]
  VFX_FireGlow_Camp      VFX_DangerZone_TRex
  VFX_Ember_Camp         (T-Rex territory)
  AmbientSound_Campfire  
  
  [1500,-2500]           [-1000,-1000]
  VFX_MoonlightForest    VFX_Rain_Storm
  (forest edge)          (storm zone)
```

---

## NEXT CYCLE PRIORITIES (Agent #18 QA)

1. **Verify all 16 VFX actors exist** in MinPlayableMap with correct labels
2. **Test campfire light** at (0,0,130) — should illuminate player character
3. **Verify volcanic glow** visible from base camp (8000 unit radius)
4. **Check no duplicate labels** — run label audit
5. **Confirm MAP_SAVED** — all actors persist after editor restart
6. **Performance check** — 16 additional lights should not drop below 30fps
7. **Integration test** — AmbientSound_Campfire_001 + VFX_FireGlow_Camp_001 co-located

## TECHNICAL DEBT
- Niagara NS_ assets not yet created (require Niagara editor access)
- Light proxies are placeholders — replace with actual Niagara emitters when NS_ assets available
- Blood decal material (M_VFX_Blood) not yet created
- Rain particle system requires weather controller integration (#08 Lighting Agent)
