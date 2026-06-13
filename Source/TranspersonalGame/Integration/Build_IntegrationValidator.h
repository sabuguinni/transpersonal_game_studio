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
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    TArray<FBuild_ValidationReport> RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FBuild_ValidationReport ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FBuild_ValidationReport ValidateCrossSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FBuild_ValidationReport ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FBuild_ValidationReport ValidateComponentSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FBuild_ValidationReport ValidateGameplayFramework();

    // Actor cap management
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    int32 GetCurrentActorCount();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool EnforceActorCap(int32 MaxActors = 8000);

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    TArray<AActor*> GetNonEssentialActors();

    // Build status reporting
    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    FString GenerateBuildStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Integration Validation")
    bool SaveValidationReport(const FString& ReportContent);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    TArray<FBuild_ValidationReport> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    float LastValidationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    int32 MaxAllowedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoEnforceActorCap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    TArray<FString> EssentialActorLabels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    TArray<FString> CriticalClassPaths;

private:
    // Internal validation helpers
    bool ValidateClassPath(const FString& ClassPath);
    bool TestActorSpawn(UClass* ActorClass, const FVector& Location);
    void CleanupTestActors();
    FString FormatValidationTime(float TimeInSeconds);

    // Test actor tracking
    UPROPERTY()
    TArray<AActor*> SpawnedTestActors;
};