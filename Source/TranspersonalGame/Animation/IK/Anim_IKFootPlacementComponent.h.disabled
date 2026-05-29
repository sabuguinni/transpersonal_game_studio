#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Anim_IKFootPlacementComponent.generated.h"

// Foot placement data for a single foot
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootData
{
    GENERATED_BODY()

    // Target location for the foot in world space
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector TargetLocation = FVector::ZeroVector;

    // Target rotation for the foot (to match terrain normal)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator TargetRotation = FRotator::ZeroRotator;

    // IK alpha (0 = FK, 1 = IK)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IKAlpha = 1.0f;

    // Distance from original foot position
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float OffsetDistance = 0.0f;

    // Is this foot currently planted on ground
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    bool bIsPlanted = false;

    // Time since last foot plant
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float TimeSincePlant = 0.0f;

    FAnim_FootData()
    {
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        IKAlpha = 1.0f;
        OffsetDistance = 0.0f;
        bIsPlanted = false;
        TimeSincePlant = 0.0f;
    }
};

// IK settings for the foot placement system
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    // Maximum distance to trace for ground detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float TraceDistance = 100.0f;

    // Offset above ground for foot placement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float FootOffset = 2.0f;

    // Speed of IK interpolation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "1.0", ClampMax = "20.0"))
    float InterpolationSpeed = 10.0f;

    // Maximum foot adjustment distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "5.0", ClampMax = "50.0"))
    float MaxFootAdjustment = 25.0f;

    // Pelvis adjustment multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PelvisAdjustmentAlpha = 0.5f;

    FAnim_IKSettings()
    {
        TraceDistance = 100.0f;
        FootOffset = 2.0f;
        InterpolationSpeed = 10.0f;
        MaxFootAdjustment = 25.0f;
        PelvisAdjustmentAlpha = 0.5f;
    }
};

/**
 * IK Foot Placement Component for realistic terrain adaptation
 * Makes characters feel grounded and weighted on uneven prehistoric terrain
 * Inspired by RDR2's foot placement system that makes Arthur feel real
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacementComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacementComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // IK settings for this character
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Left foot data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootData LeftFootData;

    // Right foot data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootData RightFootData;

    // Pelvis offset to accommodate foot placement
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector PelvisOffset = FVector::ZeroVector;

    // Enable/disable IK foot placement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableFootIK = true;

    // Bone names for foot IK (set in Blueprint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName PelvisBoneName = TEXT("pelvis");

    // Update foot IK for both feet
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    // Get foot placement for a specific foot
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool GetFootPlacement(const FName& FootBoneName, FVector& OutLocation, FRotator& OutRotation, float& OutIKAlpha);

    // Enable or disable foot IK
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootIKEnabled(bool bEnabled);

    // Check if character is on steep terrain
    UFUNCTION(BlueprintPure, Category = "Foot IK")
    bool IsOnSteepTerrain() const;

    // Get the average foot offset (for pelvis adjustment)
    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetAverageFootOffset() const;

    // Event called when foot placement changes significantly
    UFUNCTION(BlueprintImplementableEvent, Category = "Foot IK")
    void OnFootPlacementChanged(bool bLeftFootAdjusted, bool bRightFootAdjusted);

private:
    // Cached skeletal mesh component
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    // Previous pelvis offset for smooth interpolation
    FVector PreviousPelvisOffset;

    // Perform ground trace for a foot
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    // Calculate foot placement for a specific foot
    void CalculateFootPlacement(FAnim_FootData& FootData, const FName& FootBoneName, float DeltaTime);

    // Update pelvis offset based on foot positions
    void UpdatePelvisOffset(float DeltaTime);

    // Get bone location in world space
    FVector GetBoneWorldLocation(const FName& BoneName) const;

    // Convert world location to component space
    FVector WorldToComponentSpace(const FVector& WorldLocation) const;

    // Initialize the IK system
    void InitializeIKSystem();
};