#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Build_QAIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QATestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    float ExecutionTime;

    FBuild_QATestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QAIntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    TArray<FBuild_QATestResult> VFXTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    TArray<FBuild_QATestResult> PerformanceTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    TArray<FBuild_QATestResult> IntegrationTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bWithinActorLimits;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bAllTestsPassed;

    FBuild_QAIntegrationReport()
    {
        TotalActorCount = 0;
        DinosaurCount = 0;
        bWithinActorLimits = false;
        bAllTestsPassed = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_QAIntegrationManager : public AActor
{
    GENERATED_BODY()

public:
    ABuild_QAIntegrationManager();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FBuild_QAIntegrationReport ValidateQAResults();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool CheckActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    TArray<FBuild_QATestResult> ParseVFXTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    TArray<FBuild_QATestResult> ParsePerformanceTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    TArray<FBuild_QATestResult> ParseIntegrationTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void GenerateQAIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateCrossSystemCompatibility();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QAValidatorMesh;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    FBuild_QAIntegrationReport CurrentReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Integration")
    int32 MaxActorLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Integration")
    int32 MaxDinosaurLimit;

private:
    void InitializeQAValidator();
    bool ValidateTestActor(AActor* TestActor);
    FBuild_QATestResult CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMsg = TEXT(""), float ExecTime = 0.0f);
};