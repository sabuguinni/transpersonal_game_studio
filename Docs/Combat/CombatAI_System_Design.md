# Combat & Enemy AI System — Agent #12
## Transpersonal Game Studio — Prehistoric Survival Game

---

## Overview

The Combat AI system governs how dinosaurs engage the player in tactical, believable encounters. Every combat encounter is designed as a **choreographed scene** with tension, escalation, and resolution — not a damage-per-second calculation.

**Design Philosophy:** The best combat is one where the player doesn't know if they'll win until the last second — and believes they lost because of their own mistake, not because the game was unfair.

---

## Dinosaur Combat Profiles

### T-Rex (Tyrannosaurus Rex)
- **Threat Level:** APEX
- **Engagement Range:** 600 units
- **Combat Style:** Ambush → Charge → Stomp
- **Weakness:** Cannot turn quickly (pivot radius ~400 units)
- **Player Counter:** Stand ground → wait → dive perpendicular at last moment
- **AI States:** PATROL → ALERT → CHARGE → STOMP → RECOVER
- **Pack Behavior:** Solitary — never coordinates with other species

**Tactical Notes:**
- T-Rex hunts by vibration (footsteps) and vision
- Crouching + slow movement reduces detection range by 60%
- Fire/smoke creates confusion (detection range -80%)
- High ground is NOT safe — T-Rex can reach 4m elevation

### Velociraptor (Pack Hunter)
- **Threat Level:** HIGH
- **Engagement Range:** 400 units
- **Combat Style:** Coordinated flanking → Drive + Encircle
- **Weakness:** Loses coordination when pack leader is eliminated
- **Player Counter:** High ground, fire, eliminate drive raptor first
- **AI States:** SCOUT → SIGNAL → FLANK → DRIVE → STRIKE → RETREAT
- **Pack Behavior:** 2-4 raptors, designated roles (Driver, Left Flanker, Right Flanker, Reserve)

**Tactical Notes:**
- Pack leader signals with vocalizations (audio cue for player)
- If player kills pack leader, remaining raptors scatter for 15 seconds
- Raptors avoid fire — use torch as deterrent (not weapon)
- Night hunting: raptors are 40% more aggressive after sunset

### Brachiosaurus (Territorial Defender)
- **Threat Level:** MEDIUM (defensive only)
- **Engagement Range:** 800 units (territory boundary)
- **Combat Style:** Stomp → Tail Sweep → Charge (last resort)
- **Weakness:** Slow turning, predictable attack patterns
- **Player Counter:** Stay outside territory, use terrain obstacles
- **AI States:** GRAZE → WARN → STOMP → CHARGE → RETURN
- **Pack Behavior:** Herd of 3-6, defensive formation when threatened

---

## Combat Zone System

### Zone Types

| Zone Type | Radius | Behavior Trigger |
|-----------|--------|-----------------|
| CombatZone_TRex | 600 units | T-Rex enters ALERT state |
| CombatZone_Raptor | 400 units | Pack signals and begins flanking |
| CombatZone_Brachio | 800 units | Herd enters defensive formation |
| AmbushPoint | 200 units | Predator launches immediate charge |

### Flanking Formation Logic

```
Raptor Pack (3-unit formation):
                [DRIVE]
                  ↑
    [LEFT FLANK]  P  [RIGHT FLANK]
                  ↓
              [PLAYER]

Drive raptor pushes player toward flankers.
Flankers close pincer when player moves forward.
Counter: eliminate DRIVE raptor first to break coordination.
```

---

## Combat Actors in MinPlayableMap

### Spawned This Cycle (PROD_CYCLE_AUTO_20260617_014)

| Label | Type | Location | Purpose |
|-------|------|----------|---------|
| CombatZone_TRex_001 | TriggerBox | (1200, 800, 100) | T-Rex engagement zone |
| CombatZone_Raptor_001 | TriggerBox | (-800, 1500, 80) | Raptor pack zone 1 |
| CombatZone_Raptor_002 | TriggerBox | (2200, -600, 90) | Raptor pack zone 2 |
| Raptor_Flank_Left_001 | PointLight (orange) | (-300, 800, 120) | Left flanker position |
| Raptor_Flank_Right_001 | PointLight (orange) | (300, 800, 120) | Right flanker position |
| Raptor_Drive_001 | PointLight (red) | (0, 1200, 120) | Drive raptor position |
| Ambush_TRex_001 | PointLight (dark red) | (1500, 300, 150) | T-Rex ambush point 1 |
| Ambush_TRex_002 | PointLight (dark red) | (-1200, -500, 150) | T-Rex ambush point 2 |

---

## Audio Assets Generated

| File | URL | Usage |
|------|-----|-------|
| CombatNarrator.mp3 | Supabase Storage | T-Rex dodge tutorial VO |
| SurvivalGuide.mp3 | Supabase Storage | Raptor pack warning VO |

**CombatNarrator line:**
> "The T-Rex charges. Your only chance — stand your ground, wait for the last second, then dive left. It cannot turn fast enough. Move too early and it adjusts. Move too late and you are dead. This is the window. Use it."

**SurvivalGuide line:**
> "Three raptors. One in front, two circling wide. They are coordinating — this is not instinct, this is a hunt. You need high ground. Now. Get above them and they lose the angle."

---

## Combat Encounter Design: "The Raptor Gauntlet"

**Location:** Forest clearing, 800 units north of PlayerStart
**Setup:** Player must cross clearing to reach resource cache
**Encounter Flow:**
1. Player enters CombatZone_Raptor_001
2. Audio cue: raptor vocalization (pack signal)
3. Drive raptor appears at north edge (Raptor_Drive_001 position)
4. 3 seconds later: flankers emerge from sides (Flank_Left, Flank_Right)
5. Player has 3 options:
   - **Fight:** Eliminate drive raptor → pack scatters (15s window to flee)
   - **Flee:** Sprint south → raptors pursue for 400 units then return
   - **Evade:** Climb rock formation at (200, 600, 0) → raptors circle below

**Success Condition:** Player reaches resource cache or escapes zone
**Failure Condition:** Player caught in flanking pincer (no escape route)

---

## Next Steps for Agent #13 (Crowd & Traffic Simulation)

- Raptor pack coordination uses Mass AI signals — integrate with crowd system
- Herd behavior (Brachiosaurus) needs crowd simulation for 3-6 simultaneous agents
- Combat zones should feed into crowd density calculations (predators clear areas)
- Panic propagation: when T-Rex enters area, all herbivore crowds flee

---

*Generated by Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260617_014*
