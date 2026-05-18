#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float DistanceFromGround;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpolationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float PelvisAdjustmentAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableDebugDraw;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        FootHeight = 5.0f;
        InterpolationSpeed = 10.0f;
        PelvisAdjustmentAlpha = 0.8f;
        bEnableDebugDraw = false;
    }
};

/**
 * IK Foot Placement Component
 * Handles realistic foot placement on uneven terrain using line traces
 * Provides smooth ground adaptation for character animation
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

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Foot IK Data
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    // Pelvis adjustment
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float PelvisOffset;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    FAnim_FootIKData CalculateFootIK(const FVector& FootLocation, const FName& BoneName);

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    float CalculatePelvisOffset();

    UFUNCTION(BlueprintCallable, Category = "IK Foot Placement")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "IK Foot Placement")
    bool IsIKEnabled() const { return bIKEnabled; }

protected:
    // Character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    // Skeletal mesh component reference
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    // IK state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bIKEnabled;

    // Bone names for foot IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootBoneName;

private:
    // Perform line trace for ground detection
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);
    
    // Calculate foot rotation from ground normal
    FRotator CalculateFootRotation(const FVector& GroundNormal, const FRotator& CurrentRotation);
    
    // Interpolate IK values smoothly
    FAnim_FootIKData InterpolateFootIK(const FAnim_FootIKData& Current, const FAnim_FootIKData& Target, float DeltaTime);
    
    // Debug drawing
    void DrawDebugIK();

    // Previous frame data for interpolation
    FAnim_FootIKData PreviousLeftFootIK;
    FAnim_FootIKData PreviousRightFootIK;
    float PreviousPelvisOffset;
};