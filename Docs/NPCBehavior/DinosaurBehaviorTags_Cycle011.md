# Dinosaur Behavior State Tags — Cycle PROD_CYCLE_AUTO_20260710_011

## Summary
Applied data-driven AI behavior state tags to EXISTING placeholder dinosaur actors in
MinPlayableMap (no new duplicate actors created, per naming/dedup rule). This gives the
Combat & Enemy AI Agent (#12) concrete radius data to wire into Behavior Trees without
touching C++ (headless editor does not recompile C++; all changes done via ue5_execute
Python against live actors and saved with the level).

## Actions Taken (live UE5 editor, MinPlayableMap)
1. Bridge validation confirmed world loaded OK.
2. Audited all actors near hub clearing (~X=2100, Y=2400) matching labels containing
   TRex / Raptor / Trike / Brachio.
3. Applied `Tags` array (FName) to each matched actor — no new actors spawned:
   - **TRex_* actors** → `AI_State_Patrol`, `PatrolRadius_5000`, `ChaseRadius_3000`,
     `AttackRadius_300`
   - **Raptor_* actors** → `AI_State_Patrol`, `PatrolRadius_3500`, `ChaseRadius_2000`,
     `AttackRadius_200`, `PackHunter` (raptors flagged for pack coordination)
   - **Trike_* / Brachio_* actors** → `AI_State_Passive`, `FleeRadius_1500` (herbivores flee
     rather than fight)
4. Saved level (`EditorLevelLibrary.save_current_level()`).
5. Verified tag application by re-scanning all actors for `AI_State` tags.

## Design Rationale (Sociology of the World)
- T-Rex is an apex predator: wide territory (5000u patrol), commits to a chase early
  (3000u) since it has no natural predators and little reason for caution.
- Raptors are pack hunters: smaller individual territory but flagged `PackHunter` so the
  Combat AI Agent can implement coordinated flanking (multiple raptors converging on one
  chase target) rather than independent chases.
- Triceratops/Brachiosaurus are herbivores: passive by default, only react by fleeing
  when a threat (player or predator) enters `FleeRadius_1500`. They do not initiate
  chase or attack behavior — consistent with real herbivore ecology, not scripted heroism.

## NPC Voice Lines Generated (ElevenLabs TTS)
Two tribal NPC barks recorded (base64 MP3 returned; Supabase upload failed with a JWS auth
error unrelated to this agent — audio payloads exist and are reproducible on next TTS call
once storage auth is fixed):
1. **TribeElder_Warning**: "Something's out there in the treeline. Big. Keep your voice
   down and stay near the fire." (~6s) — ambient danger callout tied to nearby predator
   proximity (T-Rex ChaseRadius).
2. **TribeScout_Briefing**: "The pack moves at dawn. Watch the raptors — they never hunt
   alone." (~5s) — reinforces the PackHunter tag lore for raptors, foreshadowing
   coordinated attacks to Combat AI Agent #12.

## Handoff to #12 — Combat & Enemy AI Agent
- Read `Tags` on TRex/Raptor/Trike/Brachio actors in MinPlayableMap to drive
  `DinosaurCombatAIController` Behavior Tree Blackboard keys (PatrolRadius, ChaseRadius,
  AttackRadius, PackHunter flag).
- Herbivores should never enter combat state — only Flee → Idle/Graze loop.
- Raptor pack coordination (shared blackboard or EQS group query) is the key new mechanic
  to implement using the `PackHunter` tag.

## Known Issues
- One verification pass and one retry timed out mid-cycle (bridge momentarily
  unresponsive); tag application itself was confirmed successful in the final verification
  call within this same cycle.
- TTS Supabase storage upload returned 403 (Invalid Compact JWS) — audio generation itself
  succeeded; this is an infra auth issue outside this agent's scope.
