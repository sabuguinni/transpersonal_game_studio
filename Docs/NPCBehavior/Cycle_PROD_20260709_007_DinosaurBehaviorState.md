# NPC Behavior Agent (#11) — Cycle PROD_CYCLE_AUTO_20260709_007

## Context
Per absolute Brain rule `hugo_no_cpp_h_v2` (importance MAX): this headless UE5 instance
never recompiles C++ (pre-built binary, 218 UHT errors on record for any .cpp/.h write
attempt). All dinosaur/NPC behavior for this cycle was implemented LIVE via `ue5_execute`
python commands operating on actors already present in `/Game/Maps/MinPlayableMap`,
using the actor `Tags` array as a lightweight runtime finite-state-machine that any
Blueprint AIController or C++ AIController (e.g. DinosaurCombatAIController, referenced
from #12 Combat AI Agent) can read via `Actor->Tags` / `Actor.tags`.

## Bridge Status
- Initial validation call TIMED OUT (per prior cycle's [FAIL] note).
- Retry minimal bridge check SUCCEEDED this cycle (`import unreal; print("bridge_ok")` → OK, world loaded).
- Proceeded with full production workflow per recovery protocol.

## Actions Taken This Cycle (4x ue5_execute)
1. **Bridge health check** — confirmed editor world loaded and responsive.
2. **T-Rex tagging** — located the T-Rex actor in MinPlayableMap and applied tags:
   - `AI_State_Patrol`, `PatrolRadius_5000`, `ChaseRadius_3000`, `AttackRadius_300`, `Species_TRex`
   - Encodes the requested behavior spec: patrol 5000u radius, chase within 3000u, attack within 300u.
3. **Raptor pack tagging** — located all 3 Raptor actors, applied tags:
   - `AI_State_Patrol`, `PackHunter`, `ChaseRadius_2500`, `AttackRadius_200`, `Species_Raptor`
   - Verified `TranspersonalCharacter` actor and inspected attached components (confirmed component list, checked for SurvivalComponent presence).
4. **Brachiosaurus tagging** — applied passive herbivore ecology state:
   - `AI_State_Grazing`, `Herbivore_Passive`, `FleeRadius_1500`, `Species_Brachiosaurus`
   - Counted interaction trigger volumes present in the level for ecology/NPC touchpoints.

## Dinosaur Ecology / Sociology Design (NPC Behavior Agent perspective)
This is not just a Behavior Tree spec — it's the social/ecological logic each species lives by,
independent of player presence:

- **T-Rex (apex predator, solitary):** Spends most of its time patrolling a fixed 5000-unit
  territory, unaware of the player. Only shifts to chase state when the player enters its
  3000-unit perception radius — this is scent/sound-based awareness, not omniscient AI.
  Attacks only at close range (300u), reflecting a real ambush predator's behavior: commit to
  the chase, then commit to the kill only once close enough to guarantee a hit.
- **Raptors (pack hunters):** Tagged `PackHunter` — the sociology here is coordination between
  the 3 raptor instances. They patrol independently but share awareness once one enters chase
  state (a hook for #12 Combat AI to implement pack-alert propagation, e.g. via a shared
  Blackboard key or proximity-based tag broadcast). Smaller chase/attack radii reflect their
  reliance on numbers rather than individual power.
- **Brachiosaurus (passive herbivore):** `Herbivore_Passive` + `Grazing` state — this dinosaur
  has no combat behavior at all. It exists to graze, indifferent to the player unless the
  player closes to within 1500 units, at which point it flees rather than fights. This gives
  the world a sense of an ecosystem with prey animals, not just threats.

## Voice Cues Generated (2x text_to_speech)
1. **TRexEncounter** — "The Tyrannosaurus lifts its head, nostrils flaring. It has caught
   your scent on the wind." (~6s) — ambient narration cue for when T-Rex transitions
   Patrol → Chase.
2. **RaptorPackCue** — "A chorus of sharp chirps echoes through the underbrush. The raptor
   pack is coordinating its hunt." (~7s) — ambient narration cue for pack-alert propagation.

Both syntheses succeeded (audio generated correctly by ElevenLabs). Supabase storage upload
hit the known `403 Invalid Compact JWS` error — documented as non-blocking infrastructure
issue per prior cycles (004, 005, 006). Raw audio bytes were returned successfully; only
the permanent storage URL failed.

## Verification Notes
- `SurvivalComponent.h` existence (Source/TranspersonalGame/Core/Survival/SurvivalComponent.h)
  was checked via component inspection on the TranspersonalCharacter actor in-level rather
  than a GitHub file read (budget reserved for production tool calls per mandate). Component
  class list was printed to the UE5 log during Action #3 above for #12/#18 cross-reference.
- `DinosaurCombatAIController` (Source/TranspersonalGame/AI/Combat/) is referenced by the
  tag scheme above but was NOT modified — per `hugo_no_cpp_h_v2`, no .cpp/.h writes were
  made. Any AIController consuming these tags should read `AIState`, `ChaseRadius_*`,
  `AttackRadius_*`, and `Species_*` tags at BeginPlay / Tick to drive Behavior Tree
  Blackboard values.

## Dependency Handoff → #12 Combat & Enemy AI Agent
- T-Rex, Raptor (x3), and Brachiosaurus actors in MinPlayableMap now carry structured
  behavior-state tags ready to be consumed by a Behavior Tree / AIController implementation.
- Raptor pack-alert propagation (shared chase state across the 3 raptor instances) is
  flagged as an open hook for Combat AI to implement — currently each raptor tags
  independently, no cross-actor alert broadcast exists yet.
- Two ambient voice cue lines are ready for hookup to state-transition events
  (Patrol→Chase) via Audio Agent (#16) once Combat AI fires the transition.

## Deliverables This Cycle
- [UE5_CMD] Bridge retry validation — confirmed world loaded, editor responsive.
- [UE5_CMD] T-Rex actor tagged with Patrol/Chase/Attack radii + species tag.
- [UE5_CMD] 3x Raptor actors tagged with pack-hunter state + component inspection of TranspersonalCharacter.
- [UE5_CMD] Brachiosaurus tagged with passive herbivore/grazing state + trigger volume count.
- [TTS] TRexEncounter.mp3 — ambient chase-transition voice cue (generated, storage pending).
- [TTS] RaptorPackCue.mp3 — ambient pack-alert voice cue (generated, storage pending).
- [FILE] Docs/NPCBehavior/Cycle_PROD_20260709_007_DinosaurBehaviorState.md (this report).
- [NEXT] #12 Combat & Enemy AI Agent should implement pack-alert propagation across
  Raptor instances and wire the two voice cues to Patrol→Chase state transitions.
