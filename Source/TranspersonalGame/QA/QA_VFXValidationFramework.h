#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "Materials/Material.h"
#include "QA_VFXValidationFramework.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_VFXTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bVFXManagerLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bCampfireSystemExists = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bDustSystemExists = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bFireMaterialExists = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bDustMaterialExists = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bCharacterIntegration = false;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 ParticleSystemCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TestActorsSpawned = 0;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ValidationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FString> WarningMessages;
};

UENUM(BlueprintType)
enum class EQA_VFXTestType : uint8
{
    SystemLoad,
    ParticleSpawn,
    MaterialValidation,
    PerformanceCheck,
    IntegrationTest
};

/**
 * QA VFX Validation Framework
 * Comprehensive testing system for VFX components and integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_VFXValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_VFXValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FQA_VFXTestResult RunFullVFXValidation();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateVFXManager();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateParticleSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateVFXMaterials();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateCharacterIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    int32 CountActiveParticleSystems();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool CheckVFXPerformance();

    // Test spawning functions
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    AActor* SpawnTestCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    AActor* SpawnTestDustImpact(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    void CleanupTestActors();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    void GenerateVFXReport();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    void LogTestResults(const FQA_VFXTestResult& Results);

protected:
    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bCleanupAfterTest = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxAllowedParticleSystems = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float PerformanceThresholdMS = 16.67f; // 60 FPS target

    // Test state
    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    FQA_VFXTestResult LastTestResult;

    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    TArray<AActor*> SpawnedTestActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA State")
    bool bTestInProgress = false;

private:
    // Internal validation helpers
    bool ValidateNiagaraSystem(const FString& SystemPath);
    bool ValidateMaterial(const FString& MaterialPath);
    void AddError(const FString& ErrorMessage);
    void AddWarning(const FString& WarningMessage);
    
    // Performance monitoring
    double TestStartTime = 0.0;
    TArray<FString> CurrentErrors;
    TArray<FString> CurrentWarnings;
};