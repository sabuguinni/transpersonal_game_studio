#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Eng_ArchitectureCore.generated.h"

// Forward declarations
class ATranspersonalGameState;
class ATranspersonalCharacter;
class UBiomeManager;

/**
 * Engine Architecture Core Rules and Validation
 * Enforces technical standards across all game systems
 * Created by Agent #02 - Engine Architect
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float PerformanceScore = 0.0f;

    FEng_SystemValidationResult()
    {
        bIsValid = false;
        SystemName = TEXT("Unknown");
        ValidationMessage = TEXT("Not validated");
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeArchitectureRule
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float BiomeRadius = 25000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 MinActorCount = 500;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    int32 MaxActorCount = 2000;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TArray<FString> RequiredActorTypes;

    FEng_BiomeArchitectureRule()
    {
        BiomeType = EBiomeType::Savanna;
        BiomeCenter = FVector::ZeroVector;
        BiomeRadius = 25000.0f;
        MinActorCount = 500;
        MaxActorCount = 2000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceConstraint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString ConstraintName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxValue = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsHardLimit = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString ViolationAction;

    FEng_PerformanceConstraint()
    {
        ConstraintName = TEXT("Unknown");
        MaxValue = 100.0f;
        CurrentValue = 0.0f;
        bIsHardLimit = true;
        ViolationAction = TEXT("Log Warning");
    }
};

/**
 * Engine Architecture Core Subsystem
 * Manages technical standards and validation across all systems
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitectureCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_SystemValidationResult ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_SystemValidationResult> ValidateAllSystems();

    // Biome architecture rules
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeBiomeRules();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_BiomeArchitectureRule GetBiomeRule(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateBiomePopulation(EBiomeType BiomeType);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterPerformanceConstraint(const FEng_PerformanceConstraint& Constraint);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CheckPerformanceConstraints();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetSystemPerformanceScore();

    // Cross-module communication
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool RegisterSystemDependency(const FString& SystemA, const FString& SystemB);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetSystemDependencies(const FString& SystemName);

    // Development rules enforcement
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool EnforceModuleStandards(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture", CallInEditor = true)
    void ValidateProjectArchitecture();

protected:
    // Biome architecture rules
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<EBiomeType, FEng_BiomeArchitectureRule> BiomeRules;

    // Performance constraints
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FEng_PerformanceConstraint> PerformanceConstraints;

    // System dependencies
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, TArray<FString>> SystemDependencies;

    // Validation results cache
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TMap<FString, FEng_SystemValidationResult> ValidationCache;

private:
    // Internal validation methods
    bool ValidateWorldPartitionSetup();
    bool ValidateLumenConfiguration();
    bool ValidateNaniteSettings();
    bool ValidateMemoryBudgets();
    bool ValidateActorCounts();

    // Performance monitoring
    void UpdatePerformanceMetrics();
    float CalculateFrameTimeScore();
    float CalculateMemoryScore();
    float CalculateDrawCallScore();

    // Architecture enforcement
    void EnforceBiomeDistribution();
    void EnforceActorLimits();
    void EnforceNamingConventions();
};