#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bIsCompiled = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    bool bAllModulesLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    int32 ActiveActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> CompilationErrors;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bWorldGeneratorActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bFoliageManagerActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bCrowdSimulationActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bQAFrameworkActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    float OverallHealthScore = 0.0f;
};

/**
 * Build Integration Manager - Orchestrates all systems and validates build integrity
 * Ensures all 18 agent outputs integrate into a cohesive, playable game
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuildIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationStatus GetIntegrationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_SystemHealth GetSystemHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void RunCompilationTest();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool CheckAllModulesLoaded();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void FixCommonIntegrationIssues();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_IntegrationStatus CurrentStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    FBuild_SystemHealth SystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Build Integration")
    float LastFullValidation = 0.0f;

private:
    void ValidateWorldGeneration();
    void ValidateFoliageSystem();
    void ValidateCrowdSimulation();
    void ValidateQAFramework();
    void ValidateCharacterSystems();
    void ValidateGameplayIntegration();
    void CheckForDuplicateTypes();
    void ValidateSharedTypes();
    void CheckCompilationErrors();
    void UpdateSystemHealth();
};