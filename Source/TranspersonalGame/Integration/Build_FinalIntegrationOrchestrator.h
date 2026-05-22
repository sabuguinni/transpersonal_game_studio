#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealthReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 PopulatedBiomes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 IntegrationScore = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> WorkingModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> FailedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bCompilationSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationTime;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomePopulation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    bool bIsPopulated = false;
};

/**
 * Final Integration Orchestrator - Agent #19
 * Manages build validation, cross-system integration, and health monitoring
 * Ensures all agent outputs work together as a cohesive game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Perform comprehensive system health check
     * Tests all critical systems and generates integration report
     */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    FBuild_SystemHealthReport PerformSystemHealthCheck();

    /**
     * Validate cross-system integrations
     * Tests interactions between different agent outputs
     */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateCrossSystemIntegrations();

    /**
     * Check biome population status
     * Verifies that all biomes have adequate actor distribution
     */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    TArray<FBuild_BiomePopulation> CheckBiomePopulation();

    /**
     * Test module compilation status
     * Verifies that all C++ modules load correctly
     */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool TestModuleCompilation(TArray<FString>& OutWorkingModules, TArray<FString>& OutFailedModules);

    /**
     * Generate integration score based on system health
     * Returns 0-100 score indicating overall integration quality
     */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    int32 CalculateIntegrationScore();

    /**
     * Save current map state to persist spawned actors
     * Critical for maintaining progress between UE5 restarts
     */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool SaveMapState();

    /**
     * Perform final validation before cycle completion
     * Mandatory compilation gate check
     */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool PerformCompilationGate();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_SystemHealthReport LastHealthReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_BiomePopulation> BiomeStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bLastValidationPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastValidationTime;

private:
    // Internal validation helpers
    bool ValidatePlayerSystems();
    bool ValidateDinosaurSystems();
    bool ValidateEnvironmentSystems();
    bool ValidatePhysicsSystems();
    
    // Biome coordinate definitions
    static const TMap<FString, FVector> BiomeCoordinates;
    
    // Critical module list for testing
    static const TArray<FString> CriticalModules;
};