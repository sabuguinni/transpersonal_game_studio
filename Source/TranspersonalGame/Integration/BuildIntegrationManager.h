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
    bool bHasErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    FString LastError;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsCompiled = false;
        bHasErrors = false;
        ErrorCount = 0;
        LastError = TEXT("");
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
    bool bWithinLimits;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Build")
    bool bPerformanceGood;

    FBuild_SystemHealth()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        PropCount = 0;
        bWithinLimits = true;
        MemoryUsageMB = 0.0f;
        bPerformanceGood = true;
    }
};

/**
 * Build Integration Manager - Agent #19
 * Orchestrates integration of all agent outputs into a coherent build
 * Validates compilation, performance, and system health
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build validation
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_SystemHealth GetSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatuses();

    // Actor management
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void CleanupExcessActors();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float GetCurrentMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsPerformanceAcceptable();

    // Build reporting
    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FString GenerateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void SaveBuildSnapshot();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_SystemHealth CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bBuildValid;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FDateTime LastValidationTime;

private:
    void ValidateModules();
    void CheckActorCounts();
    void MonitorPerformance();
    void UpdateHealthStatus();
    
    // Actor limits (from brain memories)
    static constexpr int32 MAX_TOTAL_ACTORS = 8000;
    static constexpr int32 MAX_DINOSAURS = 150;
    static constexpr int32 MAX_PROPS_PER_BIOME = 1000;
    static constexpr int32 MAX_TOTAL_PROPS = 5000;
};