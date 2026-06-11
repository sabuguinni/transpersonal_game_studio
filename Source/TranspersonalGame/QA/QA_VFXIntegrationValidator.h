#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Materials/Material.h"
#include "QA_VFXIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_VFXValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    EQA_VFXValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    float ExecutionTime;

    FQA_VFXValidationReport()
    {
        TestName = TEXT("");
        Result = EQA_VFXValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_VFXPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveNiagaraComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalParticleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float VFXMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceWithinLimits;

    FQA_VFXPerformanceMetrics()
    {
        ActiveNiagaraComponents = 0;
        TotalParticleCount = 0;
        VFXMemoryUsageMB = 0.0f;
        AverageFrameTime = 0.0f;
        bPerformanceWithinLimits = true;
    }
};

/**
 * QA VFX Integration Validator - Comprehensive testing framework for VFX systems
 * Validates Niagara effects, performance metrics, audio-VFX sync, and material integration
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_VFXIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    // Core Validation Functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor = true)
    TArray<FQA_VFXValidationReport> RunFullVFXValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor = true)
    FQA_VFXValidationReport ValidateNiagaraLibrary();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor = true)
    FQA_VFXValidationReport ValidateVFXAssets();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor = true)
    FQA_VFXValidationReport ValidateCharacterVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor = true)
    FQA_VFXValidationReport ValidateAudioVFXSynchronization();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor = true)
    FQA_VFXValidationReport ValidateVFXMaterials();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor = true)
    FQA_VFXValidationReport ValidateVFXLODSystem();

    // Performance Validation
    UFUNCTION(BlueprintCallable, Category = "QA Performance", CallInEditor = true)
    FQA_VFXPerformanceMetrics GatherVFXPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance", CallInEditor = true)
    bool ValidateVFXPerformanceLimits(const FQA_VFXPerformanceMetrics& Metrics);

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration", CallInEditor = true)
    AActor* CreateVFXTestActor(FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "QA Integration", CallInEditor = true)
    bool TestVFXEffectSpawning(const FString& EffectName, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "QA Integration", CallInEditor = true)
    bool TestVFXEffectLifecycle(UNiagaraComponent* NiagaraComp);

    // Reporting Functions
    UFUNCTION(BlueprintCallable, Category = "QA Reporting", CallInEditor = true)
    void GenerateVFXValidationReport(const TArray<FQA_VFXValidationReport>& Reports);

    UFUNCTION(BlueprintCallable, Category = "QA Reporting", CallInEditor = true)
    void LogVFXValidationSummary(const TArray<FQA_VFXValidationReport>& Reports);

protected:
    // Validation Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxAllowedNiagaraComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxVFXMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxFrameTimeThresholdMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bEnablePerformanceValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bEnableIntegrationTesting;

    // Test Results Storage
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TArray<FQA_VFXValidationReport> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    FQA_VFXPerformanceMetrics LastPerformanceMetrics;

    // Helper Functions
    FQA_VFXValidationReport CreateValidationReport(const FString& TestName, EQA_VFXValidationResult Result, const FString& Details, float ExecutionTime = 0.0f);
    
    bool IsVFXClassAvailable(const FString& ClassName);
    
    int32 CountActiveNiagaraComponents();
    
    float CalculateVFXMemoryUsage();
    
    TArray<UNiagaraSystem*> GetAllNiagaraSystems();
    
    TArray<UMaterial*> GetVFXMaterials();
};