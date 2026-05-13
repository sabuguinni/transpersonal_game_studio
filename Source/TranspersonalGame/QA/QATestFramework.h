#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase GetTestResult(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetPassedTestCount();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetFailedTestCount();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    FTimerHandle TestTimerHandle;
    
    void ExecuteTest(FQA_TestCase& TestCase);
    bool ValidateClassLoading(const FString& ClassName);
    bool ValidateActorSpawning(UClass* ActorClass);
    bool ValidateComponentIntegrity(AActor* Actor);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestManager();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    UQA_TestComponent* TestComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bRunSystemTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bRunPerformanceTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bRunIntegrationTests;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void InitializeSystemTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void SetupDefaultTests();
    void ValidateModuleIntegrity();
    void CheckMemoryUsage();
    void ValidateFrameRate();
};

#include "QATestFramework.generated.h"