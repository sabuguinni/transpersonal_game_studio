// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Subsystems/EngineSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/NoExportTypes.h"
#include "ArchitectureValidationSuite.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogArchitectureValidation, Log, All);

/**
 * Architecture Validation Results
 * Stores comprehensive validation results for all engine systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchitectureValidationResults
{
    GENERATED_BODY()

    // Core Engine Systems
    UPROPERTY(BlueprintReadOnly, Category = "Core")
    bool bEngineSubsystemsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Core")
    bool bWorldPartitionSupported = false;

    UPROPERTY(BlueprintReadOnly, Category = "Core")
    bool bNaniteSupported = false;

    UPROPERTY(BlueprintReadOnly, Category = "Core")
    bool bLumenSupported = false;

    UPROPERTY(BlueprintReadOnly, Category = "Core")
    bool bVirtualShadowMapsSupported = false;

    // Asset Management
    UPROPERTY(BlueprintReadOnly, Category = "Assets")
    bool bAssetRegistryValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Assets")
    bool bStreamingManagerValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Assets")
    bool bOneFilePerActorSupported = false;

    // Performance Systems
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bHLODSupported = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bDataLayersSupported = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bLevelStreamingValid = false;

    // Module Integration
    UPROPERTY(BlueprintReadOnly, Category = "Modules")
    bool bCoreModuleLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Modules")
    bool bPhysicsModuleLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Modules")
    bool bWorldGenerationModuleLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Modules")
    bool bAIModuleLoaded = false;

    // Overall Status
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float ValidationScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FString ValidationSummary = TEXT("Not Validated");

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    TArray<FString> Warnings;

    FArchitectureValidationResults()
    {
        CriticalIssues.Empty();
        Warnings.Empty();
    }
};

/**
 * Architecture Validation Configuration
 * Defines what systems to validate and validation parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchitectureValidationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bValidateEngineSubsystems = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bValidateRenderingFeatures = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bValidateAssetManagement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bValidateModuleIntegration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bValidatePerformanceSystems = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bStrictValidation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float MinimumValidationScore = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bGenerateDetailedReport = true;
};

/**
 * Architecture Validation Suite
 * Comprehensive validation system for engine architecture compliance
 * Validates UE5 features, module integration, and system compatibility
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitectureValidationSuite : public UObject
{
    GENERATED_BODY()

public:
    UArchitectureValidationSuite();

    /**
     * Execute comprehensive architecture validation
     * @param Config Validation configuration parameters
     * @return Validation results structure
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FArchitectureValidationResults ExecuteValidation(const FArchitectureValidationConfig& Config);

    /**
     * Validate core engine subsystems
     * @return True if all critical subsystems are available
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateEngineSubsystems();

    /**
     * Validate UE5 rendering features (Nanite, Lumen, VSM)
     * @return True if all rendering features are supported
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateRenderingFeatures();

    /**
     * Validate World Partition and large world support
     * @return True if World Partition is properly configured
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateWorldPartition();

    /**
     * Validate asset management systems
     * @return True if asset systems are functioning correctly
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateAssetManagement();

    /**
     * Validate module integration and dependencies
     * @return True if all modules are properly loaded
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidateModuleIntegration();

    /**
     * Validate performance optimization systems
     * @return True if performance systems are available
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ValidatePerformanceSystems();

    /**
     * Generate detailed validation report
     * @param Results Validation results to format
     * @return Formatted report string
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FString GenerateValidationReport(const FArchitectureValidationResults& Results);

    /**
     * Get validation score based on results
     * @param Results Validation results to score
     * @return Score from 0-100
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    float CalculateValidationScore(const FArchitectureValidationResults& Results);

protected:
    // Internal validation methods
    bool CheckNaniteSupport();
    bool CheckLumenSupport();
    bool CheckVirtualShadowMapSupport();
    bool CheckHLODSupport();
    bool CheckDataLayersSupport();
    bool CheckOneFilePerActorSupport();

    // Module validation helpers
    bool IsModuleLoaded(const FString& ModuleName);
    bool ValidateModuleDependencies();

    // Error reporting
    void AddCriticalIssue(const FString& Issue, FArchitectureValidationResults& Results);
    void AddWarning(const FString& Warning, FArchitectureValidationResults& Results);

private:
    // Validation state
    FArchitectureValidationConfig CurrentConfig;
    FArchitectureValidationResults LastResults;

    // Validation thresholds
    static constexpr float CRITICAL_THRESHOLD = 70.0f;
    static constexpr float WARNING_THRESHOLD = 85.0f;
};

/**
 * Architecture Validation Subsystem
 * Engine subsystem for managing architecture validation
 */
UCLASS()
class TRANSPERSONALGAME_API UArchitectureValidationSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Get the validation suite instance
     * @return Validation suite object
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    UArchitectureValidationSuite* GetValidationSuite();

    /**
     * Execute automatic validation on engine startup
     * @return True if validation passed
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    bool ExecuteStartupValidation();

    /**
     * Schedule periodic validation checks
     * @param IntervalSeconds Interval between checks
     */
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    void SchedulePeriodicValidation(float IntervalSeconds = 300.0f);

protected:
    UPROPERTY()
    UArchitectureValidationSuite* ValidationSuite;

    FTimerHandle ValidationTimerHandle;
    
    void PeriodicValidationCheck();
};