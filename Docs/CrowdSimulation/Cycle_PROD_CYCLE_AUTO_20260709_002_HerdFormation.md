# Crowd & Traffic Simulation — Agent #13
## Cycle PROD_CYCLE_AUTO_20260709_002

**Bridge status:** HEALTHY — all 6 `ue5_execute` Python/console calls completed without timeout (3–10s each).

## Objective
Per the MAX-importance content directive (`hugo_hub_herds_v2_fix`), form 1-2 cohesive herbivore herds within ~3000 units of the hero-screenshot hub (world coords X=2100, Y=2400), individuals spaced 300-600 units apart, sharing a general orientation — a living herd grazing together, not a grid or random scatter.

## What Was Done
1. **Bridge health check** — confirmed `EditorLevelLibrary.get_editor_world()` returns a valid world; `stat unit` console roundtrip confirmed responsiveness.
2. **Actor audit near hub** — scanned all level actors within 3000-4000 units of (2100, 2400), filtering by herbivore keywords (`trike`, `triceratops`, `brach`, `stego`, `parasaur`, `edmont`) to avoid re-tagging predators or props.
3. **Deduplication check (per `hugo_naming_dedup_v2`)** — built a set of existing actor labels before any spawn, so no new actor was created if a suitable herbivore instance already existed at/near the hub. Only spawned additional `Trike_Hub_NNN` instances (cloning the `StaticMesh` reference of an existing herbivore actor, not a new asset) if the found herd had fewer than 4 members.
4. **Herd formation logic**:
   - Split the herbivore roster into up to 2 sub-herds (herd centers offset from the hub: one slightly SW, one slightly NE, both within the 3000-unit content-hub radius).
   - Placed members in a loose grid-jittered cluster per herd, spacing randomized 300-600 units apart (`random.uniform(300,600)` on both offset axes) plus ±40 unit position jitter and ±20° yaw jitter so it reads as organic grazing behavior, not a parade line.
   - Each herd shares a **base yaw** (15° or 200°) so all members face roughly the same general direction, consistent with grazing herd behavior (Jane Jacobs "emergent pattern from individual decisions" — here expressed as shared heading with individual jitter).
   - Preserved each actor's original Z height (terrain-following) rather than forcing a flat plane.
5. **Level save** — `EditorLevelLibrary.save_current_level()` called after repositioning.
6. **Verification pass** — re-queried all herbivore-labeled actors within 3500 units of the hub and logged final `x, y, yaw, dist` for QA traceability.

## Key Decisions
- **No .cpp/.h files written** — per the MAX-importance rule `hugo_no_cpp_h_v2`, this headless UE5 instance never recompiles C++, so all herd logic was implemented live via `ue5_execute` Python, directly repositioning/orienting existing actors (and cloning meshes only when the herd was under-populated).
- **Reuse over duplication** — before any spawn, the script builds `existing_labels` from all current actors and only creates `Trike_Hub_NNN` entries if that exact label doesn't already exist, and only when the found herbivore count is below the herd-viability threshold (4). This directly follows the `hugo_naming_dedup_v2` anti-pattern warning (no stacked `_AI`/subsystem-suffixed duplicates of the same creature).
- **No camera changes** — viewport camera was never touched, per `hugo_no_camera_v2`.

## Known Limitation
The Python bridge does not reliably surface `print()`/`unreal.log()` stdout back through the Remote Control response (`result` only contains `{"ReturnValue": true/false}`), so exact final coordinates are only visible in the UE5 Output Log inside the editor, not in this report. Verification pass (step 6) was executed and returned `success:true`, confirming the query ran without error, but numeric confirmation requires an in-editor log check by the next agent or QA (#18).

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Audited all herbivore-labeled actors within 3000-4000 units of hub (2100, 2400)
- [UE5_CMD] Repositioned/oriented existing herbivore actors into 1-2 cohesive herd clusters (300-600 unit spacing, shared base yaw ±20° jitter, terrain-height preserved)
- [UE5_CMD] Conditionally spawned `Trike_Hub_NNN` clones (mesh reused from existing herbivore, only if herd count < 4) — deduplicated against existing labels first
- [UE5_CMD] Saved MinPlayableMap after repositioning
- [UE5_CMD] Verification query of final herd member positions/orientations (logged to UE5 Output Log)
- [FILE] `Docs/CrowdSimulation/Cycle_PROD_CYCLE_AUTO_20260709_002_HerdFormation.md` — this documentation
- [NEXT] Agent #14 (Quest & Mission Designer): the hub now has a grazing herd as a scene anchor — consider a "observe the herd without disturbing it" or "track a lone straggler" objective. QA (#18) should open MinPlayableMap in-editor to visually confirm herd cohesion since stdout isn't recoverable via Remote Control.
