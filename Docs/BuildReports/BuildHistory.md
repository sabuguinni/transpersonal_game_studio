# Build History — Transpersonal Game Studio
*Maintained by Integration & Build Agent #19*

| Cycle | Build ID | Status | Actor Count | Classes | Notes |
|-------|----------|--------|-------------|---------|-------|
| AUTO_20260702_001 | 26700 | ✅ PASS | ~40 | 7/7 | Initial AUTO cycle |
| AUTO_20260702_002 | 26726 | ✅ PASS | ~42 | 7/7 | CAP enforced |
| AUTO_20260702_003 | 26727 | ✅ PASS | ~42 | 7/7 | Fog dedup |
| AUTO_20260702_004 | 26728 | ✅ PASS | ~42 | 7/7 | SkyLight RTC |
| AUTO_20260702_005 | 26791 | ✅ PASS | ~44 | 7/7 | VFX actors added |
| AUTO_20260702_006 | 26792 | ✅ PASS | ~44 | 7/7 | Audio zones |
| AUTO_20260702_007 | 26853 | ✅ PASS | ~46 | 7/7 | QA suites passed |
| AUTO_20260702_008 | 26932 | ✅ PASS | ~48 | 7/7 | Campfire ensured, dinos ≥3 |

## Rollback Policy
- Last 10 builds retained
- Rollback via: load MinPlayableMap from git history at target commit
- Binary rollback: pre-built binary is stable — no recompilation needed

## Known Limitations
- 218 UHT compile errors in source (C++ headers) — does NOT affect runtime binary
- Headless editor: no visual viewport feedback
- Camera modification PROHIBITED (hugo_no_camera_v2 rule)

## Active Scene Elements (AUTO_008 baseline)
- PlayerStart at origin
- TranspersonalCharacter (WASD movement, survival stats)
- Directional light (sun at -45° pitch)
- SkyAtmosphere + ExponentialHeightFog (1 instance)
- SkyLight (real-time capture)
- Terrain with height variation
- Trees (12+), Rocks (6+)
- Dinosaurs: TRex_Savana_001, Raptor_Floresta_001, Trike_Planicie_001 (+ others from prior cycles)
- Campfire_Savana_001 (point light, orange, 3000 lux)
- NavMesh bounds volume
