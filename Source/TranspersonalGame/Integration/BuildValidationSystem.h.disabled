#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "UObject/Class.h"
#include "SharedTypes.h"
#include "BuildValidationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 ClassCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 FunctionCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float ValidationTime = 0.0f;

    FBuild_ValidationResult()
    {
        bIsValid = false;
        ModuleName = TEXT("");
        ErrorMessage = TEXT("");
        ClassCount = 0;
        FunctionCount = 0;
        ValidationTime = 0.0f;
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
    TArray<FString> ErrorMessages;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        ClassCount = 0;
        LoadedClasses.Empty();
        ErrorMessages.Empty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CompilationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    bool bCompilationSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    FDateTime CompilationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 TotalModules = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 SuccessfulModules = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    int32 FailedModules = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Compilation")
    TArray<FString> CompilationErrors;

    FBuild_CompilationReport()
    {
        bCompilationSuccessful = false;
        CompilationTime = FDateTime::Now();
        TotalModules = 0;
        SuccessfulModules = 0;
        FailedModules = 0;
        ModuleStatuses.Empty();
        CompilationErrors.Empty();
    }
};

/**
 * Build Validation System - Integration Agent #19
 * Validates compilation status, module loading, and cross-system integration
 * Provides comprehensive build health monitoring for the Transpersonal Game project
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_CompilationReport GenerateCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FString> GetOrphanedHeaders();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FString> GetMissingImplementations();

    // Class and function validation
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateClassLoading(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    int32 CountLoadedClasses();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FString> GetAllLoadedClasses();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool TestCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool TestComponentRegistration();

    // Reporting and logging
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void LogValidationResults(const FBuild_ValidationResult& Result);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void SaveValidationReport(const FBuild_CompilationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FString GetLastValidationReport();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    float GetValidationPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void ResetValidationMetrics();

protected:
    // Internal validation state
    UPROPERTY(BlueprintReadOnly, Category = "Internal State")
    FBuild_CompilationReport LastCompilationReport;

    UPROPERTY(BlueprintReadOnly, Category = "Internal State")
    TArray<FBuild_ValidationResult> ValidationHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Internal State")
    float TotalValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Internal State")
    int32 ValidationCount;

    // Internal helper functions
    FBuild_ModuleStatus ValidateModuleInternal(const FString& ModuleName);
    bool CheckHeaderImplementationPairs();
    void UpdateValidationMetrics(float ValidationTime);
    void LogModuleStatus(const FBuild_ModuleStatus& Status);
};