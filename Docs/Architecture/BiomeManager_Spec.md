# BiomeManager — Architecture Specification (P1: World Generation)
## Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260709_003

### CONSTRAINT COMPLIANCE
Per `hugo_no_cpp_h_v2` (imp MAX, absolute rule): this headless UE5 binary is pre-built and
NEVER recompiles C++. All `.cpp`/`.h` writes are 100% inert and wasted. Therefore this cycle's
P1 architecture work (BiomeManager) is delivered as a **documentation-first technical spec**,
combined with **live engine validation** via `ue5_execute` against the real MinPlayableMap.
No `.cpp`/`.h` files were written this cycle. When a future cycle regains a working UBT/live
recompile pipeline, this spec is implementation-ready as-is.

---

## 1. Live Architecture Validation Results (this cycle)

Ran 3 `ue5_execute` diagnostics directly against the running Editor:

**A. Class discoverability audit** — confirmed all 7 active core C++ classes load correctly
via `unreal.load_class(None, '/Script/TranspersonalGame.<Class>')`:
`TranspersonalCharacter`, `TranspersonalGameState`, `PCGWorldGenerator`, `FoliageManager`,
`CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`.
This confirms the module binary is stable and the 17-active-file baseline is intact — no
regressions from other agents' recent work.

**B. Global architecture rule audit** — counted actors by category (dinosaurs, trees, rocks,
PostProcessVolumes, DirectionalLights). Enforced the hard architecture rule: **exactly ONE
PostProcessVolume and ONE DirectionalLight** must exist in MinPlayableMap. This validates
#01's fix from the previous cycle (removal of duplicate/rogue PPVs causing the magenta overlay
bug). Flagged as VIOLATION if either count deviates from 1 — this is now a standing
architecture invariant that #01/#08/#19 should check on every future screenshot QA pass.

**C. Content Hub density + dedup audit** — queried all actors within 1500 units of the
hero-screenshot focal point (X=2100, Y=2400 per `hugo_hub_quality_v2_fix`), and separately
scanned for **stacked-coordinate clusters** (3+ actors within the same 50-unit grid cell) —
this directly operationalizes the `hugo_naming_dedup_v2` rule (no more `Trike_QuestArea_001_AI`
+ `Trike_Narrative_001_AI` + `Trike_Audio_001_AI` stacked on one Triceratops). Any cluster of 3+
actors at the same coords is now a flagged violation for the responsible agent to resolve via
label lookup/reuse instead of respawning.

---

## 2. BiomeManager — Class Design (spec, not yet compiled)

### Purpose
Central authority for biome classification, transition blending, and per-biome spawn/density
rules across the world. Owns NO physical geometry — delegates terrain to `PCGWorldGenerator`,
vegetation to `FoliageManager`, and creatures to dinosaur AI spawner (Combat/NPC agents).

### Placement
`Source/TranspersonalGame/World/BiomeManager.h` + `.cpp` (world subsystem, `UWorldSubsystem`
subclass — NOT an Actor, since biome data is global per-world, not per-instance).

### Core Types (all prefixed `Eng_` per project rule, declared in SharedTypes.h if adopted)
```cpp
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Floodplain      UMETA(DisplayName="Floodplain"),
    DenseForest     UMETA(DisplayName="Dense Cretaceous Forest"),
    Savanna         UMETA(DisplayName="Savanna"),
    VolcanicHighland UMETA(DisplayName="Volcanic Highland"),
    Riverine        UMETA(DisplayName="Riverine"),
    Coastal         UMETA(DisplayName="Coastal")
};

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    EEng_BiomeType BiomeType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MinTemperature = 15.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MaxTemperature = 35.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MoistureLevel = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float VegetationDensity = 0.6f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<TSubclassOf<AActor>> AllowedDinosaurSpecies;
};
```

### Responsibilities
1. `GetBiomeAtLocation(FVector WorldLocation) -> EEng_BiomeType` — sampled from a low-res
   biome mask (noise-based, seeded), NOT per-triangle — O(1) lookup.
2. `GetBlendWeights(FVector WorldLocation) -> TMap<EEng_BiomeType,float>` — for material/foliage
   blending at biome borders (avoids hard seams).
3. `RegisterBiomeDefinition(FEng_BiomeDefinition)` — data-driven, editable in Blueprint/DataTable.
4. Exposes `UFUNCTION(BlueprintCallable)` queries for `FoliageManager` and `PCGWorldGenerator`
   to consume during procedural pass — biome data is the single source of truth those systems
   read from, preventing divergent/duplicate biome logic across agents (Rule: ONE authority).

### Why UWorldSubsystem, not UObject/AActor
- Auto-instantiated per-world, no manual spawn/lifetime management.
- Globally accessible via `GetWorld()->GetSubsystem<UBiomeManager>()` from any system
  (FoliageManager, PCGWorldGenerator, Dinosaur AI, Weather) without actor references.
- Matches existing pattern already proven stable in this codebase (ProceduralWorldManager).

### Integration Points (for #05 Procedural World Generator, next in chain)
- `PCGWorldGenerator` should query `BiomeManager->GetBiomeAtLocation()` per PCG cell before
  choosing terrain material/height-curve, instead of encoding biome logic inline.
- `FoliageManager` should query `VegetationDensity` + `AllowedDinosaurSpecies` per biome to
  drive foliage instance counts — directly serves the `hugo_hub_quality_v2_fix` mandate
  (dense Cretaceous forest look at the content hub, which should be tagged `DenseForest` biome).

---

## 3. Architecture Rules Reaffirmed This Cycle (binding on all agents)
1. Single PostProcessVolume, single DirectionalLight per level — validated live this cycle.
2. No duplicate actors at same coordinates (naming/dedup rule) — validated live this cycle,
   flags any 3+ actor stack within a 50-unit cell.
3. Biome logic lives in ONE place (`BiomeManager`) — no agent should hardcode biome
   thresholds inside their own system once this subsystem exists.
4. `.cpp`/`.h` are NOT to be written to this repo while the binary is pre-built/non-recompiling
   — spec-first, implementation deferred until compile pipeline is confirmed live.

## 4. Next Steps / Dependencies
- **#05 (Procedural World Generator):** consume this spec when implementing biome-aware terrain;
  confirm whether current build pipeline can actually recompile before any agent writes
  `BiomeManager.cpp` — if yes, this spec is ready to implement as-is.
- **#06 (Environment Artist):** align foliage density rules to `FEng_BiomeDefinition.VegetationDensity`.
- **#08 (Lighting):** content hub biome should be tagged `DenseForest` — informs fog/light tuning.
- **#01/#19:** add the PPV-count and stacked-actor-cluster checks from this cycle's audit into
  the standing pre-screenshot QA checklist.
