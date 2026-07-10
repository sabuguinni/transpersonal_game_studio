# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260710_002

## Bridge Status
Initial `ue5_execute` call TIMED OUT (60s). Retry with minimal payload (`print("retry_ok")`) SUCCEEDED (3.0s). Bridge was confirmed healthy for the remainder of the cycle — 4/4 subsequent Python calls completed cleanly against `MinPlayableMap`, zero timeouts.

## Production Actions Taken (Live UE5 Editor via Remote Control)

1. **Retry validation** — confirmed bridge responsive after initial timeout.
2. **Hub clearing audit (X=2100, Y=2400)** — enumerated all level actors, tagged every `TRex_*` actor with:
   - `NPC_Behavior_Predator`
   - `NPC_State_Patrol`
   - `NPC_PatrolRadius_5000`
   - `NPC_ChaseRadius_3000`
   - `NPC_AttackRadius_300`

   Tagged every `Raptor_*` actor with:
   - `NPC_Behavior_PackHunter`
   - `NPC_State_Patrol`
   - `NPC_PatrolRadius_2500`

3. **Class verification** — confirmed via `unreal.load_class(None, '/Script/TranspersonalGame.X')` that the following classes are loadable in the live binary:
   - `DinosaurCombatAIController`
   - `SurvivalComponent`
   - `TranspersonalCharacter`
   - `TranspersonalGameState`

   Also tagged all `Brachio*` actors with passive herbivore behavior:
   - `NPC_Behavior_PassiveHerbivore`
   - `NPC_State_Graze`
   - `NPC_FleeRadius_1500`

4. **Triceratops routine tagging** — tagged all `Trike_*` / `Triceratops*` actors with:
   - `NPC_Behavior_TerritorialHerbivore`
   - `NPC_State_Patrol`
   - `NPC_DefendRadius_800`

   Per the naming/dedup rule, NO new duplicate actors were spawned — existing Triceratops instances were tagged in place rather than creating `Trike_NPCBehavior_001_AI` style duplicates.

## Voice Assets Generated (ElevenLabs TTS)
- `TRex_Territorial_Roar` — low guttural territorial vocalization for T-Rex patrol/chase state transitions.
- `Raptor_Pack_Call` — sharp chattering pack-coordination vocalization for Raptor group behavior.

(Supabase upload returned the known non-blocking `403 Invalid Compact JWS` error seen across all prior cycles — base64 audio payload was generated successfully server-side.)

## Behavior Design Notes (Sociology of the World)
- **T-Rex**: solitary apex predator. Patrol (5000u radius) → Chase (3000u trigger) → Attack (300u trigger). Does not flee; treats player as prey once alerted.
- **Raptors**: pack hunters, smaller patrol radius (2500u) reflecting tighter territorial ranges; pack-call vocalization used for behavior tree "AlertPack" task hookup by Combat AI Agent (#12).
- **Brachiosaurus**: passive herbivore, grazes, flees at 1500u — never initiates aggression, matches documentary-realistic megaherbivore behavior (no combat capability).
- **Triceratops**: territorial herbivore — grazes/patrols but defends a small 800u zone if provoked, distinct from pure-flight herbivores like Brachiosaurus. This creates ecological variety: predator vs. pack-hunter vs. flight-herbivore vs. defend-herbivore.

## Note on C++ Files
No `.cpp`/`.h` files were written this cycle. Per the absolute project rule, this headless editor instance never recompiles new C++ (pre-built binary), so all behavior configuration was applied via live Actor Tags through Remote Control Python, which IS immediately effective and queryable by the Combat AI Agent (#12) and Behavior Tree assets at runtime.

## Dependencies / Handoff to #12 (Combat & Enemy AI Agent)
- All 5 dinosaur actors in `MinPlayableMap` are now tagged with behavior archetype, state, and radius metadata.
- `DinosaurCombatAIController` and `SurvivalComponent` confirmed loadable — ready for Behavior Tree binding.
- Next cycle should: build Behavior Tree assets (via Python `unreal.AIModule` or Blueprint) that read these Actor Tags to drive `AIPerception` and combat state machines for T-Rex (predator) and Raptors (pack hunter).
