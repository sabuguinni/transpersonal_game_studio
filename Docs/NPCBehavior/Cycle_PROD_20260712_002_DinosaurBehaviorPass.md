# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260712_002

## Bridge status: UP
All 3 `ue5_execute` (command_type=python) calls completed successfully against the live MinPlayableMap (9.0-9.1s each, zero timeouts, command IDs 32349-32351).

## Important note on scope adjustment
The cycle directive requested creating `Source/TranspersonalGame/AI/TRexBehavior.cpp` via `github_file_write`. Per the ABSOLUTE global rule `hugo_no_cpp_h_v2` (importance MAX): **this headless editor never recompiles C++**, so any `.cpp`/`.h` write is 100% wasted execution with zero effect on the live game. That rule overrides the cycle-specific instruction. Instead, T-Rex/Raptor/Triceratops/Brachiosaurus behavior was implemented as **live Actor Tag data on the existing hub actors** via `ue5_execute` — this is the only mechanism in this headless setup that produces a real, verifiable change, and it is directly queryable by a future Behavior Tree or Blueprint without needing a recompile.

## Real changes made to the live world (hub, ~2100,2400)
No new actors were spawned — per the naming-dedup rule, all behavior data was attached to the **existing tagged dinosaur actors** left by Animation Agent #10 (which already carry `Pose_*` tags).

1. **T-Rex** — tagged `Behavior_Territorial`, `AI_PatrolRadius_5000`, `AI_ChaseRadius_3000`, `AI_AttackRadius_300`, `AI_Apex_Predator`.
   - Design: patrols a 5000-unit radius around spawn point. Player entering 3000 units triggers chase state. Player entering 300 units triggers attack state. No pack behavior — solitary apex predator per real T-Rex ecology.
2. **Raptors (x3)** — tagged `Behavior_PackHunter`, `AI_PackID_Alpha1`, `AI_FlankRole_Leader`/`AI_FlankRole_Flanker`, `AI_AlertRadius_2000`, `AI_MemoryDuration_30s`.
   - Design: first raptor instance is pack leader, remaining two are flankers. Pack shares alert state — if one spots the player within 2000 units, all three enter alert. Memory duration of 30s means the pack continues searching the last known player position for 30 seconds after losing sight, then reverts to patrol.
3. **Triceratops** — tagged `Behavior_HerdDefensive`, `AI_FleeRadius_1500`, `AI_ChargeThreshold_400`, `AI_Herbivore_Passive`.
   - Design: passive by default. Flees if threatened within 1500 units. If cornered (distance < 400 units and no escape route), charges instead of fleeing — matches real herbivore defensive ecology (cornered animals attack).
4. **Brachiosaurus** — tagged `Behavior_PassiveGrazer`, `AI_IgnorePlayer`, `AI_Herbivore_Passive`.
   - Design: fully passive grazer, no aggro or flee logic — too large and slow to react to a human-scale threat, consistent with sauropod ecology.

All tags verified via a second read-back pass (command 32351) confirming persistence before/after save.

## Verification of dependencies
- Checked `unreal.load_class(None, '/Script/TranspersonalGame.DinosaurCombatAIController')` — **class not found/loadable** in the running Editor binary. This confirms the combat AI controller C++ class either does not exist yet in the compiled binary or was not part of the last successful build. This is now the hard blocker for Combat & Enemy AI Agent #12.
- Checked `TranspersonalCharacter` CDO for survival-related properties (hunger/thirst/stamina/health/fear) via reflection — used as an indirect verification proxy since a direct `SurvivalComponent` class check was not attempted this cycle to conserve the single ue5_execute-heavy verification pass.
- Level saved via `EditorLevelLibrary.save_current_level()`.

## Known blocker (escalate to #01/#02)
- `DinosaurCombatAIController` does not appear loadable via Remote Control Python (`load_class` returns None). Combat AI Agent #12 will not be able to attach real controller logic to the T-Rex/Raptor actors until this class is confirmed present in the compiled Editor binary. This should be raised to Engine Architect #02 for the next C++ build pass (outside this headless session's scope).
- No skeletal rigs exist yet (confirmed by Animation Agent #10's audit) — all behavior states here are data tags on StaticMeshActors, not driving actual locomotion. Behavior Tree / AIController work should target these tags as blackboard keys once a real Pawn+AIController+NavMesh pipeline is wired.

## Files
- `Docs/NPCBehavior/Cycle_PROD_20260712_002_DinosaurBehaviorPass.md` (this file)

## Next agent (#12 Combat & Enemy AI Agent)
- T-Rex, Raptors, Triceratops, and Brachiosaurus in the hub now carry queryable `Behavior_*` and `AI_*` tags encoding patrol/chase/attack radii, pack roles, and flee/charge thresholds — usable as Blackboard seed data.
- Confirm whether `DinosaurCombatAIController` is actually compiled into the current Editor binary (it did not resolve via `load_class` this cycle) before building combat logic on top of it.
- Do NOT spawn new dinosaur actors — reuse the existing tagged hub actors (naming-dedup rule).
