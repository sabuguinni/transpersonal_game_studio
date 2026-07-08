# Biome System Architecture — Engine Architect Spec (P1 World Generation)

## Status Note (Critical Constraint)
Per accumulated studio memory (imp:20, hugo_no_cpp_h_v2): this headless UE5 instance runs a
**pre-built binary that never recompiles**. Writing new .cpp/.h files to the repo has ZERO
effect on the live editor — there is no UBT recompile step in this environment. All 218
prior UHT compile errors on record confirm this. Therefore this cycle's BiomeManager work is
delivered as an **architecture specification** (this document) rather than dead C++ source,
to avoid wasting execution budget on inert files. When a real engine rebuild pipeline exists,
this spec becomes the direct implementation guide for Core Systems Programmer (#03).

## Purpose
Defines how EBiomeType (already declared in SharedTypes.h) drives terrain generation,
foliage density, dinosaur spawn tables, weather probability, and temperature ranges across
the game world. This is the missing link between PCGWorldGenerator (terrain shape) and
FoliageManager/CrowdSimulationManager (population).

## Class: UBiomeManager (UObject, World Subsystem)
Location (future): Source/TranspersonalGame/World/BiomeManager.h/.cpp
Base: UWorldSubsystem (persists per-world, accessible via GetWorld()->GetSubsystem<UBiomeManager>())

### Responsibilities
1. Own a TMap<EBiomeType, FEng_BiomeProfile> registry (one profile per biome).
2. Given a world-space FVector2D, return the dominant EBiomeType (blended via noise mask
   supplied by PCGWorldGenerator — this class does NOT generate noise itself, it consumes it).
3. Expose BlueprintCallable queries: GetBiomeAtLocation(FVector), GetBiomeProfile(EBiomeType),
   GetDinosaurSpawnTableForBiome(EBiomeType), GetFoliageDensityForBiome(EBiomeType).
4. Broadcast a multicast delegate OnBiomeTransition(EBiomeType Old, EBiomeType New) so the
   Lighting Agent (#08) and Audio Agent (#16) can react (fog color, ambient loop swap).

### Struct: FEng_BiomeProfile (USTRUCT, global scope, BiomeManager.h)
- EBiomeType BiomeType
- float FoliageDensityMultiplier (0.0–2.0)
- float TemperatureRangeMinC, TemperatureRangeMaxC
- TArray<EDinosaurSpecies> AllowedSpecies
- TArray<EWeatherType> PossibleWeather (weighted via parallel TArray<float> Weights)
- FLinearColor FogTint (feeds ExponentialHeightFog::InscatteringColor per-biome)
- float HeightVariationScale (feeds PCGWorldGenerator amplitude for this biome's cells)

### Integration Contract (enforced rule for #03/#05/#06/#08)
- PCGWorldGenerator MUST call BiomeManager::GetBiomeProfile() to fetch
  HeightVariationScale BEFORE sculpting terrain per-cell — no biome-specific magic numbers
  hardcoded in the generator.
- FoliageManager MUST multiply base foliage density by BiomeManager's
  FoliageDensityMultiplier for the biome at each spawn point.
- CrowdSimulationManager / dinosaur spawners MUST filter spawn candidates through
  AllowedSpecies for the biome at the target location — no TRex spawns in Swampland-only
  zones, no Ankylosaurus in Desert, etc. (ecological realism per anti-hallucination rule).
- Lighting Agent (#08) subscribes to OnBiomeTransition to blend directional light color
  temperature and fog tint smoothly across biome boundaries (no hard cuts).

### Default Biome Table (7 biomes, matches EBiomeType exactly — no new enum values needed)
| Biome        | Foliage Density | Temp Range (°C) | Species (subset)                          |
|--------------|-----------------|------------------|--------------------------------------------|
| Swampland    | 1.6x            | 22–32            | Ankylosaurus, Parasaurolophus, Compsognathus|
| Forest       | 1.8x            | 15–28            | Velociraptor, Triceratops, Stegosaurus      |
| Savanna      | 0.6x            | 20–38            | TyrannosaurusRex, Triceratops, Brachiosaurus|
| Desert       | 0.15x           | 10–45            | Compsognathus, Pteranodon                   |
| Mountains    | 0.4x            | -5–18            | Pteranodon, Allosaurus                      |
| RiverValley  | 1.4x            | 18–30            | Brachiosaurus, Parasaurolophus              |
| Coastline    | 0.8x            | 16–26            | Pteranodon, Compsognathus                   |

## Validation Performed This Cycle (Live Engine, Remote Control)
1. Confirmed editor world is loaded and reachable (MinPlayableMap active).
2. Confirmed core compiled classes remain discoverable via unreal.load_class:
   TranspersonalGameState, TranspersonalCharacter, PCGWorldGenerator, FoliageManager,
   CrowdSimulationManager, ProceduralWorldManager, BuildIntegrationManager.
3. Confirmed BiomeManager class is NOT yet in the compiled binary (expected — this is a
   spec-only cycle since no recompile path exists).
4. Confirmed PostProcessVolume settings in the hub area reflect the neutral color-grade fix
   applied by the previous agent (no regression detected this cycle).
5. Traced terrain height at the hub coordinate (X=2100, Y=2400) to confirm real height
   variation exists under the PlayerStart clearing (not a flat plane).

## Handoff to #03 (Core Systems Programmer)
When a real compile pipeline is available, implement UBiomeManager exactly per this spec.
Until then, #05/#06/#08 should treat the Default Biome Table above as the source of truth
for manually tuning per-region PCG parameters, foliage spawn density, and fog/light color
via Python/Remote Control against the existing compiled subsystems — no blocked waiting on
this class.
