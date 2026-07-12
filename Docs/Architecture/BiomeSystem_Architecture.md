# Biome System Architecture — Engine Architect #02
## Cycle: PROD_CYCLE_AUTO_20260712_003

## CONSTRAINT COMPLIANCE (imp:MAX rules)
- **`hugo_no_cpp_h_v2`**: Zero `.cpp`/`.h` files written this cycle (6th+ consecutive cycle honoring this).
  The headless UE5 binary is pre-built and never recompiles new C++. A `UBiomeManager` class written now
  would be dead code — invisible to Remote Control, failing the automated "Class Existence" validation test.
  Instead, the Biome System is implemented **entirely data-driven**, via Actor Tags, validated live in the
  running editor through `ue5_execute` (Python/Remote Control).
- **`hugo_no_camera_v2`**: No viewport camera modified.
- **`hugo_naming_dedup_v2`**: No duplicate actors spawned. This cycle only **tagged existing actors** —
  zero new actors created, zero risk of stacking duplicates on top of existing dinosaurs/props.

## ARCHITECTURE DECISION: Data-Driven Biome System (no native UBiomeManager class)

Since C++ cannot be compiled into this running instance, the Biome system is implemented as a
**convention-over-code** architecture using UE5's built-in Actor Tag system:

### Design
1. **Biome identity = Actor Tag**, prefix `Biome_<Name>`, e.g. `Biome_CretaceousForest`, `Biome_OpenPlains`.
2. **Classification rule**: any actor within a 1500-unit radius of the content hub center
   (world coords X=2100, Y=2400 — the primary PlayerStart clearing per `hugo_hub_quality_v2_fix`)
   is tagged `Biome_CretaceousForest`. Biome-relevant actors outside that radius are tagged `Biome_OpenPlains`.
3. **Biome-relevant actor filter**: name contains one of `tree, rock, trex, raptor, brachio, trike, dino,
   fern, foliage` (case-insensitive) — this avoids tagging lights, volumes, PlayerStart, or trigger actors
   which are not part of the ecological/visual biome composition.
4. **Extensibility**: future agents (#05 World Generator, #06 Environment Artist) can:
   - Add new tags (`Biome_Wetland`, `Biome_Volcanic`, etc.) as new zones are built.
   - Query actors by tag via `unreal.GameplayStatics` / `EditorActorSubsystem.get_all_level_actors()` filtered
     by tag, without needing any new compiled class.
   - Any future C++ `UBiomeManager` (once a real compile pipeline exists) can bootstrap directly from these
     tags — the data model is forward-compatible with a proper subsystem.

### Why not a UPROPERTY-based subsystem
A `UBiomeManager : public UWorldSubsystem` would require:
- New `.h`/`.cpp` pair → requires UBT recompilation → **not available** in this headless, pre-built binary.
- Would fail immediately on the automated "Class Existence" test (`unreal.load_class` returns None),
  costing a regression against the current baseline (134 classes / 45 functional tests passing).

Tag-based data is queryable NOW, live, with zero compilation risk, and directly improves the
content-hub composition quality bar (`hugo_hub_quality_v2_fix`) by making biome membership explicit
and auditable for every actor already in `MinPlayableMap`.

## VALIDATION PERFORMED THIS CYCLE (live, via Remote Control)
1. Confirmed active class list still resolves via `unreal.load_class`:
   - `TranspersonalGameState`, `TranspersonalCharacter`, `PCGWorldGenerator`, `FoliageManager`,
     `ProceduralWorldManager` — all checked for FOUND/MISSING status.
2. Counted total actors in `MinPlayableMap`.
3. Applied `Biome_*` tags to all untagged, biome-relevant actors (trees, rocks, dinosaurs, foliage)
   based on proximity to the hub center — **no new actors spawned**, only tag mutation on existing actors.
4. Saved the level.
5. Re-scanned and logged final `Biome_*` tag distribution as proof of the change (readback via
   `/tmp/ue5_result_engine_architect_final.txt`).

## RULE FOR ALL DOWNSTREAM AGENTS (#03 onward)
- Any system needing biome context (spawn rules, foliage density, dinosaur species per zone, weather)
  MUST read the actor's `Biome_*` tag rather than inventing a parallel biome enum in a new header.
- `SharedTypes.h` should NOT gain a new `EBiome` enum until a real C++ compile pipeline is restored —
  doing so now would be an unused, unvalidatable type per Rule 3 (One Definition Rule) with no CDO to test.
- When compilation is restored, promote this tag convention 1:1 into a `UBiomeManager` UWorldSubsystem with
  `TMap<FName, FEng_BiomeData>` keyed by these exact tag strings, so no re-authoring of existing tags is needed.

## DEPENDENCIES / NEXT STEPS
- **#05 Procedural World Generator**: consume `Biome_*` tags to bias PCG terrain generation per zone.
- **#06 Environment Artist**: use tags to decide foliage density/species per biome when populating new areas.
- **#01 Studio Director**: when image generation service (403 JWS error) is restored, request a hero shot of
  the hub confirming visual biome coherence (dense forest look at `Biome_CretaceousForest` tagged actors).
