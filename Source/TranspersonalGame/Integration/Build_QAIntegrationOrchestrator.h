#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Build_QAIntegrationOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QATestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString ErrorMessage;

    FBuild_QATestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ExecutionTime = 0.0f;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_QATestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 VFXActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bWithinActorLimits;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float OverallScore;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        VFXActorCount = 0;
        bWithinActorLimits = true;
        OverallScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_QAIntegrationOrchestrator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_QAIntegrationOrchestrator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    bool bIntegrationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "QA Integration")
    float LastValidationTime;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void ExecuteFullIntegrationTest();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void ValidateVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FBuild_IntegrationReport GetIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool IsIntegrationHealthy() const;

    UFUNCTION(CallInEditor, Category = "QA Integration")
    void RunEditorIntegrationTest();

private:
    void ParseQATestResults();
    void ValidateCrossSystemCompatibility();
    void GenerateIntegrationScore();
    
    UPROPERTY()
    TArray<AActor*> CachedActors;
    
    UPROPERTY()
    float ValidationInterval;
};