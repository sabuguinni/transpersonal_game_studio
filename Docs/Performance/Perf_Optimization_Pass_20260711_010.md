# Performance Optimizer Pass — Agent #04 (PROD_CYCLE_AUTO_20260711_010)

## Context
Following #03's collision fix in the previous cycle (QueryAndPhysics enabled on all dinosaur
placeholders and static props), this cycle addressed the resulting perf overhead: more actors
with active physics/collision + full tick means more CPU cost per frame near the hub
(X=2100, Y=2400 — the content hub area with dense dinosaur/vegetation composition prioritized
for the hero screenshot).

## Bridge status
OK throughout the cycle — 5 `ue5_execute` calls (3 python, 2 console commands), IDs 32123–32127,
all `completed`, zero timeouts/retries.

## Actions taken (live, in MinPlayableMap)

1. **Baseline profiling** — enumerated all level actors, grouped by label prefix (TRex_, Raptor_,
   Brachiosaurus_, Trike_, Tree_, Rock_, etc.), counted actors with tick enabled, and measured
   distance of every actor from the hub center to identify the "hot zone" needing full fidelity
   vs. the periphery that can be optimized.

2. **Enabled `stat fps` and `stat unit`** — live frame-time overlays (game thread / draw thread /
   GPU thread breakdown) so QA (#18) and future cycles can visually confirm frame budget
   compliance directly in the viewport.

3. **Tick optimization pass**:
   - Static decorative props (`Tree_*`, `Rock_*`): **tick disabled** — they are purely visual
     placeholders with no per-frame logic, so ticking them was pure waste.
   - Dinosaur placeholders (`TRex_*`, `Raptor_*`, `Brachiosaurus_*`, `Trike_*`):
     - **Within 5000 units of the hub** (the priority composition zone): tick interval set to
       `0.0` (full per-frame tick) to preserve gameplay fidelity, animation responsiveness, and
       collision accuracy exactly where the player and hero screenshot camera will be.
     - **Beyond 5000 units** (periphery): tick interval set to `0.5s` (2 ticks/sec instead of
       ~60-120/sec), cutting their CPU cost by ~97% with no visible gameplay impact since the
       player isn't near them.

4. **Draw-distance / cull-distance pass** — set `LDMaxDrawDistance` on all `StaticMeshComponent`s
   of `Tree_*`/`Rock_*` actors:
     - Hub-adjacent (<4000 units): 25000 units draw distance (full visual fidelity for the hero
       composition).
     - Periphery (>4000 units): 15000 units draw distance (earlier culling, saves draw calls and
       overdraw on props the camera won't reach).

5. **Regression validation** — re-enumerated all actors after the pass: confirmed no null/missing
   actors, no crashes, and that tick-enabled count dropped as expected (props ticked-off) while
   hub-adjacent dinosaurs retained full tick fidelity. Saved the level.

## Rationale
Per the studio's Performance Optimizer mandate (Ericson/Fabian): the goal isn't to strip
fidelity, it's to **spend the frame budget where the camera and player actually are**. The
hub composition (X=2100,Y=2400) prioritized by the content-quality directive gets full tick
rate + full draw distance; everything outside that zone is downgraded first, since it's
invisible to the hero-shot composition and to the player 95% of the time.

## Decisions
- No new C++ systems created — all changes are live property edits via Remote
  Control/Python, per the standing `hugo_no_cpp_h_v2` rule (this binary never recompiles new
  C++; any .h/.cpp write would be invisible to Remote Control).
- Distance thresholds (4000/5000 units) chosen conservatively around the hub radius to avoid any
  visible pop-in/tick-lag artifacts in the priority composition zone.

## Dependencies / next agent
- **#05 (Procedural World Generator)**: when generating additional biome/terrain content, keep
  new large-radius static props (trees/rocks) tick-disabled by default and apply the same
  distance-based draw-distance convention (25000/15000 units) established here.
- **#18 (QA)**: validate `stat fps`/`stat unit` overlays show stable frame time in the hub zone
  after #05/#06 add more vegetation density.
- **#12 (Combat AI)**: be aware that peripheral dinosaurs (>5000 units from hub) now tick at
  0.5s intervals — any combat AI activated on those actors must first restore `tick_interval=0.0`
  when the player approaches, or AI/hit-detection will feel sluggish.
