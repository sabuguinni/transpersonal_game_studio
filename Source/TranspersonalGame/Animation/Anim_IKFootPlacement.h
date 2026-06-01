#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

// IK foot data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float TraceDistance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    bool bValidGround;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    FVector GroundNormal;

    FAnim_FootIKData()
    {
        BoneName = NAME_None;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        TraceDistance = 50.0f;
        bValidGround = false;
        GroundNormal = FVector::UpVector;
    }
};

// IK settings
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MinGroundAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxGroundAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHipAdjustment;

    FAnim_IKSettings()
    {
        InterpSpeed = 10.0f;
        MaxIKDistance = 100.0f;
        MinGroundAngle = -45.0f;
        MaxGroundAngle = 45.0f;
        bEnableFootRotation = true;
        bEnableHipAdjustment = true;
    }
};

/**
 * IK Foot Placement Component for realistic ground adaptation
 * Automatically adjusts character feet to terrain using inverse kinematics
 */
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

    // IK foot data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Feet")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Feet")
    FAnim_FootIKData RightFootIK;

    // IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Hip adjustment
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Hip")
    float HipOffset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Hip")
    float TargetHipOffset;

    // Character references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    class ACharacter* OwnerCharacter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    class USkeletalMeshComponent* SkeletalMesh;

    // IK functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootTrace(FAnim_FootIKData& FootData, const FVector& FootLocation);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void CalculateHipOffset();

    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_FootIKData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_FootIKData GetRightFootIK() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK")
    float GetHipOffset() const { return HipOffset; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "IK")
    bool IsIKEnabled() const { return bIKEnabled; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void ResetIK();

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "IK")
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FRotator CalculateFootRotation(const FVector& GroundNormal, const FRotator& CurrentRotation);

private:
    // Internal state
    bool bIKEnabled;
    float LastUpdateTime;
    float UpdateFrequency;

    // Bone names (configurable)
    UPROPERTY(EditAnywhere, Category = "IK Bones")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, Category = "IK Bones")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, Category = "IK Bones")
    FName HipBoneName;

    // Trace settings
    UPROPERTY(EditAnywhere, Category = "IK Trace")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    UPROPERTY(EditAnywhere, Category = "IK Trace")
    float TraceUpOffset;

    UPROPERTY(EditAnywhere, Category = "IK Trace")
    float TraceDownDistance;

    // Helper functions
    void InitializeComponent();
    FVector GetBoneWorldLocation(const FName& BoneName) const;
    bool IsValidGroundAngle(const FVector& GroundNormal) const;
    void InterpolateFootIK(FAnim_FootIKData& FootData, float DeltaTime);
};