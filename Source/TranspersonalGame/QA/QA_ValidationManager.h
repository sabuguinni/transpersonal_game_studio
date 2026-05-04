#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QA_ValidationManager.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
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
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_ValidationReport()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_ValidationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Validation Tests
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    void RunAllValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateMapStructure();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidatePlayerSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateBiomeDistribution();

    // Report Management
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearValidationReports();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ExportReportsToFile();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_ValidationReport> ValidationReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bLogToConsole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bLogToFile;

private:
    float LastValidationTime;

    void AddValidationReport(const FString& TestName, EQA_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);
    void LogValidationResult(const FQA_ValidationReport& Report);
    int32 CountActorsOfClass(UClass* ActorClass);
    bool ValidateActorCount(UClass* ActorClass, int32 ExpectedMin, int32 ExpectedMax, const FString& ActorTypeName);
};