# BiomeManager — Implementation Decision (Core Systems Programmer #03, Cycle PROD_CYCLE_AUTO_20260713_008)

## Decision: REJECTED C++ compilation for `AEng_BiomeManager` — functional tag-based substitute applied instead

### Why the C++ contract cannot be implemented this cycle
Per absolute rule `hugo_no_cpp_h_v2` (imp:MAX, confirmed for the 52nd consecutive cycle):
this headless UE5 instance runs a **pre-built binary that never recompiles**. Any
`.h`/`.cpp` written via `github_file_write` is inert — it sits in the GitHub repo but
has zero runtime effect. I verified this live again this cycle:

```python
unreal.load_class(None, '/Script/TranspersonalGame.BiomeManager')      -> None
unreal.load_class(None, '/Script/TranspersonalGame.Eng_BiomeManager')  -> None
```

Both return `None`, confirming the class genuinely does not exist in the running
Editor and cannot be made to exist without a recompile the pipeline cannot perform.
Writing `AEng_BiomeManager.h/.cpp` as instructed by the Engine Architect's contract
(`BiomeManager_ArchitectureSpec_Cycle008.md`) would violate the imp:MAX rule for zero
functional gain — so I did not write those files.

### What I did instead (functional, verified live in the running editor)
Since `AEng_BiomeManager` cannot exist as a compiled class, I implemented the
**data layer as gameplay actor tags** — a mechanism that already exists in the
compiled binary (`AActor::Tags`) and is queryable at runtime by any already-compiled
system (`FoliageManager`, `PCGWorldGenerator`) via `UGameplayStatics::GetAllActorsWithTag`
without requiring any new C++.

1. Scanned all actors within the hub composition radius (X=2100, Y=2400, r=2500).
2. Tagged every hub actor not already tagged with `Biome.Forest`.
3. Read back the tags immediately after to confirm persistence (functional
   verification, not just a fire-and-forget write) — see tool call #3 result log
   `/tmp/ue5_result_core_systems_008c.txt`.
4. Ran a physics/collision sanity pass (my actual domain per role charter):
   confirmed `TranspersonalCharacter` instances have a valid
   `CharacterMovementComponent`, and logged collision-enabled state on all dinosaur
   pawns (`TRex`, `Raptor`, `Brachio`, `Trike` labels) in the level.

### Consumers (for #05 World Generator / #06 Environment Artist)
`PCGWorldGenerator` and `FoliageManager` classes are confirmed loadable and their
instance counts in the level were logged this cycle. Neither currently queries
`Biome.Forest` tags — **this is the concrete next step for whichever agent owns
world generation/foliage placement**: replace any hardcoded per-biome mesh logic
with `GetAllActorsWithTag(World, "Biome.Forest")` (or equivalent per-biome tag)
lookups, consistent with the Engine Architect's "single source of truth, no
duplicated biome detection" principle — just realized via tags instead of a new
UCLASS, since the latter is not buildable here.

### Scope note on the Engine Architect's contract
The struct/enum contract (`EEng_BiomeType`, `FEng_BiomeDefinition`) in
`BiomeManager_ArchitectureSpec_Cycle008.md` remains valid **as a future-build
target** for whenever a real compile pipeline exists. I am not discarding it — I'm
recording that it is currently unbuildable and providing the closest functional
equivalent achievable in the live, non-recompiling editor.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Pass 1: confirmed `BiomeManager`/`Eng_BiomeManager` classes absent (both `None`) + core class integrity re-check + hub actor scan (no camera changes, per `hugo_no_camera_v2`)
- [UE5_CMD] Pass 2: applied `Biome.Forest` gameplay tag to all untagged hub actors (X=2100,Y=2400,r=2500) as the functional biome-data substitute; verified `PCGWorldGenerator`/`FoliageManager` instance counts as designated consumers
- [UE5_CMD] Pass 3: read-back verification of tag persistence + physics/collision sanity check on `TranspersonalCharacter` movement component and dinosaur pawn (`TRex`/`Raptor`/`Brachio`/`Trike`) collision-enabled state
- [FILE] `Docs/Architecture/BiomeManager_ImplementationDecision_Cycle008.md` — this document, recording why C++ was rejected and what functional substitute shipped instead
- [NEXT] Whoever owns #05 (Procedural World Generator) / #06 (Environment Artist) should wire `PCGWorldGenerator`/`FoliageManager` Python setup scripts to query `GetAllActorsWithTag(World, "Biome.Forest")` (and future `Biome.Savanna`/`Biome.Wetland`/etc. tags) instead of waiting on a compiled `AEng_BiomeManager` class that cannot ship in this environment
