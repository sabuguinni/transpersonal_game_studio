# VFX Registry — Cycle AUTO_20260703_005 (Agent #17)

## Status: ACTIVE — 4 VFX placeholder lights + 3 trigger volumes deployed

---

## VFX Actors Deployed in MinPlayableMap

### Category 1 — Campfire Hub (Priority: HIGH)
| Actor Label | Type | Location | Color | Intensity | Radius |
|---|---|---|---|---|---|
| `VFX_Campfire_Hub_001` | PointLight | (2100, 2400, 90) | Orange (1.0, 0.45, 0.05) | 8000 | 800u |
| `VFXTrigger_Campfire_Hub_001` | TriggerVolume | (2100, 2400, 80) | — | — | 400u |

**Design intent:** Warm orange flickering light simulates campfire glow at the hub clearing.
**Niagara upgrade path:** Replace with `NS_Fire_Campfire` Niagara system (embers, smoke column, ash particles).
**Audio link:** `AudioZone_Campfire_Hub_001` (Freesound #681366 campfire loop, 800u radius).

---

### Category 2 — Raptor Fern Disturbance Zone (Priority: MEDIUM)
| Actor Label | Type | Location | Color | Intensity | Radius |
|---|---|---|---|---|---|
| `VFX_Raptor_FernZone_001` | PointLight | (2100, 1200, 60) | Green (0.2, 0.7, 0.1) | 1500 | 600u |
| `VFXTrigger_Raptor_FernZone_001` | TriggerVolume | (2100, 1200, 80) | — | — | 800u |

**Design intent:** Subtle green ambient light marks the raptor patrol zone south of hub.
**Niagara upgrade path:** Replace with `NS_Dino_FernDisturbance` — leaf particles + dust puffs triggered on raptor movement events.
**Audio link:** `AudioZone_Raptor_FernZone_001` (Freesound #435152 predator creatures, 1500u radius).
**Gameplay mechanic:** Player enters trigger → fern rustling particles activate → audio stinger plays → raptor aggro state increases.

---

### Category 3 — T-Rex North River Zone (Priority: HIGH)
| Actor Label | Type | Location | Color | Intensity | Radius |
|---|---|---|---|---|---|
| `VFX_TRex_Footstep_NorthRiver_001` | PointLight | (1800, 4800, 30) | Dust (0.6, 0.4, 0.2) | 2000 | 400u |
| `VFXTrigger_TRex_NorthRiver_001` | TriggerVolume | (1800, 4800, 80) | — | — | 300u |

**Design intent:** Ground-level warm dust glow marks T-Rex territory near north river.
**Niagara upgrade path:** Replace with `NS_Dino_TRexFootstep` — ground crack decal + dust cloud burst on each footstep event.
**Screen shake:** When player enters 300u trigger, Blueprint calls `CameraShake` with magnitude scaled to distance.
**Audio link:** `AudioZone_TRex_NorthRiver_001` (Freesound #278229 dinosaur growl, 2500u radius).

---

### Category 4 — Volcanic Horizon Ambient (Priority: LOW)
| Actor Label | Type | Location | Color | Intensity | Radius |
|---|---|---|---|---|---|
| `VFX_Volcanic_Ash_Horizon_001` | PointLight | (5000, 5000, 200) | Red-Orange (1.0, 0.2, 0.0) | 15000 | 3000u |

**Design intent:** Distant volcanic glow on the horizon — world-building atmosphere, not interactive.
**Niagara upgrade path:** `NS_World_VolcanicAsh` — slow-falling ash particles across entire map, density increases toward volcano zone.
**No trigger volume** — always active as ambient world effect.

---

## Niagara System Specifications (Pending Implementation)

### NS_Fire_Campfire
- **Emitters:** (1) Flame core — cone spawn, 0.3s lifetime, orange→yellow gradient; (2) Ember — sphere spawn, 2s lifetime, drift upward with turbulence; (3) Smoke column — ribbon emitter, 5s lifetime, grey→transparent, wind-affected
- **LOD:** Full at <500u, reduced at 500-1500u, billboard sprite at >1500u
- **Performance budget:** 200 particles max, 0.3ms GPU

### NS_Dino_TRexFootstep
- **Emitters:** (1) Ground crack — decal burst, 1 particle, 3s lifetime; (2) Dust cloud — sphere spawn, 50 particles, 1.5s lifetime, ground-level; (3) Debris — random velocity, 20 particles, 1s lifetime
- **Trigger:** Blueprint event `OnTRexFootstep` → Niagara system activate at footstep bone location
- **LOD:** Full at <800u, disabled at >1500u

### NS_Dino_FernDisturbance
- **Emitters:** (1) Leaf particles — 30 particles, 2s lifetime, random rotation; (2) Dust puff — 20 particles, 0.8s lifetime
- **Trigger:** Raptor movement speed > 200 cm/s → activate at raptor root bone
- **LOD:** Full at <600u, disabled at >1200u

### NS_Weather_Rain
- **Emitters:** (1) Raindrops — box spawn above camera, 0.5s lifetime, fast downward velocity; (2) Splash — ground impact, 10 particles per drop; (3) Mist — low-velocity ambient, 5s lifetime
- **LOD:** Full at <2000u from camera, reduced at >2000u

### NS_World_VolcanicAsh
- **Emitters:** (1) Ash flakes — slow drift, 8s lifetime, grey-white; (2) Cinder sparks — occasional bright orange, 3s lifetime
- **Coverage:** World-space emitter, covers entire map
- **LOD:** Density scales with distance from volcano (5000, 5000)

---

## VFX ↔ Audio ↔ Gameplay Integration Map

```
EVT_PlayerEnterCampfire (radius 400u)
  → VFX: NS_Fire_Campfire activate
  → Audio: AudioZone_Campfire_Hub_001 volume increase
  → Gameplay: Warmth stat +2/s, Fear stat -1/s

EVT_RaptorMovement (speed > 200 cm/s)
  → VFX: NS_Dino_FernDisturbance at raptor location
  → Audio: Raptor screech stinger (Freesound #435152)
  → Gameplay: Player fear stat +5, screen edge vignette

EVT_TRexFootstep
  → VFX: NS_Dino_TRexFootstep at foot bone
  → Audio: Deep rumble + ground shake sound (Freesound #278229)
  → Gameplay: CameraShake (magnitude 3.0, falloff 300u), fear +15

EVT_NightCycle_Begin
  → VFX: Firefly particles around campfire (NS_Ambient_Fireflies)
  → Audio: Night insect ambient (Freesound #681366 variant)
  → Gameplay: Predator aggro radius +30%, visibility -40%

EVT_ForestSilence_Detected (insect ambient drops)
  → VFX: Subtle green atmospheric tint shift (post-process material)
  → Audio: Silence — no ambient insects
  → Gameplay: Danger indicator activates, predator within 500u
```

---

## Files Written This Cycle
- `Content/VFX/VFXRegistry/Cycle007_VFXRegistry.md` — this file

## UE5 Actors Placed This Cycle
- `VFX_Campfire_Hub_001` — PointLight @ (2100, 2400, 90)
- `VFX_TRex_Footstep_NorthRiver_001` — PointLight @ (1800, 4800, 30)
- `VFX_Raptor_FernZone_001` — PointLight @ (2100, 1200, 60)
- `VFX_Volcanic_Ash_Horizon_001` — PointLight @ (5000, 5000, 200)
- `VFXTrigger_Campfire_Hub_001` — TriggerVolume @ (2100, 2400, 80)
- `VFXTrigger_Raptor_FernZone_001` — TriggerVolume @ (2100, 1200, 80)
- `VFXTrigger_TRex_NorthRiver_001` — TriggerVolume @ (1800, 4800, 80)

## Dependencies
- **From Audio Agent (#16):** AudioZone actors at hub, fern zone, north river — CONFIRMED present
- **To QA Agent (#18):** Verify VFX lights visible in viewport, trigger volumes have correct extents, no actor label duplicates

---
*Generated by Agent #17 — VFX Agent | Cycle AUTO_20260703_005*
