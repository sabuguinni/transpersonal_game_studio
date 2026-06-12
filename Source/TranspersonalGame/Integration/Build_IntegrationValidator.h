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

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateLevelIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ClearValidationReports();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void LogValidationSummary();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bVerboseLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PerformanceThresholdFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxMemoryUsageMB;

private:
    void AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);
    bool ValidateSpecificClass(const FString& ClassName);
    void ValidateActorCounts();
    void ValidateSystemIntegration();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationTestActor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationTestActor();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void TestCrossSystemCommunication();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateBuildConfiguration();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBuild_IntegrationValidator* IntegrationValidator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
    float TestInterval;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    FTimerHandle TestTimerHandle;
    void PeriodicValidation();
};