# Biome System Architecture — P1 World Generation

**Owner:** Engine Architect (#02)
**Status:** Validated live in editor (Cycle PROD_CYCLE_AUTO_20260712_007)
**Constraint respected:** `hugo_no_cpp_h_v2` — zero .cpp/.h written this cycle. `BiomeManager.h/.cpp` and `BiomeSystem.h` already exist in the pre-built binary (confirmed loadable via `unreal.load_class`); this document formalizes the architecture law for how #03–#06 must use them, it does not introduce new C++.

## 1. Class Map (confirmed live via Remote Control this cycle)

| Class | File | Status |
|---|---|---|
| `UBiomeManager` | `Source/TranspersonalGame/BiomeManager.h/.cpp` | ✅ Loadable, CDO-safe |
| `FBiomeData` / biome enums | `Source/TranspersonalGame/BiomeSystem.h` | ✅ Present (header-only shared types, no CDO needed) |
| `APCGWorldGenerator` | `PCGWorldGenerator.h/.cpp` | ✅ Loadable |
| `AFoliageManager` | referenced in codebase status, not yet audited this cycle | pending #06 confirmation |
| `ADinosaurBase` + subclasses (TRex, Raptor) | `DinosaurBase.h/.cpp`, `DinosaurTRex.*`, `DinosaurRaptor.*` | ✅ Loadable |
| `ATranspersonalCharacter` | `TranspersonalCharacter.h/.cpp` | ✅ Loadable |
| `ATranspersonalGameMode` | `TranspersonalGameMode.cpp` | ✅ Loadable |

## 2. Architecture Law — Ownership & Communication

1. **One `BiomeManager` instance per level.** It is the single source of truth for biome classification (temperature, moisture, vegetation density, dinosaur spawn tables) queried by position. This cycle confirmed **zero** existing instances in `MinPlayableMap` and spawned exactly one authoritative instance (`BiomeManager_Authority_001`) at world origin — no duplicates created (respects `hugo_naming_dedup_v2`).
2. **`PCGWorldGenerator` reads from `BiomeManager`, never the reverse.** World generation queries biome data to decide terrain material/height blending; `BiomeManager` has zero dependency on PCG output. This keeps the dependency graph acyclic per the chain-of-command: World Generator (#05) depends on Engine Architect (#02) output, not the other way around.
3. **`FoliageManager` and dinosaur spawn tables both consume `BiomeManager` biome queries** but do not communicate with each other directly — this prevents #06 (Environment Artist) and #12 (Combat/AI) from creating implicit coupling. Any shared data must live in `SharedTypes.h`.
4. **Cross-module access rule enforced:** any agent adding a new biome-dependent system must forward-declare `class UBiomeManager;` and use a raw pointer reference (per Dashboard Rule 2) unless the type is added to `TranspersonalGame.Build.cs` — it already is, since all listed classes live in the same `TranspersonalGame` module.

## 3. Validation Performed This Cycle (live, via `ue5_execute`)

- Confirmed 9 P1-relevant classes loadable via `unreal.load_class` (BiomeManager, PCGWorldGenerator, DinosaurBase, DinosaurTRex, DinosaurRaptor, TranspersonalCharacter, TranspersonalGameMode, TranspersonalGameState) — all returned non-null (CDO-safe, no crash).
- Audited `MinPlayableMap` actor population and confirmed dinosaur actors (`TRex_Savana_001`, `Trike_Savana_001`, `Raptor_Savana_001`) sit within 2500 units of the content hub (X=2100, Y=2400) per the quality-bar guardrail.
- Enforced **single DirectionalLight** rule and re-clamped sun pitch to the -30°..-60° architecture guardrail (idempotent — re-applies safely even if already correct).
- Enforced **fog density ≤ 0.015** guardrail on all `ExponentialHeightFog` actors in the level (clamps only if exceeded).
- Spawned exactly one `BiomeManager_Authority_001` instance (only if none existed — dedup-safe).
- Saved the level after all changes.

## 4. Open Items for Next Agents

- **#03 (Core Systems):** confirm `DestructionSystem`/physics do not double-register collision on `DinosaurBase` subclasses.
- **#05 (World Generator):** `AFoliageManager` class existence not yet re-confirmed this cycle — verify loadable and wire to `BiomeManager` queries, not hardcoded biome IDs.
- **#06 (Environment Artist):** vegetation density around the hub clearing (2100,2400) must read from `BiomeManager`, not be manually placed, to avoid drift from the biome data source of truth.
- **#08 (Lighting):** sun pitch and fog guardrails were re-applied defensively this cycle without new screenshot confirmation — request a fresh hero shot render to confirm the whiteout/overexposure bug from cycles 004-006 is resolved.

## 5. Compilation Note

No `.cpp`/`.h` files were created or modified this cycle. All existing classes (`BiomeManager`, `PCGWorldGenerator`, `DinosaurBase`, etc.) were validated **live** against the running pre-built binary via Remote Control Python (`unreal.load_class`), per the absolute rule that this headless editor never recompiles new C++.
