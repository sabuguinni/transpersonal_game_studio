#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildValidationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float ExecutionTime = 0.0f;

    FBuild_ValidationResult()
    {
        bSuccess = false;
        TestName = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }

    FBuild_ValidationResult(const FString& InTestName, bool InSuccess, const FString& InError = TEXT(""), float InTime = 0.0f)
        : bSuccess(InSuccess), TestName(InTestName), ErrorMessage(InError), ExecutionTime(InTime)
    {
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
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        ClassCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationTest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Test")
    FString TestCategory;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Test")
    FString TestDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Test")
    bool bIsRequired = true;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Test")
    TArray<FString> Dependencies;

    FBuild_IntegrationTest()
    {
        TestCategory = TEXT("");
        TestDescription = TEXT("");
        bIsRequired = true;
    }
};

/**
 * Build Validation System - Comprehensive testing and validation for the TranspersonalGame module
 * Handles compilation verification, class loading tests, integration scenarios, and build health monitoring
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateIntegrationScenarios();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateActorSpawning();

    // Module status functions
    UFUNCTION(BlueprintCallable, Category = "Module Status")
    TArray<FBuild_ModuleStatus> GetModuleStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Module Status")
    FBuild_ModuleStatus GetTranspersonalGameModuleStatus();

    UFUNCTION(BlueprintCallable, Category = "Module Status")
    bool IsModuleHealthy(const FString& ModuleName);

    // Validation results
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_ValidationResult> GetLastValidationResults() const { return LastValidationResults; }

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult GetLastValidationSummary() const { return LastValidationSummary; }

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool HasValidationErrors() const;

    // Integration test management
    UFUNCTION(BlueprintCallable, Category = "Integration Tests")
    void RegisterIntegrationTest(const FBuild_IntegrationTest& Test);

    UFUNCTION(BlueprintCallable, Category = "Integration Tests")
    TArray<FBuild_IntegrationTest> GetRegisteredTests() const { return RegisteredTests; }

    UFUNCTION(BlueprintCallable, Category = "Integration Tests")
    bool RunSpecificTest(const FString& TestName);

    // Build health monitoring
    UFUNCTION(BlueprintCallable, Category = "Build Health")
    float GetOverallBuildHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Build Health")
    FString GenerateBuildReport();

    UFUNCTION(BlueprintCallable, Category = "Build Health")
    void LogBuildStatus();

protected:
    // Internal validation functions
    bool TestClassConstruction(const FString& ClassName);
    bool TestComponentRegistration(const FString& ComponentClassName);
    bool TestSubsystemInitialization(const FString& SubsystemClassName);
    bool TestCrossSystemIntegration();

    // Test execution helpers
    FBuild_ValidationResult ExecuteValidationTest(const FString& TestName, TFunction<bool()> TestFunction);
    void LogValidationResult(const FBuild_ValidationResult& Result);

private:
    UPROPERTY()
    TArray<FBuild_ValidationResult> LastValidationResults;

    UPROPERTY()
    FBuild_ValidationResult LastValidationSummary;

    UPROPERTY()
    TArray<FBuild_IntegrationTest> RegisteredTests;

    UPROPERTY()
    TArray<FBuild_ModuleStatus> CachedModuleStatus;

    // Validation state
    bool bValidationInProgress = false;
    float LastValidationTime = 0.0f;
    int32 TotalTestsRun = 0;
    int32 TestsPassed = 0;
    int32 TestsFailed = 0;

    // Critical class paths for testing
    TArray<FString> CoreClassPaths = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };
};