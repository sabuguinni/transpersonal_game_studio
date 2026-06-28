#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootIKComponent.generated.h"

/**
 * FAnim_FootIKData — per-foot IK solve result
 * Stores the world-space target location and normal for one foot,
 * plus a blend alpha so the foot can ease in/out smoothly.
 */
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    /** World-space IK target position for this foot */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector TargetLocation = FVector::ZeroVector;

    /** Surface normal at the IK target (used to rotate the foot) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector SurfaceNormal = FVector::UpVector;

    /** Blend alpha: 0 = no IK correction, 1 = full IK correction */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float BlendAlpha = 0.0f;

    /** Height offset applied to the pelvis to keep both feet grounded */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float PelvisOffset = 0.0f;

    /** True when the foot is currently in contact with the ground */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    bool bIsGrounded = false;
};

/**
 * UFootIKComponent — Foot IK solver for the player character.
 *
 * Performs two line-traces per tick (one per foot) to find the exact
 * ground surface beneath each foot socket.  The results are stored in
 * LeftFootData / RightFootData and consumed by UTranspersonalAnimInstance
 * to drive Two-Bone IK nodes in the Animation Blueprint.
 *
 * Design goals:
 *  - Zero allocations per tick (all data is value-type structs)
 *  - Configurable trace channel so it works on any terrain type
 *  - Smooth interpolation so feet don't snap when crossing edges
 */
UCLASS(ClassGroup = "Animation", meta = (BlueprintSpawnableComponent),
       DisplayName = "Foot IK Component")
class TRANSPERSONALGAME_API UFootIKComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFootIKComponent();

    // ── UActorComponent overrides ─────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── IK Results (read by AnimInstance) ────────────────────────────────

    /** Solved IK data for the left foot */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData LeftFootData;

    /** Solved IK data for the right foot */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData RightFootData;

    // ── Configuration ─────────────────────────────────────────────────────

    /** Socket name on the character mesh for the left foot */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Sockets")
    FName LeftFootSocketName = FName("foot_l");

    /** Socket name on the character mesh for the right foot */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Sockets")
    FName RightFootSocketName = FName("foot_r");

    /** How far above the foot socket to start the downward trace */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Trace",
              meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float TraceStartOffset = 60.0f;

    /** How far below the foot socket to end the downward trace */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Trace",
              meta = (ClampMin = "10.0", ClampMax = "300.0"))
    float TraceEndOffset = 120.0f;

    /** Collision channel used for foot traces (default: Visibility) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Trace")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    /** Interpolation speed for foot position smoothing (higher = snappier) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Smoothing",
              meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float FootInterpSpeed = 12.0f;

    /** Interpolation speed for pelvis height correction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Smoothing",
              meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float PelvisInterpSpeed = 8.0f;

    /** Maximum pelvis correction in cm (prevents extreme crouching on steep slopes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Limits",
              meta = (ClampMin = "0.0", ClampMax = "60.0"))
    float MaxPelvisOffset = 30.0f;

    /** Disable IK when moving faster than this speed (cm/s) — avoids foot sliding */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Limits",
              meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float MaxSpeedForIK = 350.0f;

    /** Enable debug line draws for traces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Debug")
    bool bDrawDebugTraces = false;

    // ── Blueprint helpers ─────────────────────────────────────────────────

    /** Returns true when IK is actively solving (character is slow enough + grounded) */
    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    bool IsIKActive() const { return bIKActive; }

    /** Force-enable or disable IK solving at runtime */
    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void SetIKEnabled(bool bEnabled);

private:
    // ── Internal helpers ──────────────────────────────────────────────────

    /** Solve IK for a single foot and write result into OutData */
    void SolveFootIK(const FName& SocketName, FAnim_FootIKData& OutData, float DeltaTime);

    /** Perform a downward line trace from above the socket; returns true on hit */
    bool TraceForGround(const FVector& SocketWorldLocation,
                        FHitResult& OutHit) const;

    // ── Cached references ─────────────────────────────────────────────────

    /** Owning character — cached in BeginPlay */
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    /** Skeletal mesh component — cached in BeginPlay */
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMesh = nullptr;

    /** Character movement — cached in BeginPlay */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent = nullptr;

    /** Whether IK is currently active this frame */
    bool bIKActive = false;

    /** Whether IK has been manually disabled */
    bool bIKEnabled = true;
};
