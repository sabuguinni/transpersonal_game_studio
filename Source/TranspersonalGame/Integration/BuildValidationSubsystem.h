#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "BuildValidationSubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 TotalModulesLoaded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 FailedModules = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 TotalActorsInWorld = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 CriticalSystemsOnline = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float LastValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    bool bBuildHealthy = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FString> ValidationErrors;

    FBuild_ValidationMetrics()
    {
        TotalModulesLoaded = 0;
        FailedModules = 0;
        TotalActorsInWorld = 0;
        CriticalSystemsOnline = 0;
        LastValidationTime = 0.0f;
        bBuildHealthy = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    bool bHasErrors = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    float LoadTime = 0.0f;

    FBuild_ModuleStatus()
    {
        bIsLoaded = false;
        bHasErrors = false;
        LoadTime = 0.0f;
    }
};

/**
 * Build Validation Subsystem
 * Monitors the health of all game systems and provides real-time build validation
 * Used by Integration Agent #19 to ensure all systems are properly integrated
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildValidationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Perform comprehensive build validation
     * Checks all critical systems and reports health status
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationMetrics ValidateBuildHealth();

    /**
     * Check if a specific module is loaded and functional
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ModuleStatus ValidateModule(const FString& ModuleName);

    /**
     * Get the latest validation metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationMetrics GetLatestValidationMetrics() const { return LatestMetrics; }

    /**
     * Check if the build is currently healthy
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsBuildHealthy() const { return LatestMetrics.bBuildHealthy; }

    /**
     * Force immediate validation (normally runs on timer)
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ForceValidation();

    /**
     * Get detailed status of all monitored modules
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ModuleStatus> GetAllModuleStatuses() const { return ModuleStatuses; }

protected:
    /**
     * Validate core game systems
     */
    void ValidateCoreGameSystems(FBuild_ValidationMetrics& Metrics);

    /**
     * Validate character and player systems
     */
    void ValidateCharacterSystems(FBuild_ValidationMetrics& Metrics);

    /**
     * Validate AI and dinosaur systems
     */
    void ValidateAISystems(FBuild_ValidationMetrics& Metrics);

    /**
     * Validate world generation and environment systems
     */
    void ValidateWorldSystems(FBuild_ValidationMetrics& Metrics);

    /**
     * Count and categorize all actors in the current world
     */
    void ValidateWorldActors(FBuild_ValidationMetrics& Metrics);

    /**
     * Timer callback for periodic validation
     */
    void PerformPeriodicValidation();

private:
    UPROPERTY()
    FBuild_ValidationMetrics LatestMetrics;

    UPROPERTY()
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    FTimerHandle ValidationTimerHandle;

    // Validation frequency (seconds)
    float ValidationInterval = 30.0f;

    // Critical modules that must be loaded
    TArray<FString> CriticalModules = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("DinosaurTRex"),
        TEXT("DinosaurCombatAIController"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager")
    };
};