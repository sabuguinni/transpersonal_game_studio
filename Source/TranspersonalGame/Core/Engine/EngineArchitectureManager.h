#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "EngineArchitectureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bIsInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    float PerformanceScore = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    FString LastError;

    FEng_SystemStatus()
    {
        SystemName = TEXT("Unknown");
        bIsInitialized = false;
        bIsActive = false;
        PerformanceScore = 100.0f;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerChunk = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousDinosaurs = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float WorldSizeKm = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 BiomeCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryMB = 8192;

    FEng_ArchitectureConfig()
    {
        MaxActorsPerChunk = 500;
        TargetFrameRate = 60.0f;
        MaxSimultaneousDinosaurs = 50;
        WorldSizeKm = 16.0f;
        BiomeCount = 8;
        MaxMemoryMB = 8192;
    }
};

/**
 * Engine Architecture Manager - Core system that validates and monitors all game systems
 * Ensures performance targets, memory limits, and architectural rules are enforced
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Architecture Functions
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSystem(const FString& SystemName, UObject* SystemObject);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemStatus> GetAllSystemStatuses();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentMemoryUsageMB();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnforcePerformanceLimits();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    FEng_ArchitectureConfig GetArchitectureConfig() const { return ArchitectureConfig; }

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetArchitectureConfig(const FEng_ArchitectureConfig& NewConfig);

    // Validation and Rules
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateWorldPartitionSetup();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateMemoryLimits();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateSystemDependencies();

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void RunArchitectureDiagnostics();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void GenerateSystemReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_ArchitectureConfig ArchitectureConfig;

    UPROPERTY(BlueprintReadOnly, Category = "System Registry")
    TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    TMap<FString, FEng_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameRate = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LastMemoryUsage = 0;

private:
    void UpdateSystemStatuses();
    void CheckPerformanceThresholds();
    bool ValidateSystemIntegrity(const FString& SystemName, UObject* SystemObject);
    void LogArchitectureWarning(const FString& Warning);
    void LogArchitectureError(const FString& Error);
};