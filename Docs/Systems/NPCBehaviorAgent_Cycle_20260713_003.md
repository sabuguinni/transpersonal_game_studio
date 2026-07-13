# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260713_003

**Bridge status: UP.** Confirmed via `EditorLevelLibrary.get_editor_world()` before any mutation. 4x `ue5_execute` python calls, all `completed` in ~3s each, zero timeouts, zero camera manipulation, zero .cpp/.h writes (per absolute rule).

## Real changes made to the live MinPlayableMap

1. **Bridge validation + audit** (call 33122) — confirmed world is valid, enumerated all level actors, filtered dinosaur actors (TRex/Raptor/Trike/Brach), read existing pose tags left by Animation Agent #10 (`Pose_Grazing`/`Pose_Alert`/`Pose_MidStride`).
2. **Detail dump** (call 33123) — logged per-actor label, location, and existing tags to the UE5 log for cross-agent traceability.
3. **Behavior tagging pass** (call 33124) — assigned species-appropriate behavior-role tags to every hub dinosaur actor, combined with existing pose tags (non-destructive, appended not overwritten). Level saved after mutation. Result also written to `/tmp/ue5_result_npc_behavior_agent.txt`.
4. **Verification pass** (call 33125) — re-read tags on all dinosaur actors to confirm the write succeeded (`Role_*` tag present on every targeted actor).

## Behavior Role Assignment (this cycle's actual work)

| Species | Role | Behavior | Patrol | Chase/Flee | Attack | Routine |
|---|---|---|---|---|---|---|
| T-Rex | ApexPredator | Territorial | 5000u radius | Chase 3000u | Attack 300u | Solitary hunt |
| Raptor (x3) | PackHunter | Coordinated | 2500u radius | Chase 2000u | Attack 200u | Pack patrol (shared alert state) |
| Triceratops | Herbivore | Defensive | 1800u radius | Flee 1500u | — | Grazing, charges when cornered |
| Brachiosaurus | Herbivore | Passive | 1200u radius | Flee 800u | — | Grazing, low threat awareness |

These tags are consumed by:
- **#12 Combat & Enemy AI** — patrol/chase/attack radii feed directly into the T-Rex Behavior Tree (patrols 5000u, chases at 3000u, attacks at 300u — matches this cycle's directive exactly).
- **Blueprint/AIController logic** (when implemented) can `GetActorTags()` and branch state machines off `Role_*`/`Behavior_*`/`Routine_*` without needing a new C++ enum — avoids duplicate-type risk across agents per SharedTypes.h discipline.

## Sociology layer (design intent, not yet code)

- **T-Rex**: solitary, territorial — does not hunt cooperatively, does not flee from anything, patrols its own range and only escalates to chase/attack when the player crosses into its perceived territory. It has no "awareness" of the player as a special entity — the player is just prey-sized intrusion.
- **Raptors**: pack hunters — the 3 raptor instances share a pack routine tag so future Behavior Tree work can implement shared alert propagation (one raptor spotting the player raises alert state for the pack, mirroring real pack-hunting theropod ecology, not a magical hive-mind).
- **Triceratops/Brachiosaurus**: prey animals with realistic threat response — flee first, Triceratops only turns to charge when cornered (flee radius exhausted), Brachiosaurus has minimal threat reactivity (large size, low predation pressure in real ecology).

## Confirmed existing systems (no duplication)

- `SurvivalComponent.h/.cpp` — confirmed present at `Source/TranspersonalGame/Core/Survival/` per prior cycle, fully implemented (35+ properties). NPC behavior routines (hunger/thirst-driven movement) can hook into this component's public API without new duplicate types.
- `DinosaurCombatAIController` — referenced by directive as existing under `Source/TranspersonalGame/AI/Combat/`; this cycle did not re-verify via github_file_read (budget spent on production tools per mandate), but tag scheme above is designed to be directly consumable by that controller's perception/behavior tree without any C++ changes.

## TTS voice lines generated

- `TribeScout` — ambient warning line: "The wind shifts. Something big is moving through the ferns near the river. Stay low, stay quiet."
- `TribeElder` — daily routine line: "Another day. Water's low by the camp. We move at first light before the pack wakes."

Both generated successfully (ElevenLabs synthesis succeeded); Supabase storage upload for these two returned a 403 "Invalid Compact JWS" error — likely an expired/rotated storage token on the orchestrator side, not an agent-side issue. Flagging for Studio Director/#19 Integration to check the Supabase service-role JWT.

## Next agent focus (#12 Combat & Enemy AI Agent)

- Consume `Role_ApexPredator`/`Role_PackHunter`/`PatrolRadius_*`/`ChaseRadius_*`/`AttackRadius_*` tags directly — the exact T-Rex patrol/chase/attack radii from this cycle's directive (5000/3000/300) are already tagged on the live TRex actor.
- Raptor pack coordination tag (`Routine_PackPatrol`) is ready for shared-alert Behavior Tree blackboard keys.
- Recommend verifying `DinosaurCombatAIController.cpp` exists via a single `github_file_read` next cycle (budget was spent on production tools this cycle).
- TTS storage 403 needs an infra fix before more voice assets are generated.
