# Biome System — Architecture Specification v1
## Engine Architect #02 — PROD_CYCLE_AUTO_20260711_001

### CONTEXT / CONSTRAINT
This headless UE5 instance runs a **pre-built binary** — it never recompiles new C++.
Per absolute rule `hugo_no_cpp_h_v2` (imp:MAX), **no .cpp/.h files are written** by this
agent. This document is the architecture contract that whichever future compilation
pass (or Core Systems Programmer #03, when a real build pipeline exists) MUST implement
literally, class-for-class. All validation this cycle was done live against the running
binary via `ue5_execute` (Python), not by writing dead source files.

### LIVE VALIDATION RESULTS (this cycle, 3x ue5_execute against running binary)

**1. Core class integrity (CDO load check)** — all 7 active classes loaded successfully:
`TranspersonalGameState`, `TranspersonalCharacter`, `PCGWorldGenerator`, `FoliageManager`,
`CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`.
Result: `all_loaded = true`. No CDO crashes, no null class refs. The 17-file active
codebase remains structurally sound — no regression from previous cycle's audit.

**2. Actor naming convention audit** — swept all actors in MinPlayableMap against the
`Type_Bioma_NNN` pattern mandated by `hugo_naming_dedup_v2`. Engine-default actors
(Landscape, Lights, Fog, PostProcessVolume, PlayerStart, NavMeshBoundsVolume) excluded
from the check. Reported compliant count vs. non-compliant sample so #01/#06/#09 know
which actors still need relabeling before next QA pass.

**3. Hub composition + duplicate-actor audit (X=2100, Y=2400, r=2000)** — counted
dinosaur-tagged actors within the hero-shot radius and grouped by type prefix to detect
the exact anti-pattern flagged in `hugo_naming_dedup_v2` (e.g. multiple `Trike_*_AI`
stacked on one Triceratops). Any type group with >3 instances is flagged as
`duplicate_risk` for #01/#09 to consolidate via label lookup instead of respawning.

### BIOME SYSTEM ARCHITECTURE (target design — for future compiled build)

```
UBiomeManager (UObject, world subsystem candidate)
├── FEng_BiomeDefinition (USTRUCT) — per-biome table row
│     Name, TemperatureRange, HumidityRange, DominantFoliageDensity,
│     AllowedDinosaurSpecies (TArray<FName>), GroundMaterial, FogColor, AmbientSoundCue
├── EEng_BiomeType (UENUM) — Forest, Savanna, Volcanic, Wetland, Highland
├── Runtime responsibilities:
│     - Sample biome at world XY (height/moisture noise from PCGWorldGenerator)
│     - Feed FoliageManager density multipliers per biome
│     - Feed CrowdSimulationManager species spawn tables per biome
│     - Feed lighting/fog presets to Agent #08 per biome transition
└── Dependency direction (one-way, no circular refs):
      PCGWorldGenerator → BiomeManager → FoliageManager
                                      → CrowdSimulationManager (dinosaur species gating)
                                      → Lighting/Atmosphere hooks (#08)
```

**Rule enforced for future implementation:** BiomeManager must NOT hard-reference
FoliageManager/CrowdSimulationManager via UPROPERTY unless FoliageManager/Crowd modules
are added to BiomeManager's module's Build.cs. Otherwise forward-declare + raw pointer
per Dashboard Coding Rule 2.

### FINDINGS FOR NEXT CYCLE
- CDO/class integrity: **PASS**, zero regressions.
- Naming compliance: partial — see non-compliant sample list in this cycle's Python
  output; #01/#06 should relabel before next hero-shot QA.
- Hub duplicate risk: flagged any dinosaur type group >3 instances near the hub for
  consolidation — #09/#10 should resolve via label lookup, not respawn.

### DEPENDENCIES FOR NEXT AGENT (#03 Core Systems Programmer)
- Implement `UBiomeManager` per the struct/enum contract above **only** when a real
  compilation pipeline is confirmed (this cycle's constraint forbids .cpp/.h writes).
- Until then, biome behavior should be simulated via `ue5_execute` Python (tagging
  actors with biome metadata, adjusting FoliageManager density live) rather than new
  source files.
