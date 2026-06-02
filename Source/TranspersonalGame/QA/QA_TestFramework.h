#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Functionality   UMETA(DisplayName = "Functionality"),
    Memory          UMETA(DisplayName = "Memory"),
    Assets          UMETA(DisplayName = "Assets"),
    Blueprints      UMETA(DisplayName = "Blueprints"),
    VFX             UMETA(DisplayName = "VFX"),
    Audio           UMETA(DisplayName = "Audio"),
    Gameplay        UMETA(DisplayName = "Gameplay")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    bool bIsCritical;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Category = EQA_TestCategory::Functionality;
        Result = EQA_TestResult::NotRun;
        Description = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        bIsCritical = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TotalActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 VFXActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bWithinActorLimits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bWithinDinosaurLimits;

    FQA_PerformanceMetrics()
    {
        TotalActorCount = 0;
        DinosaurCount = 0;
        VFXActorCount = 0;
        MemoryUsageMB = 0.0f;
        FrameRate = 0.0f;
        bWithinActorLimits = true;
        bWithinDinosaurLimits = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestFramework();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TestMarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bLogResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    int32 MaxActorLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    int32 MaxDinosaurLimit;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunTestCategory(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateCompilation();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateAssets();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_PerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    int32 GetCriticalFailureCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool IsSystemReady() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void CreateTestZones();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void CleanupTestActors();

private:
    FTimerHandle TestTimerHandle;
    
    void InitializeTestCases();
    void LogTestResult(const FQA_TestCase& TestCase);
    FQA_TestCase CreateTestCase(const FString& Name, EQA_TestCategory Category, const FString& Description, bool bCritical = false);
    void ExecuteTest(FQA_TestCase& TestCase);
    bool CheckActorLimits();
    bool CheckDinosaurLimits();
    bool CheckMemoryUsage();
    bool ValidateModuleClasses();
    bool ValidateBlueprintAssets();
    bool ValidateNiagaraSystems();
    bool ValidateAudioAssets();
    void SpawnTestMarkers();
    void CleanupTestMarkers();
};