#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Materials/Material.h"
#include "Components/AudioComponent.h"
#include "QA_VFXValidator.generated.h"

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
class TRANSPERSONALGAME_API UQA_VFXValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXValidator();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    TArray<FQA_VFXTestResult> ValidateAllVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateNiagaraSystem(class UNiagaraSystem* System);

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateVFXMaterials();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateParticlePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateAudioVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult ValidateVFXActorComponents();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    float GetParticleSystemFrameTime();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    int32 GetActiveParticleCount();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    bool IsVFXPerformanceAcceptable();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    TArray<AActor*> FindActorsWithVFXComponents();

    UFUNCTION(BlueprintCallable, Category = "QA|Integration")
    bool ValidateVFXAudioSync(AActor* Actor);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA|Reporting")
    void GenerateVFXValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA|Reporting")
    FString GetLastValidationSummary();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_VFXTestResult> LastTestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float MaxAcceptableFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 MaxAcceptableParticleCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bVerboseLogging;

private:
    FQA_VFXTestResult CreateTestResult(const FString& TestName, EQA_VFXValidationResult Result, const FString& Details, float ExecutionTime);
    void LogTestResult(const FQA_VFXTestResult& Result);
    bool IsSystemValid(class UNiagaraSystem* System);
    bool IsMaterialValid(class UMaterial* Material);
};