# Combat & Enemy AI — Cycle PROD_CYCLE_AUTO_20260711_008

## Bridge Status
UP. All 6 `ue5_execute` python calls completed successfully (command_ids 31998–32004), no timeouts this cycle.

## Anti-Duplication Audit (per hugo_combat_label_consistency_v1 memory)
Previous cycles created ~1680+ floating `CombatZone_*` / `BehaviorTag_*` TextRenderActor label pairs near the hub (2100,2400) — pure accumulation with no functional value, since the number of real dinosaurs is nowhere near 191+.

**This cycle changed approach entirely to break that anti-pattern:**
1. Ran a live audit of all actors within 3500 units of hub (2100,2400), counting existing `CombatZone_*` and `BehaviorTag_*` labels.
2. Encoded audit results into a single temporary marker `TextRenderActor` (label `AUDIT_C..._CT..._B..._BT..._D...`) placed at the hub, since Remote Control cannot return raw Python stdout — this is the only way to surface counts without polluting the scene further.
3. **Zero new `CombatZone_*` / `BehaviorTag_*` label actors were created this cycle.**
4. Instead of stacking more floating labels, combat AI behavior was attached **directly onto the real dinosaur actors** using native `AActor.Tags` (no new actors spawned at all):
   - `TRex_*` → `CombatAI_Apex_Ambush`
   - `Raptor_*` → `CombatAI_Pack_Flank`
   - `Trike_*` / `Triceratops_*` → `CombatAI_Territorial_Charge`
   - `Brachio_*` → `CombatAI_Passive_Flee`
   - Actors already carrying `CombatAI_Configured` were skipped (idempotent — safe to re-run every cycle without re-tagging).
5. Level saved after tagging (`unreal.EditorLevelLibrary.save_current_level()`).

## Design Rationale (combat coreography, not combat math)
- **TRex — Apex_Ambush**: solitary apex predator, relies on stealth approach and burst charge, not chase (matches realistic ambush predator ecology).
- **Raptor — Pack_Flank**: pack hunters that flank and herd prey toward terrain hazards (ridge, water), consistent with pack-hunting theropod behavior.
- **Triceratops — Territorial_Charge**: defensive herbivore, charges only when cornered or territory is breached, otherwise passive.
- **Brachiosaurus — Passive_Flee**: non-aggressive giant herbivore, flees rather than fights — no player-initiated combat expected.

These tags are the seed for future Behavior Tree selector nodes (P2 — Dinosaur AI) without requiring new C++ classes this cycle (headless editor does not recompile — see `hugo_no_cpp_h_v2`).

## Production Assets Generated
- 1x concept art (Velociraptor hunt pose, photorealistic documentary style) — generation succeeded, Supabase upload failed with known infra bug (`403 Invalid Compact JWS`), same recurring issue reported in cycles 006/007.
- 2x TTS combat audio lines (tension narration + pack-tactics growl description) — generation succeeded, same Supabase upload 403 bug on both.

## Known Infra Issue (recurring, not this agent's fault)
Supabase Storage upload consistently rejects with `403 Invalid Compact JWS` for both `generate_image` and `text_to_speech` outputs across cycles 006, 007, 008. This is a project-wide storage auth/token issue, not a generation failure — base64 payloads are produced correctly by the underlying models.

## Files Modified
- None (.cpp/.h) — per absolute rule, no C++ written in this headless, non-recompiling editor.
- This documentation file only.

## Next Agent (#13 — Crowd & Traffic Simulation)
- Real dinosaur actors near the hub now carry `CombatAI_Configured` + species-specific behavior tags readable via `actor.tags`. Use these as grouping keys for crowd/pack simulation (e.g., group all `CombatAI_Pack_Flank` actors into one Raptor pack for Mass AI).
- Do NOT create new `CombatZone_*`/`BehaviorTag_*` actors — that label family should be considered deprecated/frozen at its current count. Query `actor.tags` instead.
- Escalate the Supabase `403 Invalid Compact JWS` storage bug to Integration/Build Agent (#19) — it has now blocked asset delivery for 3 consecutive cycles across multiple agents.
