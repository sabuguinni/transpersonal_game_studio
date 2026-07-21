# T-Rex Behavior Specification & NPC Audit — Cycle PROD_CYCLE_AUTO_20260719_003
Agent #11 — NPC Behavior Agent

## Why no new .cpp was written this cycle
Absolute rule `hugo_no_cpp_h_v2` (imp:20): this headless UE5 editor never recompiles C++. Any
`.cpp`/`.h` write via `github_file_write` is 100% inert — it sits in the repo, has zero effect on
the running world, and wastes the cycle. The directive asked for `TRexBehavior.cpp`; instead the
behavior parameters below were applied as **live Actor Tags on the actual T-Rex actors in the
running UE5 world**, which is the only mechanism in this pipeline that has real, verifiable effect
without a recompile. Combat AI Agent (#12) can read these tags directly from Blueprint/Behavior
Tree (`GetActorTags` → parse `PatrolRadius_`, `ChaseRadius_`, `AttackRadius_`) with zero extra
C++ needed.

## Verification results (this cycle)

### 1. `DinosaurCombatAIController.cpp`
`github_file_read` on `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp`
returned content = the literal 9-character string `"undefined"`. This is a dead placeholder file,
not real C++ — confirms the finding from PROD_CYCLE_AUTO_20260713_008 and PROD_CYCLE_AUTO_20260719_001.
It has never compiled and never will in this environment. No further C++ work should be queued
against this path; combat AI logic must live in Blueprint/Behavior Tree assets edited live via
`ue5_execute` python, not in this file.

### 2. `SurvivalComponent.h`
Confirmed present and fully authored (8,112 bytes) at
`Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` — real component definition with
Health/Hunger/Thirst/Stamina/Fear/Temperature stats, drain rates, damage thresholds, biome
integration hooks, and Blueprint-callable API (`ApplyHealthDamage`, `ConsumeFood`, `ConsumeWater`,
`AddFear`, `UpdateBiomeConditions`, etc.). This is dead C++ in the same sense (never recompiled),
but it's a coherent, real design artifact that documents the intended survival data model —
useful as a reference contract for whatever Blueprint equivalent gets built.

### 3. Live world audit (`ue5_execute`)
Queried all level actors: counted `TRex*`/`*Rex*` labeled actors and `Human_Savana*` NPC actors,
recorded their locations, and located `PLAYER0` (read-only, untouched — no position/mobility/
component change per `hugo_mobility_rule_v1`). Applied behavior-parameter Tags to every T-Rex
actor found inside the playable core bounds (X -3000..5000, Y -1000..5500):

- `PatrolRadius_5000`
- `ChaseRadius_3000`
- `AttackRadius_300`

Tags were only added where missing (no duplicate tags on re-run), respecting the
`REUSE FIRST` rule — no new actors spawned, only existing T-Rex actors updated in place.

## T-Rex behavior spec (sociology, not code)

The T-Rex is a solitary apex predator, not a scripted boss. Its "life" independent of the player:

- **Patrol (0–5000u from home point):** wanders its territory at a slow gait, sniffing/scanning,
  occasionally vocalizing. No player awareness required — this state runs whether or not the
  player exists.
- **Alert/Chase (player within 3000u):** if the player is within chase radius AND the T-Rex has
  line of sight or recent scent trail, it commits to pursuit at sprint speed. Chase persists even
  around obstacles for a limited memory window (it doesn't teleport-track the player forever —
  losing line of sight for >8–10s should decay pursuit back toward patrol).
  This models realistic predator persistence, not omniscience.
- **Attack (within 300u):** bite/tail-swipe attack pattern, favors flanking over head-on charges
  when the target is mobile. Should NOT feel like it "waits" for the player — territorial defense
  first, hunger-driven pursuit second, exactly the ordering a real ambush predator would follow.

This spec is intentionally engine-agnostic: whoever (Combat & Enemy AI Agent #12, or a future
Blueprint pass) implements it should read the Tag-encoded radii directly off the actor rather than
hardcoding constants, so tuning stays data-driven and inspectable in the live world.

## NPC (Human_Savana) note
Human NPC audit and idle/pose work already handled by Animation Agent #10 in the immediately
preceding cycle (idle poses applied via `ue5_execute`). No overlapping changes made here — Agent
#11 scope this cycle stayed on dinosaur behavior per directive, leaving human daily-routine /
memory-system design for a dedicated future cycle once Combat AI (#12) has consumed the T-Rex tags.

## Handoff to Combat & Enemy AI Agent (#12)
- T-Rex actors in the playable core now carry `PatrolRadius_5000`, `ChaseRadius_3000`,
  `AttackRadius_300` tags — read these instead of hardcoding constants in Behavior Tree.
- `DinosaurCombatAIController.cpp` is confirmed dead — do not extend it via github_file_write;
  build/extend the AI Controller logic live in the UE5 editor (Blueprint or Python-driven BT
  assets) via `ue5_execute`.
- `SurvivalComponent.h` data model (Fear stat especially) is relevant for tuning T-Rex aggression
  thresholds against player fear/exposure state, if that hook exists on the player side.
