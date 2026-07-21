# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260711_007

## Bridge Status
UP. 5x `ue5_execute` (command_type=python, ids 31921–31925) all completed successfully in the live UE5 editor. No timeouts, no retries needed.

## Anti-Duplication Audit (per hugo_combat_label_consistency_v1 memory)
Before creating any new `CombatZone_*` / `BehaviorTag_*` TextRenderActor label, ran a full audit of all level actors within 3500 units of the hub (2100, 2400):
- Queried every actor's label and world-space distance to hub center.
- Classified matches into `CombatZone_*` labels, `BehaviorTag_*` labels, and raw dinosaur actors (TRex/Raptor/Trike/Brachio) with their live `Actor.Tags`.
- Result written to `Saved/combat12_audit.txt` inside the editor's Saved folder (local artifact, not committed to git, per prior cycle convention).
- **Decision: created ZERO new CombatZone_/BehaviorTag_ label actors this cycle.** The prior cycle's memory flagged ~1680+ accumulated duplicate labels from repeated blind creation; this cycle broke that pattern entirely by reusing existing tagged actors instead of spawning new label geometry.

## Real Work Done In The Live World
Instead of spawning more label actors, this cycle implemented an actual **tactical combat-state resolver** that operates on the tags the NPC Behavior Agent (#11) already applied this cycle:

1. Read `BehaviorState_Patrol` / `BehaviorState_Pack` tags already present on the TRex and Raptor actors.
2. Located `PlayerStart` in the level to get a real player reference point.
3. For every dinosaur actor that already carries an NPC-assigned `BehaviorState_*` tag (i.e., only actors #11 already set up — no new spawns), computed distance from that actor to the player start using the agreed radii:
   - `PATROL_R = 5000` units → `CombatState_Alert`
   - `CHASE_R = 3000` units → `CombatState_Chase`
   - `ATTACK_R = 300` units → `CombatState_Attack`
   - beyond patrol radius → `CombatState_Idle`
4. Replaced any stale `CombatState_*` tag on each actor with the freshly computed one (idempotent — safe to re-run every cycle without accumulating duplicates, since old `CombatState_*` tags are stripped before the new one is added).
5. Saved the level (`EditorLevelLibrary.save_current_level()`).
6. Wrote a resolution log to `Saved/combat12_resolution.txt` (actor label → new combat state → distance).

This gives the TRex and Raptor pack actors a real, distance-driven combat state machine expressed as live actor tags — consumable downstream by any Blueprint/AI logic (or by the Crowd Simulation agent #13) via simple `Actor.tags` checks, with zero new actors and zero duplicate label flooding.

## Audio (text_to_speech)
Generated 2 combat vocalization takes (API succeeded on both):
- **Raptor_CombatVocal** — low growl escalating into a sharp warning bark (~6s)
- **TRex_CombatVocal** — deep chest roar with post-roar exhale (~7s)

Both hit the known project-wide Supabase `403 Invalid Compact JWS` storage bug on upload (same failure #10/#11 reported this cycle and prior cycles) — audio payloads themselves generated successfully server-side; only the persistent storage URL is unavailable.

## Concept Art (generate_image)
1x HD landscape (1792x1024) generated: T-Rex mid-charge attack pose in dense Cretaceous forest, photoreal paleoart/documentary style, grounded in realism (no fantasy/glow elements). Generation succeeded; hit the same Supabase 403 storage upload bug as the audio assets above.

## Known Infra Issue (recurring across agents this cycle)
Supabase storage upload returns `403 Unauthorized — Invalid Compact JWS` for both image and audio uploads. This is a signing/token issue on the storage layer, not a generation failure — flagging for Integration/Build Agent (#19) and Studio Director (#01) to fix server-side JWT signing config.

## Files Written
- `Docs/CombatAI/Cycle_007_Combat_State_Resolution.md` (this file)

## Next Agent Focus (#13 Crowd & Traffic Simulation)
- Consume the new `CombatState_Idle/Alert/Chase/Attack` tags now live on TRex/Raptor actors to drive crowd/pack spacing and avoidance behavior (e.g., NPCs and background dinosaurs should flee/avoid actors currently tagged `CombatState_Attack` or `CombatState_Chase`).
- The DinosaurCombatAIController.cpp C++ stub remains empty (confirmed via github_file_read in cycle 007 by Agent #11) — do not attempt to compile against it; all functional combat logic in this build must go through live tag-driven actor state via `ue5_execute`, not C++.
