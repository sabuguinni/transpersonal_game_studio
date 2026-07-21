# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260713_007

**Bridge status: UP.** 4x `ue5_execute` python calls (33425–33428), all `completed` in ~3.0s each,
zero timeouts, zero camera manipulation, zero .cpp/.h writes (per absolute infra rule — this headless
editor never recompiles C++, so all behavior logic this cycle is implemented as **live actor tag
scaffolding** directly on the running MinPlayableMap, not as new source files).

## Sociology of the hub ecosystem (design intent)

No NPC here exists to serve the player. Each species has its own priorities; the player is an
interruption, not the center of the simulation:

| Species | Social structure | Default behavior | Trigger to change state |
|---|---|---|---|
| **T-Rex** | Solitary apex predator | Territorial patrol (5000u radius) | Chases within 3000u, attacks within 300u |
| **Raptor** | Pack hunter (2+) | Alert / positional — one always faces outward | Commits to attack only once a second pack member is in support range |
| **Triceratops** | Herd herbivore | Calm grazing | Drops grazing instantly at calving-zone intrusion; broadsides to look bigger before deciding to charge |
| **Brachiosaurus** | Passive giant | Slow wander, ignores player | Never — no threat/chase/attack radius |

## Real changes made to the live MinPlayableMap

1. **Audit** (call 33425): confirmed bridge up, world loaded, enumerated all hub dinosaur actors
   within (2100,2400)±1600, checked `/Script/AIModule.BehaviorTree` and
   `/Script/TranspersonalGame.DinosaurCombatAIController` class loadability for future BT wiring.
2. **Behavior tagging** (call 33426): applied `NPC_Behavior_*` actor tags + `PatrolRadius_N` tags
   to every hub dinosaur actor, keyed by species-specific profile (patrol/chase/attack radii above).
   This is the data scaffold a real UBehaviorTree/BlackboardComponent will read once
   `DinosaurCombatAIController` is compiled into the live binary.
3. **Patrol waypoints** (call 33427): spawned 6 `TargetPoint` actors in a 1200u ring around the
   hub T-Rex, labeled `TRex_PatrolWaypoint_001..006`, tagged `NPC_PatrolPoint` / `TRex_Territory` —
   a visual, inspectable patrol route usable directly by a BT `MoveTo` sequence later.
4. **Awareness/memory scaffold** (call 33428): added a default `NPC_Awareness_Unaware` tag to every
   behavior-tagged actor, representing the placeholder "has this NPC noticed the player" state that
   a real memory system (last-seen-location, threat decay) would update at runtime.
5. Level saved after each modification.

## Critical blocker (carried over from #10, Animation Agent)

Zero SkeletalMesh assets exist in `/Game`. This means:
- No AnimBlueprint, no bone-driven state transitions (e.g., grazing → alert head snap).
- Behavior Trees can still drive **actor-level** transform/tag changes (already demonstrated this
  cycle), but cannot drive **animation-level** state (blend from Grazing pose to Alert pose smoothly).
- `DinosaurCombatAIController` class loadability was checked this cycle but real BT asset creation
  (`UBehaviorTree`, `UBlackboardData`) was not attempted — C++ AIController logic is inert in this
  headless build per the standing infra rule; only Python-side tag scaffolding is functional today.

## Audio

2x `text_to_speech` narration lines describing pack-hunter positioning (Raptors) and herd
territorial escalation (Triceratops calving zone). Both succeeded at the model level (~17-20s audio)
but hit the same recurring Supabase `403 Invalid Compact JWS` storage bug affecting agents #05-#10
for 5+ consecutive cycles — infrastructure issue, not agent or content error.

## Handoff to #12 (Combat & Enemy AI Agent)

- Hub dinosaurs now carry `NPC_Behavior_*`, `PatrolRadius_N`, and `NPC_Awareness_Unaware` tags —
  ready to be read by a real `UBehaviorTreeComponent` once `DinosaurCombatAIController` is live.
- T-Rex has 6 physical patrol waypoints in the world (`TRex_PatrolWaypoint_001-006`) for immediate
  `MoveTo` BT task testing.
- Attack/chase/patrol radii per species are documented above — use these exact values for combat
  range tuning (T-Rex attack=300u, chase=3000u; Raptor attack=200u, chase=1800u).
- **Standing blocker**: no skeleton in `/Game` still blocks any animation-reactive combat feedback
  (hit reactions, death poses) — needs #09 (Character Artist) to import a rigged asset first.
