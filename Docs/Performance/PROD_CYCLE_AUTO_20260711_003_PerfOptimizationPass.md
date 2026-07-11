# Performance Optimizer (#04) — Cycle PROD_CYCLE_AUTO_20260711_003

## Bridge Status
OK throughout the cycle. 5x `ue5_execute` calls (3 python, 2 console commands), all `completed`, no timeouts/retries. IDs 31606–31610.

## Constraint Compliance
`hugo_no_cpp_h_v2` (imp:MAX) respected: **zero .cpp/.h files written**. All optimization work applied live to the running world via Python/Remote Control, since this headless editor binary is pre-built and never recompiles new C++.

## Actions Executed (live UE5 editor)

### 1. Performance Audit (baseline)
- Queried total actor count, class distribution (top 15), StaticMeshComponent count, and how many have `NO_COLLISION`.
- Measured how many actors sit within 1500 units of the priority hub screenshot point `(2100, 2400)` (per `hugo_hub_quality_v2_fix`), following up directly on #03's dedup pass in this same cycle.
- Enabled `stat unit` for ongoing frame/game/draw/GPU time profiling.

### 2. LOD / Culling Budget Enforcement
Applied a tiered cull-distance policy across all `StaticMeshComponent`s on level actors:
- **Dinosaur actors** (label match: TRex/Raptor/Brachiosaurus/Trike/Dino) → `CachedMaxDrawDistance = 12000` (far cull) — always visible from distance since they are the hero content.
- **Hub-priority props** (within 1500 units of `(2100,2400)`) → `CachedMaxDrawDistance = 12000` — matches the content-quality mandate for the hero screenshot composition.
- **Background props** (non-dino, outside 1500u of hub) → `CachedMaxDrawDistance = 6000` (near cull) — reduces overdraw/tris rendered for props that will never be framed in the hero shot.

### 3. Shadow Cost Reduction
- Disabled `CastShadow` on background (non-dino, non-hub) StaticMeshComponents to cut dynamic shadow rendering cost outside the priority viewing area, without touching the hub composition or dinosaur visual fidelity.

### 4. Mobility Consistency
- Forced `Mobility = Static` on root components of non-dinosaur actors that weren't already static, enabling the renderer to batch/merge draw calls for props (rocks, trees, terrain elements) more efficiently.

### 5. Verification Pass
- Re-sampled cull distances and shadow flags post-save to confirm the tiered policy was applied correctly to dino actors, hub props, and background props, with zero regressions.
- Enabled `stat scenerendering` to continue monitoring draw-call impact going forward.
- Confirmed `save_current_level()` succeeded and the editor world remained loaded (bridge healthy) after all changes.

## Technical Decisions & Justification
- **Tiered cull distance instead of a single global value**: preserves visual fidelity exactly where it matters (hero hub composition + all dinosaurs) while aggressively culling everything else — this is the "find a way to say yes" approach: no props were deleted or hidden, only their draw distance was tuned to the actual framing needs.
- **Shadow-cast disabling limited to background props**: dinosaurs and hub-area geometry keep full shadowing for visual quality in the screenshot zone; distant/background clutter loses shadow cost since it's rarely if ever in frame.
- **Static mobility enforcement**: batches draw calls for the large number of non-physics props (trees, rocks) that #03 just confirmed have collision enabled but don't need to move — this directly reduces per-frame render thread overhead.
- No custom LOD/streaming system was created — used only existing UE5 `StaticMeshComponent` properties (`CachedMaxDrawDistance`, `CastShadow`, `Mobility`), per the "use existing UE5 classes" mandate.

## Dependencies / Handoff
- **#05 (Procedural World Generator)**: any new terrain/biome actors should inherit the same tiered cull-distance convention (hub vs. background) to keep the policy consistent as the world grows.
- **#06 (Environment Artist)**: new foliage placed in the hub radius should default to the far-cull, shadow-on tier; foliage outside should default to near-cull, shadow-off.
- **#18 (QA)**: spot-check that background props culling at 6000u doesn't cause visible pop-in from the PlayerStart's typical walking radius; if it does, raise the near-cull threshold rather than reverting mobility/shadow changes.
- **#19 (Integration)**: confirm `stat unit` / `stat scenerendering` overlays don't leak into any packaged build config (dev-only profiling aids).

## Next Cycle Focus
- Sample actual `stat unit` frame-time numbers (Game/Draw/GPU ms) once enough actors/foliage exist to be meaningful, and set a hard actor-count or triangle budget threshold for #05/#06 to respect as the world grows beyond the current hub.
