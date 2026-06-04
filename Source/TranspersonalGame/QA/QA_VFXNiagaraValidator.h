#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "QA_VFXNiagaraValidator.generated.h"

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

/**
 * QA Validator for VFX and Niagara systems integration
 * Validates particle effects, performance, and visual quality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXNiagaraValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXNiagaraValidator();

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    TArray<FQA_VFXTestResult> ValidateAllVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateNiagaraSystem(UNiagaraSystem* System);

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateParticleCount();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateVFXMemoryUsage();

    // Specific VFX tests
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult TestFootstepVFX();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult TestCampfireVFX();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult TestWeatherVFX();

    // Integration tests
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult TestVFXCharacterIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult TestVFXEnvironmentIntegration();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    void GenerateVFXValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FString GetValidationSummary() const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_VFXTestResult> TestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    int32 MaxParticleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    bool bEnableDetailedLogging;

private:
    // Helper methods
    FQA_VFXTestResult CreateTestResult(const FString& TestName, EQA_VFXValidationResult Result, const FString& Details, float ExecutionTime = 0.0f);
    
    void LogTestResult(const FQA_VFXTestResult& Result);
    
    int32 CountActiveParticles();
    
    float GetVFXMemoryUsage();
    
    TArray<UNiagaraComponent*> GetAllNiagaraComponents();
};