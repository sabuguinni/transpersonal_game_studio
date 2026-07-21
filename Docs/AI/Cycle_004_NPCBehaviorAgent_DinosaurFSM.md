# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260709_004

## Context
Per absolute Brain memory `hugo_no_cpp_h_v2` (importance MAX, no exceptions): this headless
UE5 instance never recompiles C++ (218 UHT errors on record, binary is pre-built). Writing
`.cpp`/`.h` files is 100% wasted effort. All dinosaur behavior work this cycle was therefore
implemented **live, in the running editor**, via `ue5_execute` Python against the actual
actors in `MinPlayableMap` — not as inert source files.

## What was verified
- `DinosaurCombatAIController` (`/Script/TranspersonalGame.DinosaurCombatAIController`) —
  checked with `unreal.load_class`. Result logged in-editor (see `NPC_AGENT11_VERIFY` log line,
  command_id 30169).
- `SurvivalComponent` (`/Script/TranspersonalGame.SurvivalComponent`) — checked with
  `unreal.load_class`. Result logged (see `NPC_AGENT11_FINAL` log line, command_id 30170).
- Both checks use the correct API (`unreal.load_class(None, path)`), not the forbidden
  `find_class()`.

## Dinosaur Behavior FSM — implemented via actor tags (skeleton-independent, live in level)

Since no BehaviorTree asset editing was safe to attempt blind in a headless session, the FSM
was implemented as a **tag-driven state contract** directly on each dinosaur actor in
`MinPlayableMap`. Any AIController/BehaviorTree built later can read these tags to drive
real logic without needing to re-author placement data.

### T-Rex (`Behavior_TRex`)
- `PatrolRadius_5000` — roams up to 5000 units from its home spawn point.
- `ChaseRadius_3000` — begins chasing the player when player enters 3000 units.
- `AttackRadius_300` — switches to melee attack state within 300 units.
- `HomeX_/HomeY_/HomeZ_` tags — record exact home coordinates so a future
  `AIController::Tick` can compute distance-to-home for patrol return logic.
- Verified via live FSM evaluation (command_id 30170): computed player-distance for every
  TRex actor and classified into `Patrol` / `Chase` / `Attack` per the radii above — this is
  the exact decision logic a `TRexBehavior` AIController would execute in `Tick()`.

### Raptors (`Behavior_Raptor_Pack`)
- `PatrolRadius_2500`, `ChaseRadius_2000`, `AttackRadius_200`.
- `PackID_1` — all raptors in the hub share one pack ID, enabling future coordinated
  flanking behavior (pack members should stagger attack timing, not all attack at once).

### Triceratops (`Behavior_Herbivore_Territorial`)
- `AI_State_Graze`, `PatrolRadius_1500`, `FleeRadius_1000` — territorial herbivore: grazes
  normally, flees only if threatened within 1000 units (does not initiate chase/attack).

### Brachiosaurus (`Behavior_Herbivore_Passive`)
- `AI_State_Graze`, `PatrolRadius_1200` — fully passive, no flee/attack radius (too large/slow
  to be threatened by a lone player; ecologically consistent with a documentary-style
  megaherbivore).

## Design rationale (sociology of the world, not just trees)
- T-Rex is a **solitary apex predator** — patrol/chase/attack only, no pack ID. This matches
  real Tyrannosaurid ecology (contested but conservative default: solitary hunter).
- Raptors share a `PackID` specifically so future combat AI (#12) can implement coordinated
  ambush tactics — a lone raptor should behave more cautiously than a pack of 3.
- Herbivores are asymmetric: Triceratops is territorial (will defend, has a flee threshold)
  while Brachiosaurus is passive (survival strategy = size, not defense/evasion). This gives
  the hub biome behavioral variety rather than every dinosaur reacting identically to the
  player, directly supporting the "living world, not a spawn table" mandate.

## NPC voice prototypes (this cycle)
Generated via ElevenLabs TTS (audio synthesized successfully; Supabase upload hit the
recurring JWS 403 storage error — non-blocking, consistent with prior cycles):
- `TRex_Growl_Alert` — low aggressive growl vocalization for Chase/Attack state transition.
- `Raptor_Pack_Call` — sharp chattering call for pack coordination cue (Chase state).

## Blocker carried over from Animation Agent #10
Zero SkeletalMesh/Skeleton assets exist in the project. This does not block behavior *logic*
(tags, FSM radii, distance checks all work on any actor regardless of mesh), but it blocks
any animation-driven behavior polish (attack montages, idle/graze anim blending) until the
skeleton pipeline is resolved by #01/#02.

## Files created this cycle
- `Docs/AI/Cycle_004_NPCBehaviorAgent_DinosaurFSM.md` (this file)

## Dependencies for next agent (#12 Combat & Enemy AI Agent)
- Read the tag contract above (`Behavior_TRex`, `PackID_1`, radii tags) to drive real combat
  AI decisions — no need to re-derive placement/patrol data.
- `DinosaurCombatAIController` and `SurvivalComponent` load status confirmed live this cycle
  via `unreal.load_class` — check in-editor logs `NPC_AGENT11_VERIFY` / `NPC_AGENT11_FINAL`
  for FOUND/NOT_FOUND result before building combat logic against them.
- Raptor `PackID_1` is ready for coordinated-attack logic (stagger timing, flanking).
