#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Engine/Engine.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float DistanceFromGround;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    bool bIsValidPlacement;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bIsValidPlacement = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHipAdjustment;

    FAnim_IKSettings()
    {
        TraceDistance = 100.0f;
        IKInterpSpeed = 10.0f;
        FootOffset = 5.0f;
        MaxIKAdjustment = 50.0f;
        bEnableFootIK = true;
        bEnableHipAdjustment = true;
    }
};

/**
 * Animation component for foot IK placement on uneven terrain
 * Automatically adjusts foot positions to match ground surface
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float HipOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class USkeletalMeshComponent* SkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Bone names for IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName HipBoneName;

    // Collision settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> TraceChannel;

private:
    // Internal state
    float CurrentHipOffset;
    float TargetHipOffset;
    
    // Interpolation targets
    FVector LeftFootTarget;
    FVector RightFootTarget;
    FRotator LeftFootRotationTarget;
    FRotator RightFootRotationTarget;

    // Methods
    void UpdateFootIK(float DeltaTime);
    void UpdateHipOffset(float DeltaTime);
    
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, float DeltaTime);
    bool PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    
    FRotator CalculateFootRotationFromNormal(const FVector& Normal) const;
    float CalculateHipAdjustment() const;

public:
    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK")
    float GetHipOffset() const { return HipOffset; }

    UFUNCTION(BlueprintPure, Category = "IK")
    bool IsIKEnabled() const { return IKSettings.bEnableFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void ResetIK();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugIK(bool bDrawDebug = true);
};