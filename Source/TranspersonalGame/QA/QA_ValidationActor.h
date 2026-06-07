#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QA_TestFramework.h"
#include "QA_ValidationActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationActor : public AActor
{
    GENERATED_BODY()
    
public:    
    AQA_ValidationActor();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    // QA Testing Interface
    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor)
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor)
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor)
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor)
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor)
    void ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void GenerateQAReport();

    // Automated testing
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    void StartAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    void StopAutomatedTesting();

    // Results access
    UFUNCTION(BlueprintCallable, Category = "QA Results")
    TArray<FQA_TestCase> GetLastTestResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA Results")
    FString GetValidationSummary() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Framework")
    UQA_TestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float AutoTestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bGenerateReports;

private:
    FTimerHandle AutoTestTimer;
    bool bIsRunningAutomatedTests;
    
    void RunAutomatedTestCycle();
    void LogValidationResults(const TArray<FQA_TestCase>& Results);
};