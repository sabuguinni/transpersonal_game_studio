#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_IntegrationValidator.generated.h"

/**
 * Integration validation status for build health monitoring
 */
UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Healthy         UMETA(DisplayName = "Healthy"),
    Warning         UMETA(DisplayName = "Warning"),
    Critical        UMETA(DisplayName = "Critical"),
    Failed          UMETA(DisplayName = "Failed")
};

/**
 * Module validation result structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bHasValidClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ErrorMessage;

    FBuild_ModuleValidation()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasValidClasses = false;
        ClassCount = 0;
        ErrorMessage = TEXT("");
    }
};

/**
 * Comprehensive integration report structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float SuccessRate;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveCustomActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ModuleValidation> ModuleValidations;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ValidationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildVersion;

    FBuild_IntegrationReport()
    {
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        SuccessRate = 0.0f;
        LoadedModules = 0;
        TotalModules = 0;
        ActiveCustomActors = 0;
        TotalActorsInLevel = 0;
        ValidationTimestamp = FDateTime::Now();
        BuildVersion = TEXT("1.0.0");
    }
};

/**
 * Integration Validator - Monitors build health and module integration
 * Validates that all agent outputs integrate correctly into a cohesive build
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Perform comprehensive integration validation
     * @return Integration report with detailed status
     */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationReport ValidateIntegration();

    /**
     * Validate specific module integration
     * @param ModuleName Name of the module to validate
     * @return Module validation result
     */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ModuleValidation ValidateModule(const FString& ModuleName);

    /**
     * Get current integration status
     * @return Current overall integration status
     */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetIntegrationStatus() const;

    /**
     * Force integration re-validation
     */
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ForceRevalidation();

    /**
     * Get last integration report
     * @return Most recent integration report
     */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GetLastReport() const { return LastIntegrationReport; }

    /**
     * Check if build is healthy for release
     * @return True if build meets release criteria
     */
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsBuildHealthy() const;

protected:
    /**
     * Validate core game modules
     */
    void ValidateCoreModules();

    /**
     * Validate actor integration in current level
     */
    void ValidateActorIntegration();

    /**
     * Validate shared types integration
     */
    void ValidateSharedTypes();

    /**
     * Calculate overall integration status
     */
    void CalculateOverallStatus();

private:
    UPROPERTY()
    FBuild_IntegrationReport LastIntegrationReport;

    UPROPERTY()
    TArray<FString> CoreModuleNames;

    UPROPERTY()
    float HealthyThreshold;

    UPROPERTY()
    float WarningThreshold;

    UPROPERTY()
    bool bAutoValidateOnStartup;
};