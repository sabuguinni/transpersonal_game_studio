#pragma once

#include "CoreMinimal.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Animation/AnimationAsset.h"
#include "SurvivalLocomotionSchema.generated.h"

/**
 * Motion Matching Schema specifically designed for survival locomotion
 * Focuses on cautious, reactive movement patterns with environmental awareness
 * 
 * Based on RDR2's approach where every step tells a story about the character's state of mind
 */

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API USurvivalLocomotionSchema : public UPoseSearchSchema
{
    GENERATED_BODY()

public:
    USurvivalLocomotionSchema();

    // Survival-specific schema configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Schema")
    float CautiousMovementWeight = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Schema")
    float HeadTrackingWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Schema")
    float FootPlacementWeight = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Schema")
    float ShoulderTensionWeight = 1.2f;

    // Terror level influences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terror Response")
    float PanicMovementMultiplier = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terror Response")
    float StartleReactionWeight = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terror Response")
    float EnvironmentalAwarenessRange = 500.0f;

protected:
    virtual void PostInitProperties() override;
    
    // Setup survival-specific pose search channels
    void SetupSurvivalChannels();
    void SetupTrajectoryChannel();
    void SetupPoseChannel();
    void SetupHeadTrackingChannel();
    void SetupFootIKChannel();

private:
    // Internal configuration
    bool bIsInitialized = false;
};