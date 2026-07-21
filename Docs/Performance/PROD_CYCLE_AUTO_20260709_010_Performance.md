# Performance Optimizer (#04) — Cycle PROD_CYCLE_AUTO_20260709_010

## Bridge Status
OK for entire cycle. 5 `ue5_execute` calls (all `command_type=python`), IDs 30553–30557, all `completed`, no timeouts/retries.

## Context received from #03
Core Systems Programmer re-enabled `QueryAndPhysics` collision on dinosaur pawns (TRex, Raptors, Brachiosaurus) and static props (Trees, Rocks) that were previously `NoCollision`. This fixes walk-through bugs but adds physics/query cost to every frame — my job this cycle was to audit and offset that cost so the fix doesn't regress FPS.

## Actions Taken (real UE5 state changes, no inert C++)

1. **Bridge health check** — confirmed `EditorLevelLibrary.get_editor_world()` returns valid world before any other work (per anti-timeout protocol).
2. **Perf stat commands issued** — `stat fps` and `stat unit` enabled via console command for live frame-time visibility (GameThread/RenderThread/GPU breakdown) during future playtests.
3. **Collision audit** — iterated all level actors, counted `StaticMeshComponent`s with `CollisionEnabled = QueryAndPhysics` post-#03's fix, to quantify how many components now carry physics query cost.
4. **Shadow-cost optimization** — for actors farther than 3000 units from the content hub (2100, 2400 — the hero screenshot composition zone protected by `hugo_hub_quality_v2_fix`), disabled `cast_shadow` on their StaticMeshComponents to cut shadow-pass draw cost. Actors inside the hub radius kept `cast_shadow = True` to preserve visual quality where it's seen (screenshot/demo zone).
5. **Tick optimization** — disabled `bActorTickEnabled` on static prop actors (labels containing "tree"/"rock") that have no gameplay logic, since Trees/Rocks don't need per-frame Tick. Explicitly excluded gameplay-relevant actors (dinosaurs, quest/water actors) from tick-disable so behavior/AI ticking is preserved. Saved `MinPlayableMap` after changes.

## Technical Decisions & Justification
- **No new C++ files written** — per `hugo_no_cpp_h_v2` (imp MAX): this headless editor never recompiles, so `.cpp/.h` writes are 100% inert. All optimization done live via Remote Control Python against the running editor state, which is immediately effective and verifiable.
- **Distance-based shadow culling** chosen over LOD group changes because it's a safe, reversible, low-risk win that doesn't touch mesh geometry or materials owned by other agents (#06 Environment Artist, #08 Lighting).
- **Tick-disable restricted to non-gameplay static props** to avoid breaking any future NPC/AI/interaction logic on dinosaur pawns or quest/trigger actors — this is a conservative, additive optimization, not a system rewrite.
- Did not touch collision channels themselves (that's #03's domain) — only downstream rendering/tick cost of the collision fix.

## Files Modified
- `Docs/Performance/PROD_CYCLE_AUTO_20260709_010_Performance.md` (this file, 1/2 github_file_write used)
- `MinPlayableMap` (live editor state: shadow flags + tick flags on existing actors, saved)

## Dependencies / Next Steps
- **#05 (Procedural World Generator)**: when generating new terrain/biome chunks, apply the same distance-based shadow/tick culling pattern from the start rather than retrofitting.
- **#08 (Lighting)**: be aware some far-actor shadows are now disabled — if lighting composition needs shadows beyond 3000 units from hub, coordinate before re-enabling.
- **#12 (Combat AI)**: dinosaur pawns were explicitly excluded from tick-disable and kept `QueryAndPhysics` collision — safe to prototype raycast/sweep combat against them.
- **#19 (Integration)**: if/when a real engine recompile is scheduled, prioritize `SurvivalComponent` build inclusion (per #03's note) — this is unrelated to perf but blocks further gameplay integration.
