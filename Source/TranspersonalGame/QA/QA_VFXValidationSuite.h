#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Materials/Material.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "QA_VFXValidationSuite.generated.h"

/**
 * QA_VFXValidationSuite - Comprehensive validation system for VFX components
 * Validates Niagara systems, particle effects, materials, and performance impact
 * Part of the QA framework for ensuring VFX quality and stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXValidationSuite : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXValidationSuite();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA|VFX Validation")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX Validation")
    bool ValidateNiagaraSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX Validation")
    bool ValidateVFXMaterials();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX Validation")
    bool ValidateVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX Validation")
    bool ValidateCretaceousVFXController();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    float GetVFXMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    int32 GetActiveParticleCount();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    float GetVFXFrameTime();

    // Validation results
    UPROPERTY(BlueprintReadOnly, Category = "QA|Results")
    bool bVFXSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Results")
    bool bNiagaraSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Results")
    bool bVFXMaterialsValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Results")
    bool bVFXPerformanceAcceptable;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Results")
    int32 TotalNiagaraComponents;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Results")
    int32 TotalVFXMaterials;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Results")
    float CurrentVFXMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Results")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Results")
    TArray<FString> ValidationWarnings;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Config")
    float MaxAcceptableVFXMemoryMB = 512.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Config")
    int32 MaxAcceptableParticleCount = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Config")
    float MaxAcceptableVFXFrameTimeMS = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Config")
    bool bEnableDetailedLogging = true;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Internal validation helpers
    bool ValidateNiagaraAssets();
    bool ValidateVFXActorComponents();
    bool ValidateParticleSystemIntegrity();
    bool CheckVFXControllerFunctionality();
    
    // Performance monitoring helpers
    void UpdatePerformanceMetrics();
    void LogValidationResults();
    void ClearValidationResults();

    // Validation state
    float LastValidationTime;
    bool bValidationInProgress;
    
    // Performance tracking
    float AccumulatedFrameTime;
    int32 FrameCount;
    float AverageFrameTime;
};