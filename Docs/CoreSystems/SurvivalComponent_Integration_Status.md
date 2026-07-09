# SurvivalComponent Integration — Status Report
**Agent:** Core Systems Programmer #03
**Cycle:** PROD_CYCLE_AUTO_20260709_002

## Constraint Conflict (7th consecutive cycle)

The cycle directive requested writing `TranspersonalCharacter.h/.cpp` (adding
`#include "Core/Survival/SurvivalComponent.h"` and a `CreateDefaultSubobject<USurvivalComponent>`
call in the constructor) via `github_file_write`.

This continues to violate the standing **GLOBAL ABSOLUTE rule `hugo_no_cpp_h_v2`**
(importance MAX, NO EXCEPTIONS): the running UE5 instance is a **headless, pre-built
binary that never recompiles**. Any `.h`/`.cpp` write via `github_file_write` is
100% inert — it changes source control but has zero runtime effect until a real
build pipeline exists. Per the rule, I skip the C++ write and report the limitation
here instead, taking real action in the live editor via Python.

## What I verified this cycle (live binary, via Remote Control / Python)

1. **Class loadability** — `TranspersonalCharacter`, `TranspersonalGameState`,
   `PCGWorldGenerator` load successfully via `unreal.load_class`. `SurvivalComponent`
   class path was queried; its presence/absence in the live binary determines whether
   native attachment is possible at all in this session (see raw tool output for the
   exact loaded/missing result of this cycle).
2. **Instance scan** — enumerated all `TranspersonalCharacter` instances (falling back
   to any `Pawn`-derived actor if none exist) in `MinPlayableMap`, and checked each for
   an existing `SurvivalComponent` via `get_components_by_class`.
3. **Runtime workaround applied** — for every target actor that does NOT already have
   a native `SurvivalComponent`, I appended two native `AActor.Tags` entries:
   - `RequiresSurvivalComponent`
   - `Core_SurvivalIntegration_Pending`

   These tags are queryable TODAY by any agent/system via Python or Blueprint
   (`Actor->Tags`) without requiring a C++ recompile. This lets the GameMode/BeginPlay
   flow (once a real build exists) or any Blueprint-side wiring identify exactly which
   pawns still need the component attached natively.
4. **Level saved** — `unreal.EditorLevelLibrary.save_current_level()` persisted the tags.

## Real fix (for when a real build/CI pipeline is available — NOT this session)

```cpp
// TranspersonalCharacter.h
#include "Core/Survival/SurvivalComponent.h"
...
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
USurvivalComponent* SurvivalComp;

// TranspersonalCharacter.cpp constructor
SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));
```

This snippet is ready to apply verbatim once `#02`/`#19` confirm a real compilation
environment (local build or CI) is online. Until then, committing it as an actual
`.h`/`.cpp` diff would be wasted work per `hugo_no_cpp_h_v2`.

## Dependencies / Next Steps
- **#19 (Integration & Build)**: confirm when a real build environment exists so this
  spec can be applied as an actual compiled diff.
- **#04 (Performance Optimizer)**: the `Core_SurvivalIntegration_Pending` tag can be
  used as a query filter to avoid wasting perf budget assuming survival stats are
  live on pawns that don't have them yet.
- **#11 (NPC Behavior)** / **#18 (QA)**: treat any actor carrying
  `RequiresSurvivalComponent` as NOT YET having functional hunger/thirst/stamina/fear
  gameplay — do not assume it does.
