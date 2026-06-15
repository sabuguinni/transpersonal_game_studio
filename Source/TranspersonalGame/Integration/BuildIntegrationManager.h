#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    TArray<FString> CompilationErrors;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsCompiled = false;
        bIsLoaded = false;
        ClassCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 PropCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bIsPerformanceHealthy;

    FBuild_SystemHealth()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        PropCount = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        bIsPerformanceHealthy = true;
    }
};

/**
 * Integration & Build Manager - Orchestrates compilation, validation, and system health monitoring
 * Ensures all agent outputs integrate correctly into a playable build
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_ModuleStatus GetModuleStatus(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetAllModuleStatuses();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_SystemHealth GetSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void EnforceActorCaps();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateGameplayReadiness();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsMinimumViablePrototypeReady();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogBuildReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_SystemHealth CurrentSystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIsMonitoringPerformance;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastFrameRateCheck;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 MaxAllowedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 MaxAllowedDinosaurs;

private:
    void CheckModuleCompilation();
    void UpdateSystemHealth();
    void ValidateActorCounts();
    void CheckPerformanceMetrics();
    bool ValidateMinimumGameplayElements();
};