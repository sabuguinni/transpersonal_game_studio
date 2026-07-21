#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngArchitect_WorldSystemManager.generated.h"

class UBiomeManager;
class UTerrainManager;
class UWeatherManager;

/**
 * Engine Architect - World System Manager
 * Core architectural subsystem that coordinates all world-level systems
 * Manages biome transitions, terrain streaming, weather cycles
 * Ensures proper initialization order and system dependencies
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngArchitect_WorldSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArchitect_WorldSystemManager();

    // USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World System Management
    UFUNCTION(BlueprintCallable, Category = "World System")
    void InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World System")
    void ShutdownWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World System")
    bool AreWorldSystemsInitialized() const { return bSystemsInitialized; }

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetCurrentBiome(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeTransition(const FVector& Location, EBiomeType FromBiome, EBiomeType ToBiome);

    // System References
    UFUNCTION(BlueprintCallable, Category = "System Access")
    UBiomeManager* GetBiomeManager() const { return BiomeManager; }

    UFUNCTION(BlueprintCallable, Category = "System Access")
    UTerrainManager* GetTerrainManager() const { return TerrainManager; }

    UFUNCTION(BlueprintCallable, Category = "System Access")
    UWeatherManager* GetWeatherManager() const { return WeatherManager; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetWorldSystemsPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerformanceLevel NewLevel);

protected:
    // Core System References
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    UBiomeManager* BiomeManager;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    UTerrainManager* TerrainManager;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    UWeatherManager* WeatherManager;

    // System State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bSystemsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EPerformanceLevel CurrentPerformanceLevel;

    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveBiomeRegions;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 StreamedTerrainChunks;

private:
    // Internal System Management
    void InitializeBiomeSystem();
    void InitializeTerrainSystem();
    void InitializeWeatherSystem();
    void ValidateSystemDependencies();

    // Performance Monitoring
    void UpdatePerformanceMetrics();
    void AdjustSystemsForPerformance();

    // Biome Transition Cache
    TMap<FVector, EBiomeType> BiomeTransitionCache;
    
    // System Initialization Order
    TArray<FString> SystemInitOrder;
    TArray<FString> InitializedSystems;
};