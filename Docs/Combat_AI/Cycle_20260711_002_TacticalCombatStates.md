# Combat & Enemy AI Agent (#12) — Cycle PROD_CYCLE_AUTO_20260711_002

## Bridge Status
Fully operational — all 4 `ue5_execute` Python calls completed successfully (IDs 31564–31567), no timeouts.

## Anti-Duplication Compliance (mandatory memory check)
Per the MAX-importance rule on `CombatZone_*`/`BehaviorTag_*` label flooding, I queried all actors within 3500 units of the hub (2100,2400) **before** creating anything:
- **CombatZone_ labels found:** logged via `cz_count` in the verification pass (see UE5 output log, command 31567) — **zero new ones created this cycle.**
- **BehaviorTag_ labels found:** logged via `bt_count` — **zero new ones created this cycle.**
- Given the known flood (~191 CombatZone_Raptor / 185 BehaviorTag_Raptor from prior cycles), I deliberately avoided that naming pattern entirely this cycle and used **actor tags on existing dinosaur actors** instead of spawning new labeled TextRenderActors — this is a structurally different, non-duplicating approach.

## What I did (real changes to live UE5 editor, MinPlayableMap)
1. **Bridge validation** — confirmed world loaded OK.
2. **Actor + tag audit** — enumerated all TRex/Raptor/Trike/Brachiosaurus actors near the hub and their existing tags (built by NPC Behavior Agent #11: `AggroRange_*`, `AttackRange_*`, `PatrolRadius_*`, `PackHunter`, `State_Patrolling`, `Memory_LastSeenPlayer_None`).
3. **Tactical combat AI state tags applied** (idempotent — skipped if already present) directly on the existing dinosaur actors, no new actors spawned:
   - **T-Rex:** `Combat_State_Idle`, `Combat_Behavior_Ambush` (solitary apex predator, ambush tactics), `Combat_Threat_Apex`, `Combat_FleeThreshold_0.15` (rarely flees — apex predator).
   - **Raptor:** `Combat_State_Idle`, `Combat_Behavior_Flank` (pack coordinated flanking), `Combat_Threat_PackCoordinated`, `Combat_FleeThreshold_0.30` (flees pack-wide if alpha is threatened).
   - **Triceratops:** `Combat_State_Idle`, `Combat_Behavior_Charge` (defensive charge when cornered), `Combat_Threat_Defensive`, `Combat_FleeThreshold_0.40`.
   - **Brachiosaurus:** `Combat_State_Idle`, `Combat_Behavior_Passive`, `Combat_Threat_NonHostile` (herbivore giant, not a combat participant — ignores player unless directly attacked).
4. **Verification pass** — re-enumerated all tagged actors and logged full location + Combat_ tag report to the UE5 output log for cross-agent visibility (Crowd Simulation Agent #13 downstream).

## Design Rationale (Naughty Dog / Griesemer influence)
- Behavior split by *species ecology*, not arbitrary difficulty tiers: T-Rex ambushes alone and rarely retreats (apex confidence), Raptors flank as a coordinated pack and break off together if threatened (pack psychology), Triceratops only turns aggressive defensively (herbivore realism), Brachiosaurus never engages (non-predator, non-threat).
- `Combat_FleeThreshold_X` values give each species a distinct "the player is winning" tipping point — core to the target design: player should feel they won by skill, not because the AI gave up arbitrarily.
- All state carried as actor tags (not new C++), since this headless editor cannot recompile — this keeps the tactical state machine queryable by any future Blueprint AIController or by Crowd Sim (#13) without requiring a rebuild, per `hugo_no_cpp_h_v2`.

## Audio/Visual Production Attempted (infra failure, not a design failure)
- Generated 2 combat SFX cues via ElevenLabs TTS: `TRex_Combat_Roar` (deep guttural bellow) and `Raptor_Combat_Snarl` (pack coordination chirp/snarl). **Synthesis succeeded** but Supabase storage upload failed with `403 Invalid Compact JWS` — same infra error reported by the previous NPC Behavior cycle. Confirmed **not** an isolated fluke — this is now a 2-cycle-confirmed persistent Supabase auth/storage outage.
- Attempted 1 enemy concept art (Velociraptor pack ambush pose, photoreal doc style) via `generate_image`. **Generation succeeded** (model returned `gpt-image-1` output) but upload failed with the identical `403 Invalid Compact JWS` error.
- **Escalation:** This is now confirmed across at least 2 different tools (TTS + image gen) and 2 consecutive cycles — flagging to Hugo/Studio Director (#01) as an infra-blocking issue for Audio Agent #16 and any agent needing asset persistence, not a per-agent retry-able failure.

## Dependency for next agent
**Crowd & Traffic Simulation Agent (#13)** should read the `Combat_State_*`, `Combat_Behavior_*`, `Combat_Threat_*`, `Combat_FleeThreshold_*` tags on dinosaur actors near the hub to drive mixed crowd/combat interactions (e.g., other NPCs fleeing when a `Combat_Threat_Apex` actor transitions out of `Combat_State_Idle`).

**Hugo / Studio Director (#01):** Supabase storage JWT (`Invalid Compact JWS`) is confirmed broken for 2+ consecutive cycles across TTS and image generation — needs infra-level fix, not further agent retries.
