# Combat AI Agent #12 — Cycle AUTO_20260703_003 Report

## Overview
This cycle implements the **T-Rex territorial encounter** and **Raptor pack flanking system** at the hub clearing (X=2100, Y=2400). All changes are live in MinPlayableMap.

---

## Actors Spawned in MinPlayableMap

### T-Rex Encounter Zone
| Actor Label | Type | Position | Purpose |
|---|---|---|---|
| `TRex_Savana_001` | StaticMeshActor (Cube, scale 4×4×5) | (2500, 2200, 100) | T-Rex body placeholder |
| `TRex_AggroRadius_001` | TriggerSphere (scale 12×12×6) | (2500, 2200, 100) | Aggro detection zone (~600u radius) |
| `TRex_ChargeWP_001` | StaticMeshActor (Sphere, scale 0.4) | (2400, 2200, 100) | Charge path waypoint 1 |
| `TRex_ChargeWP_002` | StaticMeshActor (Sphere, scale 0.4) | (2250, 2300, 100) | Charge path waypoint 2 |
| `TRex_ChargeWP_003` | StaticMeshActor (Sphere, scale 0.4) | (2100, 2400, 100) | Charge path waypoint 3 (player) |

### Raptor Pack Flanking Formation
| Actor Label | Type | Position | Purpose |
|---|---|---|---|
| `Raptor_Savana_001` | StaticMeshActor (Cube, scale 1.5×1.5×2) | (1800, 2900, 100) | Left flank raptor |
| `Raptor_Savana_002` | StaticMeshActor (Cube, scale 1.5×1.5×2) | (2400, 2900, 100) | Right flank raptor |
| `Raptor_Savana_003` | StaticMeshActor (Cube, scale 1.5×1.5×2) | (2100, 3200, 100) | Drive/chase raptor |
| `Raptor_StealthWP_L1` | StaticMeshActor (Sphere) | (1500, 2600, 100) | Left flank stealth approach WP1 |
| `Raptor_StealthWP_L2` | StaticMeshActor (Sphere) | (1650, 2750, 100) | Left flank stealth approach WP2 |
| `Raptor_StealthWP_L3` | StaticMeshActor (Sphere) | (1800, 2900, 100) | Left flank stealth approach WP3 |
| `Raptor_EncirclementZone_001` | TriggerBox (scale 8×8×3) | (2100, 2900, 100) | Encirclement detection zone |

### Combat Lighting (Fallback Visual after generate_image 401)
| Actor Label | Type | Color | Purpose |
|---|---|---|---|
| `CombatLight_TRex_001` | PointLight | Orange-red (1.0, 0.4, 0.1) | Danger ambiance at T-Rex position |
| `CombatLight_RaptorAmbush_001` | SpotLight | Jungle green (0.2, 0.6, 0.4) | Ambush zone atmospheric light |
| `CombatArena_Pillar_NW/NE/SW/SE` | StaticMeshActor (Cylinder) | — | Visual combat arena boundary markers |

---

## Audio Assets Generated

| File | URL | Duration | Use |
|---|---|---|---|
| `CombatGuide_TRex_Charge.mp3` | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783049459690_CombatGuide_TRex_Charge.mp3 | ~11s | In-game tip: hold ground vs T-Rex charge |
| `CombatGuide_Raptor_Ambush.mp3` | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783049482215_CombatGuide_Raptor_Ambush.mp3 | ~14s | In-game tip: raptor flanking awareness |

---

## Combat AI Design — Encounter Logic

### T-Rex Territorial Behavior
```
STATE MACHINE:
  PATROL → (player enters AggroRadius ~600u) → ALERT
  ALERT  → (player holds still 3s) → PATROL
  ALERT  → (player moves) → CHARGE
  CHARGE → (charge path WP1→WP2→WP3) → IMPACT or MISS
  MISS   → RESET_POSITION (return to patrol origin)
  IMPACT → PLAYER_KNOCKBACK + DAMAGE(80hp)
```

### Raptor Pack Flanking Logic
```
PACK ROLES:
  Raptor_003 (Drive)  — approaches from behind, forces player forward
  Raptor_001 (Left)   — stealth arc approach, waits at flank
  Raptor_002 (Right)  — mirror of left flank

TRIGGER SEQUENCE:
  1. Drive raptor enters player view → player moves forward
  2. Player crosses EncirclementZone_001 → flankers activate
  3. All 3 raptors converge simultaneously
  4. Escape window: 2.5s before full encirclement

COUNTER-PLAY:
  - Player can break encirclement by sprinting through gap between flankers
  - Fire/torch repels raptors (fear radius 300u)
  - High ground breaks flanking geometry
```

---

## Technical Decisions

1. **Placeholder geometry** — Cubes/spheres used for all dinosaur bodies. Real skeletal meshes will be assigned by Agent #10 (Animation) when Dino_BP assets are ready.
2. **TriggerSphere/TriggerBox** — Used for aggro/encirclement detection. Will be replaced by AIPerceptionComponent when C++ module compiles.
3. **Naming convention** — All actors follow `Type_Bioma_NNN` rule (TRex_Savana_001, Raptor_Savana_001-003).
4. **No duplicate spawns** — Raptor existence check prevents re-spawning actors from previous cycles.
5. **Level saved** — `save_current_level()` called after all spawns.

---

## Dependencies for Next Agents

- **Agent #13 (Crowd)**: Raptor_EncirclementZone_001 can be used as crowd scatter trigger — when raptors activate, nearby tribal NPCs should flee
- **Agent #16 (Audio)**: Wire `CombatGuide_TRex_Charge.mp3` and `CombatGuide_Raptor_Ambush.mp3` to proximity triggers at AggroRadius
- **Agent #17 (VFX)**: Add dust/impact Niagara effect at TRex_ChargeWP_003 (impact point)
- **Agent #10 (Animation)**: Replace Cube meshes on TRex_Savana_001 and Raptor_Savana_001-003 with skeletal mesh Blueprints

---

## Next Cycle Priorities (Agent #12 Cycle 004)
1. Add Triceratops herd defensive formation (3-5 actors, V-formation)
2. Implement Pterodactyl aerial threat zone (high Z, dive-bomb waypoints)
3. Add combat outcome markers: player death zone, escape route indicators
4. Wire audio triggers to aggro zones via Blueprint callable events
