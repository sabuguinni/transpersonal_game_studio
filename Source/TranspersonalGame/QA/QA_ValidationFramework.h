#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationFramework.generated.h"

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

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FDateTime Timestamp;

    FQA_ValidationReport()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 CompiledSystems;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 ProjectAssets;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 QATestActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bCriticalFailures;

    FQA_SystemMetrics()
    {
        TotalActors = 0;
        CompiledSystems = 0;
        MemoryUsagePercent = 0.0f;
        ProjectAssets = 0;
        QATestActors = 0;
        bCriticalFailures = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_ValidationFramework();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_ValidationReport> ValidationReports;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    FQA_SystemMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bLogToConsole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bSaveReportsToFile;

public:
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateCoreSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearValidationReports();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool HasCriticalFailures() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void SaveReportsToFile(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogValidationSummary();

protected:
    void AddValidationReport(const FString& TestName, EQA_ValidationResult Result, const FString& Details, float ExecutionTime = 0.0f);
    
    bool ValidateClassCompilation(const FString& ClassName);
    
    void UpdateSystemMetrics();
    
    void CheckBiomeDistribution();
    
    void ValidateMemoryUsage();

private:
    FTimerHandle ValidationTimerHandle;
    
    void PeriodicValidation();
};