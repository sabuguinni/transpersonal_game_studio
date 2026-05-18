#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QAValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct FQA_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTimeMs;

    FQA_ValidationTest()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQA_ValidationResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTimeMs = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemHealthReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MemoryUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 PhysicsActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 VFXActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bBridgeConnected;

    FQA_SystemHealthReport()
    {
        TotalActors = 0;
        MemoryUsagePercent = 0.0f;
        PhysicsActors = 0;
        DinosaurActors = 0;
        VFXActors = 0;
        bBridgeConnected = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQAValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQAValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Validation test storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_ValidationTest> ValidationTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FQA_SystemHealthReport SystemHealth;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunAllValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateBridgeConnection();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateLightingAndAudio();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidatePerformanceMetrics();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemHealthReport GenerateHealthReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogValidationResults();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void SaveValidationReport(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsSystemHealthy();

private:
    // Internal validation helpers
    FQA_ValidationTest CreateTest(const FString& Name, const FString& Description);
    void AddTestResult(FQA_ValidationTest& Test, EQA_ValidationResult Result, const FString& ErrorMsg = TEXT(""));
    void LogTest(const FQA_ValidationTest& Test);

    // Performance monitoring
    UPROPERTY()
    float LastValidationTime;

    UPROPERTY()
    bool bAutoValidation;

    UPROPERTY()
    float ValidationInterval;
};