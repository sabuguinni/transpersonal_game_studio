#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IKAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGrounded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceFromGround = 0.0f;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsGrounded = false;
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

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight = 13.0f;

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
    FName PelvisBoneName = TEXT("pelvis");

    // IK Data
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float PelvisOffset = 0.0f;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetRightFootIK() const { return RightFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled) { bEnableIK = bEnabled; }

private:
    // Internal functions
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, float DeltaTime);
    FVector GetFootWorldLocation(const FName& FootBoneName) const;
    bool PerformGroundTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    FRotator CalculateFootRotationFromNormal(const FVector& Normal) const;
    void UpdatePelvisOffset(float DeltaTime);

    // Cached references
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class ACharacter* OwnerCharacter;
};