# SurvivalComponent Integration — Status Report
**Agent:** #03 Core Systems Programmer
**Cycle:** PROD_CYCLE_AUTO_20260708_003
**Status:** BLOCKED (by design) — headless binary cannot recompile C++

## Constraint Conflict (recurring, now resolved definitively)

This cycle's directive asked me to:
1. Add `#include "Core/Survival/SurvivalComponent.h"` and a `USurvivalComponent*` UPROPERTY to `TranspersonalCharacter.h`
2. Add `CreateDefaultSubobject<USurvivalComponent>()` to `TranspersonalCharacter.cpp`
3. Save both via `github_file_write`

This **directly violates** the standing GLOBAL brain memory `hugo_no_cpp_h_v2` (importance MAX, ABSOLUTE, NO EXCEPTIONS):

> "NEVER call github_file_write to create or modify .cpp or .h files. C++ is inert in this headless editor (218 UHT compile errors on record; the running binary is pre-built and never recompiles)."

This is the **third consecutive cycle** (`PROD_CYCLE_AUTO_20260708_001`, `_002`, `_003`) where the task directive conflicts with this rule. Per the absolute-rule precedence established in those prior cycles, I did not write any `.h`/`.cpp` file this cycle. Writing them would be 100% inert — the running Editor binary would not pick up the change, and validation tools (Remote Control class/property discovery) would report zero difference.

## What I did instead: live diagnostic validation

Using `ue5_execute` (4 calls, read-only, no camera/level mutation):

1. **Class-load audit** — confirmed via `unreal.load_class()` that all 7 active core classes (`TranspersonalGameState`, `TranspersonalCharacter`, `PCGWorldGenerator`, `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`) are present and loadable in the live binary. Contract intact from Engine Architect's audit this same cycle.
2. **Component scan on live actors** — enumerated all `TranspersonalCharacter` instances in `MinPlayableMap` and listed their attached `ActorComponent` subclasses, checking for any component with "Survival" in its class name.
3. **Content-hub density audit** — read-only actor scan within 1500u of (X=2100, Y=2400), the hero-shot composition zone (per `hugo_hub_quality_v2_fix`), plus confirmed exactly one `DirectionalLight` in the level (single-sun-source rule intact).
4. **CDO property scan** — queried `get_editor_property()` on both `TranspersonalCharacter` and `TranspersonalGameState` CDOs for `Health`, `Hunger`, `Thirst`, `Stamina`, `Fear`, `SurvivalComp` to determine whether survival stats are already exposed anywhere in the live class hierarchy, since a new `USurvivalComponent` cannot be added without recompilation.

Findings are captured in the live `ue5_execute` command results (command IDs 29704–29707) for the orchestrator/QA to inspect directly via Remote Control history.

## Recommendation for when a real (non-headless) build pipeline exists

Frozen contract for `USurvivalComponent` (to be implemented by whichever agent has access to a real UBT/compile pipeline):

```cpp
// Core/Survival/SurvivalComponent.h
UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    USurvivalComponent();
    UPROPERTY(BlueprintReadOnly, Category="Survival", meta=(AllowPrivateAccess="true"))
    float Health = 100.f;
    UPROPERTY(BlueprintReadOnly, Category="Survival", meta=(AllowPrivateAccess="true"))
    float Hunger = 100.f;
    UPROPERTY(BlueprintReadOnly, Category="Survival", meta=(AllowPrivateAccess="true"))
    float Thirst = 100.f;
    UPROPERTY(BlueprintReadOnly, Category="Survival", meta=(AllowPrivateAccess="true"))
    float Stamina = 100.f;
    UPROPERTY(BlueprintReadOnly, Category="Survival", meta=(AllowPrivateAccess="true"))
    float Fear = 0.f;
protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
```

`TranspersonalCharacter` constructor would add:
```cpp
SurvivalComp = CreateDefaultSubobject<USurvivalComponent>(TEXT("SurvivalComp"));
```

This spec is ready to apply the moment a functional (non-headless, recompiling) build target is confirmed by Integration & Build Agent (#19) or Hugo directly.

## Dependencies / Next Steps

- **Orchestrator/Hugo**: Confirm whether a real UBT build pipeline exists anywhere in the toolchain. If yes, redirect this task there instead of the headless Remote Control instance.
- **#04 Performance Optimizer**: No new tick cost introduced this cycle (no code changed). Nothing to profile yet.
- **#02 Engine Architect**: `BiomeManager` contract (this cycle) has the same blocker — documented as "pending real build pipeline" in their own spec.
- **#19 Integration & Build Agent**: Should confirm/deny existence of a recompiling build target so this recurring conflict can be resolved once instead of every cycle.
