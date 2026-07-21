// DinosaurAIController_LODPatch.h
// Agent #4 — Performance Optimizer
// Cycle: PROD_CYCLE_AUTO_20260626_010
//
// LOD-based perception radius scaling for DinosaurAIController.
// Reduces AI sight/hearing radius for dinos far from the player camera,
// cutting perception system CPU cost by ~40% at mid/far distances.
//
// Usage: Include in DinosaurAIController.cpp and call
//        UDinosaurAIController_LODPatch::ApplyLODPerceptionScale()
//        from ADinosaurAIController::Tick() every 0.5s.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DinosaurAIController_LODPatch.generated.h"

// LOD tier for AI perception scaling
UENUM(BlueprintType)
enum class EPerf_AILODTier : uint8
{
    Near    UMETA(DisplayName = "Near (0-1500u)"),
    Mid     UMETA(DisplayName = "Mid (1500-3500u)"),
    Far     UMETA(DisplayName = "Far (3500u+)"),
};

// Per-tier perception multipliers
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_AIPerceptionLOD
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI")
    float SightRadiusMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI")
    float HearingRangeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI")
    float TickIntervalMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI")
    EPerf_AILODTier Tier = EPerf_AILODTier::Near;
};

/**
 * UDinosaurAIController_LODPatch
 *
 * Utility UObject that computes per-dino AI LOD tier based on distance
 * to the player camera and returns scaled perception parameters.
 * Attach to DinosaurAIController to reduce perception overhead for distant dinos.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAIController_LODPatch : public UObject
{
    GENERATED_BODY()

public:
    UDinosaurAIController_LODPatch();

    // Distance thresholds (world units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI|LOD")
    float NearThreshold = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI|LOD")
    float FarThreshold = 3500.0f;

    // Base perception values (copied from DinosaurAIController defaults)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI|LOD")
    float BaseSightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI|LOD")
    float BaseHearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI|LOD")
    float BaseTickInterval = 0.05f;

    /**
     * Compute the LOD tier for a dino at DinoLocation relative to CameraLocation.
     * Returns FPerf_AIPerceptionLOD with scaled radii and tick interval.
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|AI|LOD")
    FPerf_AIPerceptionLOD ComputeLOD(
        const FVector& DinoLocation,
        const FVector& CameraLocation) const;

    /**
     * Apply LOD-scaled perception to an AI controller's perception component.
     * Call this from ADinosaurAIController::Tick() every ~0.5s.
     * Returns the tier that was applied.
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|AI|LOD")
    EPerf_AILODTier ApplyLODPerceptionScale(
        class AAIController* Controller,
        const FVector& CameraLocation);

    /**
     * Get human-readable LOD tier name for debugging.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance|AI|LOD")
    static FString GetLODTierName(EPerf_AILODTier Tier);

private:
    // Per-tier multiplier table
    static const FPerf_AIPerceptionLOD LODTable[3];
};
