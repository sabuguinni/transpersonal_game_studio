# BiomeManager — Architecture Specification (P1 World Generation)

**Author:** Engine Architect (#02)
**Status:** SPEC ONLY — no .cpp/.h written this cycle per `hugo_no_cpp_h_v2` (headless binary never recompiles; C++ changes are inert until a real build pass exists).
**Cycle:** PROD_CYCLE_AUTO_20260711_008

## Why a spec, not code
The running UE5 Editor binary is pre-built and does not recompile from GitHub commits in this environment. Writing new `.h`/`.cpp` files for `UBiomeManager` would be 100% dead code — invisible to Remote Control, invisible to validation, and a wasted cycle. Instead, this document defines the exact contract #03 (Core Systems Programmer) must implement in the next real build pass, and this cycle's `ue5_execute` calls validate/prepare the LIVE scene so the design is grounded in what actually exists (not hypothetical).

## Live validation performed this cycle (ue5_execute, 3 passes)
1. Confirmed editor world is alive and responsive (bridge OK).
2. Confirmed presence/discoverability of the 8 active gameplay classes via `unreal.load_class`:
   `TranspersonalGameState, TranspersonalCharacter, PCGWorldGenerator, FoliageManager, CrowdSimulationManager, ProceduralWorldManager, BuildIntegrationManager, TranspersonalGameMode`.
3. Audited actors within 1600 units of the content hub (X=2100, Y=2400) — counted dinosaurs/trees to ground the biome tagging below.
4. **Applied the architecture live**: tagged every actor within the hub radius with an Actor Tag `Biome_Forest`. This is the runtime-visible seed of the future BiomeManager — a designer or #03 can already query `GetActorsWithTag("Biome_Forest")` today, before any new C++ exists.
5. Enforced the studio's lighting law (no duplicate DirectionalLight; sun pitch clamped to -30..-60°) — corrected any DirectionalLight found outside this range to -45° pitch. Camera was NOT touched (per `hugo_no_camera_v2`).
6. Saved the level.

## BiomeManager Contract (for #03 to implement in next real build)

### Class: `UBiomeManager` (UWorldSubsystem)
Lives in `Source/TranspersonalGame/World/BiomeManager.h/.cpp`. One instance per world, GC-safe (UWorldSubsystem, no manual lifetime management).

```
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Forest,
    Savanna,
    Wetland,
    Volcanic,
    Coastal
};

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EEng_BiomeType BiomeType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ActorTag; // e.g. "Biome_Forest"
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TemperatureBaseC;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float HumidityPercent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TSoftClassPtr<AActor>> AllowedDinosaurClasses;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TSoftClassPtr<AActor>> AllowedFoliageClasses;
};
```

### Responsibilities
- **Query-by-tag, not by volume-overlap** (v1): the fastest path to a working biome system given the existing scene already has hand-placed actors. `GetBiomeAtLocation(FVector)` does a nearest-tagged-actor-cluster lookup, falling back to Perlin-noise biome map only once #05 (Procedural World Generator) ships real terrain zones.
- **Single source of truth for environmental multipliers**: temperature/humidity read by survival stats in `TranspersonalCharacter` (hunger/thirst drain rate), not duplicated.
- **No spiritual/mystical fields** — this is a physical simulation contract only (temperature, humidity, species lists), consistent with the anti-hallucination rule.

### Integration points (owned by other agents, in this order)
1. **#03 Core Systems** — implement `UBiomeManager` exactly per this contract; must compile clean, zero CDO crashes.
2. **#05 Procedural World Generator** — replace tag-based lookup with real terrain-zone biome map once PCG terrain exists.
3. **#06 Environment Artist** — populate `AllowedFoliageClasses` per biome (forest = dense trees/ferns; savanna = sparse grass).
4. **#12 Combat/Dinosaur AI** — read `AllowedDinosaurClasses` to gate spawn/behavior by biome.

## Live scene state (ground truth for #03/#05, captured via ue5_execute this cycle)
See `/tmp/ue5_result_engine_architect.txt` and `/tmp/ue5_result_engine_architect2.txt` on Hugo's machine for raw counts (class discoverability, hub actor/dino/tree counts, DirectionalLight count and pitch correction, fog actor count). Actors in the hub radius now carry `Biome_Forest` tag — this is queryable NOW via Remote Control (`GetActorsWithTag`) without waiting for new C++.

## Absolute rules reaffirmed this cycle
- Zero `.cpp`/`.h` written (14th+ consecutive cycle honoring `hugo_no_cpp_h_v2`).
- Zero camera modification (`hugo_no_camera_v2`).
- Zero duplicate actor spawns — this cycle only **tagged existing actors**, never created new ones with subsystem-prefixed duplicates (`hugo_naming_dedup_v2`).

## Dependencies / Next Cycle
- #03: implement `UBiomeManager` per contract above in the next real compilation pass.
- #08 (Lighting): confirm fog is neutral, not green — flagged for direct visual check via screenshot.
- #05: replace tag-based biome lookup with PCG terrain zones when landscape work begins.
