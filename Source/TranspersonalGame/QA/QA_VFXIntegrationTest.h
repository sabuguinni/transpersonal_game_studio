#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "QA_VFXIntegrationTest.generated.h"

/**
 * QA VFX Integration Test Framework
 * Validates VFX systems integration with game actors and performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_VFXIntegrationTest : public AActor
{
    GENERATED_BODY()

public:
    AQA_VFXIntegrationTest();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // VFX Test Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA VFX Testing")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA VFX Testing")
    class UNiagaraComponent* TestParticleComponent;

    // Test Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA VFX Config")
    bool bRunContinuousTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA VFX Config")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA VFX Config")
    int32 MaxParticleCount;

    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 ActiveParticleCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    float VFXMemoryUsage;

    // Test Results
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bVFXSystemsHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TArray<FString> TestResults;

    // VFX Integration Test Functions
    UFUNCTION(BlueprintCallable, Category = "QA VFX Testing")
    void RunVFXIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Testing")
    void TestCampfireVFX();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Testing")
    void TestFootstepVFX();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Testing")
    void TestWeatherVFX();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Testing")
    void TestCombatVFX();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void MeasureVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateFrameRate(float MinFPS = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "QA Results")
    void GenerateVFXReport();

    UFUNCTION(BlueprintCallable, Category = "QA Results")
    void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = "");

private:
    // Internal test state
    float LastTestTime;
    int32 TestsRun;
    int32 TestsPassed;
    int32 TestsFailed;

    // VFX System References
    UPROPERTY()
    class AVFX_ParticleManager* VFXManager;

    // Helper functions
    void InitializeVFXTests();
    void CleanupVFXTests();
    bool IsVFXSystemReady();
    void UpdatePerformanceMetrics();
};