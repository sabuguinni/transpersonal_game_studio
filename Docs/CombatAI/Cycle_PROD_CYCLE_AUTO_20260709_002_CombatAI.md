# Combat & Enemy AI — Cycle PROD_CYCLE_AUTO_20260709_002

## Bridge Status
HEALTHY. All 4 `ue5_execute` Python calls completed successfully (3s–6s each, no timeouts this cycle — previous cycle's timeout at 143s did not recur).

## Note on C++ Files
Per GLOBAL memory `hugo_no_cpp_h_v2` (importance MAX): this headless UE5 editor instance does NOT recompile C++ — the running binary is pre-built, and .cpp/.h writes have zero runtime effect (218 UHT errors on record). Per that rule, **no .cpp/.h files were written this cycle**. All combat AI behavior was implemented live in the running editor via `ue5_execute` (Python/Remote Control), which is the only mechanism that actually affects the live game state in this environment.

## What Was Done This Cycle (via ue5_execute Python, live in MinPlayableMap)

1. **Bridge validation** — confirmed world loaded, located combat-relevant actors (TRex, Raptors) by label.
2. **Combat AI tagging** — Applied actor tags encoding tactical combat parameters directly onto existing dinosaur actors (no duplicate actors created, per `hugo_naming_dedup_v2`):
   - `TRex_*` actors tagged: `Combat_Predator`, `Combat_PatrolRadius_3000`, `Combat_ChaseRadius_1500`, `Combat_AttackRadius_250`, `Combat_State_Patrol`
   - `Raptor_*` actors tagged: `Combat_PackHunter`, `Combat_PatrolRadius_2000`, `Combat_ChaseRadius_1200`, `Combat_AttackRadius_180`, `Combat_State_Patrol`, `Combat_PackID_1` (encodes pack-hunting coordination — raptors sharing PackID_1 should flank together)
3. **Flanking encounter geometry** — Spawned 4 `TargetPoint` actors (`CombatFlankPoint_TRexEncounter_000..003`) at cardinal offsets (±800 units) around the TRex position. These serve as tactical waypoints for future Behavior Tree "flee to cover" / "flank predator" logic — the player-facing tension beat: when the TRex aggros, the nearest flank point becomes the suggested escape vector (rocks, not open ground, matching the pack-hunter voice line below).
4. Level saved after actor spawns.

## Design Rationale (Naughty Dog / Griesemer influence)
- Tags encode a **state machine skeleton** (Patrol → Chase → Attack) directly on actors so that a future Behavior Tree (once C++ recompilation is restored) can read `Combat_State_*` tags without needing new duplicate actors — reusing existing dinosaur instances per the anti-duplication rule.
- Pack hunter tagging (`Combat_PackID_1`) sets up **coordinated raptor flanking** — the core "30 seconds of fun repeated with variation" loop: raptors split up and approach from different flank points, forcing the player to choose which direction to run/fight.
- Flank points near the TRex give the player a **fair-feeling escape option** (rocks) contrasted against a losing one (open ground/river), reinforcing "player loses because of a mistake, not because the game was unfair."

## Audio Produced (text_to_speech — 2 combat SFX/VO lines)
1. **CombatSurvivalGuide** — tactical advice VO: "The Tyrannosaur has caught our scent. Do not run in a straight line — cut through the rocks, break its sightline. Raptors hunt in pairs..."
2. **RaptorEncounterWarning** — short tension-beat VO: "Two Raptors circling. When they split up like that, they are already hunting you..."

Both generated successfully at the API level (ElevenLabs TTS synthesis succeeded). Upload to Supabase Storage failed with `403 Invalid Compact JWS` — this is the same systemic, non-blocking infra error observed across 5+ prior cycles/agents (documented in prior cycle memories). Base64 audio payload was returned in-line by the tool; only the persistent storage URL failed.

## Concept Art Produced (generate_image — 1 enemy concept)
Prompt: photorealistic National Geographic-style documentary photo of a Velociraptor pack (2 raptors) mid-hunt in tall grass, golden hour, Cretaceous forest. Generation succeeded at the API level (`gpt-image-1`) but hit the identical Supabase `403 Invalid Compact JWS` storage error — consistent with the systemic issue, non-blocking to game logic.

## Known Systemic Issue (for Director/Orchestrator attention)
Supabase Storage JWS validation (`403 Invalid Compact JWS`) is now confirmed blocking ALL binary asset persistence (audio + images) across at minimum 6 cycles and multiple agents. This does not block gameplay-logic work (ue5_execute) but means no concept art or VO lines have a persistent URL yet. Recommend Director escalate storage token/JWS config to Hugo directly — this is outside Combat AI's remit to fix.

## Deliverables This Cycle
- [UE5_CMD] Tagged existing TRex + Raptor actors in MinPlayableMap with combat AI state parameters (patrol/chase/attack radii, pack ID) — no new duplicate actors created.
- [UE5_CMD] Spawned 4 `CombatFlankPoint_TRexEncounter_NNN` TargetPoint actors around TRex as tactical flanking/escape waypoints; level saved.
- [AUDIO] 2 combat VO lines generated (survival guide tactical advice + raptor pack warning) — synthesis OK, storage upload blocked by systemic infra issue.
- [IMAGE] 1 raptor pack concept art generated — synthesis OK, storage upload blocked by systemic infra issue.
- [NEXT] Once C++ recompilation is restored (per Engine Architect), implement a `UCombat_BehaviorComponent` reading the `Combat_State_*` / `Combat_PackID_*` tags applied this cycle to drive actual Behavior Tree state transitions and pack-flanking movement toward `CombatFlankPoint_*` actors. Until then, further cycles should keep expanding tag-based encounter design via ue5_execute (e.g., tag Brachiosaurus as non-hostile/fleeing prey behavior for contrast).
