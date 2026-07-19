# Crowd & Traffic Simulation — Agent #13 — Cycle PROD_CYCLE_AUTO_20260719_002

## Objective this cycle
Form 1–2 cohesive herbivore herds within ~3000 units of the hub (X=2100, Y=2400),
individuals spaced 300–600 units apart, sharing a general grazing orientation,
per `hugo_hub_herds_v2_fix` and `hugo_herd_consistency_v1`.

## What was executed (real ue5_execute calls, all `status: completed`)
1. **Audit pass #1** — scanned all level actors, computed 2D distance to hub,
   looked for any existing `Herd_*` tag (case-insensitive) and listed herbivore
   keyword matches (Trike/Triceratops, Para/Parasaurolophus, Stego, Iguanodon,
   Edmontosaurus) within radius 3500.
2. **Audit pass #2** — retry using `EditorActorSubsystem` (in case the first
   editor-world lookup was unreliable), same logic.
3. **Readback attempt** — tried to surface the audit JSON via a forced
   assertion message. Confirmed a **bridge limitation**: the Remote Control
   Python channel used by `ue5_execute` only reports `{"ReturnValue": bool}`
   from the executed script; it does **not** relay `print()`/`unreal.log()`
   output or exception/assert messages back to the calling agent. This matches
   the pattern seen in `PROD_CYCLE_AUTO_20260719_001`'s notes ("Force a
   deliberate assertion failure... to test whether...").
4. **Herd formation (idempotent, self-contained)** — because readback isn't
   available, audit + reuse-check + placement were combined into a single
   script so the reuse decision doesn't depend on a prior round-trip:
   - Collected herbivore-labeled actors within radius 3000 of the hub.
   - Checked each candidate's existing tags; any tag matching `herd_*` found
     on a candidate is reused verbatim (per `hugo_herd_consistency_v1`) instead
     of inventing a new scheme.
   - Actors with no `Herd_*` tag were split into up to two clusters (interleaved
     by current distance to hub, capped 6 per herd) and repositioned around two
     grazing centers near the hub (`hub-400,-300` and `hub+600,+500`), with a
     line trace to the Landscape at each new XY to get a grounded Z (no
     hardcoded Z), spacing ~450 units with small organic jitter (within the
     300–600 band), and a shared yaw per herd (45° / 200°) with ±12° per-individual
     variation so it reads as a herd facing roughly the same way, not a grid.
   - Newly placed actors were tagged `Herd_HubGrazing_A` / `Herd_HubGrazing_B`
     **only if no existing Herd_ tag was found on any candidate in range**;
     if one was found, that exact tag was reused for the group instead.
5. **Verification pass** — re-queried the same herbivore set near the hub and
   asserted that all herbivore candidates in range now carry a `Herd_*` tag.
   The call returned `status: completed` with no error surfaced, consistent
   with the tagging having taken effect, though (per point 3) the bridge does
   not let us print the exact resulting count/labels back to this agent.

## Known limitation (flagged honestly, not invented data)
The RC Python bridge on this instance does not return script `print`/`log`
output or assert messages to the tool caller — only `{"ReturnValue": bool}`.
This means exact actor labels/coordinates touched this cycle cannot be quoted
verbatim in this report; only that the scripts executed with `status:
completed` and consistent (non-instant, ~3s) execution time, i.e. did real
work. This should be raised to the Engine Architect (#02) / Integration Agent
(#19): a lightweight file-based log (e.g. append to a JSON in
`Saved/AgentLogs/`) that a follow-up agent reads via a separate mechanism would
close this verification gap for all agents, not just #13.

## Consistency check result (per `hugo_herd_consistency_v1`)
- Explicitly checked for any pre-existing `Herd_*` / `herd_*` tag within 3500
  units of the hub before acting.
- Logic was written to **reuse** the first such tag found for the group it
  belongs to, and to only mint `Herd_HubGrazing_A` / `Herd_HubGrazing_B` for
  actors that had no tag at all.
- Because of the readback limitation above, this agent cannot state with
  certainty from this cycle's tool output alone whether a prior tag existed
  and was reused, or whether both new tags were minted fresh. The script logic
  guarantees correctness either way (no duplicate scheme was hardcoded), but
  the next cycle's audit should confirm and record the actual tag set found.

## Rules respected
- No `.cpp`/`.h` files written (all changes via `ue5_execute` python).
- No camera, no PLAYER0, no Landscape/foliage/Terrain_Savana touched.
- No mobility changes on any Character/Pawn.
- Z values obtained via line trace to Landscape, never hardcoded.
- Reused existing actors by label lookup; zero new actors spawned this cycle.

## Handoff to #14 (Quest & Mission Designer)
- The hub (2100,2400) now has (or already had, pending next-cycle confirmation)
  up to two tagged herbivore herd clusters (`Herd_HubGrazing_A/B` or a reused
  prior tag) within 3000 units, suitable as a backdrop for an early
  observation/tracking-type objective (e.g. "follow the herd without
  spooking it") — no quest logic was authored here, only the world-state
  precondition (a living herd) that #14 can hook into.
- Recommend #14 avoid combat objectives directly on top of the grazing
  cluster coordinates to preserve the "living herd" composition for the hero
  screenshot.
