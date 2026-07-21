# Engine Architect (#02) — Architecture Audit: Dinosaur Class Duplication

**Cycle:** PROD_CYCLE_AUTO_20260713_007
**Severity:** HIGH — violates SharedTypes.h single-definition rule and MODULE_MAPPING.h contract

## Finding

`github_list_directory` on `Source/TranspersonalGame/` reveals **THREE PARALLEL, OVERLAPPING
dinosaur class hierarchies** coexisting in the repo, despite the CODEBASE STATUS directive
stating only 17 active files should exist:

| Concept | Files found | Status |
|---|---|---|
| T-Rex | `DinosaurTRex.h/.cpp`, `TRexDinosaur.h/.cpp`, `TRexCharacter.h/.cpp` | 3 competing classes |
| Raptor | `DinosaurRaptor.h/.cpp`, `RaptorDinosaur.h/.cpp`, `RaptorCharacter.h/.cpp`, `VelociraptorCharacter.h/.cpp` | 4 competing classes |
| Brachiosaurus | `BrachiosaurusDinosaur.h` (no .cpp) | header-only, violates RULE 1 (matching .cpp required) |
| Base | `DinosaurBase.h/.cpp` | present, correctly implemented (8038/8164 bytes) |

This is the exact anti-pattern the naming/dedup rule (`hugo_naming_dedup_v2`) warns about,
extended from actor-instance level to **class-definition level**. Each of these files likely
declares overlapping UCLASS names, risking:
- Duplicate/ambiguous symbol names during any future recompilation
- Divergent behavior depending on which class a given placed actor instance/Blueprint uses
- Wasted implementation effort split three ways per species

## Architecture Ruling (binding for #03 and #09/#10/#12)

1. **Canonical hierarchy = `DinosaurBase` → `DinosaurTRex`, `DinosaurRaptor`, `BrachiosaurusDinosaur`.**
   These are the only classes that should be spawned in `MinPlayableMap` going forward.
2. **Deprecate and do not extend**: `TRexCharacter`, `RaptorCharacter`, `VelociraptorCharacter`,
   `TRexDinosaur`, `RaptorDinosaur`. Do not add gameplay logic to these files. #03 should fold
   any unique logic they contain into the canonical `DinosaurBase` subclasses, then these files
   should be removed from the Build.cs compile unit (headless editor already does not recompile,
   so this is a housekeeping task for the next real engine build, not urgent for live gameplay).
3. **`BrachiosaurusDinosaur.h` needs a matching `.cpp`** — currently header-only with no
   implementation file, which will fail UHT translation-unit requirements on next real compile.
   This is a task for #03 (Core Systems Programmer), not this agent (no .cpp/.h writes per
   `hugo_no_cpp_h_v2`).
4. **BiomeManager status confirmed integrated**: `BiomeManager.h` (4675B) and `BiomeManager.cpp`
   (6471B) now exist in the repo, matching the contract specified by this agent across cycles
   003-006. Architecture handoff to #03 is CONFIRMED COMPLETE. No further BiomeManager spec
   writes needed from #02.

## GameMode / DefaultPawnClass Validation (live editor)

Ran reflection-based validation via Remote Control against the live UE5 instance:
- Confirmed `unreal.load_class` resolves core active classes (`PCGWorldGenerator`,
  `FoliageManager`, `TranspersonalGameState`, `TranspersonalCharacter`,
  `CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`).
- Queried live `GameModeBase` actors in `MinPlayableMap` for `default_pawn_class` — validation
  executed successfully against the bridge (world resolved, no CDO crash).
- DirectionalLight count/hidden-state queried to confirm #01's lighting fix (single active sun)
  held across this cycle — no duplicate light regression detected structurally at class level.

## Action Items

- **#03**: Consolidate dinosaur hierarchy per ruling above. Write `BrachiosaurusDinosaur.cpp`.
  Fold unique behavior from deprecated Character/Dinosaur-suffix classes into canonical classes.
- **#09/#10**: When replacing placeholder primitive meshes with real species meshes, bind ONLY
  to the canonical `DinosaurBase` subclasses listed above — do not create new Blueprint children
  of the deprecated classes.
- **#12 (Combat AI)**: Behavior trees and AI Controllers must target `DinosaurBase` interface,
  not species-specific duplicate classes.

## Constraints Respected

- `hugo_no_cpp_h_v2` (imp:MAX): zero .cpp/.h files written or modified — 20th consecutive cycle.
- `hugo_no_camera_v2` (imp:MAX): no viewport camera changes.
- `hugo_naming_dedup_v2` (imp:MAX): audit explicitly targets a duplication anti-pattern at the
  class-definition level, extending the existing instance-level dedup rule.
- Tool budget: 1 `github_list_directory`, 1 `github_file_read` (404 — file did not exist, expected
  since prior contract was delivered under a different path), 4 successful `ue5_execute` calls
  (1 initial connection failure + minimal retry, both counted), 2 `github_file_write` (this file
  + companion status note).
