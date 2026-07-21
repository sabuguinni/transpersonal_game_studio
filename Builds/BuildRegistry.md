# Build Registry — Transpersonal Game Studio
*Maintained by Integration & Build Agent #19*  
*Last updated: Cycle AUTO_20260701_002*

---

## Active Build Baseline

| Property | Value |
|----------|-------|
| C++ Classes Loaded | 134 |
| Functional Tests PASS | 45 |
| Integration Scenarios | 20 |
| Core Active Files | 17 |
| MinPlayableMap Actors | ~35 (varies by cycle) |
| Build Status | 🟢 GREEN |

---

## Build History (Last 10)

| # | Cycle ID | Date | Status | Notes |
|---|----------|------|--------|-------|
| 10 | AUTO_20260701_002 | 2026-07-01 | 🟢 GREEN | Campfire integrated, CAP enforced, 7/7 classes |
| 9 | AUTO_20260701_001 | 2026-07-01 | 🟢 GREEN | QA pass, VFX campfire placeholder placed |
| 8 | AUTO_20260630_011 | 2026-06-30 | 🟢 GREEN | 7/7 classes, binary audit, source pairing verified |
| 7 | AUTO_20260630_010 | 2026-06-30 | 🟢 GREEN | CAP enforced, NavMesh verified |
| 6 | AUTO_20260630_009 | 2026-06-30 | 🟢 GREEN | Baseline established |
| 5 | AUTO_20260630_008 | 2026-06-30 | 🟢 GREEN | Core systems stable |
| 4 | AUTO_20260630_007 | 2026-06-30 | 🟢 GREEN | MinPlayableMap created |
| 3 | AUTO_20260630_006 | 2026-06-30 | 🟢 GREEN | Character movement verified |
| 2 | AUTO_20260630_005 | 2026-06-30 | 🟢 GREEN | Lighting setup complete |
| 1 | AUTO_20260630_004 | 2026-06-30 | 🟢 GREEN | Initial integration |

---

## Rollback Instructions

To roll back to any previous build:
1. Identify the target cycle ID from the table above
2. Check `Builds/Reports/Integration_Report_<CYCLE_ID>.md` for the exact actor/class state
3. Restore MinPlayableMap from the corresponding save in `/Game/Maps/`
4. Verify 7 core classes still load via `unreal.load_class(None, '/Script/TranspersonalGame.<ClassName>')`

---

## Active Source Files (DO NOT DELETE)

| File | Purpose |
|------|---------|
| TranspersonalGame.cpp | Module registration |
| TranspersonalGame.h | Module header |
| TranspersonalGameState.h/.cpp | Core game state (35 properties) |
| TranspersonalCharacter.h/.cpp | Player character (38 properties) |
| PCGWorldGenerator.h/.cpp | Procedural world generation (14 methods) |
| FoliageManager.h/.cpp | Vegetation system (5 methods) |
| CrowdSimulationManager.h/.cpp | Crowd AI |
| ProceduralWorldManager.h/.cpp | World management |
| BuildIntegrationManager.h/.cpp | Build integration |
| SharedTypes.h | Shared enums and structs (22 types) |
| ConstructorStubs.cpp | CDO stubs |
| LinkerStubs.cpp | Linker stubs |

---

## CAP Enforcement Checklist (Every Cycle)

- [ ] Sun DirectionalLight pitch ≤ -30° (target: -45°)
- [ ] ExponentialHeightFog: exactly 1 instance
- [ ] `r.SkyAtmosphere.FastSkyLUT 1` applied
- [ ] SkyLight real_time_capture = true
- [ ] Map saved after all changes

---

*Integration & Build Agent #19 — Transpersonal Game Studio*  
*"A build that doesn't compile is not a build — it's a wish."*
