#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float DistanceFromGround = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    bool bIsValidPlacement = false;

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
    float TraceDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight = 13.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKAdjustment = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MinSpeedForIK = 10.0f;

    FAnim_IKSettings()
    {
        TraceDistance = 150.0f;
        InterpSpeed = 15.0f;
        FootHeight = 13.0f;
        MaxIKAdjustment = 50.0f;
        bEnableFootIK = true;
        bEnableFootRotation = true;
        MinSpeedForIK = 10.0f;
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

    // Main IK Functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIKData; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIKData; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    bool IsIKEnabled() const { return IKSettings.bEnableFootIK; }

    // Bone Name Configuration
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootBoneNames(const FName& LeftFootBone, const FName& RightFootBone);

protected:
    // IK Data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float PelvisOffset = 0.0f;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Bone Names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName PelvisBoneName = TEXT("pelvis");

    // Component References
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* OwnerMovementComponent;

    // Internal State
    UPROPERTY()
    float TargetPelvisOffset = 0.0f;

    UPROPERTY()
    bool bIKSystemInitialized = false;

    // Internal Functions
    void CacheComponentReferences();
    void InitializeIKSystem();
    
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, float DeltaTime);
    FVector GetFootWorldLocation(const FName& FootBoneName) const;
    bool PerformFootTrace(const FVector& FootLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    
    void UpdatePelvisOffset(float DeltaTime);
    float CalculateRequiredPelvisOffset() const;
    
    bool ShouldUseIK() const;
    void ResetIKData();

    // Utility Functions
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const;
    float GetCharacterSpeed() const;
    bool IsCharacterGrounded() const;
};