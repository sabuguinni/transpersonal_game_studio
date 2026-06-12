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
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FBuild_ValidationReport()
    {
        ModuleName = TEXT("");
        Result = EBuild_ValidationResult::Success;
        Message = TEXT("");
        ValidationTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCoreClasses();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateQAIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateLevelActors();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ClearValidationReports();

    // Build status functions
    UFUNCTION(BlueprintCallable, Category = "Build")
    bool CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Build")
    int32 GetLoadedModuleCount() const;

    UFUNCTION(BlueprintCallable, Category = "Build")
    FString GetBuildStatusSummary() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float LastValidationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ValidationInterval;

private:
    void AddValidationReport(const FString& ModuleName, EBuild_ValidationResult Result, const FString& Message);
    bool ValidateSpecificClass(const FString& ClassName);
    void LogValidationResult(const FBuild_ValidationReport& Report);

    // Core class paths for validation
    TArray<FString> CoreClassPaths;
    TArray<FString> QAClassPaths;
    
    float TimeSinceLastValidation;
};