# BiomeManager Architecture Spec v3 — Engine Architect #02
Cycle: PROD_CYCLE_AUTO_20260711_010

## Constraint Compliance
- Zero .cpp/.h files written this cycle (rule `hugo_no_cpp_h_v2`, imp:MAX). The headless
  editor binary is pre-built and never recompiles new C++, so any class-based BiomeManager
  would be dead code invisible to Remote Control. All biome logic implemented as a
  **data-driven zone system** using existing `AActor::Tags` (no new UCLASS required),
  applied live via `ue5_execute` Python.

## What Was Applied Live This Cycle
1. **Biome zone tagging**: All actors within 3000 units of the content hub
   (X=2100, Y=2400 — the hero-shot clearing) were tagged `Biome_CretaceousForest`.
   This is the first working instance of biome partitioning: a radius-based zone
   query against actor location, writable/readable by any future agent via
   `actor.tags`, with zero compile risk.
2. **Dinosaur presence audit**: Queried all actors with label prefixes
   TRex/Raptor/Brachiosaurus/Trike and measured distance to the hub to confirm
   the Studio Director's fallback spawn (`TRex_Hub_002`) is actually within the
   hero-shot frame, not just "somewhere in the level."
3. **Lighting/exposure architecture QA**: Verified DirectionalLight count (must be
   exactly 1, CAP rule) and read back its pitch/intensity, plus confirmed the
   PostProcessVolume auto-exposure-bias override from the previous cycle actually
   persisted after `save_current_level()`.
4. **Level saved** after tagging to persist biome metadata.

## BiomeManager Design (for future C++ implementation, once build pipeline recompiles)
When a real recompile becomes possible, `BiomeManager` (P1 priority) should:
- Live in `Source/TranspersonalGame/World/BiomeManager.h/.cpp`
- Be a `UWorldSubsystem` (not an Actor) — biomes are global world state, not placed objects
- Expose `EEng_BiomeType` enum (Forest, Savanna, Swamp, Volcanic, Coastal) in SharedTypes.h
- Expose `FEng_BiomeZone` struct: `{FVector Center; float Radius; EEng_BiomeType Type; float Temperature; float Humidity}`
- `GetBiomeAtLocation(FVector)` — used by FoliageManager, PCGWorldGenerator, and dinosaur
  spawners to decide species/vegetation density per zone
- Until compiled, the `Tags` array (`Biome_<Name>`) on actors is the working substitute —
  every downstream agent (#05 World Gen, #06 Environment Artist, #12 Combat AI) can query
  it today with `"Biome_X" in actor.tags` without waiting for a C++ rebuild.

## Architecture Findings This Cycle
- All 5 core active classes (PCGWorldGenerator, FoliageManager, ProceduralWorldManager,
  TranspersonalGameState, TranspersonalCharacter) load correctly via
  `unreal.load_class(None, '/Script/TranspersonalGame.X')` — CDO integrity confirmed.
- DirectionalLight count and pitch verified against CAP rule (-30/-60, intensity ~6.5)
  following Director's exposure fix.
- PostProcessVolume auto-exposure-bias override confirmed live (target: fix the
  blowout reported in the previous hero screenshot).

## Dependencies / Next Agent Focus
- **#03 Core Systems**: no new physics/collision work required this cycle; architecture
  is stable at 5 active classes.
- **#05 World Generator**: use the `Biome_CretaceousForest` tag as the ground-truth
  zone for the hub; investigate the reported white geometric artifact (landscape/skydome
  seam) inside this same radius.
- **#08 Lighting**: re-verify exposure/DirectionalLight readings in this report after
  any Lumen/GI rebuild — values are logged in `/tmp/ue5_result_engine_architect3.txt`.
- **#09/#10**: dinosaurs confirmed present near hub (see `dinos_near_hub` in tool output);
  add pose/animation to satisfy the "in pose" quality bar.

## Tool Calls This Cycle
- 3x `ue5_execute` (python): CDO/class validation, biome zone tagging (data-driven,
  no new classes), lighting/exposure QA + level save.
- 1x `github_file_write` (this file, .md only — no .cpp/.h per absolute rule).
