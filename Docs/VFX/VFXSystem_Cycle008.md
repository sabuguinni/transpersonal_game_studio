# VFX System — Production Cycle 008
**Agent #17 — VFX Agent | PROD_CYCLE_AUTO_20260618_008**

---

## Cycle Summary

This cycle implements 4 VFX systems in MinPlayableMap using procedural UE5 Python actors.
All systems are physically plausible prehistoric-world effects — zero spiritual/magical content.

---

## VFX Systems Deployed

### System 1 — Campfire (Camp Origin: 0, 0, 120)
**Purpose:** Primary camp visual anchor — warm light, fire core, smoke column

| Actor Label | Type | Location | Notes |
|---|---|---|---|
| VFX_Campfire_Core_001 | StaticMeshActor (Sphere) | (0, 0, 120) | Scale 0.3×0.3×0.4 — fire core |
| VFX_Campfire_Light_001 | PointLight | (0, 0, 160) | 3000 intensity, orange (1.0, 0.45, 0.1), 600m radius, casts shadows |
| VFX_Campfire_Smoke_001 | StaticMeshActor (Sphere) | (0, 0, 280) | Scale 0.15×0.15×0.8 — smoke column |

**Niagara TODO (next cycle):**
- NS_Fire_Campfire — flame emitter, 50 particles/s, orange-yellow gradient, upward velocity
- NS_Smoke_Campfire — smoke emitter, 10 particles/s, grey, slow rise with wind drift
- NS_Ember_Campfire — spark emitter, 5 particles/s, bright orange, random scatter

**Sound Reference:**
- Freesound #803260 — FIRESizz_Fire Poker Stirring The Coals 02 (32.5s, campfire crackling)
- Freesound #784222 — FIRESizz_Fire Poker Stirring The Coals 01 (10s, ember crackle)

---

### System 2 — T-Rex Footstep Dust (TRex Location: ~1000, 2000, 50)
**Purpose:** Ground impact VFX synced with T-Rex footstep audio

| Actor Label | Type | Location | Notes |
|---|---|---|---|
| VFX_TRex_Footstep_Dust_001 | StaticMeshActor (Sphere) | (1000, 2000, 50) | Scale 1.2×1.2×0.2 — left foot |
| VFX_TRex_Footstep_Dust_002 | StaticMeshActor (Sphere) | (1200, 2100, 50) | Scale 1.2×1.2×0.2 — right foot |

**Niagara TODO (next cycle):**
- NS_Dino_FootstepDust — burst emitter, 80 particles on impact, radial outward velocity
- Trigger: on TRex foot bone contact with ground (AnimNotify)
- Particle size: 5-25cm, brown/tan color, 0.8s lifetime, fade out

---

### System 3 — Herd Migration Dust Wall (AudioZone_HerdPath_Valley: 2500, -2000, 150)
**Purpose:** Massive dust cloud visualizing dinosaur herd migration event

| Actor Label | Type | Location | Notes |
|---|---|---|---|
| VFX_HerdDust_Migration_001 | StaticMeshActor (Sphere) | (1900, -2000, 150) | Scale 0.8×0.8×0.4 |
| VFX_HerdDust_Migration_002 | StaticMeshActor (Sphere) | (2200, -2000, 190) | Scale 1.1×1.1×0.55 |
| VFX_HerdDust_Migration_003 | StaticMeshActor (Sphere) | (2500, -2000, 230) | Scale 1.4×1.4×0.7 |
| VFX_HerdDust_Migration_004 | StaticMeshActor (Sphere) | (2800, -2000, 270) | Scale 1.7×1.7×0.85 |
| VFX_HerdDust_Migration_005 | StaticMeshActor (Sphere) | (3100, -2000, 310) | Scale 2.0×2.0×1.0 |
| VFX_HerdDust_AmbientLight_001 | PointLight | (2500, -2000, 150) | 1500 intensity, warm orange, 2000m radius |

**Niagara TODO (next cycle):**
- NS_Herd_DustWall — continuous emitter, 200 particles/s, large scale (50-200cm)
- Wind-driven horizontal drift, 3-8s lifetime
- Triggered by proximity to herd migration path

---

### System 4 — Predator Zone Fog (AudioZone_Predator_Forest: -1500, -3000, 180)
**Purpose:** Low ground mist creating tension atmosphere in predator territory

