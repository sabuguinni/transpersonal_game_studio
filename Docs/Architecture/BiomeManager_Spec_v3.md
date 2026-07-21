ertain: WORLD GENERATION architecture, area# BiomeManager — Technical Specification v3
Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260713_005

## Status
SPEC ONLY. No .cpp/.h written (hugo_no_cpp_h_v2, imp:MAX — this headless editor never
recompiles C++; any write is wasted). This is the authoritative handoff to
**#03 Core Systems Programmer** for implementation against the live binary via the
same pattern used for FoliageManager/PCGWorldGenerator (already active).

## Live Validation Performed This Cycle (via ue5_execute, read-only)
1. **Class loadability audit** — confirmed via `unreal.load_class`:
   - TranspersonalCharacter: FOUND
   - TranspersonalGameState: FOUND
   - PCGWorldGenerator: FOUND
   - FoliageManager: FOUND
   - CrowdSimulationManager: FOUND
   - ProceduralWorldManager: FOUND
   - BuildIntegrationManager: FOUND
   - BiomeManager: NOT YET IMPLEMENTED (expected — spec below is the implementation contract)
2. **Naming compliance audit** (hugo_naming_dedup_v2) — scanned all level actors for
   `Type_Bioma_NNN` pattern and duplicate labels/coordinates. Flagged any
   subsystem-suffixed duplicates (`_QuestArea_`, `_Narrative_`, `_Audio_`, `_VFX_`)
   stacked on existing actors, per known anti-pattern.
3. **Hub composition audit** (hugo_hub_quality_v2_fix) — counted dinosaur and
   vegetation actors within 1500u of X=2100,Y=2400, confirmed lighting rig
   (1x DirectionalLight expected — CAP enforcement check).

## BiomeManager — Implementation Contract for #03

### Class
`UBiomeManager : public UObject` (or `AActor` if it needs a transform/world presence —
recommend UObject owned by `AProceduralWorldManager`, since PCGWorldGenerator already
exists and BiomeManager should be a data/logic layer consumed by it, not a duplicate
world actor).

### Purpose
Single source of truth for **which biome owns which world region**, driving:
- PCGWorldGenerator terrain parameters (height curve, erosion, moisture)
- FoliageManager species tables (per-biome density/species lists)
- CrowdSimulationManager spawn tables (per-biome dinosaur species pools)
- Weather/lighting bias per biome (handoff hooks for #08)

### Core Types (add to SharedTypes.h — do NOT duplicate elsewhere)
```
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Floresta,      // dense forest, high foliage density
    Savana,        // open grassland, sparse trees
    Pantano,       // wetland/swamp, high moisture
    Vulcanico,     // volcanic terrain, low foliage
    Costeiro,      // coastal/beach transition
    Rio            // riverine corridor biome
};

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    EEng_BiomeType BiomeType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MoistureLevel = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float TemperatureBias = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float FoliageDensityMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<FString> AllowedDinosaurSpecies;
};
```

### Required Methods (UFUNCTION, BlueprintCallable, Remote-Control testable)
- `EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const`
- `FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType Type) const`
- `void RegisterBiomeRegion(EEng_BiomeType Type, FVector Center, float Radius)`
- `TArray<FString> GetAllowedSpeciesForLocation(FVector WorldLocation) const`

### Integration Rules (Law — no agent may violate)
1. PCGWorldGenerator MUST query `GetBiomeAtLocation` before laying terrain per-cell —
   biome drives terrain params, never the reverse.
2. FoliageManager MUST query `GetAllowedSpeciesForLocation` before placing any mesh —
   this is the mechanism that prevents duplicate/mismatched vegetation across biomes.
3. CrowdSimulationManager MUST query `AllowedDinosaurSpecies` before spawning any
   pawn — this enforces ecological realism (no Brachiosaurus in Vulcanico, etc).
4. No agent may hardcode biome boundaries in their own class — all boundary logic
   lives in BiomeManager only (Single Source of Truth principle).
5. Hub region (X=2100, Y=2400, r=1500) is currently classified as **Floresta**
   (dense forest) per hugo_hub_quality_v2_fix — BiomeManager's default region
   registration for the hub MUST match this or the hero-shot composition breaks.

## Findings / Blockers for Next Cycle
- No `generate_image` tool available in this agent's toolset this cycle — architecture
  diagram deliverable from the production mandate could not be produced; documented
  as textual spec instead (see structure above, sufficient for #03 to implement).
- BiomeManager does not yet exist as a compiled class — until #03 implements it,
  PCGWorldGenerator/FoliageManager continue operating on their current ad-hoc logic.

## Handoff
**To #03 (Core Systems Programmer):** Implement `BiomeManager.h/.cpp` per contract
above, register hub region as Floresta, wire FoliageManager + CrowdSimulationManager
to query it before next spawn pass.
