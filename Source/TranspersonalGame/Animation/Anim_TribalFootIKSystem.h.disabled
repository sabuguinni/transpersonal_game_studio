#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_TribalFootIKSystem.generated.h"

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
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsGrounded = false;
    }
};

/**
 * Tribal Foot IK System for realistic ground adaptation
 * Ensures character feet properly plant on uneven prehistoric terrain
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_TribalFootIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TribalFootIKSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Foot IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    bool bEnableFootIK;

    // Bone Names for IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Settings")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Settings")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Settings")
    FName HipBoneName;

    // IK Data Access
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIKData; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIKData; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetHipOffsetZ() const { return CurrentHipOffsetZ; }

    // Manual IK Update
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootIKEnabled(bool bEnabled) { bEnableFootIK = bEnabled; }

protected:
    // IK Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIKData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    float CurrentHipOffsetZ;

    // Cached References
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> CachedMeshComponent;

    UPROPERTY()
    TObjectPtr<UAnimInstance> CachedAnimInstance;

    // Internal Methods
    void CacheComponents();
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, const FVector& FootSocketLocation);
    FVector PerformGroundTrace(const FVector& StartLocation, float TraceLength);
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal);
    void UpdateHipOffset();
    float GetLowestFootOffset() const;
};