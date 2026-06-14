#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Core/SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

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
    float JointTargetOffset;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        JointTargetOffset = 0.0f;
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

    // IK Data for feet
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_FootIKData RightFootIK;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK|Settings")
    float TraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK|Settings")
    float IKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK|Settings")
    float FootOffset = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK|Settings")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK|Settings")
    bool bEnableDebugDraw = false;

    // Bone names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK|Bones")
    FName LeftFootBoneName = "foot_l";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK|Bones")
    FName RightFootBoneName = "foot_r";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK|Bones")
    FName PelvisBoneName = "pelvis";

    // Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetLeftFootIKData() const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetRightFootIKData() const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetPelvisOffset() const;

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    // Internal state
    float PelvisOffset;
    float TargetPelvisOffset;

    // Helper functions
    void CacheReferences();
    FAnim_FootIKData CalculateFootIK(FName FootBoneName, float DeltaTime);
    FVector GetFootWorldLocation(FName FootBoneName) const;
    bool PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    FRotator CalculateFootRotation(const FVector& HitNormal, const FVector& FootForward) const;
    void UpdatePelvisOffset(float LeftFootOffset, float RightFootOffset, float DeltaTime);
    void DrawDebugInfo() const;
};