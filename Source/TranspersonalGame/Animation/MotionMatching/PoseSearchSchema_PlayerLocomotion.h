#pragma once

#include "CoreMinimal.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "PoseSearchSchema_PlayerLocomotion.generated.h"

/**
 * Motion Matching Schema for Player Locomotion
 * Optimized for survival gameplay with fear-based movement patterns
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPoseSearchSchema_PlayerLocomotion : public UPoseSearchSchema
{
    GENERATED_BODY()

public:
    UPoseSearchSchema_PlayerLocomotion();

    // Fear state influences on movement quality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System")
    float FearInfluenceWeight = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System")
    float TensionMultiplier = 1.5f;

    // Terrain adaptation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TerrainAdaptationWeight = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SlopeThreshold = 25.0f;

protected:
    virtual void PostLoad() override;
    virtual void PostInitProperties() override;

private:
    void SetupLocomotionChannels();
    void SetupFearChannels();
    void SetupTerrainChannels();
};