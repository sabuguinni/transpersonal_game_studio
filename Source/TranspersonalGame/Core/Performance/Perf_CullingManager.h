#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_CullingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxDrawDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float FrustumCullingMargin = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float OcclusionCullingThreshold = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    int32 MaxVisibleActors = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling = false;

    FPerf_CullingSettings()
    {
        MaxDrawDistance = 10000.0f;
        FrustumCullingMargin = 500.0f;
        OcclusionCullingThreshold = 0.1f;
        MaxVisibleActors = 2000;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
        bEnableOcclusionCulling = false;
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
    int32 DistanceCulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 FrustumCulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 OcclusionCulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CullingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MemorySavedMB = 0.0f;

    FPerf_CullingStats()
    {
        TotalActors = 0;
        VisibleActors = 0;
        DistanceCulledActors = 0;
        FrustumCulledActors = 0;
        OcclusionCulledActors = 0;
        CullingTime = 0.0f;
        MemorySavedMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_CullingManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_CullingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Culling Management
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void UpdateCulling(const FVector& ViewLocation, const FVector& ViewDirection, float FOV);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void SetCullingSettings(const FPerf_CullingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    FPerf_CullingSettings GetCullingSettings() const { return CullingSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    FPerf_CullingStats GetCullingStats() const { return CullingStats; }

    // Actor Registration
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void RegisterAllWorldActors();

    // Culling Controls
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void EnableDistanceCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void EnableFrustumCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void EnableOcclusionCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void SetMaxDrawDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void SetMaxVisibleActors(int32 MaxActors);

    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling", CallInEditor = true)
    void AnalyzeCullingPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling", CallInEditor = true)
    void OptimizeCullingSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling", CallInEditor = true)
    void ResetCullingStats();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void ToggleDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void DrawCullingDebugInfo();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_CullingSettings CullingSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    FPerf_CullingStats CullingStats;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredActors;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> VisibleActors;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> CulledActors;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowDebugInfo = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawCullingBounds = false;

    // Internal Methods
    void PerformDistanceCulling(const FVector& ViewLocation);
    void PerformFrustumCulling(const FVector& ViewLocation, const FVector& ViewDirection, float FOV);
    void PerformOcclusionCulling(const FVector& ViewLocation);
    void UpdateActorVisibility();
    void CalculateMemorySavings();
    bool IsActorInFrustum(AActor* Actor, const FVector& ViewLocation, const FVector& ViewDirection, float FOV) const;
    float CalculateActorMemoryUsage(AActor* Actor) const;

    // Timer Handles
    FTimerHandle CullingUpdateTimer;
    FTimerHandle StatsUpdateTimer;

    // Performance Tracking
    double LastCullingTime = 0.0;
    int32 CullingFrameCounter = 0;
};