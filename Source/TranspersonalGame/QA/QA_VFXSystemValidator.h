#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "QA_VFXSystemValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_VFXTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_VFXValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_VFXTestResult()
    {
        TestName = TEXT("");
        Result = EQA_VFXValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXSystemValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXSystemValidator();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA|VFX", CallInEditor)
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX", CallInEditor)
    bool ValidateNiagaraSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX", CallInEditor)
    bool ValidateParticlePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX", CallInEditor)
    bool ValidateVFXAssets();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    float MeasureVFXFrameTime();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    int32 CountActiveParticleSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    bool CheckVFXMemoryUsage();

    // Asset validation
    UFUNCTION(BlueprintCallable, Category = "QA|Assets")
    TArray<FString> FindMissingVFXAssets();

    UFUNCTION(BlueprintCallable, Category = "QA|Assets")
    bool ValidateVFXMaterials();

    UFUNCTION(BlueprintCallable, Category = "QA|Assets")
    bool ValidateVFXTextures();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool TestVFXCharacterIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool TestVFXEnvironmentIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool TestVFXCombatIntegration();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA|Reporting")
    TArray<FQA_VFXTestResult> GetLastTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA|Reporting")
    void GenerateVFXQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA|Reporting")
    void ExportTestResultsToFile();

protected:
    virtual void BeginPlay() override;

    // Internal validation helpers
    bool ValidateNiagaraSystem(UNiagaraSystem* System);
    bool ValidateParticleComponent(UNiagaraComponent* Component);
    bool CheckVFXLODSettings(UNiagaraSystem* System);
    bool ValidateVFXCulling(UNiagaraComponent* Component);

    // Performance monitoring
    void StartPerformanceTimer();
    float EndPerformanceTimer();
    void LogPerformanceMetrics();

    // Test result management
    void AddTestResult(const FString& TestName, EQA_VFXValidationResult Result, const FString& Details);
    void ClearTestResults();

private:
    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_VFXTestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QA|Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxFrameTimeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QA|Thresholds", meta = (AllowPrivateAccess = "true"))
    int32 MaxParticleCountThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QA|Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxMemoryUsageThreshold;

    // Timing
    double PerformanceStartTime;
    
    // Asset validation cache
    TArray<FString> CachedVFXAssets;
    bool bAssetCacheValid;
};