#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitectureValidator.generated.h"

/**
 * Engine Architecture Validator
 * Ensures all systems follow architectural rules and validates cross-system dependencies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureValidator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitectureValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Validate all architectural rules and dependencies
     * @return True if all validations pass
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateArchitecture();

    /**
     * Check for duplicate system instances
     * @return Number of duplicates found and cleaned
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 CleanupDuplicateSystems();

    /**
     * Validate biome system architecture
     * @return True if biome system is properly configured
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateBiomeArchitecture();

    /**
     * Check module dependencies and loading order
     * @return True if all modules are loaded correctly
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleDependencies();

    /**
     * Validate performance constraints
     * @return True if performance targets are met
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidatePerformanceConstraints();

protected:
    /** Last validation timestamp */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    float LastValidationTime;

    /** Validation results cache */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, bool> ValidationResults;

    /** Performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FEng_PerformanceMetrics CurrentMetrics;

private:
    /** Internal validation helpers */
    bool ValidateLightingSetup();
    bool ValidateWorldPartitioning();
    bool ValidateCharacterSystems();
    bool ValidateDinosaurSystems();
    
    /** Cleanup helpers */
    void CleanupDuplicateLighting();
    void CleanupDuplicateManagers();
    
    /** Performance monitoring */
    void UpdatePerformanceMetrics();
    bool CheckFrameRate();
    bool CheckMemoryUsage();
};

/**
 * Architecture validation data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationReport
{
    GENERATED_BODY()

    /** Overall validation status */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid;

    /** Individual test results */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TMap<FString, bool> TestResults;

    /** Error messages */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Errors;

    /** Warning messages */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> Warnings;

    /** Performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FEng_PerformanceMetrics Metrics;

    FEng_ValidationReport()
    {
        bIsValid = false;
    }
};

/**
 * Performance metrics structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    /** Current frame rate */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    /** Memory usage in MB */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    /** Draw calls per frame */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    /** Triangle count */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    /** Actor count in level */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    FEng_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        ActorCount = 0;
    }
};