#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Perf_CullingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float FrustumCullingDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float OcclusionCullingDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float DistanceCullingThreshold = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    int32 MaxVisibleActors = 1000;

    FPerf_CullingSettings()
    {
        FrustumCullingDistance = 15000.0f;
        OcclusionCullingDistance = 8000.0f;
        bEnableFrustumCulling = true;
        bEnableOcclusionCulling = true;
        bEnableDistanceCulling = true;
        DistanceCullingThreshold = 20000.0f;
        MaxVisibleActors = 1000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 FrustumCulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 DistanceCulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 OcclusionCulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CullingUpdateTime = 0.0f;

    FPerf_CullingStats()
    {
        TotalActors = 0;
        VisibleActors = 0;
        FrustumCulledActors = 0;
        DistanceCulledActors = 0;
        OcclusionCulledActors = 0;
        CullingUpdateTime = 0.0f;
    }
};

/**
 * Advanced Culling System for Performance Optimization
 * Implements frustum culling, distance culling, and occlusion culling
 * Manages visibility of actors based on camera view and distance
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_CullingSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // Culling Operations
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void UpdateCulling(const FVector& ViewLocation, const FRotator& ViewRotation, float FOV);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    bool IsActorInFrustum(AActor* Actor, const FVector& ViewLocation, const FRotator& ViewRotation, float FOV) const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    bool IsActorOccluded(AActor* Actor, const FVector& ViewLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void CullActorByDistance(AActor* Actor, const FVector& ViewLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void CullActorByFrustum(AActor* Actor, bool bShouldCull);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void CullActorByOcclusion(AActor* Actor, bool bShouldCull);

    // Settings Management
    UFUNCTION(BlueprintCallable, Category = "Performance|Settings")
    void SetCullingSettings(const FPerf_CullingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance|Settings")
    FPerf_CullingSettings GetCullingSettings() const { return CullingSettings; }

    // Stats and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|Stats")
    FPerf_CullingStats GetCullingStats() const { return CullingStats; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Stats")
    void ResetCullingStats();

    // Actor Registration
    UFUNCTION(BlueprintCallable, Category = "Performance|Management")
    void RegisterActorForCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Management")
    void UnregisterActorFromCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Management")
    void RegisterAllActorsForCulling();

    // Performance Controls
    UFUNCTION(BlueprintCallable, Category = "Performance|Controls")
    void SetCullingEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance|Controls")
    bool IsCullingEnabled() const { return bCullingEnabled; }

protected:
    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling Settings")
    FPerf_CullingSettings CullingSettings;

    // Runtime Data
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredActors;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> CulledActors;

    UPROPERTY()
    FPerf_CullingStats CullingStats;

    UPROPERTY()
    bool bCullingEnabled;

    // Internal Methods
    void PerformFrustumCulling(const FVector& ViewLocation, const FRotator& ViewRotation, float FOV);
    void PerformDistanceCulling(const FVector& ViewLocation);
    void PerformOcclusionCulling(const FVector& ViewLocation);
    void UpdateVisibilityForActor(AActor* Actor, bool bShouldBeVisible);
    FVector GetActorBoundsCenter(AActor* Actor) const;
    float GetActorBoundsRadius(AActor* Actor) const;
    void CleanupInvalidActors();

private:
    // Timer for periodic culling updates
    FTimerHandle CullingUpdateTimer;
    void PeriodicCullingUpdate();
    
    // Performance tracking
    double LastCullingUpdateTime;
    TArray<float> CullingUpdateTimes;
    void UpdatePerformanceStats(float UpdateTime);
};