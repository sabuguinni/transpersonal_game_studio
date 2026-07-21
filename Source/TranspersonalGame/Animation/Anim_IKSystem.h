#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_IKSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_IKType : uint8
{
    FootIK UMETA(DisplayName = "Foot IK"),
    HandIK UMETA(DisplayName = "Hand IK"),
    LookAtIK UMETA(DisplayName = "Look At IK"),
    FullBodyIK UMETA(DisplayName = "Full Body IK")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    bool bIsActive;

    FAnim_IKTarget()
    {
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        IKAlpha = 1.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float LeftFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float RightFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator LeftFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator RightFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKInterpSpeed;

    FAnim_FootIKData()
    {
        LeftFootOffset = 0.0f;
        RightFootOffset = 0.0f;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        HipOffset = 0.0f;
        IKInterpSpeed = 15.0f;
    }
};

/**
 * Advanced IK System for realistic character animation
 * Handles foot IK for terrain adaptation, hand IK for object interaction,
 * and look-at IK for natural character behavior
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core IK Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHandIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableLookAtIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    // Foot IK Data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData FootIKData;

    // IK Targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Targets")
    FAnim_IKTarget LeftHandTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Targets")
    FAnim_IKTarget RightHandTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Targets")
    FAnim_IKTarget LookAtTarget;

    // Component References
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    class ACharacter* OwnerCharacter;

public:
    // Foot IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    float PerformFootTrace(FName SocketName, float& OutFootRotation);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetFootIKEnabled(bool bEnabled);

    // Hand IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetLeftHandIKTarget(FVector Location, FRotator Rotation, float Alpha = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetRightHandIKTarget(FVector Location, FRotator Rotation, float Alpha = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void ClearHandIKTargets();

    // Look At IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetLookAtTarget(FVector Location, float Alpha = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void ClearLookAtTarget();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void ResetAllIK();

    UFUNCTION(BlueprintPure, Category = "IK System")
    FAnim_FootIKData GetFootIKData() const { return FootIKData; }

    UFUNCTION(BlueprintPure, Category = "IK System")
    bool IsFootIKEnabled() const { return bEnableFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK System")
    FAnim_IKTarget GetLeftHandTarget() const { return LeftHandTarget; }

    UFUNCTION(BlueprintPure, Category = "IK System")
    FAnim_IKTarget GetRightHandTarget() const { return RightHandTarget; }

    UFUNCTION(BlueprintPure, Category = "IK System")
    FAnim_IKTarget GetLookAtTarget() const { return LookAtTarget; }

private:
    // Internal Functions
    void InitializeIKSystem();
    void UpdateHandIK(float DeltaTime);
    void UpdateLookAtIK(float DeltaTime);
    FVector GetSocketWorldLocation(FName SocketName) const;
    FRotator CalculateFootRotationFromNormal(FVector SurfaceNormal) const;
};