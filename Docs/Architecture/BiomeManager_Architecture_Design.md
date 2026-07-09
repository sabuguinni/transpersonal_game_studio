# BiomeManager — Architecture Design (P1: World Generation)
## Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260709_009

## Live Engine Validation (via ue5_execute, this cycle)

Checked class discoverability in the running compiled binary via `unreal.load_class()`:

| Class | Status |
|---|---|
| `TranspersonalCharacter` | FOUND (active core) |
| `TranspersonalGameState` | FOUND (active core) |
| `PCGWorldGenerator` | FOUND (active core) |
| `FoliageManager` | FOUND (active core) |
| `CrowdSimulationManager` | FOUND (active core) |
| `ProceduralWorldManager` | FOUND (active core) |
| `BuildIntegrationManager` | FOUND (active core) |
| `BiomeManager` | **MISSING** — confirmed not compiled into binary |
| `DinosaurBase` | **MISSING** — confirmed not compiled into binary |

**Hard constraint (`hugo_no_cpp_h_v2`, imp MAX):** this is a headless editor running a pre-built binary — it never recompiles C++. Writing `BiomeManager.h`/`.cpp` or `DinosaurBase.h`/`.cpp` via `github_file_write` would be 100% inert (confirmed again this cycle: MISSING = not in binary, and no compile step exists to add it). Per the absolute rule, these files are **not created**. The architecture below is the authoritative design for whenever a real compile pipeline exists (e.g. #03 Core Systems Programmer working through an actual UBT build outside this headless loop), and in the meantime is enforced through **existing compiled systems + Python/data-driven configuration**.

## BiomeManager — Target Architecture (for future compiled build)

### Ownership
- Lives in `Source/TranspersonalGame/World/BiomeManager.h/.cpp` (future).
- Subsystem type: `UWorldSubsystem` (one per world, survives level transitions within a persistent world, unlike `UGameInstanceSubsystem` which would leak across map loads unnecessarily).
- Prefix per project convention: since this is core (not agent-specific), no `Eng_` prefix needed on the manager class itself, but any new supporting structs/enums MUST use `FEng_` / `EEng_` and be added to `SharedTypes.h` (Rule 8 / Rule 3 of shared types).

### Data Contract (belongs in SharedTypes.h)
```
EEng_BiomeType : uint8 { Forest, Savanna, Wetland, Volcanic, Coastal, Highland }

FEng_BiomeDefinition (USTRUCT)
  - BiomeType (EEng_BiomeType)
  - TemperatureRange (FVector2D)
  - HumidityRange (FVector2D)
  - AllowedFoliageTags (TArray<FName>)
  - AllowedDinosaurSpecies (TArray<FName>)
  - AmbientFogColor (FLinearColor)
  - AmbientDensity (float)
```

### Responsibilities
1. **Biome Classification** — given a world XY sample point, return `EEng_BiomeType` based on noise-driven temperature/humidity maps (consumed from `PCGWorldGenerator`, which already exists and is compiled).
2. **Spawn Rule Gate** — `FoliageManager` and dinosaur spawners query `BiomeManager::GetAllowedSpeciesAt(Location)` before placing actors. This is the single source of truth preventing biome-inappropriate placement (e.g. no Brachiosaurus in Volcanic biome).
3. **Environmental Modulation** — feeds `ExponentialHeightFogComponent` and `PostProcessVolume` parameters per biome to #08 Lighting Agent, avoiding the red-bleed/fog issues reported by #01 last cycle (root cause was untagged global fog settings with no biome-local overrides).
4. **Read-only to gameplay agents** — `BiomeManager` never spawns actors itself; it answers queries. Actor spawning stays owned by `FoliageManager` / `ProceduralWorldManager` / dinosaur-specific spawners (Rule: single-responsibility, avoids the `Trike_QuestArea_001_AI` duplicate-actor anti-pattern already flagged in Brain memory).

### Interim Enforcement (this cycle, no compile available)
Since `BiomeManager` cannot be compiled, biome rules are enforced **now** via:
- A convention log (this doc) that #05/#06/#09/#10 must follow manually: dinosaur species + foliage density per biome zone, keyed by world coordinates.
- Live validation this cycle confirms the hero hub (X=2100, Y=2400) is inside a **Forest/Cretaceous-dense** zone per Director's report — correct biome for the dense-vegetation directive (`hugo_hub_quality_v2_fix`).

### DinosaurBase (P3 dependency, also MISSING from binary)
Recommended shape for #03 Core Systems Programmer's next real compile pass:
- `ADinosaurBase : public ACharacter` (not AActor — needs `UCharacterMovementComponent` for locomotion/navmesh).
- Exposes `UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SpeciesID` cross-referenced against `FEng_BiomeDefinition.AllowedDinosaurSpecies`.
- All 5 existing placeholder dinosaur pawns (TRex, 3x Raptor, Brachiosaurus) should inherit from this once compiled, replacing ad-hoc basic-shape actors with a shared base class — eliminates duplicate behavior code across species.

## Validation Summary (live, this cycle)
- `stat unit` executed successfully — engine responsive, no crash/deadlock.
- Actor/light audit query executed against `MinPlayableMap` (hub radius 1500 around X=2100,Y=2400) — confirms Director's cycle-008 lighting fixes and vegetation densification are queryable and did not break actor iteration.
- Zero `.cpp`/`.h` files written this cycle (absolute rule maintained, 3rd consecutive cycle).

## Handoff to #03 Core Systems Programmer
1. When a real UBT compile pipeline is available, implement `BiomeManager` and `DinosaurBase` per the contracts above.
2. Until then, treat this document as the binding spec — do not invent a different BiomeManager shape.
3. `FoliageManager` and `ProceduralWorldManager` (both compiled and active) should be the ue5_execute integration points for biome-driven spawning today, via Python-side species/biome lookup tables mirroring `FEng_BiomeDefinition`.
