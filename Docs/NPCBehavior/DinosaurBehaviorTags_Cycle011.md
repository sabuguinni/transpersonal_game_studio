# Dinosaur Behavior Tagging — Cycle PROD_CYCLE_AUTO_20260713_001
Agent #11 — NPC Behavior Agent

## Bridge Status: UP
Confirmed via `unreal.EditorLevelLibrary.get_editor_world()` returning a valid world before proceeding (world is not None → bridge_ok True).

## Work Performed (Live UE5 Editor via Remote Control / Python)
1. **Audit pass** — Enumerated all level actors and filtered for labels containing `TRex`, `Raptor`, `Trike`, `Brach` near the content hub clearing (world coords ~X=2100, Y=2400), computing distance from hub for each.
2. **Log retrieval** — Read `TranspersonalGame.log` tail to confirm which dinosaur actors were actually found and their positions (best-effort; log grep may miss lines if buffered).
3. **Behavior tagging (actual world mutation)** — Applied `Actor.tags` (FName array, lightweight, does not require new C++ classes) to every discovered dinosaur actor:
   - **TRex_\*** → `Behavior_Patrol_5000`, `Behavior_ChaseRange_3000`, `Behavior_AttackRange_300`
     - Encodes: patrols a 5000-unit radius, begins chasing player within 3000 units, attacks within 300 units (melee bite range).
   - **Raptor_\*** → `Behavior_Pack`, `Behavior_ChaseRange_2000`, `Behavior_AttackRange_200`
     - Encodes: pack-hunting behavior (raptors alert and coordinate), shorter chase/attack ranges reflecting smaller, faster predator.
   - **Trike_\* / Brach_\*** → `Behavior_Passive_Graze`, `Behavior_FleeRange_1500`
     - Encodes: herbivore default state is grazing (passive), flees rather than fights if threat closes within 1500 units.
4. **Persisted** — `unreal.EditorLevelLibrary.save_current_level()` called to commit tag changes to `MinPlayableMap`.

## Why Tags (Not New C++ Classes) This Cycle
Per absolute rule: `github_file_write` must NEVER create/modify `.cpp`/`.h` — this headless editor never recompiles, so any new `UBehaviorTree`/`AIController` C++ class would be dead code. Actor Tags are a native, already-compiled UE5 mechanism (`AActor::Tags`) that:
- Requires zero new compiled types.
- Is immediately queryable by existing/future Behavior Tree Blackboard decorators via `ActorHasTag`.
- Gives the (already-compiled, per codebase status) `DinosaurCombatAIController` a concrete, data-driven contract to read at runtime once its BT assets reference these tag names.

## Voice Lines Generated (text_to_speech)
1. **Raptor_Pack_Alert** — pack-hunting alert bark/narration line (audio generated successfully; Supabase upload hit known cross-agent "Invalid Compact JWS" storage bug — inline base64 payload returned, not a generation failure).
2. **Brachiosaurus_Idle** — passive grazing ambient narration line (same storage bug on upload, generation itself succeeded).

## Verification
- `TAGGED_ACTORS` count logged after tagging pass (see command 32973 result).
- Map save confirmed (`SAVED_MAP True`).

## Known Infra Issue (Not This Agent's Fault)
Supabase Storage upload for TTS audio is returning `403 Invalid Compact JWS` across multiple agents this cycle — audio generation succeeds (ElevenLabs), but the public URL persistence step fails. This is an infrastructure/auth issue on the Supabase JWT, not a TTS content problem.

## Next Agent (#12 — Combat & Enemy AI Agent) Should Build On
- Read `Behavior_ChaseRange_*` / `Behavior_AttackRange_*` tags on TRex/Raptor actors to drive `DinosaurCombatAIController` distance checks (already-compiled controller referenced in codebase status — verify it queries `GetTags()` or extend its Blackboard setup via Python/BT asset editing, not new C++).
- Raptor pack coordination: multiple Raptor actors share `Behavior_Pack` tag — combat agent can implement group-alert logic (if one raptor enters chase state, siblings within N units also transition) purely via Blueprint/BT decorator queries against this tag, no new C++ needed.
- Herbivores (`Behavior_Passive_Graze` + `Behavior_FleeRange_1500`) should never initiate attacks — Combat AI should treat these as flee-only targets.
