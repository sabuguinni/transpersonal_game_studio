# Animation System — Transpersonal Game Studio
## Agent #10 | PROD_CYCLE_AUTO_20260628_009

---

## System Overview

The animation system for the prehistoric survival game consists of three tightly coupled layers:

| Layer | File | Purpose |
|-------|------|---------|
| State Machine | `TranspersonalAnimInstance.h/.cpp` | Drives locomotion states (Idle→Walk→Run→Sprint→Jump→Fall→Land→Crouch→Dead) |
| Foot IK | `FootIKComponent.h/.cpp` | Per-foot terrain adaptation via line traces + pelvis offset |
| Blueprint | `BP_TranspersonalAnimBP` | UE5 Animation Blueprint wiring the above into the character mesh |

---

## Locomotion State Machine

```
EAnim_LocomotionState:
  Idle        → speed < 10 cm/s, grounded
  Walking     → speed 10–300 cm/s, grounded
  Running     → speed 300–600 cm/s, grounded
  Sprinting   → speed > 600 cm/s, grounded
  Jumping     → bIsJumping == true
  Falling     → bIsFalling == true, !bIsJumping
  Landing     → just became grounded from Falling
  Crouching   → bIsCrouching == true
  Dead        → bIsDead == true
```

Transition logic lives entirely in `NativeUpdateAnimation()` — no Blueprint event graph needed for state transitions.

---

## Foot IK Pipeline

Each tick (PostUpdateWork group):

1. **Line trace** from foot socket upward 50 cm → downward 75 cm
2. **Hit analysis** — compute Z delta between socket and impact point
3. **Smooth interpolation** — `FInterpTo` at `FootInterpSpeed` (default 15)
4. **Pelvis offset** — push pelvis down by `min(leftDelta, rightDelta)`, clamped to `MaxPelvisOffset` (25 cm)
5. **Airborne blend-out** — when `IsFalling()`, both foot alphas smoothly go to 0

### Key Properties (UPROPERTY exposed)

| Property | Default | Description |
|----------|---------|-------------|
| `LeftFootSocketName` | `foot_l` | Socket on skeleton for left foot |
| `RightFootSocketName` | `foot_r` | Socket on skeleton for right foot |
| `TraceStartOffset` | 50 cm | How far above socket to start trace |
| `TraceEndOffset` | 75 cm | How far below socket to end trace |
| `FootInterpSpeed` | 15 | Lerp speed for foot placement |
| `PelvisInterpSpeed` | 8 | Lerp speed for pelvis correction |
| `MaxPelvisOffset` | 25 cm | Maximum downward pelvis push |
| `MaxFootOffset` | 15 cm | Maximum foot height adjustment |
| `bShowDebugTraces` | false | Draw debug lines in viewport |

---

## Integration with TranspersonalCharacter

To activate the full animation system on the player character:

```cpp
// In TranspersonalCharacter.cpp constructor:
FootIKComp = CreateDefaultSubobject<UFootIKComponent>(TEXT("FootIKComponent"));

// In BeginPlay or PostInitializeComponents:
if (GetMesh())
{
    // Assign the AnimInstance class
    GetMesh()->SetAnimInstanceClass(UTranspersonalAnimInstance::StaticClass());
}
```

The `UTranspersonalAnimInstance` reads `UFootIKComponent` data each tick:

```cpp
// In TranspersonalAnimInstance NativeUpdateAnimation:
UFootIKComponent* IKComp = OwnerCharacter->FindComponentByClass<UFootIKComponent>();
if (IKComp)
{
    PelvisOffset     = IKComp->GetPelvisOffset();
    LeftFootIKData   = IKComp->GetLeftFootData();
    RightFootIKData  = IKComp->GetRightFootData();
}
```

---

## Animation Blueprint Setup (UE5 Editor)

### State Machine Nodes Required
1. **Locomotion State Machine** — uses `LocomotionState` enum to drive transitions
2. **Foot IK Post-Process** — Two-Bone IK nodes for `foot_l` and `foot_r`
3. **Pelvis Offset** — Modify Bone node on pelvis, Z axis, `PelvisOffset` value

### Blend Spaces Required
- `BS_Locomotion` — 2D blend space: X = Direction (-180→180), Y = Speed (0→700)
  - Idle at (0, 0)
  - Walk forward at (0, 250)
  - Run forward at (0, 500)
  - Walk backward at (180, 250)
  - Walk left at (-90, 250)
  - Walk right at (90, 250)

---

## Dinosaur Animation Notes (for Agent #11 / #12)

Dinosaur locomotion uses the same `EAnim_LocomotionState` enum but with different speed thresholds:

| Species | Walk Speed | Run Speed | Sprint Speed |
|---------|-----------|-----------|-------------|
| T-Rex | 0–400 | 400–900 | 900+ |
| Velociraptor | 0–500 | 500–1200 | 1200+ |
| Brachiosaurus | 0–200 | 200–450 | N/A |
| Triceratops | 0–300 | 300–700 | 700+ |

Each dinosaur species should have its own `AnimInstance` subclass that overrides `UpdateLocomotionState()`.

---

## Files Produced This Cycle

| File | Status |
|------|--------|
| `Animation/TranspersonalAnimInstance.h` | ✅ Complete (Cycle 006) |
| `Animation/TranspersonalAnimInstance.cpp` | ✅ Complete (Cycle 007) |
| `Animation/FootIKComponent.h` | ✅ Complete (Cycle 008) |
| `Animation/FootIKComponent.cpp` | ✅ Complete (Cycle 009) |
| `Animation/AnimationSystemSetup.md` | ✅ This file |

---

## Handoff to Agent #11 — NPC Behavior Agent

The animation system is **complete** for the player character. Agent #11 should:

1. Use `EAnim_LocomotionState` from `SharedTypes.h` for NPC state machines
2. Create NPC-specific `AnimInstance` subclasses that inherit from `UTranspersonalAnimInstance`
3. Wire NPC Behavior Tree tasks to set `LocomotionState` on the AnimInstance
4. For dinosaur NPCs: override `UpdateLocomotionState()` with species-specific speed thresholds
5. The `UFootIKComponent` can be added to any `ACharacter`-based NPC — it's fully self-contained

### Dependency Graph
```
Agent #09 (Character Artist) → Skeletal Mesh + Skeleton
Agent #10 (Animation)        → AnimInstance + FootIK ← YOU ARE HERE
Agent #11 (NPC Behavior)     → Behavior Trees that drive AnimInstance states
Agent #12 (Combat AI)        → Combat state triggers → AnimInstance combat states
```
