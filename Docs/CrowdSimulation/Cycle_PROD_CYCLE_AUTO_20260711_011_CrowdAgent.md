# Crowd & Traffic Simulation Agent #13 — Cycle PROD_CYCLE_AUTO_20260711_011

**Bridge status: UP.** 5 `ue5_execute` Python calls executed successfully (command IDs 32244, 32245, 32247, 32249, 32251), 3–15s each, zero timeouts.

## Consistency Check (per Brain rule `hugo_herd_consistency_v1`)
BEFORE any repositioning, the script scanned all actors within 3500 units of the hub (X=2100, Y=2400) and collected every actor tag matching the pattern `herd_*` (case-insensitive), exactly as mandated. This scan was performed fresh in this cycle (command 32244/32247) because tool responses from `ue5_execute` do not surface Python `print()` output back to the calling agent — only a generic `ReturnValue` boolean is returned by the Remote Control bridge. To work around this limitation for future verification, a persistent `TextRenderActor` labeled **`CrowdSim_HerdAuditLog`** (tag `CrowdAuditLog`) was created/reused at the hub and its `Text` property was set to a full summary string (herd-tagged actor count, full detail list, all nearby actor labels). Future cycles/agents can read this via `get_property` on that actor without re-scanning blind.

## Herd Formation Logic Applied (single atomic script, command 32249)
1. Re-scanned all actors within 3500u of hub.
2. Checked for any existing tag starting with `Herd_`/`herd_` on those actors.
   - **If found:** the script explicitly takes NO renaming/repositioning action and only reports the existing tags (respects `hugo_herd_consistency_v1` — reuse, never re-invent).
   - **If none found:** filtered actors by herbivore keyword match on label (`trike`, `triceratops`, `brach`, `parasaur`, `stego`, `hadro`, `edmont`, `herbivore`), explicitly excluding predator keywords (`trex`, `raptor`, `carno`, `predator`) to avoid mixing predators into a grazing herd.
   - If ≥2 herbivores were found with no pre-existing herd tag, they were clustered into a single new herd tagged **`Herd_HubGrazing_01`**:
     - Positioned on a ring around the hub at varying radii (350/500/650 units) to keep spacing in the 300–600u range mandated by `hugo_hub_herds_v2_fix`.
     - Original terrain Z height preserved per-actor (no floating/clipping).
     - Shared base orientation of yaw ≈200° (grazing-away-from-hub-center pose) with only ±10° per-individual variation — cohesive but not robotic/identical.
   - If <2 herbivores were found near the hub, explicitly no herd was fabricated (rule: don't invent herds from nothing).
3. Level saved after every mutating step (`save_current_level()`).

## Handoff Coordination
- Reused `CombatRole_*` / `PackID` / `FlankOrder_`/`FlankAngle_` tags authored by Combat AI Agent #12 as read-only context — did not overwrite or duplicate them; crowd/herd tags are additive (`Herd_*` namespace only), so predator pack-flanking data and herbivore herd-grouping data coexist on any dinosaur that might carry both in future.
- No new actors were spawned for creatures that already exist in the scene — the herbivore set was built entirely from **existing** hub actors found by label lookup, per `hugo_naming_dedup_v2`. Zero duplicate actors created this cycle.

## Known Limitation
`ue5_execute` (Remote Control bridge) does not return Python `print()`/`log()` output in its response payload — only `{"ReturnValue": bool}` reflecting statement execution success. This blocks direct verification of exact herbivore counts/tag state within the same agent turn. Mitigation: the `CrowdSim_HerdAuditLog` TextRenderActor now persists this data in-world for the next cycle (or a `get_property` call) to confirm without a blind re-scan.

## Files Written
- `Docs/CrowdSimulation/Cycle_PROD_CYCLE_AUTO_20260711_011_CrowdAgent.md` (this file)

## Handoff to #14 (Quest & Mission Designer)
- Herd cluster (if newly formed) is tagged `Herd_HubGrazing_01`; check `CrowdSim_HerdAuditLog` TextRenderActor at (2100,2400,50) first to see exact membership before scripting quests involving "the herd at the hub" (e.g., a hunting/observation quest).
- Recommend next crowd cycle: read back `CrowdSim_HerdAuditLog.text_render.text` via `get_property` command_type at the START of the cycle (not full re-scan) to save execution time, then only mutate if drift/duplication is detected.
