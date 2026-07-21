#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerBox.h"
#include "Crowd_PerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class ECrowd_OptimizationLevel : uint8
{
    Ultra_High UMETA(DisplayName = "Ultra High"),
    High UMETA(DisplayName = "High"),
    Medium UMETA(DisplayName = "Medium"),
    Low UMETA(DisplayName = "Low"),
    Minimal UMETA(DisplayName = "Minimal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PerformanceZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    ECrowd_OptimizationLevel OptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    FCrowd_PerformanceZone()
    {
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        OptimizationLevel = ECrowd_OptimizationLevel::Medium;
        MaxCrowdAgents = 100;
        UpdateFrequency = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableOcclusionCulling;

    FCrowd_LODSettings()
    {
        HighDetailDistance = 500.0f;
        MediumDetailDistance = 1000.0f;
        LowDetailDistance = 2000.0f;
        CullingDistance = 5000.0f;
        bEnableOcclusionCulling = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_PerformanceOptimizer : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_PerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Zones")
    TArray<FCrowd_PerformanceZone> PerformanceZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    FCrowd_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAdaptiveOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 StreamingPriority;

private:
    UPROPERTY()
    float LastPerformanceUpdate;

    UPROPERTY()
    float CurrentFrameRate;

    UPROPERTY()
    int32 CurrentCrowdCount;

    UPROPERTY()
    TArray<AActor*> ManagedCrowdActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCrowdPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterCrowdActor(AActor* CrowdActor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterCrowdActor(AActor* CrowdActor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    ECrowd_OptimizationLevel GetOptimizationLevelForLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldCullCrowdActor(const FVector& ActorLocation, const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateStreamingZones();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentPerformanceMetric();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void SetupPerformanceZones();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void VisualizePerformanceZones();
};

#include "Crowd_PerformanceOptimizer.generated.h"