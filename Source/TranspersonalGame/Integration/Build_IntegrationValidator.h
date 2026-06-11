#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Success     UMETA(DisplayName = "Success"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationReport()
    {
        TestName = TEXT("");
        Result = EBuild_ValidationResult::Success;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    bool bHasErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> FailedClasses;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasErrors = false;
        ClassCount = 0;
    }
};

/**
 * Integration and Build Validation System
 * Validates compilation, module loading, and cross-system integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateProjectStructure();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateClassRegistration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool RunFullValidationSuite();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GenerateIntegrationReport() const;

    // Build status functions
    UFUNCTION(BlueprintCallable, Category = "Build", CallInEditor = true)
    bool CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Build", CallInEditor = true)
    bool ValidateBinaryFiles();

    UFUNCTION(BlueprintCallable, Category = "Build", CallInEditor = true)
    bool TestActorSpawning();

protected:
    // Internal validation helpers
    bool ValidateWorldGeneration();
    bool ValidateCharacterSystems();
    bool ValidateAISystems();
    bool ValidateEnvironmentSystems();
    bool ValidateQuestSystems();

    // Error tracking
    void AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);
    void ClearValidationReports();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float ValidationTimeout;

public:
    // Getters for private members
    FORCEINLINE bool GetAutoValidateOnBeginPlay() const { return bAutoValidateOnBeginPlay; }
    FORCEINLINE bool GetLogDetailedResults() const { return bLogDetailedResults; }
    FORCEINLINE float GetValidationTimeout() const { return ValidationTimeout; }
};

/**
 * Actor that contains the Integration Validator component
 * Can be placed in levels for automated validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationValidatorActor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationValidatorActor();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Integration")
    UBuild_IntegrationValidator* GetValidatorComponent() const;

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool RunValidation();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBuild_IntegrationValidator* ValidatorComponent;
};