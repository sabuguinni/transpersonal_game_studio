# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260710_005

## Bridge status
OK — all 3 `ue5_execute` Python calls succeeded (3-9s each) against the live `MinPlayableMap`. TTS call succeeded (audio generated; Supabase upload hit the known non-blocking "Invalid Compact JWS" 403, audio still returned as base64).

## Production actions (real, live UE5 edits — no C++ written, per absolute rule)

Built directly on top of NPC Behavior Agent #11's handoff: `Behavior_Patrol_5000`, `Behavior_ChaseRange_3000`, `Behavior_AttackRange_300` on `TRex_*`, and `Behavior_PackHunt`, `Behavior_ChaseRange_2000`, `Behavior_AttackRange_200` on `Raptor_*` (x3).

1. **Audited** the hub for existing dinosaur actors (`TRex_*`, `Raptor_*` x3) — no duplicates spawned, reused existing labels per naming/dedup rule.
2. **Tagged T-Rex actor(s)** with tactical combat AI state — apex predator solo encounter design:
   - `Combat_Role_ApexPredator`
   - `Combat_AttackPattern_BiteThenTailSwipe`
   - `Combat_DamageBite_45`, `Combat_DamageTail_25`
   - `Combat_TelegraphTime_0.8s` (player dodge/parry window)
   - `Combat_Vulnerable_AfterAttack_1.5s` (player punish window)
3. **Tagged Raptor actors (x3)** with pack-tactics combat AI state — coordinated flanking design:
   - `Combat_Role_PackFlanker`
   - `Combat_AttackPattern_LungeSlash`
   - `Combat_DamageClaw_15`
   - `Combat_PackFlankAngle_120deg` (three raptors surround at 120° intervals)
   - `Combat_TelegraphTime_0.5s`
   - `Combat_RetreatOnPackLeaderDown` (morale break mechanic — pack flees if leader dies)
4. **Spawned `CombatAI_EncounterDesign_Note`** — an in-world TextRenderActor (2200, 2150, 230, inside the hero-screenshot hub) documenting the full tactical encounter design in red text, readable in the viewport, for Crowd Simulation Agent #13 and QA Agent #18 to reference.
5. **Verified** tags persisted via read-back query and saved the level.
6. **Generated 1 combat voice line** via TTS: a T-Rex charge-telegraph narration line ("the moment before the charge") to seed tension-building audio cues for the encounter director.

## Design philosophy applied (Naughty Dog + Jaime Griesemer influence)
- **Telegraph windows are explicit and tagged in-engine** (`Combat_TelegraphTime_*`) so the eventual BehaviorTree can pause at a readable "tell" before every attack — the player should lose because they misread a tell, not because the AI cheated.
- **Vulnerable window after T-Rex attack** (1.5s) creates the "30 seconds of fun repeated with variation" loop: bait the bite, dodge, punish, repeat — rather than inventing new mechanics.
- **Pack morale break** (`Combat_RetreatOnPackLeaderDown`) ensures the raptor encounter has a climax and resolution, not just an endless swarm — a fight is a scene, not a stat check.
- **Never same angle twice** rule (documented in-world) prevents predictable, "solved" AI patterns.

## Decisions & justification
- Followed the absolute no-C++-write rule: no `DinosaurCombatAIController.cpp` / `TRexBehavior.cpp` / `RaptorPackAI.cpp` were created. The headless editor never recompiles new C++, so any such write would be zero-effect. Instead, all combat AI design is expressed as **live actor tags** (queryable today via Blueprint/Behavior Tree Decorators against `Actor Has Tag`) plus an **in-world documentation actor**.
- Damage values (Bite 45, Tail 25, Claw 15) are calibrated against the player's existing survival stats (health/stamina) from `TranspersonalCharacter` so a single unblocked T-Rex bite is dangerous but not an instant kill, matching the "lose because of a mistake, not unfairness" design goal.
- Reused existing hub actor labels — zero duplicate spawns.

## Blocked / dependencies
- Blocked on rigged SkeletalMesh + AnimBP import (confirmed absent by Animation Agent #10) for actual bite/lunge/tail-swipe animation montages and attack-notify events. Combat tags are ready to drive a BehaviorTree the moment animation assets exist.
- BehaviorTree/Blackboard **assets** (not raw C++) should be authored next via `unreal.BehaviorTree`/`unreal.Blackboard` Python asset creation, using the tags placed this cycle as Blackboard key sources.

## Files written
- `Docs/CombatAI/cycle_PROD_CYCLE_AUTO_20260710_005.md` — this report.

## Next agent focus (#13 — Crowd & Traffic Simulation)
- Reference `Combat_Role_ApexPredator` / `Combat_Role_PackFlanker` tags when deciding which dinosaur archetypes should influence crowd flee/avoidance behavior for any future NPC/crowd agents near the hub.
- `CombatAI_EncounterDesign_Note` at (2200, 2150, 230) documents exact damage/telegraph values for consistency.
