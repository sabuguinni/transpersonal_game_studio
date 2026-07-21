# NPC/Dinosaur Behavior Agent — Cycle PROD_CYCLE_AUTO_20260711_002

## Status: Bridge OPERATIONAL (recovered from previous cycle's timeout)

## Actions Taken (Live UE5 Editor, MinPlayableMap)

1. **Bridge validation** — confirmed `EditorLevelLibrary.get_editor_world()` returns valid world.
2. **Dinosaur actor audit** — scanned all level actors, located existing TRex/Raptor actors near hub clearing (no duplicates created, per naming/dedup rule).
3. **Behavior tags applied** (actor `tags` array, used as lightweight runtime AI state until Blueprint Behavior Tree assets are authored):
   - `AI_Patrol` — base patrol flag, all TRex/Raptor actors.
   - `PatrolRadius_5000` (TRex) — patrols a 5000-unit radius area around spawn.
   - `AggroRange_3000` / `AttackRange_300` (TRex) — matches directive: chase player within 3000 units, attack within 300 units.
   - `AggroRange_2000` / `AttackRange_200` / `PackHunter` (Raptor) — raptors have shorter aggro/attack range but hunt in packs.
   - `State_Patrolling` — initial behavior state (idle/patrol loop).
   - `Memory_LastSeenPlayer_None` — placeholder for NPC memory system (tracks last known player position; currently unset).
4. **Verification pass** — re-read all TRex/Raptor/Trike/Brachiosaurus actors and logged their tag sets + locations to the UE5 output log for cross-agent visibility (Combat AI Agent #12 can query these tags via `AActor.tags`).

## Design Notes (Sociology of Dinosaur "NPCs")

Following the transpersonal-studio brief reframed for this project's real theme (prehistoric survival, no mysticism):
- T-Rex is a **solitary apex predator** — wide territory (5000u patrol), no pack tag, aggressive aggro distance (3000u) reflecting poor stealth tolerance of large prey.
- Raptors are **pack hunters** — smaller individual territory but `PackHunter` tag signals to Combat AI (#12) that raptor aggro should trigger nearby pack members (coordinated behavior), consistent with real pack-hunting theropod ecology.
- `Memory_LastSeenPlayer` tag is a stub for a future NPC memory component — dinosaurs should not have omniscient player tracking; this tag is the hook for Combat AI to update/clear based on line-of-sight.

## Voice Lines Generated (TTS)
Two human-tribe NPC dialogue lines synthesized via ElevenLabs (audio returned as base64; Supabase storage upload failed with a 403/Invalid JWT error — infra issue, not a content issue):
1. **TribeScout**: "Something moved out there. Big. Everyone stay low and don't make a sound." — reaction to dinosaur sighting (survival tension, no player-service framing).
2. **TribeElder**: "The fire's dying. Someone needs to gather more wood before nightfall, or we won't last till morning." — daily routine/needs-driven dialogue, NPC has own concerns independent of player.

## Known Issues
- Supabase storage upload for TTS audio failed (`Invalid Compact JWS` — auth token issue on the storage backend). Audio was generated successfully by ElevenLabs; only the persistence step failed. This is an infra/ops issue for Hugo, not a content/agent error.
- Actor `tags` are a temporary carrier for AI state. A proper Behavior Tree / Blackboard implementation requires C++ compilation (blocked per `hugo_no_cpp_h_v2` rule — this headless editor never recompiles new C++). Recommend Combat AI Agent (#12) implement AIController-driven Behavior Trees using Blueprint-only assets (no C++) if AI logic beyond tags is needed, OR flag to Hugo that a real engine rebuild is needed to activate `DinosaurCombatAIController.cpp`.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Bridge validation — confirmed world loaded.
- [UE5_CMD] Tagged all TRex/Raptor actors in hub clearing with patrol radius, aggro range, attack range, pack-hunter flag.
- [UE5_CMD] Applied initial behavior state (`State_Patrolling`) and memory stub tag to all tagged dinosaurs.
- [UE5_CMD] Verification pass — logged full tag/location report for TRex/Raptor/Trike/Brachiosaurus actors to UE5 output log.
- [TTS] TribeScout voice line — dinosaur sighting reaction.
- [TTS] TribeElder voice line — daily routine dialogue (firewood).
- [NEXT] Combat & Enemy AI Agent (#12): read the `AggroRange_*`, `AttackRange_*`, `PackHunter`, `PatrolRadius_*` tags on TRex/Raptor actors and drive actual AIController chase/attack logic from them (Blueprint-level, since C++ won't recompile). Also resolve Supabase storage auth (403 Invalid JWS) so TTS assets persist for Audio Agent #16.
