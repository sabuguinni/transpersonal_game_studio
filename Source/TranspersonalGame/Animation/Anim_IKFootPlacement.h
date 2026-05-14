#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float LeftFootOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float RightFootOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float PelvisOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float IKAlpha = 1.0f;

    FAnim_FootIKData()
    {
        LeftFootOffset = 0.0f;
        RightFootOffset = 0.0f;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        PelvisOffset = 0.0f;
        IKAlpha = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight = 13.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableDebugDraw = false;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        MaxFootOffset = 30.0f;
        FootHeight = 13.0f;
        bEnableIK = true;
        bEnableDebugDraw = false;
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

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetFootIKData() const { return FootIKData; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKAlpha(float Alpha);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetFootOffset(bool bIsLeftFoot) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator GetFootRotation(bool bIsLeftFoot) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName PelvisBoneName = TEXT("pelvis");

private:
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Smoothed values for interpolation
    float SmoothedLeftFootOffset;
    float SmoothedRightFootOffset;
    FRotator SmoothedLeftFootRotation;
    FRotator SmoothedRightFootRotation;
    float SmoothedPelvisOffset;

    // Internal methods
    float PerformFootTrace(const FVector& FootLocation, FRotator& OutFootRotation);
    FVector GetFootWorldLocation(bool bIsLeftFoot) const;
    void CalculatePelvisOffset();
    void SmoothIKValues(float DeltaTime);
    void DrawDebugInfo() const;
};