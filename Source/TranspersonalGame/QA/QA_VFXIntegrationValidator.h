#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "QA_VFXIntegrationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_VFXTestResult
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

    FQA_VFXTestResult()
    {
        bTestPassed = false;
        TestName = TEXT("Unknown Test");
        ErrorMessage = TEXT("");
        PerformanceScore = 0.0f;
        ParticleCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_DinosaurVFXProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur VFX")
    EDinosaurSpecies Species = EDinosaurSpecies::TRex;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur VFX")
    float ExpectedDustIntensity = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur VFX")
    float ExpectedParticleSize = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur VFX")
    float MassKg = 7000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur VFX")
    bool bShouldCreateGroundCracks = true;

    FQA_DinosaurVFXProfile()
    {
        Species = EDinosaurSpecies::TRex;
        ExpectedDustIntensity = 1.0f;
        ExpectedParticleSize = 100.0f;
        MassKg = 7000.0f;
        bShouldCreateGroundCracks = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_VFXIntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    AQA_VFXIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // VFX System Validation Methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor = true)
    FQA_VFXTestResult ValidateFootstepVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor = true)
    FQA_VFXTestResult ValidateCampfireVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor = true)
    FQA_VFXTestResult ValidateEnvironmentalVFX();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor = true)
    FQA_VFXTestResult ValidateVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor = true)
    TArray<FQA_VFXTestResult> RunFullVFXValidationSuite();

    // Dinosaur-specific VFX validation
    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor = true)
    FQA_VFXTestResult ValidateDinosaurVFXProfile(EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor = true)
    bool TestFootstepVFXForAllSpecies();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetCurrentVFXPerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetActiveParticleSystemCount();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool IsVFXSystemWithinPerformanceBudget();

protected:
    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxAllowedFrameTime = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxParticleSystemsAllowed = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float VFXTestRadius = 2000.0f;

    // Test results storage
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TArray<FQA_VFXTestResult> LastTestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    float LastPerformanceScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bAllTestsPassed = false;

    // Dinosaur VFX profiles for validation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Testing")
    TArray<FQA_DinosaurVFXProfile> DinosaurVFXProfiles;

private:
    // Internal validation helpers
    bool ValidateParticleSystemComponent(UParticleSystemComponent* ParticleComp);
    float CalculateVFXPerformanceImpact(AActor* VFXActor);
    bool CheckVFXAudioSynchronization(AActor* VFXActor);
    void InitializeDinosaurVFXProfiles();
    FQA_VFXTestResult CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMsg = TEXT(""));
};