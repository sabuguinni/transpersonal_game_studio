#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"), 
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float PerformanceScore;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        Status = EBuild_ValidationResult::Pass;
        Message = TEXT("");
        PerformanceScore = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomeDistribution
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Distribution")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Distribution")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Distribution")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Distribution")
    float DistributionScore;

    FBuild_BiomeDistribution()
    {
        BiomeName = TEXT("");
        ActorCount = 0;
        BiomeCenter = FVector::ZeroVector;
        DistributionScore = 0.0f;
    }
};

/**
 * Integration validation system for cross-agent compatibility
 * Validates that all 18 agent outputs work together correctly
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UIntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UIntegrationValidator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_ValidationResult ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemStatus> GetSystemStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_BiomeDistribution> GetBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateDinosaurLimits();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetOverallIntegrationScore();

    // Cross-system validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCharacterWorldIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAIDinosaurIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateEnvironmentFoliageIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateQuestNarrativeIntegration();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidatePerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageScore();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTotalActorCount();

protected:
    // Internal validation helpers
    bool ValidateCoreClasses();
    bool ValidateModuleDependencies();
    bool CheckBiomeDistribution();
    float CalculatePerformanceScore();

    // System status tracking
    UPROPERTY()
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY()
    TArray<FBuild_BiomeDistribution> BiomeDistributions;

    // Validation thresholds
    UPROPERTY(EditAnywhere, Category = "Limits")
    int32 MaxTotalActors = 8000;

    UPROPERTY(EditAnywhere, Category = "Limits")
    int32 MaxDinosaurs = 150;

    UPROPERTY(EditAnywhere, Category = "Limits")
    int32 MaxActorsPerBiome = 4000;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float MinPerformanceScore = 0.7f;

    // Core system classes to validate
    UPROPERTY()
    TArray<FString> CoreSystemClasses;

    // Biome centers for distribution validation
    UPROPERTY()
    TMap<FString, FVector> BiomeCenters;

private:
    void InitializeBiomeCenters();
    void InitializeCoreSystemClasses();
    EBuild_ValidationResult ValidateSystemClass(const FString& ClassName);
    float CalculateBiomeDistributionScore();
};