#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass,
    Fail,
    Warning,
    Error
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_TestResult::Pass;
        ErrorMessage = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestFramework : public AActor
{
    GENERATED_BODY()

public:
    AQATestFramework();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    UStaticMeshComponent* TestMarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float TestInterval;

public:
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunSystemIntegrationTest();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunGameplayTest();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase CreateTestCase(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogTestResult(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetTestSuccessRate() const;

private:
    void InitializeTestMarker();
    void ValidateActorCount();
    void ValidateLighting();
    void ValidateNavigation();
    void ValidateGameMode();
};