#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Core/SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float GroundDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector CurrentOffset;

    FAnim_FootIKData()
    {
        BoneName = NAME_None;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsGrounded = true;
        GroundDistance = 0.0f;
        CurrentOffset = FVector::ZeroVector;
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
    float MaxFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootRotationLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    FAnim_IKSettings()
    {
        TraceDistance = 100.0f;
        InterpSpeed = 15.0f;
        MaxFootOffset = 50.0f;
        bEnableFootRotation = true;
        FootRotationLimit = 45.0f;
        TraceChannel = ECC_WorldStatic;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bShowDebugTraces;

    UPROPERTY(BlueprintReadOnly, Category = "Hip Adjustment")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip Adjustment")
    float HipInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hip Adjustment")
    bool bEnableHipAdjustment;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void TraceForGround(FAnim_FootIKData& FootData, const FVector& FootLocation);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector GetFootBoneLocation(const FName& BoneName);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void ResetIK();

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float CalculateHipOffset();

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetHipOffset() const { return HipOffset; }

private:
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class ACharacter* OwnerCharacter;

    void InitializeFootBones();
    void UpdateSingleFootIK(FAnim_FootIKData& FootData, float DeltaTime);
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal);
    bool IsCharacterMoving() const;
};