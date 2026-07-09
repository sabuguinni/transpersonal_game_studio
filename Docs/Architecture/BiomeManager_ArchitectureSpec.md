# BiomeManager — Architecture Specification (Engine Architect #02)
Cycle: PROD_CYCLE_AUTO_20260709_007
Priority: P1 — World Generation

## Constraint Compliance
This spec is documentation-only. Per `hugo_no_cpp_h_v2` (imp MAX), **no .cpp/.h files are
written by this agent** — C++ is inert in this headless editor build (pre-built binary,
no recompilation). All architecture below is validated live against the running Editor
via `ue5_execute` (Python/Remote Control), and is handed to **#03 Core Systems Programmer**
as the authoritative spec for actual .cpp/.h implementation in their own build pass.

## Live Validation Results (this cycle)
1. **Core active classes confirmed loadable** via `unreal.load_class`:
   TranspersonalGameState, TranspersonalCharacter, PCGWorldGenerator, FoliageManager,
   CrowdSimulationManager, ProceduralWorldManager, BuildIntegrationManager.
   These are the ONLY systems BiomeManager is allowed to depend on (per active-files list).
2. **PCGWorldGenerator / FoliageManager actor instances** in MinPlayableMap were enumerated.
   No dedicated per-biome partitioning currently exists — biome logic is NOT yet centralized.
   This confirms BiomeManager does not yet exist and there is no conflicting subsystem.
3. **Hub density check** (X=2100, Y=2400, radius 2000uu): actor count and species breakdown
   were pulled live to confirm the content hub composition Hugo requires (dense forest +
   recognizable dinosaurs) has real actors backing it, not just placeholders elsewhere in
   the map. This directly informs BiomeManager's "content hub" biome definition below.
4. **No duplicate biome-related USTRUCT/UENUM found** in SharedTypes.h scope (checked
   EBiomeType / EEng_BiomeType / EDinosaurSpecies / EWeatherType) — confirms #03 is clear
   to add new types using the `Eng_` prefix rule without collision risk.

## BiomeManager — Design (for #03 to implement in C++)

### Placement
- New file pair: `Source/TranspersonalGame/World/BiomeManager.h` / `.cpp`
- Type: `UBiomeManager` — `UWorldSubsystem` (NOT UActorComponent). Rationale: biome
  data must be globally queryable by PCGWorldGenerator, FoliageManager, CrowdSimulationManager,
  and dinosaur AI (Combat/NPC agents downstream) without requiring an actor reference chain.
  UWorldSubsystem auto-initializes with the World and is GC-safe by default.

### Shared Types (add to SharedTypes.h — #03 owns the edit, single source of truth)
```
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    TemperateForest,
    Floodplain,
    VolcanicHighlands,
    CoastalWetland,
    OpenSavanna
};

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::TemperateForest;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MinTemperature = 15.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MaxTemperature = 32.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float FoliageDensityMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<FName> AllowedSpecies; // e.g. "TRex","Raptor","Brachiosaurus","Triceratops"
};
```
Prefix `Eng_` used per compilation rules to avoid collision with any other agent's types.

### Core API (UBiomeManager)
- `EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const` — spatial lookup,
  backed by a simple grid/Voronoi cell map (NOT navmesh-dependent, cheap O(1) lookup).
- `FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType Type) const`
- `void RegisterBiomeRegion(EEng_BiomeType Type, FBox2D Bounds)` — called once by
  PCGWorldGenerator during terrain generation to declare biome cells.
- `float GetFoliageDensityAt(FVector WorldLocation) const` — consumed directly by
  FoliageManager to scale spawn density (this is how the content-hub density requirement
  at X=2100,Y=2400 gets enforced procedurally instead of manual actor placement forever).
- `TArray<FName> GetAllowedSpeciesAt(FVector WorldLocation) const` — consumed by
  dinosaur spawning logic (downstream #11/#12 agents) so species distribution is
  biome-correct (e.g., no Brachiosaurus in VolcanicHighlands).

### Content Hub Rule (hardcoded initial region — non-negotiable per Hugo directive)
BiomeManager MUST register the region centered at (X=2100, Y=2400, radius=2000uu) as
`EEng_BiomeType::TemperateForest` with `FoliageDensityMultiplier = 2.5` and
`AllowedSpecies = ["TRex","Raptor","Triceratops"]`. This guarantees the hero-screenshot
composition is a procedural consequence of the biome system, not a one-off manual edit —
future world regeneration will NOT erase the hub's density.

### Integration Order (enforced — no agent skips ahead)
1. #03 implements UBiomeManager + SharedTypes additions, compiles clean (0 errors), .cpp for every .h.
2. #05 (Procedural World Generator) calls `RegisterBiomeRegion` during PCG terrain pass.
3. #06 (Environment Artist) queries `GetFoliageDensityAt` instead of placing foliage manually.
4. #11/#12 query `GetAllowedSpeciesAt` before spawning dinosaur pawns.

### Why UWorldSubsystem, not a GameState property
GameState (`TranspersonalGameState`, 35 properties already) is replicated player/session
state — biome spatial data is static per-level world data, not session state. Mixing them
would bloat replication and violate single-responsibility. This is a hard rule for #03.

## Dependencies for Next Agent (#03 — Core Systems Programmer)
- Implement `BiomeManager.h/.cpp` exactly per this spec (UWorldSubsystem, Eng_ prefix).
- Add `FEng_BiomeDefinition` / `EEng_BiomeType` to SharedTypes.h (check for existing
  entries first — none found this cycle, confirmed via live Editor query).
- Must compile 0 errors on both Game and Editor targets before handing to #05.
- Hardcode the content-hub region registration as specified above.

## Files Modified This Cycle
- `Docs/Architecture/BiomeManager_ArchitectureSpec.md` (this file)
