#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Anim_IKController.generated.h"

/**
 * IK Controller for foot placement and terrain adaptation
 * Handles Two Bone IK for feet to adapt to uneven terrain
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset = 30.0f;

    // Foot bone names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftThighBoneName = TEXT("thigh_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightThighBoneName = TEXT("thigh_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftCalfBoneName = TEXT("calf_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightCalfBoneName = TEXT("calf_r");

    // Current IK values
    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    float LeftFootIKOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    float RightFootIKOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    FRotator LeftFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    FRotator RightFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    float HipOffset = 0.0f;

    // IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    float TraceForGround(const FVector& StartLocation, const FVector& EndLocation, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FRotator CalculateFootRotation(const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "IK")
    bool IsIKEnabled() const { return bEnableFootIK; }

    // Advanced IK features
    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetFootBoneNames(const FName& LeftFoot, const FName& RightFoot);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetLegBoneNames(const FName& LeftThigh, const FName& LeftCalf, const FName& RightThigh, const FName& RightCalf);

protected:
    // Internal state
    class ACharacter* OwnerCharacter;
    class USkeletalMeshComponent* SkeletalMeshComp;
    
    // Target values for smooth interpolation
    float TargetLeftFootOffset = 0.0f;
    float TargetRightFootOffset = 0.0f;
    FRotator TargetLeftFootRotation = FRotator::ZeroRotator;
    FRotator TargetRightFootRotation = FRotator::ZeroRotator;
    float TargetHipOffset = 0.0f;

    // Trace settings
    UPROPERTY(EditAnywhere, Category = "Trace Settings")
    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

    UPROPERTY(EditAnywhere, Category = "Trace Settings")
    bool bTraceComplex = false;

    UPROPERTY(EditAnywhere, Category = "Trace Settings")
    bool bIgnoreSelf = true;

    // Helper functions
    FVector GetFootWorldLocation(const FName& BoneName) const;
    void InitializeComponent();
    void UpdateHipOffset();
    float CalculateHipAdjustment() const;
};

/**
 * Struct for IK bone configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKBoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bones")
    FName FootBone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bones")
    FName ThighBone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bones")
    FName CalfBone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxOffset = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TraceDistance = 50.0f;

    FAnim_IKBoneConfig()
    {
        FootBone = NAME_None;
        ThighBone = NAME_None;
        CalfBone = NAME_None;
        MaxOffset = 30.0f;
        TraceDistance = 50.0f;
    }
};

/**
 * Enum for IK states
 */
UENUM(BlueprintType)
enum class EAnim_IKState : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Enabled     UMETA(DisplayName = "Enabled"),
    Transitioning UMETA(DisplayName = "Transitioning")
};

#include "Anim_IKController.generated.h"