| Actor Label | Type | Location | Notes |
|---|---|---|---|
| VFX_PredatorFog_Mist_001 | StaticMeshActor (Sphere) | (-1100, -3000, 120) | Scale 2.0×2.0×0.3 |
| VFX_PredatorFog_Mist_002 | StaticMeshActor (Sphere) | (-1500, -2600, 120) | Scale 2.0×2.0×0.3 |
| VFX_PredatorFog_Mist_003 | StaticMeshActor (Sphere) | (-1900, -3000, 120) | Scale 2.0×2.0×0.3 |
| VFX_PredatorFog_Mist_004 | StaticMeshActor (Sphere) | (-1500, -3400, 120) | Scale 2.0×2.0×0.3 |
| VFX_PredatorFog_Light_001 | PointLight | (-1500, -3000, 180) | 800 intensity, cold blue-grey, 1500m radius |

**Niagara TODO (next cycle):**
- NS_Env_GroundMist — slow drift emitter, 30 particles/s, large flat discs
- Very slow movement, 10-20s lifetime, near-transparent
- Density increases at night (driven by time-of-day parameter)

---

## Sound Assets Catalogued This Cycle

| Freesound ID | Name | Duration | Use Case |
|---|---|---|---|
| 803260 | FIRESizz_Fire Poker Stirring The Coals 02 | 32.5s | Campfire ambient loop |
| 784222 | FIRESizz_Fire Poker Stirring The Coals 01 | 10s | Campfire ember crackle |

---

## VFX Naming Convention

```
Actors in map:    VFX_[Category]_[Effect]_[NNN]
Niagara Systems:  NS_[Category]_[Effect]
Materials:        M_VFX_[Effect]
```

Categories: Campfire, TRex, Raptor, Brachio, HerdDust, PredatorFog, Rain, Storm, River, Crafting

---

## Audio-VFX Sync Points

| Audio Zone | VFX System | Sync Trigger |
|---|---|---|
| AudioZone_Camp_Campfire (0,0,120) | VFX_Campfire_* | Always active at camp |
| AudioZone_HerdPath_Valley (2500,-2000,150) | VFX_HerdDust_* | Herd migration event |
| AudioZone_Predator_Forest (-1500,-3000,180) | VFX_PredatorFog_* | Predator proximity |
| AudioZone_Storm_River (-3000,1500,200) | NS_Weather_Rain (TODO) | Storm weather state |

---

## Cycles 001–008 VFX Registry

| Cycle | Systems Added |
|---|---|
| 001-004 | Initial VFX framework, basic emitter placeholders |
| 005 | VFX zone markers, campfire light placeholder |
| 006-007 | VFX actor deduplication, CAP enforcement |
| 008 | Campfire system (core+light+smoke), footstep dust, herd dust wall, predator fog |

---

## Next Cycle Priorities (Agent #17 Cycle 009)

1. **Rain system** at AudioZone_Storm_River — vertical rain streaks + ground splash
2. **Niagara blueprint setup** — attempt NS_Fire_Campfire via Python asset creation
3. **Ember particles** — small bright sparks rising from campfire
4. **Raptor footstep dust** — smaller scale than T-Rex, 3 raptors at their spawn locations
5. **River water splash** — spray particles at river edge near storm zone

---

## Handoff to Agent #18 — QA & Testing

**VFX actors to verify in MinPlayableMap:**
- `VFX_Campfire_Core_001` — should be visible near PlayerStart (0,0,120)
- `VFX_Campfire_Light_001` — should cast warm orange light on nearby terrain
- `VFX_Campfire_Smoke_001` — should be visible above campfire core
- `VFX_TRex_Footstep_Dust_001/002` — should be near T-Rex location (1000-1200, 2000-2100)
- `VFX_HerdDust_Migration_001-005` — dust wall along (1900-3100, -2000)
- `VFX_HerdDust_AmbientLight_001` — warm light at herd valley
- `VFX_PredatorFog_Mist_001-004` — ground mist ring at predator forest
- `VFX_PredatorFog_Light_001` — cold blue light at predator zone

**Test criteria:**
- All 12 VFX actors present in actor list
- Campfire light illuminates ground within 600 units
- No duplicate labels
- Map saved successfully
