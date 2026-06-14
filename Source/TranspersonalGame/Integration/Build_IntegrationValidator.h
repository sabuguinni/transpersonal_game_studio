#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
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

/**
 * Integration validator that ensures all game systems work together correctly
 * Validates module compilation, cross-system compatibility, and performance metrics
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
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor)
    bool RunFullValidationSuite();

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ClearValidationReports();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ExportValidationReport(const FString& FilePath);

protected:
    // Internal validation helpers
    bool ValidateClassLoading();
    bool ValidateActorSpawning();
    bool ValidateComponentIntegration();
    bool ValidateWorldGeneration();
    bool ValidateCharacterSystems();
    bool ValidateAISystems();
    bool ValidateVFXPerformance();

    void AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);

private:
    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    float PerformanceThresholdMS;

    // Core classes to validate
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CoreClassPaths;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    int32 DinosaurActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;
};