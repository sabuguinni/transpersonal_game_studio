#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FVector IKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FRotator IKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    float DistanceFromGround = 0.0f;

    FAnim_FootIKData()
    {
        IKAlpha = 0.0f;
        IKLocation = FVector::ZeroVector;
        IKRotation = FRotator::ZeroRotator;
        DistanceFromGround = 0.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float HipOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bDrawDebugTraces = false;

    // Foot bone names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName HipBoneName = TEXT("pelvis");

    // IK Data - accessible to Animation Blueprint
    UPROPERTY(BlueprintReadOnly, Category = "IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data", meta = (AllowPrivateAccess = "true"))
    float HipIKOffset = 0.0f;

    // Public functions for Animation Blueprint
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetHipOffset() const { return HipIKOffset; }

private:
    // Internal functions
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, const FVector& FootLocation);
    FVector GetBoneWorldLocation(const FName& BoneName);
    bool PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);
    float CalculateHipOffset();

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Previous frame data for interpolation
    float PreviousLeftFootOffset = 0.0f;
    float PreviousRightFootOffset = 0.0f;
    float PreviousHipOffset = 0.0f;
};