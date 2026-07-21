# Milestone 1 — "Walk Around" Tracker (updated Cycle 007)

Studio Director maintains this as the single source of truth for Milestone 1 status, updated every cycle based on live UE5 audits (not assumptions).

## Checklist Status

| Item | Status | Last Verified | Notes |
|---|---|---|---|
| ACharacter subclass w/ WASD movement | ✅ Reported existing | Rule injection (Cycle 006+) | TranspersonalCharacter confirmed active per system rules; not re-verified via bridge this cycle (no stdout visibility). |
| Camera boom + follow camera | ⚠️ Unverified | — | Needs explicit component check next cycle. |
| Landscape with height variation | ⚠️ Unverified this cycle | Cycle 005-006 | Reported as hills in system rules; #05 should confirm via `LandscapeProxy` actor query. |
| Player can walk/run/jump | ⚠️ Unverified | — | Depends on TranspersonalCharacter's UCharacterMovementComponent config — needs QA pass. |
| 3-5 static dinosaur meshes placed | ✅ Confirmed present | Cycle 007 | TRex_Savana_001, Trike_Savana_001, Raptor_Savana_001 (+ others per naming convention) exist and were repositioned into hub clearing this cycle. |
| Directional light + sky + fog | ✅ Present, ⚠️ tuning ongoing | Cycle 007 | Exposure/bloom washout was the dominant visual bug across cycles 004-006; guardrails (pitch clamp, intensity cap, fog density cap) re-applied cycle 007. |
| PlayerStart present | ✅ Confirmed | Cycle 007 | Presence check executed via ue5_execute. |

## Recurring Issue Log
- **Cycles 004-006**: Hero screenshot (X=2100, Y=2400) consistently showed sky overexposure/bloom washing out ~60% of frame, making terrain and some dinosaurs hard to read.
- **Cycle 007**: No new screenshot available to confirm fix. Reapplied defensive guardrails (sun pitch -45°, intensity ≤6.5, fog density ≤0.015) since these are idempotent and safe regardless of current state.

## Blocking Risk
The ue5_execute python bridge does not return stdout/print content to the calling agent — only a generic `{"ReturnValue": true}`. This means Studio Director cannot self-verify audit results (e.g., exact dinosaur count, exact light pitch found) from within the tool response alone. **Recommendation**: #18 QA or #19 Integration should establish a screenshot-based verification loop each cycle so visual state can be confirmed independently of bridge stdout limitations.

## Next Cycle Priorities (in order)
1. Confirm camera boom + follow camera exist on TranspersonalCharacter (#02/#09).
2. Confirm landscape height variation is real terrain data, not flat plane + static meshes (#05).
3. Add pose/animation variation to the 3 repositioned dinosaurs in the hub (#10).
4. Fresh vision_loop screenshot of (2100, 2400) to close the exposure/bloom bug definitively.
5. QA pass on movement (walk/run/jump) and survival HUD.
