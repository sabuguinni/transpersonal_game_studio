#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Anim_AdvancedIKSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKBoneTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bIsActive;

    FAnim_IKBoneTarget()
    {
        BoneName = NAME_None;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        BlendWeight = 1.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float GroundDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bHasValidGround;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        GroundDistance = 0.0f;
        bHasValidGround = false;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_AdvancedIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_AdvancedIKSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Foot IK System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName PelvisBoneName;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float PelvisOffset;

    // Look At IK System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    bool bEnableLookAtIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    FName HeadBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    FVector LookAtTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    float LookAtBlendSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    float MaxLookAtAngle;

    // Spine IK System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine IK")
    bool bEnableSpineIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine IK")
    TArray<FName> SpineBoneNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine IK")
    float SpineFlexibility;

    // General IK Targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Targets")
    TArray<FAnim_IKBoneTarget> IKBoneTargets;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Look At IK")
    void UpdateLookAtIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Spine IK")
    void UpdateSpineIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetLookAtTarget(const FVector& NewTarget);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void AddIKBoneTarget(const FName& BoneName, const FVector& TargetLocation, const FRotator& TargetRotation, float BlendWeight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void RemoveIKBoneTarget(const FName& BoneName);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void EnableLookAtIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void EnableSpineIK(bool bEnable);

private:
    UPROPERTY()
    USkeletalMeshComponent* OwnerMeshComponent;

    // Internal foot IK functions
    bool PerformFootTrace(const FVector& FootLocation, FAnim_FootIKData& FootData);
    FRotator CalculateFootRotationFromNormal(const FVector& Normal);
    void UpdatePelvisOffset(float DeltaTime);

    // Internal look at IK functions
    FRotator CalculateLookAtRotation(const FVector& CurrentLocation, const FVector& TargetLocation);
    bool IsLookAtTargetValid();

    // Internal spine IK functions
    void CalculateSpineBendRotations(const FVector& TargetDirection, TArray<FRotator>& OutRotations);
};