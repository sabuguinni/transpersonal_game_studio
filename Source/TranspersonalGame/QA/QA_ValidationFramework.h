#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Skipped;
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
    int32 ComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bBridgeHealthy;

    FQA_SystemHealth()
    {
        ActorCount = 0;
        ComponentCount = 0;
        MemoryUsageMB = 0.0f;
        FrameRate = 0.0f;
        bBridgeHealthy = false;
    }
};

/**
 * QA Validation Framework - Comprehensive testing and quality assurance system
 * Provides automated testing, performance monitoring, and system health checks
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateActorSpawning(TSubclassOf<AActor> ActorClass, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateComponentFunctionality(UActorComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemHealth GetSystemHealth();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult RunPerformanceTest(const FString& TestName, int32 IterationCount);

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_TestResult> RunFullValidationSuite();

    // Bridge health monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsBridgeHealthy();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogValidationResults(const TArray<FQA_TestResult>& Results);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> LastTestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FQA_SystemHealth CurrentSystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float LastValidationTime;

private:
    // Internal validation helpers
    FQA_TestResult ValidateActorProperties(AActor* Actor);
    FQA_TestResult ValidateComponentProperties(UActorComponent* Component);
    void UpdateSystemHealth();
    float MeasureExecutionTime(TFunction<void()> TestFunction);
};