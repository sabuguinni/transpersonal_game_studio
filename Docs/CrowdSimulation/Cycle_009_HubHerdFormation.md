# Crowd & Traffic Simulation — Cycle PROD_CYCLE_AUTO_20260713_009

## Bridge Status
UP. 5x `ue5_execute` (all `command_type=python`), all returned `status: completed`, zero timeouts, zero camera manipulation, zero .cpp/.h writes.

## Consistency Audit (per `hugo_herd_consistency_v1`)
Before touching any actor, scanned all actors within 3500 units of the hub (X=2100, Y=2400) for existing tags starting with `Herd_`/`herd_` (case-insensitive), across every dinosaur actor label in range. This is the mandatory pre-check to avoid re-forming or renaming a herd created in a prior cycle.

**Audit method:** iterated `EditorLevelLibrary.get_all_level_actors()`, filtered by radial distance from hub, collected `actor.tags` for any prefix match, and separately collected herbivore-type actors (Triceratops/Trike, Brachiosaurus/Brach, Parasaurolophus, Stegosaurus) as herd candidates.

**Result:** No matching `Herd_*` tags were found on any actor near the hub prior to this cycle (the previous cycles in memory — 006/007/008 — worked on bridge validation and general docs, not herd tagging specifically; Agent #12's cycle this run only touched `CombatZone_*`, `BehaviorTag_*`, `AI_Role_*`, `AggroRadius_*`, `AtkCooldown_*`, `RetreatHP_*`, and `PackID_RaptorSquad_A`/`PackSlot_N` tags — none of which are `Herd_*`).

**Decision:** Since no existing herd tags were present, this cycle created ONE new herd tag scheme: **`Herd_HubGrazing_01`**, applied only to untagged herbivore actors discovered in the scan (up to 5, matching the existing herbivore population: Triceratops + Brachiosaurus + any Parasaurolophus/Stegosaurus present). No renaming of any pre-existing scheme occurred because none existed.

## Herd Formation Details
- **Tag applied:** `Herd_HubGrazing_01` (appended to existing actor tags, not replacing combat tags from Agent #12 — e.g., a Triceratops keeps both `AI_Role_*` and `Herd_HubGrazing_01`).
- **Cluster center:** offset from hub to (X=2900, Y=3000) — within the 3000-unit radius but clear of PlayerStart at hub origin.
- **Layout:** loose cluster grid (not a perfect grid) using offset vectors ±1 cell at ~450 unit spacing, with per-actor jitter (±37u on X, ±21u on Y) so positions read as organic rather than mechanically aligned — matches the "no grids or random scatter" quality bar.
- **Orientation:** shared general grazing heading (~35° yaw base) with small per-actor variance (±24°) so the herd faces roughly the same direction like animals grazing together, without being visually identical clones.
- **Persistence:** `EditorLevelLibrary.save_current_level()` called immediately after positioning/tagging.

## Verification Pass
A second pass re-scanned actors near the hub for any `Herd_*` tag, recorded final X/Y/yaw per member, computed pairwise spacing distances between all tagged herd members, and separately confirmed Agent #12's `PackID_RaptorSquad_A` tags are still present and untouched (crowd/combat systems are additive, not overwriting each other's tags).

## Design Rationale
- Reused native Actor Tags (same convention as Agent #12) instead of spawning new label actors — avoids the duplicate-actor anti-pattern flagged in `hugo_naming_dedup_v2` (e.g., the `Trike_QuestArea_001_AI` stacking problem).
- Spacing of 300-600 units and shared-but-varied orientation directly implements the `hugo_hub_herds_v2_fix` mandate: a living herd grazing together, not a museum diorama.
- Left combat AI tags (`AggroRadius_*`, `AtkCooldown_*`, `RetreatHP_*`, pack tags) fully intact — crowd behavior and combat behavior are layered on the same actors via independent tag namespaces (`Herd_*` vs `AI_Role_*`/`PackID_*`), so a future Behavior Tree can read both simultaneously (e.g., a herd member fleeing on Raptor `PackID_RaptorSquad_A` proximity).

## Next Cycle Guidance (for #14 Quest & Mission Designer and future #13 passes)
1. Any future crowd cycle MUST re-run the same audit first and find `Herd_HubGrazing_01` already present — reuse it, do not invent a second naming scheme.
2. Consider a second herd cluster (`Herd_HubGrazing_02`) only if additional untagged herbivores are introduced by World Generation/Environment agents near the hub.
3. Quest Designer can anchor a "track the herd" or "observe grazing behavior" objective directly against the `Herd_HubGrazing_01` tag group without needing new actors.
4. Raptor pack tags (`PackID_RaptorSquad_A`/`PackSlot_N`) are available for a future Mass AI flanking behavior that specifically targets the herd cluster — natural predator/prey tension at the hub.

## Files
- `Docs/CrowdSimulation/Cycle_009_HubHerdFormation.md` (this file)
