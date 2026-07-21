# Combat & Enemy AI System — Agent #12
## Transpersonal Game Studio — Prehistoric Survival Game

---

## Overview

The Combat AI system governs how dinosaurs detect, pursue, and attack the player. Each species has distinct behavioral traits modeled on real predator/prey dynamics. The system is implemented via UE5 Python (procedural actor placement) and Blueprint configuration assets.

---

## Species Combat Profiles

### T-Rex (`Combat_TRex_AggroZone_001`)
- **Detection Type**: Motion-based vision cone (stationary player = invisible)
- **Aggro Radius**: 1500 units (marked by `VisionCone_TRex_001` red point light)
- **Attack Radius**: 300 units (instant kill zone `Combat_DeathZone_TRex_001`)
- **Speed**: Slow pursuit, high damage
- **Behavior States**:
  - `Idle` → patrol between WP_TRex_001 through WP_TRex_004
  - `Alert` → player enters aggro zone, T-Rex turns toward movement
  - `Chase` → player runs, T-Rex accelerates to full speed
  - `Attack` → player within 300 units, bite animation + death
  - `Lose` → player hides in water or dense foliage for 10+ seconds
- **Counter-Strategy**: Stay still, move only when T-Rex looks away, use water to break scent trail

### Raptor Pack (`Combat_Raptor_AmbushZone_001`)
- **Detection Type**: Sound + movement, pack coordination
- **Aggro Radius**: 800 units
- **Flanking Positions**:
  - `Combat_Raptor_Flank_Left_001` — orange marker
  - `Combat_Raptor_Flank_Right_001` — orange marker
  - `Combat_Raptor_Flank_Rear_001` — orange marker
- **Behavior States**:
  - `Idle` → loose patrol in hunting zone
  - `Alert` → one raptor spots player, calls pack (screech SFX)
  - `Flank` → raptors split to 3 flanking positions simultaneously
  - `Attack` → coordinated pounce from multiple directions
  - `Retreat` → if player kills 2 of 3, surviving raptor flees
- **Counter-Strategy**: Climb to high ground, use fire, fight one at a time

### Ankylosaurus (`Combat_Ankylo_DefenseZone_001`)
- **Detection Type**: Proximity only — not aggressive unless approached
- **Aggro Radius**: 600 units
- **Behavior States**:
  - `Graze` → slow movement, feeding animation
  - `Warn` → player within 600 units, tail swings side to side (warning)
  - `Charge` → player within 300 units or attacks, tail club swing
  - `Retreat` → after 3 hits, moves away slowly
- **Counter-Strategy**: Keep distance, circle wide, never approach from behind

---

## In-Map Assets (MinPlayableMap)

| Label | Type | Location | Purpose |
|-------|------|----------|---------|
| `Combat_TRex_AggroZone_001` | TriggerBox | (2000, 1500, 50) | T-Rex detection boundary |
| `Combat_Raptor_AmbushZone_001` | TriggerBox | (-1500, 2000, 50) | Raptor pack territory |
| `Combat_Ankylo_DefenseZone_001` | TriggerBox | (500, -2500, 50) | Ankylosaurus grazing zone |
| `VisionCone_TRex_001` | PointLight (Red) | (2000, 1500, 200) | Visual debug: T-Rex detection radius |
| `Combat_Raptor_Flank_Left_001` | PointLight (Orange) | (-1200, 1800, 100) | Raptor left flank position |
| `Combat_Raptor_Flank_Right_001` | PointLight (Orange) | (-1800, 2200, 100) | Raptor right flank position |
| `Combat_Raptor_Flank_Rear_001` | PointLight (Orange) | (-1500, 2500, 100) | Raptor rear flank position |
| `Combat_DeathZone_TRex_001` | TriggerBox | (2000, 1500, 50) | Instant kill radius (T-Rex bite) |
| `BP_CombatAIConfig` | Blueprint Actor | /Game/AI/Combat/ | Per-species AI parameter store |

---

## Audio Assets

| Character | Line | URL |
|-----------|------|-----|
| SurvivalNarrator | "It charges when it sees movement. Stand perfectly still..." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693417971_SurvivalNarrator.mp3 |
| TribalElder | "Three raptors, always three. One draws your attention from the front..." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693436726_TribalElder.mp3 |

---

## Combat Design Philosophy

> "The best combat is the one where the player doesn't know if they'll win until the last second — and believes they lost because they made a mistake, not because the game was unfair."

### 30-Second Loop (Jaime Griesemer principle)
Each dinosaur encounter follows a repeatable 30-second tension arc:
1. **0-10s**: Detection phase — player notices dinosaur, assesses threat
2. **10-20s**: Tension phase — player attempts evasion or positioning
3. **20-30s**: Resolution phase — successful escape OR combat engagement

### Fairness Rules
- Every attack has a visible telegraph (animation windup ≥ 0.5s)
- Every species has a reliable counter-strategy the player can discover
- Death always has a clear cause (player entered death zone, player ran from T-Rex)
- No instant kills without warning (except `Combat_DeathZone_TRex_001` which requires entering the T-Rex's mouth range)

---

## Integration with Agent #11 (NPC Behavior)

The following assets from Agent #11 are used by Combat AI:
- `Zone_TRex_Patrol_001` → defines T-Rex patrol boundary
- `WP_TRex_001` through `WP_TRex_004` → T-Rex patrol waypoints
- `Zone_Raptor_Hunt_001` → raptor hunting territory (overlaps with `Combat_Raptor_AmbushZone_001`)
- `NavMesh_World_001` → enables `AIMoveTo` pathfinding for all dinosaurs
- `BP_NPCBehaviorConfig` → base config extended by `BP_CombatAIConfig`

---

## Next Steps for Agent #13 (Crowd & Traffic Simulation)

1. Use `Combat_TRex_AggroZone_001` as a **crowd avoidance zone** — herbivore herds flee when T-Rex enters
2. Use raptor flanking markers as **crowd scatter points** — prey animals disperse to opposite positions
3. Implement **stampede behavior** — when T-Rex enters savanna zone, trigger mass movement of herbivores
4. Connect `Combat_DeathZone_TRex_001` to crowd simulation — agents within this zone are "consumed" and removed
5. Use `VisionCone_TRex_001` light radius as crowd awareness boundary — agents outside red light radius are unaware

---

*Generated by Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260617_005*
