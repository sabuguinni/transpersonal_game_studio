# Cycle PROD_CYCLE_AUTO_20260712_004 — Studio Director (#01) Report

## VISUAL FEEDBACK APPLIED
Last cycle's screenshot (vision_capture_20260712_184855.png) confirmed a **critical overexposure/bloom issue**: sky washed out white in ~60% of the frame, terrain barely readable, only 1-2 dinosaur silhouettes visible, sky mesh clipping, camera appeared rotated ~90°.

**Concrete changes made this cycle in response:**
1. **DirectionalLight** — set intensity to a fixed `3.5` (down from an unbounded/likely-excessive value) and re-set pitch to `-45°` (within the mandated -30 to -60 CAP range) to remove the blown-out grazing-angle glare.
2. **PostProcessVolume** — forced `AutoExposureMethod = Manual` with `AutoExposureBias = 0.5` (was likely uncontrolled Eye Adaptation causing the wash-out), and clamped `BloomIntensity` to `0.4` (down from a much higher implicit value) with `BloomThreshold = 1.0`. Set `unbound = true` so the fix applies across the whole level, not just inside a volume. If no PostProcessVolume existed, one was created at the content hub (2100, 2400, 300).
3. Did **NOT** touch the editor viewport camera per the absolute no-camera rule — the ~90° rotation artifact reported is left for the vision_loop SceneCapture2D system to resolve, not this session.

## Actions Taken
- Queried all `DirectionalLight`, `PostProcessVolume`, and `SkyAtmosphere` actors in `MinPlayableMap`.
- Applied exposure/bloom clamp (see above).
- Checked existing dinosaur-labeled actors near the content hub (X=2100, Y=2400) using label lookup (per naming/dedup rule) before spawning anything new — avoided creating duplicate `_AI`/`_QuestArea` suffix clones.
- Spawned up to 3 new placeholder dinosaur actors (`Stego_Clareira_001`, `Raptor_Clareira_002`, `Raptor_Clareira_003`) ONLY if fewer than 5 dinosaur actors existed near the hub, using basic shape meshes (Cube/Cone) as procedural placeholders — no Meshy credits consumed.
- Added up to 4 additional tree placeholders (`Tree_Clareira_10x`) near the hub if tree count was below 15, to push toward the "dense vegetation, living Cretaceous forest" composition target.
- Saved the level (`EditorLevelLibrary.save_current_level()`).
- Ran a verification pass listing final counts of DirectionalLights, PostProcessVolumes (with their exposure/bloom values), dinosaur actors, and tree actors — logged to `/tmp/ue5_result_studio_director_verify.txt`.

## Technical Decisions & Justification
- Chose fixed Manual exposure over Auto Exposure because eye-adaptation was the primary suspect for cycle-over-cycle overexposure recurrence (3 cycles in a row reporting the same defect) — a fixed bias removes the variable entirely.
- Used basic shape meshes (Cube/Cone) for any new dinosaur/tree placeholders rather than calling `meshy_generate` or `generate_image`, consistent with budget discipline and the existing MinPlayableMap convention of "basic shapes as placeholders" per Rule 3.
- Enforced the naming/dedup rule strictly: performed label lookup before every spawn; skipped spawning if an actor with that exact label already existed; did not create subsystem-suffixed duplicates of existing dinosaurs.

## Dependencies / Next Steps for Other Agents
- **#08 Lighting & Atmosphere Agent**: verify the new Manual exposure/bloom values read well through Lumen GI at different times of day; tune warm Cretaceous color grading on top of this baseline.
- **#05/#06 World Generator / Environment Artist**: continue increasing terrain height variation and vegetation density at the hub (2100, 2400) — current fix only adds a handful of placeholder trees.
- **#09/#10 Character/Animation Agent**: replace Cube/Cone dinosaur placeholders with proper skeletal meshes and idle poses once available.
- **#18 QA**: re-run the vision_loop screenshot capture next cycle to confirm the overexposure defect is resolved before continuing with new features.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Clamped DirectionalLight intensity (3.5) + pitch (-45°) — fixes overexposure/glare.
- [UE5_CMD] Clamped PostProcessVolume AutoExposure (Manual, bias 0.5) + Bloom (0.4) — fixes wash-out.
- [UE5_CMD] Conditionally spawned up to 3 dinosaur placeholders + 4 tree placeholders near content hub (2100, 2400), with dedup/label-lookup safeguard.
- [UE5_CMD] Verification pass logging final light/volume/actor state to /tmp/ue5_result_studio_director_verify.txt.
- [FILE] Docs/CycleReports/Cycle_004_StudioDirector_ExposureFix.md — this report.
- [NEXT] #08 should confirm exposure fix via new screenshot; #05/#06 should keep growing terrain relief and vegetation density at the hub; #09/#10 should upgrade placeholder dinosaur meshes.
