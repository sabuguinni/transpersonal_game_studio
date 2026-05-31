#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitectureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsageMB;

    FEng_SystemPerformanceMetrics()
    {
        FrameTime = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleLoadInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    float LoadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ClassCount;

    FEng_ModuleLoadInfo()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        LoadTime = 0.0f;
        ClassCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_SystemPerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsPerformanceMonitoringActive() const { return bPerformanceMonitoringActive; }

    // Module management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_ModuleLoadInfo> GetLoadedModuleInfo();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void LogSystemArchitecture();

    // Actor limit enforcement
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetBiomeActorCount(EBiomeType BiomeType) const;

    // System health checks
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckFrameRate();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateArchitectureReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceMonitoringActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_SystemPerformanceMetrics LastPerformanceMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FEng_ModuleLoadInfo> LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float LastHealthCheckTime;

    // Actor limit constants
    static constexpr int32 MAX_TOTAL_ACTORS = 20000;
    static constexpr int32 MAX_ACTORS_PER_BIOME = 4000;

private:
    void UpdatePerformanceMetrics();
    void CheckSystemHealth();
    void ValidateActorCounts();
    void CleanupExcessActors();
};