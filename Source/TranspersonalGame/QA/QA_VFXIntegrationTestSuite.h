#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/NoExportTypes.h"
#include "QA_VFXIntegrationTestSuite.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXTestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct FQA_VFXTestMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 DinosaurActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 VFXActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 QATestActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    float QAScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    bool bActorLimitOK = true;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    bool bDinosaurLimitOK = true;

    FQA_VFXTestMetrics()
    {
        TotalActors = 0;
        DinosaurActors = 0;
        VFXActors = 0;
        QATestActors = 0;
        QAScore = 0.0f;
        bActorLimitOK = true;
        bDinosaurLimitOK = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXIntegrationTestSuite : public UObject
{
    GENERATED_BODY()

public:
    UQA_VFXIntegrationTestSuite();

    // Core VFX integration tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_VFXTestResult RunFootstepImpactTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_VFXTestResult RunCampfireVFXTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_VFXTestResult RunDustCloudTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_VFXTestResult RunParticleSystemValidation();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_VFXTestMetrics GetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateDinosaurLimits();

    // Comprehensive test suite
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float RunCompleteVFXTestSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void CreateVFXTestScenarios();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void CleanupTestActors();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    FQA_VFXTestMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    TArray<AActor*> TestActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    bool bTestSuiteInitialized = false;

private:
    // Internal validation helpers
    void UpdateMetrics();
    bool IsVFXActorValid(AActor* Actor);
    bool IsDinosaurActor(AActor* Actor);
    void LogTestResult(const FString& TestName, EQA_VFXTestResult Result);
};