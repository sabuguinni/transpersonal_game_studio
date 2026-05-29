#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_IKFootPlacementSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float DistanceFromGround;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootAdjustment;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        FootOffset = 5.0f;
        bEnableFootRotation = true;
        MaxFootAdjustment = 30.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacementSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacementSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK Foot Data
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Bone Names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName PelvisBoneName;

    // Hip Adjustment
    UPROPERTY(BlueprintReadOnly, Category = "Hip Adjustment")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip Adjustment")
    float HipInterpSpeed;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateHipOffset(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    FVector PerformFootTrace(const FVector& FootLocation, float& OutDistance);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "IK System")
    bool IsIKEnabled() const { return bEnableIK; }

private:
    // Internal state
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComp;

    float LastUpdateTime;
    
    // Cache for performance
    FVector CachedActorLocation;
    FRotator CachedActorRotation;
    
    // Helper functions
    FVector GetBoneWorldLocation(const FName& BoneName) const;
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const;
    bool ShouldUpdateIK() const;
};