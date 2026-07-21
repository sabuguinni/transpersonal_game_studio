# Milestone 1 "Walk Around" — Concrete Task List per Agent (Cycle 009)

Status check: MinPlayableMap exists with ground terrain, 12 trees, 6 rocks, 5 dinosaur placeholders (TRex, 3 Raptors, Brachiosaurus), Sun/Sky/Fog, PlayerStart, TranspersonalCharacter (movement+survival stats), TranspersonalGameMode. This list tracks what still needs REAL, VERIFIABLE ue5_execute output (not reports).

## #05 — Procedural World Generator
- [ ] MEASURABLE: Confirm terrain height variation exists across the whole playable area (not just near spawn) via `ue5_execute` landscape height sampling.
- [ ] Deliverable: python script output showing min/max Z across a grid sample of the landscape.

## #08 — Lighting & Atmosphere
- [ ] MEASURABLE: Capture a screenshot (via vision_loop or SceneCapture2D) confirming DirectionalLight pitch -45°/intensity 6.5 does not overexpose the hub at X=2100,Y=2400 (set this cycle — needs visual confirmation next cycle).
- [ ] Deliverable: report exact light intensity/pitch that reads well + before/after visual diff.

## #09/#10 — Character Artist / Animation
- [ ] MEASURABLE: Verify each of the 5 dinosaur placeholders has a valid collision capsule/box and an idle pose (no T-pose, no clipping into ground/vegetation) after this cycle's reposition of near-hub dinosaurs.
- [ ] Deliverable: ue5_execute audit listing each dino actor's collision component status.

## #12 — Combat & Enemy AI
- [ ] MEASURABLE: Implement/verify survival HUD (health/hunger/thirst/stamina bars) is bound to TranspersonalCharacter's existing survival stat variables and renders on screen.
- [ ] Deliverable: Blueprint widget reference + ue5_execute confirmation that HUD widget is added to viewport in MinPlayableMap.

## #18 — QA
- [ ] MEASURABLE: Confirm no duplicate actors exist at hub coordinates (per naming dedup rule) — run label uniqueness check across all actors within 1500 units of X=2100,Y=2400.

## Blocking Issue Flagged This Cycle
- Image generation pipeline (generate_image → Supabase storage) is returning HTTP 403 "Invalid Compact JWS" on upload. This blocks concept art and hero screenshot persistence. Needs infra-level fix outside agent scope — flagging to Miguel/orchestrator maintainers.
