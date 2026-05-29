#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "SharedTypes.h"
#include "QA_VFXFootstepValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_FootstepTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    bool bTestPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    float PerformanceScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    int32 ParticleCount = 0;

    FQA_FootstepTestResult()
    {
        bTestPassed = false;
        TestName = TEXT("Unknown Test");
        ErrorMessage = TEXT("");
        PerformanceScore = 0.0f;
        ParticleCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_VFXIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 TotalVFXActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 ActiveNiagaraSystems = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    int32 DinosaurActorsWithVFX = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA Metrics")
    bool bFootstepSystemReady = false;

    FQA_VFXIntegrationMetrics()
    {
        TotalVFXActors = 0;
        ActiveNiagaraSystems = 0;
        AverageFrameTime = 0.0f;
        DinosaurActorsWithVFX = 0;
        bFootstepSystemReady = false;
    }
};

/**
 * QA Validator for VFX Footstep System Integration
 * Validates the VFX_FootstepManager system created by Agent #17
 * Tests performance, integration, and functionality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXFootstepValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXFootstepValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_FootstepTestResult ValidateFootstepManager();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_FootstepTestResult TestNiagaraSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_FootstepTestResult TestPerformanceWithMultipleEffects();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_FootstepTestResult TestDinosaurIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_VFXIntegrationMetrics GetVFXIntegrationMetrics();

    // Comprehensive validation suite
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_FootstepTestResult> RunFullVFXValidationSuite();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestVFXPerformanceUnderLoad(int32 MaxParticleSystems = 50);

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXSystemPaths();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestFootstepEffectScaling();

    // Cleanup and utilities
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void CleanupTestActors();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateVFXValidationReport();

protected:
    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxTestParticles = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float PerformanceThreshold = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval = 5.0f;

    // Test state
    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    TArray<FQA_FootstepTestResult> LastTestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    FQA_VFXIntegrationMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    bool bValidationInProgress = false;

    // Test actors for cleanup
    UPROPERTY()
    TArray<AActor*> TestActors;

    // Timing
    float LastTestTime = 0.0f;

private:
    // Helper methods
    bool IsVFXSystemValid(const FString& SystemPath);
    int32 CountActiveNiagaraSystems();
    float MeasureFrameTime();
    bool SpawnTestFootstepEffect(const FVector& Location, EQA_DinosaurSpecies Species);
    void LogTestResult(const FQA_FootstepTestResult& Result);
};