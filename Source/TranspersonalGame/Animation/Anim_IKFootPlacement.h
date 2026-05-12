#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float DistanceFromGround = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    bool bIsGrounded = false;
};

USTRUCT(BlueprintType)
struct FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootRotationAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootIKBoneName = TEXT("ik_foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootIKBoneName = TEXT("ik_foot_r");
};

/**
 * Foot IK system for adaptive terrain walking
 * Provides realistic foot placement on uneven surfaces
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    FAnim_IKSettings IKSettings;

    // Current IK data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData LeftFootData;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData RightFootData;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* SkeletalMeshComponent;

    // IK state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    bool bEnableIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float IKUpdateRate = 60.0f;

private:
    // Internal tracking
    float LastUpdateTime;
    FAnim_FootIKData PreviousLeftFootData;
    FAnim_FootIKData PreviousRightFootData;

    // Helper functions
    void UpdateFootIK(FAnim_FootIKData& FootData, const FName& FootBoneName, float DeltaTime);
    FVector GetFootWorldLocation(const FName& FootBoneName) const;
    bool TraceForGround(const FVector& StartLocation, FVector& HitLocation, FVector& HitNormal) const;
    FRotator CalculateFootRotation(const FVector& HitNormal) const;
    void InterpolateFootData(FAnim_FootIKData& CurrentData, const FAnim_FootIKData& TargetData, float DeltaTime);

public:
    // Blueprint interface
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled) { bEnableIK = bEnabled; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    bool IsIKEnabled() const { return bEnableIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootData; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateIKSettings(const FAnim_IKSettings& NewSettings) { IKSettings = NewSettings; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_IKSettings GetIKSettings() const { return IKSettings; }

    // Animation Blueprint integration
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void GetFootIKTransforms(FTransform& LeftFootTransform, FTransform& RightFootTransform, float& LeftIKAlpha, float& RightIKAlpha) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void ForceUpdateIK();
};