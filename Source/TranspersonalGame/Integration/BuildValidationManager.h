#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildValidationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 ErrorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 WarningCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FString> WarningMessages;

    FBuild_ValidationResult()
    {
        bIsValid = false;
        ValidationMessage = TEXT("Not validated");
        ErrorCount = 0;
        WarningCount = 0;
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
    int32 ClassCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    TArray<FString> FailedClasses;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("Unknown");
        bIsLoaded = false;
        bHasErrors = false;
        ClassCount = 0;
    }
};

/**
 * Build Validation Manager - Integration Agent #19
 * Validates build integrity, module loading, and class registration
 * Provides real-time build status monitoring for the development pipeline
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Build validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateFullBuild();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateModuleIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateClassRegistration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ModuleStatus> GetModuleStatusReport();

    // Real-time monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void StartContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void StopContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool IsContinuousValidationActive() const { return bContinuousValidationActive; }

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult TestCrossModuleIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateMinPlayableMap();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void LogValidationResults(const FBuild_ValidationResult& Results);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FString GetBuildStatusSummary();

protected:
    // Internal validation methods
    bool ValidateTranspersonalGameModule();
    bool ValidateCoreClasses();
    bool ValidateGameplayClasses();
    bool ValidateSubsystems();
    
    // Module testing
    TArray<FString> GetExpectedClasses();
    bool TestClassLoading(const FString& ClassName);
    bool TestClassSpawning(const FString& ClassName);

    // Continuous validation
    void PerformValidationTick();
    
    UPROPERTY()
    FTimerHandle ValidationTimerHandle;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    bool bContinuousValidationActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float ValidationInterval = 30.0f; // seconds

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FBuild_ValidationResult LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FBuild_ModuleStatus> CachedModuleStatus;

private:
    // Critical class paths for validation
    TArray<FString> CoreClassPaths;
    TArray<FString> GameplayClassPaths;
    TArray<FString> SubsystemClassPaths;

    // Validation state
    bool bInitialized = false;
    int32 ValidationRunCount = 0;
    double LastValidationTime = 0.0;
};