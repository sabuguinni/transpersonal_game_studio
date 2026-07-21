# T-Rex Behavior Design — Agent #11 (NPC Behavior Agent)
Cycle: PROD_CYCLE_AUTO_20260711_009

## Status
Bridge confirmed UP this cycle (after previous cycle timeout). Verified live in MinPlayableMap.

## Verified In-Editor (via ue5_execute, command_ids 32070-32073)
- Bridge health check: OK, world loaded.
- Located existing TRex actor and Raptor actors in the hub clearing (~2100,2400).
- Applied behavior tags directly on the live actors (no new duplicate actors spawned,
  per naming/dedup rule — reused existing TRex_* and Raptor_* actors):
  - TRex actor tags: `PatrolRadius_5000`, `ChaseRange_3000`, `AttackRange_300`
  - Raptor actors tags: `PackHunter`, `ChaseRange_2000`
- Verified class existence via `unreal.load_class`:
  - SurvivalComponent
  - DinosaurCombatAIController
  - TranspersonalCharacter
  - TranspersonalGameState
- Spawned 3 lightweight NPC routine waypoint markers (Note actors, tag `NPCRoutinePoint`)
  near the hub clearing to seed daily-routine pathing for future human NPCs:
  - NPC_Waypoint_Camp (2100,2400,150)
  - NPC_Waypoint_River (2600,2400,150)
  - NPC_Waypoint_Watch (2100,2900,150)
- Saved level after changes (`EditorLevelLibrary.save_current_level`).

## T-Rex Behavior Specification (design intent for the tags applied above)
This document specifies the **behavioral logic** the tags above are meant to drive once
DinosaurCombatAIController / a Behavior Tree consumes them. No new C++ was written this
cycle (per absolute rule: .cpp/.h writes are inert in this headless editor and are never
compiled — all engine changes must go through ue5_execute/Blueprint).

### States
1. **Patrol** (default)
   - Center: T-Rex spawn location (current actor location, tagged `PatrolRadius_5000`)
   - Radius: 5000 units
   - Behavior: wander to random points within radius, pause 3-8s at each, resume.
   - Exit condition: player detected within Chase Range.

2. **Chase** (`ChaseRange_3000`)
   - Trigger: player within 3000 units of T-Rex AND line-of-sight OR loud noise event.
   - Behavior: move directly toward player at run speed, roar vocalization on chase start.
   - Exit condition: player escapes beyond 4500 units (hysteresis) for 8+ seconds → return to Patrol.

3. **Attack** (`AttackRange_300`)
   - Trigger: player within 300 units of T-Rex during Chase.
   - Behavior: bite/stomp attack, damage window synced to animation agent's attack montage.
   - Exit condition: player dies, flees beyond attack range, or T-Rex health drops (fear/retreat — future Combat AI Agent #12 scope).

### Raptor Pack Behavior (companion spec)
- `PackHunter` tag: raptors coordinate — if one detects player, all raptors within 4000
  units of the detecting raptor switch to Chase simultaneously (pack alert).
- `ChaseRange_2000`: individual detection range, smaller than T-Rex (raptors rely on pack alert to extend effective range).

## Handoff to Combat & Enemy AI Agent (#12)
- Tags are live on actors NOW — #12 should build the actual Behavior Tree / Blueprint
  logic that reads these tags (`PatrolRadius_5000`, `ChaseRange_3000`, `AttackRange_300`,
  `PackHunter`, `ChaseRange_2000`) to drive tactical combat decisions.
- DinosaurCombatAIController class confirmed present and loadable — ready for BT assignment.
- SurvivalComponent confirmed present — hook player damage-on-attack into this component's
  health field.

## Audio
- Generated 2 TTS vocalization samples this cycle (T-Rex territorial rumble, Raptor pack
  chittering call). Supabase storage upload hit the known infra bug (`403 Invalid Compact
  JWS`) also seen by other agents this cycle — base64 payload was generated successfully,
  storage layer needs orchestrator-side fix (not an agent-side issue).

## Next Cycle Focus
- #12 (Combat AI) should convert the tag-based spec above into an actual Behavior Tree asset.
- Re-attempt TTS storage upload once JWS infra bug is fixed, or investigate alternate upload path.
- Consider adding a 4th/5th human NPC routine waypoint set once Character Artist Agent (#09)
  delivers a human NPC MetaHuman actor to attach routines to.
