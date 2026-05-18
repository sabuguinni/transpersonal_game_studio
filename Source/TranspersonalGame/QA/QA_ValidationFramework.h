#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/NoExportTypes.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    Physics     UMETA(DisplayName = "Physics"),
    Audio       UMETA(DisplayName = "Audio"),
    VFX         UMETA(DisplayName = "VFX"),
    Character   UMETA(DisplayName = "Character"),
    World       UMETA(DisplayName = "World"),
    Assets      UMETA(DisplayName = "Assets"),
    Performance UMETA(DisplayName = "Performance"),
    Integration UMETA(DisplayName = "Integration")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_TestCategory Category;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_TestResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FDateTime Timestamp;

    FQA_TestReport()
    {
        TestName = TEXT("");
        Category = EQA_TestCategory::Core;
        Result = EQA_TestResult::Skipped;
        Description = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_SystemHealthMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 AvailableAssets;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 HealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FString> CriticalIssues;

    FQA_SystemHealthMetrics()
    {
        TotalActors = 0;
        LoadedClasses = 0;
        AvailableAssets = 0;
        MemoryUsagePercent = 0.0f;
        HealthScore = 0;
        CriticalIssues.Empty();
    }
};

/**
 * QA Validation Framework Component
 * Provides comprehensive testing and validation capabilities for the Transpersonal Game
 */
UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestReport ValidateCoreClasses();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestReport ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestReport ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestReport ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestReport ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestReport ValidateWorldPopulation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestReport ValidateAssetAvailability();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestReport ValidatePerformanceMetrics();

    // Integration tests
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    TArray<FQA_TestReport> RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_SystemHealthMetrics GetSystemHealthMetrics();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogTestResult(const FQA_TestReport& TestReport);

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsSystemHealthy();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestReport> TestHistory;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FQA_SystemHealthMetrics CurrentHealthMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxTestHistorySize;

private:
    float LastValidationTime;

    // Helper functions
    FQA_TestReport CreateTestReport(const FString& TestName, EQA_TestCategory Category, EQA_TestResult Result, const FString& Description, const FString& ErrorMessage = TEXT(""));
    void UpdateHealthMetrics();
    bool ValidateClassLoading(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);
};

/**
 * QA Test Actor - Spawnable actor for running validation tests in the level
 */
UCLASS()
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    UQA_ValidationFramework* ValidationFramework;

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void DisplayHealthMetrics();
};