# Survival System — Technical Specification
**Author:** Agent #03 — Core Systems Programmer  
**Version:** 1.0  
**Last Updated:** Cycle 009 (2026-07-02)

---

## Overview

The survival system governs the player's physiological state in the prehistoric world. It is implemented via `SurvivalComponent` (integrated into `TranspersonalCharacter` in Cycle 006) and drives core gameplay tension.

---

## Survival Stats

| Stat | Range | Default | Drain Rate | Critical Threshold |
|---|---|---|---|---|
| **Health** | 0–100 | 100 | Damage-driven | ≤ 20 |
| **Hunger** | 0–100 | 100 | 0.5/min | ≤ 15 |
| **Thirst** | 0–100 | 100 | 1.0/min | ≤ 10 |
| **Stamina** | 0–100 | 100 | Sprint-driven | ≤ 5 |
| **Fear** | 0–100 | 0 | Threat-driven | ≥ 80 |

### Drain Rates — Design Rationale
- **Thirst drains 2× faster than hunger** — realistic physiology (dehydration kills faster)
- **Stamina recovers when not sprinting** — recovery rate = 15/s, drain rate = 25/s (sprint)
- **Fear accumulates near predators** — radius-based, decays when safe (5/s decay)
- **Health does NOT drain passively** — only from damage, starvation (hunger=0), or dehydration (thirst=0)

---

## Starvation & Dehydration Cascade

When **Hunger = 0**:
- Health drains at 2.0/min
- Movement speed reduced by 30%
- Stamina max reduced by 50%

When **Thirst = 0**:
- Health drains at 5.0/min (faster than starvation)
- Movement speed reduced by 50%
- Vision blur effect (post-process)

When **Health ≤ 0**:
- Player death → respawn at last campfire or PlayerStart

---

## Fear System

Fear is a unique stat that affects gameplay without directly killing the player:

| Fear Level | Effect |
|---|---|
| 0–30 | Normal gameplay |
| 30–60 | Slight camera shake, heartbeat audio |
| 60–80 | Reduced accuracy (if ranged weapons), faster stamina drain |
| 80–100 | Panic state — involuntary sprint, reduced control |

**Fear Sources:**
- TRex within 50m: +20 fear/s
- Raptor within 20m: +10 fear/s
- Triceratops charging: +15 fear/s
- Darkness (night, no campfire): +2 fear/s

**Fear Decay:**
- Near campfire: -10 fear/s
- In shelter: -15 fear/s
- Passive decay (safe): -5 fear/s

---

## Tick Implementation

### Current State (Cycle 009)
- `SurvivalComponent` registered as subobject in `TranspersonalCharacter` constructor
- CDO constructs without crash (validated Cycle 009)
- Tick rate: **1Hz** (every 1 second) — performance-safe

### Implementation Path (Headless Editor Constraint)
Since the editor binary is pre-built and C++ cannot be recompiled:
1. Survival drain logic is **defined in C++ defaults** (compile-time)
2. **Blueprint override** is the correct path for tuning values at runtime
3. **Python property sets** can adjust CDO defaults for testing

### Recommended Blueprint Setup
```
Event Tick (1s interval via Timer)
  → GetSurvivalComponent
  → DrainHunger(0.5 * DeltaTime)
  → DrainThirst(1.0 * DeltaTime)
  → CheckCriticalThresholds
  → BroadcastStatChanged
```

---

## Character Movement Integration

Movement speed is modulated by survival stats:

```
EffectiveWalkSpeed = BaseWalkSpeed (400)
  × HungerModifier (1.0 if hunger>30, 0.7 if hunger=0)
  × ThirstModifier (1.0 if thirst>20, 0.5 if thirst=0)
  × FearModifier (1.2 if fear>80 — panic sprint)
```

**Tuned values (applied Cycle 009):**
- `max_walk_speed`: 400 cm/s
- `jump_z_velocity`: 420 cm/s  
- `gravity_scale`: 1.0
- `max_step_height`: 45 cm
- `walkable_floor_angle`: 45°

---

## Food & Water Sources

### Food Items (to be implemented by Agent #09/#14)
| Item | Hunger Restored | Notes |
|---|---|---|
| Raw meat | +20 | Risk of illness (50% chance) |
| Cooked meat | +40 | Requires campfire |
| Berries | +10 | Safe, found near trees |
| Eggs | +25 | Found near nests |

### Water Sources
| Source | Thirst Restored | Notes |
|---|---|---|
| River/stream | +50 | Safe |
| Puddle | +20 | Risk of illness (30% chance) |
| Waterskin (crafted) | +60 | Portable, requires crafting |

---

## Integration Points

| System | Integration |
|---|---|
| **DinosaurAI (#12)** | Fear accumulation based on predator proximity |
| **Campfire props** | Fear decay, food cooking trigger |
| **Shelter actors** | Fear decay, temperature regulation |
| **NavMesh (#11)** | Movement speed affects pathfinding cost |
| **Audio (#16)** | Heartbeat SFX at fear >60, hunger rumble at hunger <30 |
| **VFX (#17)** | Blur post-process at thirst=0, heat shimmer |
| **Quest (#14)** | "Find water" quest triggers at thirst <20 |

---

## Performance Budget

| Operation | Cost | Frequency |
|---|---|---|
| Stat drain tick | <0.01ms | 1Hz |
| Fear radius check | <0.1ms | 2Hz |
| Movement modifier recalc | <0.01ms | On stat change |
| UI broadcast | <0.05ms | On stat change |

**Total survival system budget: <0.2ms/frame** — well within 60fps target.

---

## Open Issues

1. **Blueprint not yet created** — survival tick is C++ only, needs BP_TranspersonalCharacter override
2. **Illness system** — not yet implemented (raw food risk)
3. **Temperature stat** — planned but not in current SurvivalComponent (add in Cycle 010+)
4. **Crafting integration** — waterskin/cooked meat requires Agent #09 crafting system

---

## References
- `Source/TranspersonalGame/Core/GameFramework/TranspersonalCharacter.h` — character class
- `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` — component definition
- `Docs/Architecture/SceneActorRegistry.md` — actor registry for campfire/shelter props
- `Docs/Architecture/Cycle009_EngineArchitect_Report.md` — scene state from Agent #02
