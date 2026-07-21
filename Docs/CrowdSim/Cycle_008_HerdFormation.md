# Crowd & Traffic Simulation — Cycle PROD_CYCLE_AUTO_20260711_008

## Bridge status
UP. All Python `ue5_execute` calls succeeded (command IDs 32005–32011). One `get_property` readback attempt (32009) failed because TextRenderActor sub-object path resolution via Remote Control didn't match the actor's internal component name — non-blocking, audit was still completed via direct in-script Python logic (no need for cross-call readback).

## Consistency audit performed (per `hugo_herd_consistency_v1`)
Before touching any actor, scanned all level actors within 3500 units of the hub (X=2100, Y=2400) for any pre-existing tag starting with `Herd_`/`herd_` (case-insensitive).

**Result: no existing `Herd_*` tags were found on any actor near the hub.** This is the first cycle to tag herd members at this location — previous cycles (005/007) reported working on herd formation but no persistent `Herd_*` tags existed on real actors when re-audited this cycle (likely prior attempts were lost/reverted, or were performed on marker actors rather than the dinosaurs themselves).

Since no existing tags were found, a **new tag was created**: `Herd_HubGrazing_01`, applied to all herbivore actors found without a herd tag.

## Actors identified as herbivores near hub
Species matched by label keywords: `trike`/`triceratops`, `brachio`, `stego`, `paras`, `hadro`, `duck`. These were queried from the **real existing dinosaur actors already in MinPlayableMap** — no new actors were spawned (per `hugo_naming_dedup_v2`).

## Herd formation applied
- All untagged herbivore actors found within the hub radius were:
  1. Repositioned into a grazing cluster centered at approximately (2500, 2100) — offset ~400/-300 units from the hub center, keeping clear of PlayerStart at origin.
  2. Spaced 300–600 units apart from each other using a radial distribution (angle-based placement with radius variance 300/450/600).
  3. Given a shared base orientation (yaw ≈ 35°) with ±15° individual jitter, so the group reads as "grazing together" rather than robotically aligned.
  4. Tagged with `Herd_HubGrazing_01` (new tag, since no prior tag family existed to reuse).
- Level saved after repositioning.

## Verification
A final pass queried all actors carrying any `Herd_*` tag within the hub radius and logged their resolved label, position, tags, and distance from hub center — confirming the tag was applied and the cluster geometry is in place.

## Decisions & rationale
- Reused Combat AI Agent #12's existing species tags (`CombatAI_Territorial_Charge`, `CombatAI_Passive_Flee`, etc.) as-is — did not touch or duplicate them, only added the new `Herd_HubGrazing_01` tag alongside on the same real actors.
- No new actors were spawned; no `.cpp`/`.h` files were written (per `hugo_no_cpp_h_v2` — this headless editor never recompiles new C++).
- Did not touch the editor viewport camera (per `hugo_no_camera_v2`).

## Next steps for #14 (Quest & Mission Designer)
- Herbivore herd `Herd_HubGrazing_01` is now grouped and spatially coherent at the hub — usable as a quest objective location (e.g., "observe/track a herd" or "avoid disturbing the herd").
- Combat AI tags (`CombatAI_Configured` + species tag) from Agent #12 remain the grouping key for predator behavior; `Herd_*` tags are now the grouping key for herbivore social/crowd behavior — these two tag families are complementary and should not be merged or renamed.
- If future cycles add more herbivores near the hub, check for `Herd_HubGrazing_01` first and reuse it; do not invent a second herd tag scheme for the same location.
