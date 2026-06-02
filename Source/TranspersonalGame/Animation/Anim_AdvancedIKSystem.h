#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Anim_AdvancedIKSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_IKTargetType : uint8
{
    LeftFoot     UMETA(DisplayName = "Left Foot"),
    RightFoot    UMETA(DisplayName = "Right Foot"),
    LeftHand     UMETA(DisplayName = "Left Hand"),
    RightHand    UMETA(DisplayName = "Right Hand"),
    Pelvis       UMETA(DisplayName = "Pelvis"),
    Spine        UMETA(DisplayName = "Spine")
};

USTRUCT(BlueprintType)
struct FAnim_IKTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    EAnim_IKTargetType TargetType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    bool bIsActive;

    FAnim_IKTarget()
    {
        TargetType = EAnim_IKTargetType::LeftFoot;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        IKAlpha = 1.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootOffset = 0.0f;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 1.0f;
        bIsGrounded = true;
    }
};

/**
 * Advanced IK System for character animation
 * Handles foot placement, hand IK, and full-body IK adjustments
 * Integrates with terrain adaptation and environmental interaction
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_AdvancedIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_AdvancedIKSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core IK functionality
    UFUNCTION(BlueprintCallable, Category = "Advanced IK")
    void UpdateIKTargets(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Advanced IK")
    void SetIKTarget(EAnim_IKTargetType TargetType, FVector Location, FRotator Rotation, float Alpha = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Advanced IK")
    FAnim_IKTarget GetIKTarget(EAnim_IKTargetType TargetType) const;

    // Foot IK system
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData CalculateFootPlacement(const FVector& FootLocation, const FName& SocketName);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetTerrainHeightAtLocation(const FVector& Location, float TraceDistance = 200.0f);

    // Hand IK system
    UFUNCTION(BlueprintCallable, Category = "Hand IK")
    void UpdateHandIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Hand IK")
    void SetHandIKTarget(bool bIsLeftHand, AActor* TargetActor, FVector Offset = FVector::ZeroVector);

    // Pelvis adjustment
    UFUNCTION(BlueprintCallable, Category = "Pelvis IK")
    void UpdatePelvisOffset(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Pelvis IK")
    float CalculatePelvisOffset() const;

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "IK Debug", CallInEditor)
    void ToggleIKDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "IK Debug")
    void DrawIKDebugInfo();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHandIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnablePelvisAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxPelvisOffset;

    // IK targets storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK State")
    TMap<EAnim_IKTargetType, FAnim_IKTarget> IKTargets;

    // Foot IK data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK State")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK State")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pelvis IK State")
    float PelvisOffset;

    // Debug settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowIKDebug;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowFootTraces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowIKTargets;

private:
    // Internal state
    float LastIKUpdateTime;
    USkeletalMeshComponent* OwnerMeshComponent;
    
    // Cached socket names
    FName LeftFootSocketName;
    FName RightFootSocketName;
    FName LeftHandSocketName;
    FName RightHandSocketName;

    // Helper functions
    void InitializeIKTargets();
    void CacheSocketNames();
    FVector GetSocketWorldLocation(const FName& SocketName) const;
    FRotator GetSocketWorldRotation(const FName& SocketName) const;
    bool PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const;
};