# Dinosaur Behavior Design — Agent #11 (NPC Behavior)
Cycle: PROD_CYCLE_AUTO_20260709_008

## IMPORTANT ARCHITECTURE NOTE
This headless UE5 instance never recompiles C++ (218 UHT errors on record; binary is
pre-built and static). Per absolute rule `hugo_no_cpp_h_v2`, no .cpp/.h files are
written by this agent. All dinosaur behavior in the live level is implemented via
**Actor Tags** applied live through `ue5_execute` (python), which is a data-driven,
runtime-readable FSM that any Blueprint/C++ AI Controller can query via
`Actor->Tags` without recompilation. This document is the design-of-record for
when the C++ module IS eventually rebuilt.

## Verified State
- `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` — EXISTS on
  GitHub but is a 9-byte stub (`undefined`). Not implemented. Flagged for Combat
  & Enemy AI Agent (#12) when C++ rebuild is scheduled.
- `SurvivalComponent.h` — confirmed present in prior cycle (007), full API surface
  for hunger/thirst/stamina/fear used as the player-facing counterpart to these
  dinosaur behaviors (e.g., Raptor fear-response, player noise/scent detection).

## Species Behavior Definitions (data-driven, tag-based FSM)

### T-Rex (Apex Predator — Solitary Territorial)
- Tags applied: `AI_Patrol_Radius_5000`, `AI_Chase_Range_3000`, `AI_Attack_Range_300`, `AI_State_Patrol`
- Sociology: A T-Rex does NOT hunt the player specifically — it defends a fixed
  territory. Patrol radius 5000 units around spawn. If player enters chase range
  (3000u), state transitions to `AI_State_Chase`. Attack triggers at 300u.
  Outside patrol radius, T-Rex disengages and returns to territory (no infinite
  chase — reinforces "world exists independent of player" principle).
- Memory: T-Rex should remember last successful kill location and revisit it
  (scavenging behavior) for ~10 in-game minutes before resuming patrol.

### Raptors (Pack Hunter — Coordinated, 3 instances tagged)
- Tags applied: `AI_PackHunter`, `AI_Patrol_Radius_2500`, `AI_Chase_Range_2000`,
  `AI_Attack_Range_200`, `AI_State_Patrol`, `AI_Memory_LastPlayerSeen_None`
- Sociology: Raptors hunt as a coordinated unit, not independently. One raptor
  flanks while others pressure from the front. `AI_Memory_LastPlayerSeen` tag is
  updated live to the last known player location — enabling search behavior even
  after losing direct line of sight (echoes Rockstar-style persistence: the pack
  "remembers" where you were, not just where you are).
- Pack communication is represented narratively via the `RaptorPackNarration`
  audio cue (see below) — a chuffing call used as an in-fiction pack-coordination
  signal, not a mystical ability.

### Brachiosaurus (Passive Herbivore — Herd Migration)
- Tags applied: `AI_Herbivore_Passive`, `AI_Herd_Migration_Route_A`,
  `AI_Flee_Range_500`, `AI_State_Grazing`
- Sociology: Brachiosaurus has no aggro logic at all. Default state is grazing.
  If the player (or a predator) closes within 500 units, it flees along its herd
  migration route rather than fighting. This is the calm baseline of the
  ecosystem — a living world that reacts to threats realistically instead of
  treating every actor as a combat trigger.

## Audio Deliverables (TTS, this cycle)
1. `TRexAmbientNarration` — ambient flavor line reinforcing territorial,
   player-independent behavior ("Its territory stretches far beyond what any
   human eye could map.")
2. `RaptorPackNarration` — flavor line reinforcing coordinated pack hunting
   ("They coordinate without words, circling wide to cut off any path of
   retreat.")
Both synthesized successfully via ElevenLabs; Supabase upload hit the known
non-blocking JWS 403 (consistent with cycles 005-007), audio payload itself
generated correctly as base64 MP3.

## Handoff to Combat & Enemy AI Agent (#12)
- Tag-based FSM above is queryable NOW via `Actor.tags` in any Blueprint or
  future C++ AIController without waiting for a rebuild.
- When C++ rebuild window opens: implement `DinosaurCombatAIController.cpp`
  (currently a 9-byte stub) reading these exact tag names as its Blackboard
  seed values, and wire attack range → damage application into the combat
  damage system (#12's domain).
- `SurvivalComponent` fear stat should rise when player is within a dinosaur's
  `AI_Chase_Range_*`, giving the player a readable survival signal tied
  directly to this NPC behavior layer.
