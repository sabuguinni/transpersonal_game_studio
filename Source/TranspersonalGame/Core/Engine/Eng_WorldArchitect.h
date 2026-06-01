#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "Eng_BiomeManager.h"
#include "Eng_WorldArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FVector WorldSize = FVector(200000, 200000, 10000);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 MaxTotalActors = 20000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 MaxDinosaurs = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 MaxVegetation = 15000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 MaxProps = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bEnableWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float WorldStreamingDistance = 50000.0f;

    FEng_WorldConfiguration()
    {
        // Default values already set above
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceGood = true;

    FEng_PerformanceMetrics()
    {
        // Default values already set above
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_WorldArchitect : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitect();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ValidateWorldConfiguration();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool CanSpawnActor(const FString& ActorType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void RegisterSpawnedActor(AActor* Actor, const FString& ActorType);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void UnregisterActor(AActor* Actor, const FString& ActorType);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    FEng_PerformanceMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void OptimizeWorldPerformance();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void CleanupExcessActors();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    TArray<FVector> GetOptimalSpawnLocations(const FString& ActorType, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void SetWorldConfiguration(const FEng_WorldConfiguration& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    FEng_WorldConfiguration GetWorldConfiguration() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void EnablePerformanceMonitoring(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void LogWorldStatistics();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool IsWorldHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ForceGarbageCollection();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_WorldConfiguration WorldConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bPerformanceMonitoringEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float PerformanceCheckInterval = 5.0f;

    UPROPERTY()
    float LastPerformanceCheck = 0.0f;

    UPROPERTY()
    int32 CurrentDinosaurCount = 0;

    UPROPERTY()
    int32 CurrentVegetationCount = 0;

    UPROPERTY()
    int32 CurrentPropCount = 0;

    UPROPERTY()
    int32 CurrentTotalActors = 0;

private:
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void UpdateActorCounts();
    bool ShouldOptimizePerformance() const;
    void PerformLODOptimization();
    void PerformCullingOptimization();
    UEng_BiomeManager* GetBiomeManager() const;
};