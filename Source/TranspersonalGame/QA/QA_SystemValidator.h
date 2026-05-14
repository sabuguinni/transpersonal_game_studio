#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_SystemValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_ValidationReport()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_SystemValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_SystemValidator();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearValidationReports();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_ValidationReport> ValidationReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxActorCountThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MaxFrameTimeThreshold;

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void AddValidationReport(const FString& TestName, EQA_ValidationResult Result, const FString& Details, float ExecutionTime);
    bool ValidateClassLoading(const FString& ClassName, const FString& ClassPath);
    bool ValidateActorSpawning(UClass* ActorClass);
    bool CheckMemoryUsage();
    bool CheckFrameRate();

    FTimerHandle ValidationTimerHandle;
    float LastValidationTime;
};