#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    NotTested   UMETA(DisplayName = "Not Tested")
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
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationReport()
    {
        TestName = TEXT("");
        Result = EBuild_ValidationResult::NotTested;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateEssentialSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateQAIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateBuildSystem();

    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_ValidationReport> GetValidationReports() const { return ValidationReports; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    bool IsSystemHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ClearValidationReports();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxDinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoFixActorCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bVerboseLogging;

private:
    void AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Details, float ExecutionTime = 0.0f);
    void LogValidationResult(const FString& TestName, EBuild_ValidationResult Result, const FString& Details);
    TArray<AActor*> GetActorsOfType(const TArray<FString>& TypeLabels);
    bool ValidateClassLoading(const FString& ClassName);
};