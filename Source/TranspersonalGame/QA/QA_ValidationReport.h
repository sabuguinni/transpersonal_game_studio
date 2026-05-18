#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "QA_ValidationReport.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationStatus : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"), 
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Status = EQA_ValidationStatus::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 LoadedClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 AvailableAssetCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bBridgeHealthy;

    FQA_SystemHealth()
    {
        ActorCount = 0;
        MemoryUsagePercent = 0.0f;
        LoadedClassCount = 0;
        AvailableAssetCount = 0;
        bBridgeHealthy = false;
    }
};

/**
 * QA Validation Report System
 * Tracks test results and system health across production cycles
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationReport : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQA_ValidationReport();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void AddTestResult(const FString& TestName, EQA_ValidationStatus Status, const FString& Message, float ExecutionTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void UpdateSystemHealth(int32 ActorCount, float MemoryUsage, int32 ClassCount, int32 AssetCount, bool bHealthy);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearResults();

    UFUNCTION(BlueprintPure, Category = "QA")
    TArray<FQA_TestResult> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintPure, Category = "QA")
    FQA_SystemHealth GetSystemHealth() const { return SystemHealth; }

    UFUNCTION(BlueprintPure, Category = "QA")
    int32 GetPassCount() const;

    UFUNCTION(BlueprintPure, Category = "QA")
    int32 GetFailCount() const;

    UFUNCTION(BlueprintPure, Category = "QA")
    bool IsSystemHealthy() const { return SystemHealth.bBridgeHealthy; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FQA_SystemHealth SystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString CurrentCycleID;

private:
    void LogTestResult(const FQA_TestResult& Result);
    void SaveReportToFile();
};