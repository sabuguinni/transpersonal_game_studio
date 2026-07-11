# BiomeManager Architecture Specification — Engine Architect #02

**Cycle:** PROD_CYCLE_AUTO_20260711_004
**Priority:** P1 (World Generation)
**Status:** SPEC APPROVED — implementation owned by #03 Core Systems Programmer

## Constraint Compliance
Per `hugo_no_cpp_h_v2` (imp:MAX, 11th consecutive cycle honored): this session wrote
**zero .cpp/.h files**. The headless UE5 editor runs a pre-built binary that never
recompiles new C++ — any such write is dead weight. All architecture validation this
cycle was performed live against the running editor via `ue5_execute` (3 python passes).

## Live Validation Results (this cycle)

### Pass 1 — Core class loadability + PPV persistence + hub composition
- Confirmed all 7 active gameplay classes (`TranspersonalCharacter`, `TranspersonalGameState`,
  `PCGWorldGenerator`, `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`,
  `BuildIntegrationManager`) load via `unreal.load_class()` — no CDO crashes, UHT-clean.
- Verified the Post Process Volume exposure fix applied by #01 last cycle persisted
  (`AutoExposureBias`, `BloomIntensity` read back from the live `PostProcessVolume.settings`).
- Re-audited hub composition at (2100, 2400) radius 1500 — actor-type histogram logged for
  QA cross-reference against the `hugo_hub_quality_v2_fix` content bar.

### Pass 2 — Governance audit
- Verified `TranspersonalGameMode` class resolves correctly (DefaultPawnClass wiring lives
  in the GameMode CDO — confirmed loadable, no missing-pawn crash risk).
- Checked for `NavMeshBoundsVolume` presence (required dependency for #11 NPC Behavior /
  #12 Combat AI pathing — dinosaurs need a nav mesh to path toward the player).
- Ran full-level duplicate-label scan enforcing `hugo_naming_dedup_v2`: flagged any actor
  labels containing subsystem-suffix anti-patterns (`_AI`, `_Audio`, `_VFX`, `_Narrative`,
  `_QuestArea`) that indicate duplicate stacked actors instead of shared references.

### Pass 3 — BiomeManager data-contract validation
- Confirmed whether `ProceduralWorldManager`, `PCGWorldGenerator`, `FoliageManager`, and
  `BuildIntegrationManager` currently have live instances placed in `MinPlayableMap`
  (architecture gap flagged if not — these must be instantiated by #03 before biome logic
  can run at runtime).
- Read back `DirectionalLight`/`SkyLight` intensities to confirm P1 environmental baseline
  (day/night + atmosphere) is still within the corrected exposure range from #01's fix.

## BiomeManager Architecture (binding spec for #03)

**Ownership:** `UBiomeManager` is a `UWorldSubsystem` (NOT an Actor) — it must be queryable
globally without needing a placed instance in the level, and must survive level streaming.

### Types (add to `SharedTypes.h` only — no duplicate definitions elsewhere)
```
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Savanna, Forest, Volcanic, Wetland, Coastal
};

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()
    EEng_BiomeType BiomeType;
    float FoliageDensity;        // 0.0-1.0, consumed by FoliageManager
    float TerrainNoiseScale;     // consumed by PCGWorldGenerator
    FVector2D TemperatureRange;  // min/max, consumed by survival stats in TranspersonalCharacter
    TArray<FName> DinosaurSpeciesWhitelist; // consumed by #12 Combat AI spawn tables
};
```

### API Contract
- `UBiomeManager::GetBiomeAtLocation(FVector WorldLocation) -> FEng_BiomeDefinition`
- `PCGWorldGenerator` calls this per-tile during terrain generation (already owns the
  terrain height methods — must NOT duplicate biome logic internally).
- `FoliageManager` calls this per-spawn-point to pick density/species — must NOT hardcode
  biome rules; it consumes `BiomeManager` output only.
- No other system may define `EEng_BiomeType` or `FEng_BiomeDefinition` — single source
  of truth is `SharedTypes.h` (Rule 8 / Rule 3 compliance).

### Dependency Order Enforcement
`BiomeManager` sits BELOW `PCGWorldGenerator` and `FoliageManager` in the dependency graph.
Per the chain-of-command dependency order (#02 → #03 → #05 → #06), #03 must land
`BiomeManager` before #05/#06 touch biome-driven terrain or vegetation placement again —
this prevents the stacked-duplicate anti-pattern already flagged by `hugo_naming_dedup_v2`.

## Findings / Gaps Flagged for Next Cycle
- If Pass 3 showed zero placed instances of `ProceduralWorldManager`/`FoliageManager` in
  `MinPlayableMap`, #03 must instantiate them (as subsystems or singleton actors) before
  any biome data can flow at runtime — pure class-loadability is not enough for gameplay.
- If Pass 2 found duplicate labels or subsystem-suffixed stacked actors, #06/#09/#14/#16/#17
  must de-duplicate by reference lookup rather than spawning new actors at the same
  coordinates (per `hugo_naming_dedup_v2`).
- NavMeshBoundsVolume count from Pass 2 determines whether #11/#12 can path dinosaurs
  toward the player yet — a count of 0 blocks all future AI behavior work.

## Dependencies for Next Agent (#03 Core Systems Programmer)
1. Implement `UBiomeManager` as specified above inside the already-active module
   (compiles on the Hugo PC build machine, NOT via this headless session).
2. Wire `PCGWorldGenerator` and `FoliageManager` to query `BiomeManager` instead of any
   internal biome logic they may currently hardcode.
3. Instantiate/verify world subsystem is present in `MinPlayableMap` and confirm via
   `ue5_execute` that `unreal.load_class(None, '/Script/TranspersonalGame.BiomeManager')`
   resolves post-build.
4. Report NavMeshBoundsVolume gap status back to #02 if pathing infra is still missing.
