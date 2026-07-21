#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "SharedTypes.h"
#include "QA_VFXValidationSuite.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_VFXTestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TSoftObjectPtr<UNiagaraSystem> ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FVector TestLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float ExpectedParticleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bTestPassed;

    FQA_VFXTestCase()
    {
        TestName = TEXT("Default VFX Test");
        TestLocation = FVector::ZeroVector;
        ExpectedParticleCount = 100.0f;
        MaxFrameTime = 16.67f; // 60 FPS target
        bTestPassed = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float AverageFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MinFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 TotalParticleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MemoryUsageMB;

    FQA_PerformanceMetrics()
    {
        AverageFrameTime = 0.0f;
        MinFrameTime = 0.0f;
        MaxFrameTime = 0.0f;
        TotalParticleCount = 0;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_VFXValidationSuite : public AActor
{
    GENERATED_BODY()

public:
    AQA_VFXValidationSuite();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // VFX Test Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_VFXTestCase> VFXTestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bRunAutomaticTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    // VFX Validation Functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllVFXTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateFireParticleSystem(const FVector& TestLocation);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateWaterParticleSystem(const FVector& TestLocation);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateBloodParticleSystem(const FVector& TestLocation);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateSmokeParticleSystem(const FVector& TestLocation);

    // Performance Testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void MeasureVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool IsPerformanceWithinLimits() const;

    // Test Result Management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void SaveTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void LoadTestResults();

    // Editor Testing Functions
    UFUNCTION(CallInEditor, Category = "QA Testing")
    void RunQuickVFXValidation();

    UFUNCTION(CallInEditor, Category = "QA Testing")
    void TestAllParticleSystems();

    UFUNCTION(CallInEditor, Category = "QA Testing")
    void ValidateVFXIntegration();

protected:
    // Internal test tracking
    UPROPERTY()
    float TestTimer;

    UPROPERTY()
    int32 CurrentTestIndex;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveTestComponents;

    // Performance monitoring
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    float PerformanceTestDuration;

    // Helper functions
    void InitializeTestCases();
    void CleanupTestComponents();
    bool SpawnTestParticleSystem(UNiagaraSystem* System, const FVector& Location);
    void UpdatePerformanceMetrics(float DeltaTime);
    void LogTestResult(const FString& TestName, bool bPassed, const FString& Details);
};