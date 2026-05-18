#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float PerformanceScore;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Pending;
        ValidationMessage = TEXT("");
        ActorCount = 0;
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomePopulationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 VegetationCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bMeetsPopulationTarget;

    FBuild_BiomePopulationData()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        VegetationCount = 0;
        bMeetsPopulationTarget = false;
    }
};

/**
 * Final Integration Orchestrator - Agent #19
 * Validates all systems, manages build integration, and ensures game stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateAllCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateAssetIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void GenerateIntegrationReport();

    // System validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemValidationResult ValidateVFXSystem();

    // Biome validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_BiomePopulationData> GetBiomePopulationData();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBiomeTargets();

    // Asset validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 CountDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 CountEnvironmentAssets();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAssetPipeline();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetCurrentFrameRate();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidatePerformanceTargets();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_SystemValidationResult> GetValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    EBuild_IntegrationStatus GetOverallStatus() const { return OverallStatus; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    FString GetLastValidationReport() const { return LastValidationReport; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_BiomePopulationData> BiomeData;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

private:
    void LogValidationResult(const FBuild_SystemValidationResult& Result);
    void UpdateOverallStatus();
    FString GenerateTimestamp();
};