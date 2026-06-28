// FootIKComponent.h
// Animation Agent #10 — Cycle PROD_CYCLE_AUTO_20260628_011
// Per-foot terrain IK component — solves foot placement on uneven ground
// using line traces and pelvis offset compensation.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootIKComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// FAnim_FootIKData — per-foot IK solve result
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    /** World-space target location for the foot effector */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector TargetLocation = FVector::ZeroVector;

    /** Surface normal at the foot contact point */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector SurfaceNormal = FVector::UpVector;

    /** IK blend weight (0 = no IK, 1 = full IK) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float BlendAlpha = 0.f;

    /** Pelvis Z offset contribution from this foot */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float PelvisOffset = 0.f;

    /** True when foot trace hit valid ground */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    bool bIsGrounded = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// UFootIKComponent
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Animation", meta = (BlueprintSpawnableComponent),
       DisplayName = "Foot IK Component")
class TRANSPERSONALGAME_API UFootIKComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFootIKComponent();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Public API ─────────────────────────────────────────────────────────

    /** Solve IK for a single foot socket. Returns solve result. */
    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    FAnim_FootIKData SolveFootIK(const FName& SocketName, float DeltaTime);

    /** Get the last solved data for the left foot */
    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    FAnim_FootIKData GetLeftFootData() const { return LeftFootData; }

    /** Get the last solved data for the right foot */
    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    FAnim_FootIKData GetRightFootData() const { return RightFootData; }

    /** Combined pelvis offset (lowest foot drives this) */
    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    FVector GetPelvisOffset() const { return CombinedPelvisOffset; }

    // ── Configuration ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Sockets")
    FName LeftFootSocket = FName("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Sockets")
    FName RightFootSocket = FName("foot_r");

    /** How far above the socket to start the trace */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Trace",
              meta = (ClampMin = "0", ClampMax = "200"))
    float TraceStartOffset = 50.f;

    /** How far below the socket to end the trace */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Trace",
              meta = (ClampMin = "0", ClampMax = "300"))
    float TraceEndOffset = 75.f;

    /** Collision channel for foot traces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Trace")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    /** Interpolation speed for foot target position */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Interp",
              meta = (ClampMin = "1", ClampMax = "30"))
    float FootInterpSpeed = 12.f;

    /** Interpolation speed for pelvis offset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Interp",
              meta = (ClampMin = "1", ClampMax = "20"))
    float PelvisInterpSpeed = 6.f;

    /** Maximum pelvis Z displacement allowed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Limits",
              meta = (ClampMin = "0", ClampMax = "50"))
    float MaxPelvisOffset = 20.f;

    /** IK blend-in speed when grounded */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Interp",
              meta = (ClampMin = "1", ClampMax = "20"))
    float IKBlendInSpeed = 8.f;

    /** IK blend-out speed when in air */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Interp",
              meta = (ClampMin = "1", ClampMax = "20"))
    float IKBlendOutSpeed = 4.f;

    /** Draw debug traces in editor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK|Debug")
    bool bDrawDebugTraces = false;

private:
    // ── Cached references ──────────────────────────────────────────────────
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* OwnerMesh;

    // ── Solve results ──────────────────────────────────────────────────────
    FAnim_FootIKData LeftFootData;
    FAnim_FootIKData RightFootData;

    FVector CombinedPelvisOffset;

    // ── Smoothed targets ───────────────────────────────────────────────────
    FVector SmoothedLeftTarget;
    FVector SmoothedRightTarget;
    float   SmoothedPelvisZ;

    // ── Internal helpers ───────────────────────────────────────────────────
    bool PerformFootTrace(const FVector& StartLoc, const FVector& EndLoc,
                          FHitResult& OutHit) const;
    void UpdatePelvisOffset(float DeltaTime);
    void PushDataToAnimInstance();
};
