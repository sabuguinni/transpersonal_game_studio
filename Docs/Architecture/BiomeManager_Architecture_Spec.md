# BiomeManager Architecture Specification — P1 World Generation
## Engine Architect (#02) — PROD_CYCLE_AUTO_20260710_004

## CONSTRAINT COMPLIANCE
Per `hugo_no_cpp_h_v2` (imp:MAX, GLOBAL): **zero .cpp/.h files written this cycle**. This headless
editor runs a pre-built binary that never recompiles — any C++ write would be committed but 100%
inert on the live game. This spec is the AUTHORITATIVE DESIGN for whichever future cycle regains
a build pipeline capable of compiling new C++ (or for #03 Core Systems Programmer if a build step
exists outside this session). All architecture validation this cycle was performed live against
the running UE5 instance via `ue5_execute` (3 python calls).

## LIVE VALIDATION RESULTS THIS CYCLE
1. **Class registry check** — confirmed via `unreal.load_class`: PCGWorldGenerator, FoliageManager,
   ProceduralWorldManager, TranspersonalGameState, TranspersonalCharacter, CrowdSimulationManager,
   BuildIntegrationManager all present in the loaded module (matches CODEBASE STATUS active-file list).
   `BiomeManager` class does **NOT** yet exist in the compiled binary — confirms it is still design-only.
2. **Content hub inspection (X=2100, Y=2400)** — enumerated all actors within 900uu radius, captured
   DirectionalLight/Fog/SkyAtmosphere/PostProcessVolume actors and checked for duplicate labels
   (naming-rule compliance per `hugo_naming_dedup_v2`). No duplicate Type_Bioma_NNN labels found this
   pass — good, no repeat of the Trike_*_AI stacking anti-pattern from prior cycles.
3. **CAP enforcement pass** — any DirectionalLight outside the mandated pitch range (-30 to -60) was
   corrected to -45°. All actors inside the content-hub radius were tagged `BiomeHub_Cretaceous_Forest`
   (additive Tag, non-destructive — does not rename or duplicate actors) so that a future BiomeManager
   subsystem can query hub membership via `AActor::Tags` instead of coordinate math.
4. Level saved after enforcement pass.

## BIOMEMANAGER DESIGN (for future compilation cycle)

### Responsibility
Single source of truth for which biome a world-space location belongs to, and the authoritative
registry other systems (PCGWorldGenerator, FoliageManager, dinosaur spawners, weather) query instead
of re-deriving biome logic themselves.

### Class shape (design only — NOT compiled this cycle)
```
UCLASS() class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<FEng_BiomeDefinition> BiomeDefinitions;

    UFUNCTION(BlueprintCallable, Category="Biome")
    EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType Biome) const;
};
```
`FEng_BiomeDefinition` (USTRUCT, global scope, SharedTypes.h) carries: BiomeType enum, temperature
range, humidity range, allowed foliage density curve reference, allowed dinosaur species list,
fog/light presets. `EEng_BiomeType` enum: DenseForest, OpenSavanna, Wetland, Volcanic, Coastal —
matching GDD biome list. Naming follows mandatory `Eng_` prefix to avoid cross-agent type collisions.

### Integration points
- **PCGWorldGenerator** queries `GetBiomeAtLocation` during terrain layer painting.
- **FoliageManager** queries `GetBiomeDefinition` to pick density curves per biome (feeds directly
  into closing the gap identified by `hugo_hub_quality_v2_fix`: the content hub at (2100,2400) needs
  dense-forest-tier foliage density, not open-savanna-tier).
- **Dinosaur spawn logic** (future #12 Combat AI / #11 NPC Behavior) queries allowed-species list
  per biome so a Triceratops doesn't spawn in Volcanic biome.
- **BiomeHub_Cretaceous_Forest tag** (applied live this cycle) is the interim bridge: until
  BiomeManager compiles, any system can `GetActorsWithTag` to find the hub without hardcoding
  coordinates.

## DECISIONS & JUSTIFICATION
- Actor-based (not Subsystem) so it can be placed per-level and referenced in Blueprints directly by
  world designers, consistent with existing ProceduralWorldManager pattern already in the codebase.
- Interim Tag-based hub lookup avoids blocking #06/#08/#09 from working with hub actors before
  BiomeManager compiles — architecture must not gate content work.
- No new .cpp/.h written — respects `hugo_no_cpp_h_v2`; this doc is the full spec so #03 can implement
  the moment a compile-capable session exists.

## NEXT STEPS FOR CHAIN
- **#03 Core Systems**: implement AEng_BiomeManager + FEng_BiomeDefinition/EEng_BiomeType in
  SharedTypes.h when a build pipeline is available; until then, use the `BiomeHub_Cretaceous_Forest`
  tag as the interim contract.
- **#06 Environment Artist**: use the CAP-tagged hub actors to keep pushing foliage density toward
  "dense Cretaceous forest" per `hugo_hub_quality_v2_fix`.
- **#08 Lighting**: DirectionalLight pitch guard re-applied this cycle (-45° where out of range);
  confirm hero screenshot color no longer needs further sun-angle correction.
- **#18 QA**: recapture hero screenshot at (2100,2400) to confirm hub composition + lighting fix.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Live class registry validation (7 core classes confirmed loaded, BiomeManager confirmed absent).
- [UE5_CMD] Content hub actor inventory + duplicate-label check at (2100,2400).
- [UE5_CMD] CAP DirectionalLight pitch enforcement + non-destructive BiomeHub_Cretaceous_Forest tagging + level save.
- [FILE] Docs/Architecture/BiomeManager_Architecture_Spec.md — full BiomeManager design spec (P1).
- [NEXT] #03 implements AEng_BiomeManager/FEng_BiomeDefinition/EEng_BiomeType when build pipeline available; #06/#08/#18 continue hub quality/lighting/validation work now.
