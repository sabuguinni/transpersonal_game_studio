#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    NotTested,
    Passed,
    Failed,
    Warning
};

USTRUCT(BlueprintType)
struct FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationResult()
    {
        TestName = TEXT("");
        Status = EBuild_ValidationStatus::NotTested;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> LoadedClasses;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        ClassCount = 0;
    }
};

/**
 * Integration validation system for build quality assurance
 * Validates module loading, class registration, and cross-system compatibility
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateClassRegistration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateLevelIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationResult GetLastValidationResult() const { return LastValidationResult; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationResult> GetAllValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const { return ModuleStatuses; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemHealthy() const;

    // Editor-only validation
    UFUNCTION(CallInEditor, Category = "Integration")
    void RunEditorValidation();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FBuild_ValidationResult LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TotalTestsRun;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TestsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 TestsFailed;

private:
    void AddValidationResult(const FString& TestName, EBuild_ValidationStatus Status, const FString& Message);
    void ClearValidationResults();
    FBuild_ModuleStatus CheckModuleStatus(const FString& ModuleName);
    bool ValidateClassExists(const FString& ClassName);
    void LogValidationSummary();
};