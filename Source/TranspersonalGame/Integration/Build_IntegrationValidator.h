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
    EBuild_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 WarningCount;

    FBuild_ValidationReport()
    {
        Result = EBuild_ValidationResult::Success;
        ModuleName = TEXT("");
        Message = TEXT("");
        ErrorCount = 0;
        WarningCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ValidationReport ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAssetIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateClassDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bValidationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

private:
    bool ValidateCoreClasses();
    bool ValidateWorldState();
    bool ValidateQAResults();
    void LogValidationResult(const FBuild_ValidationReport& Report);
};