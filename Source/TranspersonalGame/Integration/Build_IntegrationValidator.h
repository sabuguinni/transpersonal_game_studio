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
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> FailedClasses;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        ClassCount = 0;
    }
};

/**
 * Integration validation component that performs comprehensive build and runtime validation
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Integration), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    // === VALIDATION METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration|Validation")
    bool ValidateProjectStructure();

    UFUNCTION(BlueprintCallable, Category = "Integration|Validation")
    bool ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration|Validation")
    bool ValidateLevelIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration|Validation")
    bool ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Integration|Validation")
    bool ValidateComponentSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration|Validation", CallInEditor = true)
    void RunFullValidationSuite();

    // === REPORTING METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration|Reporting")
    TArray<FBuild_ValidationReport> GetValidationReports() const { return ValidationReports; }

    UFUNCTION(BlueprintCallable, Category = "Integration|Reporting")
    TArray<FBuild_ModuleStatus> GetModuleStatuses() const { return ModuleStatuses; }

    UFUNCTION(BlueprintCallable, Category = "Integration|Reporting")
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration|Reporting")
    void LogValidationSummary();

    // === UTILITY METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "Integration|Utility")
    void ClearValidationData();

    UFUNCTION(BlueprintCallable, Category = "Integration|Utility")
    bool IsValidationPassing() const;

    UFUNCTION(BlueprintCallable, Category = "Integration|Utility")
    int32 GetErrorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Integration|Utility")
    int32 GetWarningCount() const;

protected:
    // === INTERNAL VALIDATION METHODS ===
    
    void ValidateSourceFiles();
    void ValidateClassRegistration();
    void ValidateAssetReferences();
    void ValidateGameplayIntegration();
    void ValidatePerformanceMetrics();

    void AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);
    void AddModuleStatus(const FString& ModuleName, bool bLoaded, const TArray<FString>& LoadedClasses, const TArray<FString>& FailedClasses);

private:
    // === VALIDATION DATA ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration|Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration|Settings", meta = (AllowPrivateAccess = "true"))
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration|Settings", meta = (AllowPrivateAccess = "true"))
    float ValidationTimeout;

    // === CRITICAL CLASSES TO VALIDATE ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration|Classes", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CriticalClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration|Classes", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CriticalModules;
};

/**
 * Actor that hosts the integration validation component
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
    UBuild_IntegrationValidator* GetValidationComponent() const { return ValidationComponent; }

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunValidation();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    UBuild_IntegrationValidator* ValidationComponent;
};