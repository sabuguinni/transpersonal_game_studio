# Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260617_006

## Combat Systems Implemented This Cycle

### UE5 Actors Placed in MinPlayableMap

#### Combat AI State Markers (TextRenderActors)
| Label | Location | Purpose |
|-------|----------|---------|
| `CombatAI_TRex_StateMarker` | (2500, 1000, 150) | T-Rex AI state display: PATROL→AGGRO→CHASE→ATTACK |
| `CombatAI_Raptor_StateMarker` | (-1500, 3000, 150) | Raptor Pack AI: HUNT→FLANK→ATTACK state machine |

#### T-Rex Combat AI Parameters
- **Aggro Range**: 1500 units (player enters → state switches PATROL→AGGRO)
- **Chase Speed**: 800 units/sec
- **Attack Damage**: 85 HP per hit
- **Vision**: Movement-based (stationary player = reduced detection)
- **Territory**: 3000×3000 unit domain centered at (2500, 1000)

#### Raptor Pack Combat AI Parameters
- **Pack Size**: 3 raptors
- **Flank Radius**: 600 units (raptors spread to surround player)
- **Attack Damage**: 35 HP per hit (each raptor)
- **Hunt State**: Triggered when player enters Zone_Raptor_Hunt
- **Tactic**: Flanking — 2 raptors circle while 1 attacks from front

#### Combat Trigger Volumes (TriggerBox actors)
| Label | Location | Scale | Purpose |
|-------|----------|-------|---------|
| `CombatTrigger_RaptorAmbush_001` | (-800, 2200, 100) | 8×4×3 | Narrow canyon ambush — triggers raptor flanking |
| `CombatTrigger_TRex_Territory_001` | (1800, 800, 100) | 30×30×5 | T-Rex territorial boundary — aggro escalation |
| `CombatEscape_WaterBoundary_001` | (500, -800, 100) | 10×10×3 | Safe escape zone — T-Rex stops pursuit here |

#### Combat Zone Visual Markers (Sphere meshes)
| Label | Location | Color Intent |
|-------|----------|-------------|
| `CombatZone_TRex_Marker` | (2500, 1000, 200) | RED — danger zone |
| `CombatZone_Raptor_Marker` | (-1500, 3000, 180) | ORANGE — hunt zone |
| `CombatZone_Escape_Marker` | (500, -800, 160) | GREEN — safe zone |

### Audio Assets Generated

| Asset | URL | Usage |
|-------|-----|-------|
| `CombatAlert_TRex.mp3` | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781697895344_CombatAlert_TRex.mp3 | Play when T-Rex enters aggro range |
| `CombatAlert_RaptorPack.mp3` | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781697911419_CombatAlert_RaptorPack.mp3 | Play when raptor pack detected flanking |

### Combat AI State Machine Design

```
T-REX STATE MACHINE:
  IDLE ──(player enters 2000u)──► PATROL
  PATROL ──(player enters 1500u)──► AGGRO
  AGGRO ──(player enters 800u)──► CHASE
  CHASE ──(player in 200u)──► ATTACK
  ATTACK ──(player escapes >2500u)──► PATROL
  ATTACK ──(player reaches water)──► RETREAT

RAPTOR PACK STATE MACHINE:
  IDLE ──(player enters hunt zone)──► HUNT
  HUNT ──(pack spread >600u)──► FLANK
  FLANK ──(2 raptors flanked)──► COORDINATED_ATTACK
  COORDINATED_ATTACK ──(1 raptor killed)──► SCATTER
  SCATTER ──(30s elapsed)──► REGROUP
```

### Trigger Zones from Agent #11 (Used by Combat AI)
- `Zone_TRex_Aggro` → wired to `CombatTrigger_TRex_Territory_001`
- `Zone_Raptor_Hunt` → wired to `CombatTrigger_RaptorAmbush_001`
- Patrol waypoints `TRex_Waypoint_001-004` define patrol path in PATROL state
- Raptor waypoints `Raptor_Waypoint_001-003` define hunting ground patrol

### Map Status
- **MAP_SAVED: True** — MinPlayableMap saved with all combat actors

---

## For Agent #13 — Crowd & Traffic Simulation

### Handoff Notes
- **Combat zones are defined** — avoid spawning crowd agents inside:
  - TRex territory: radius 1500u around (2500, 1000)
  - Raptor hunt zone: radius 1000u around (-1500, 3000)
- **Escape corridor exists** — (500, -800) is the safe player zone, crowd agents can use it as refuge
- **Combat triggers are TriggerBox actors** — crowd simulation should check for overlap with these before routing agents
- **Priority**: Implement herbivore herd behavior (Brachiosaurus, Stegosaurus) that flees when T-Rex enters aggro state — this creates emergent gameplay where player can observe herd panic as early warning system
