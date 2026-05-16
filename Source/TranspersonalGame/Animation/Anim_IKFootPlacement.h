#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootData
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
    bool bValidFootPlacement;

    FAnim_FootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bValidFootPlacement = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnablePelvisAdjustment;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        FootOffset = 5.0f;
        InterpSpeed = 15.0f;
        MaxFootAdjustment = 30.0f;
        bEnableFootIK = true;
        bEnablePelvisAdjustment = true;
    }
};

/**
 * IK Foot Placement Component for realistic foot positioning on uneven terrain
 * Provides foot IK data for animation blueprints
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Foot data
    UPROPERTY(BlueprintReadOnly, Category = "IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootData LeftFootData;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootData RightFootData;

    // Pelvis adjustment
    UPROPERTY(BlueprintReadOnly, Category = "IK Data", meta = (AllowPrivateAccess = "true"))
    float PelvisOffset;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    ACharacter* OwnerCharacter;

    // Bone names for IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName PelvisBoneName;

public:
    // IK calculation functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootData GetLeftFootData() const { return LeftFootData; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootData GetRightFootData() const { return RightFootData; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    bool IsIKEnabled() const { return IKSettings.bEnableFootIK; }

private:
    // Internal IK calculation functions
    FAnim_FootData CalculateFootIK(const FName& FootBoneName, float DeltaTime);
    FHitResult PerformFootTrace(const FVector& FootLocation);
    float CalculatePelvisOffset();
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal);

    // Interpolation values for smooth IK transitions
    float TargetPelvisOffset;
    float CurrentPelvisOffset;
    FAnim_FootData TargetLeftFootData;
    FAnim_FootData TargetRightFootData;
};