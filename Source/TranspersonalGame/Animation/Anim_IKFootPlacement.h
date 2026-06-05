#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Anim_IKFootPlacement.generated.h"

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
    float DistanceToGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector GroundNormal;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceToGround = 0.0f;
        bIsGrounded = false;
        GroundNormal = FVector::UpVector;
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
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHipAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float HipAdjustmentInterpSpeed;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        FootHeight = 5.0f;
        MaxFootAdjustment = 30.0f;
        bEnableFootRotation = true;
        bEnableHipAdjustment = true;
        HipAdjustmentInterpSpeed = 10.0f;
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

    // IK Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    // Bone names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName HipBoneName;

    // Hip adjustment
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hip Adjustment")
    float HipOffset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hip Adjustment")
    float TargetHipOffset;

    // Main IK functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateSingleFootIK(FAnim_FootIKData& FootData, const FName& BoneName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateHipAdjustment(float DeltaTime);

    // Trace functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector GetBoneWorldLocation(const FName& BoneName);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator CalculateFootRotation(const FVector& GroundNormal);

    // Setup functions
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void InitializeIKSystem();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetSkeletalMeshComponent(USkeletalMeshComponent* MeshComp);

    // Getters for Animation Blueprint
    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetHipOffset() const { return HipOffset; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    bool IsIKEnabled() const;

    // Enable/Disable IK
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void ResetIK();

private:
    // Internal state
    bool bIKEnabled;
    bool bIsInitialized;

    // Smoothing variables
    float PreviousHipOffset;
    FVector PreviousLeftFootLocation;
    FVector PreviousRightFootLocation;

    // Internal helper functions
    void SmoothFootTransition(FAnim_FootIKData& FootData, const FVector& TargetLocation, const FRotator& TargetRotation, float DeltaTime);
    float CalculateFootIKAlpha(const FAnim_FootIKData& FootData);
    void ValidateIKData(FAnim_FootIKData& FootData);
    FVector GetCharacterVelocity();
    bool ShouldUpdateIK();
};