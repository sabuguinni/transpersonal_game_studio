#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "QA_VFXValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    NotTested   UMETA(DisplayName = "Not Tested")
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
        Result = EQA_VFXValidationResult::NotTested;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_VFXValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXValidator();

protected:
    virtual void BeginPlay() override;

public:
    // === VFX VALIDATION METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateNiagaraSystem(UNiagaraSystem* NiagaraSystem);

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateNiagaraComponent(UNiagaraComponent* NiagaraComponent);

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    TArray<FQA_VFXTestResult> ValidateAllVFXInLevel();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateVFXAudioSync();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateVFXLightingIntegration();

    // === CAMPFIRE VFX SPECIFIC VALIDATION ===
    
    UFUNCTION(BlueprintCallable, Category = "QA|VFX|Campfire")
    FQA_VFXTestResult ValidateCampfireVFX(AActor* CampfireActor);

    UFUNCTION(BlueprintCallable, Category = "QA|VFX|Campfire")
    FQA_VFXTestResult ValidateCampfireParticleCount();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX|Campfire")
    FQA_VFXTestResult ValidateCampfireLightInteraction();

    // === FOOTSTEP VFX SPECIFIC VALIDATION ===
    
    UFUNCTION(BlueprintCallable, Category = "QA|VFX|Footsteps")
    FQA_VFXTestResult ValidateFootstepVFX(AActor* CharacterActor);

    UFUNCTION(BlueprintCallable, Category = "QA|VFX|Footsteps")
    FQA_VFXTestResult ValidateFootstepDustSpawn();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX|Footsteps")
    FQA_VFXTestResult ValidateFootstepTerrainInteraction();

    // === PERFORMANCE VALIDATION ===
    
    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    FQA_VFXTestResult ValidateVFXFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    FQA_VFXTestResult ValidateVFXMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    FQA_VFXTestResult ValidateVFXLODSystem();

    // === INTEGRATION VALIDATION ===
    
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    FQA_VFXTestResult ValidateVFXCharacterIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    FQA_VFXTestResult ValidateVFXEnvironmentIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    FQA_VFXTestResult ValidateVFXWeatherIntegration();

    // === AUTOMATED TESTING ===
    
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA|Automation")
    void RunFullVFXValidationSuite();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA|Automation")
    void GenerateVFXValidationReport();

protected:
    // === VALIDATION DATA ===
    
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_VFXTestResult> ValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    float PerformanceThresholdFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    int32 MaxParticleCountThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    float MemoryUsageThresholdMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    bool bEnableDetailedLogging;

private:
    // === HELPER METHODS ===
    
    FQA_VFXTestResult CreateTestResult(const FString& TestName, EQA_VFXValidationResult Result, const FString& Details, float ExecutionTime = 0.0f);
    
    void LogTestResult(const FQA_VFXTestResult& TestResult);
    
    float MeasureExecutionTime(TFunction<void()> TestFunction);
    
    bool IsVFXSystemValid(UNiagaraSystem* System);
    
    bool IsVFXComponentValid(UNiagaraComponent* Component);
    
    int32 CountActiveParticles();
    
    float GetCurrentFrameRate();
    
    float GetVFXMemoryUsage();
};