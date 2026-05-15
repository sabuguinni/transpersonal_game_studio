#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Anim_FootIKSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector LeftFootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector RightFootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float HipOffset;

    FAnim_FootIKData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootIKAlpha = 0.0f;
        RightFootIKAlpha = 0.0f;
        HipOffset = 0.0f;
    }
};

/**
 * Foot IK System for realistic character ground adaptation
 * Implements two-bone IK for feet to adapt to uneven terrain
 * Based on RDR2's character grounding system principles
 */
UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_FootIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_FootIKSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Get current IK data for animation blueprint
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetFootIKData() const { return FootIKData; }

    // Enable/disable foot IK
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool IsFootIKEnabled() const { return bFootIKEnabled; }

protected:
    // IK configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    bool bFootIKEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float HipOffsetInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float MaxHipOffset;

    // Bone names for IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    FName HipBoneName;

    // Current IK data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData FootIKData;

private:
    // Component references
    UPROPERTY()
    class ACharacter* OwningCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    // IK calculation methods
    void UpdateFootIK(float DeltaTime);
    FVector PerformFootTrace(const FVector& FootLocation, bool& bHitGround, FRotator& GroundRotation);
    float CalculateHipOffset(const FVector& LeftFootTarget, const FVector& RightFootTarget);
    FRotator CalculateFootRotationFromNormal(const FVector& Normal);
    
    // Helper methods
    FVector GetBoneWorldLocation(const FName& BoneName) const;
    bool IsCharacterMoving() const;
};