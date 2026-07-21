#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "QA_VFXTestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXTestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Partial     UMETA(DisplayName = "Partial"),
    Fail        UMETA(DisplayName = "Fail"),
    Error       UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FQA_VFXTestReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    EQA_VFXTestResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    int32 ComponentsFound;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    int32 ComponentsExpected;

    FQA_VFXTestReport()
    {
        TestName = TEXT("");
        Result = EQA_VFXTestResult::Error;
        Details = TEXT("");
        ComponentsFound = 0;
        ComponentsExpected = 0;
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_VFXTestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXTestFramework();

    // VFX System Testing Functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor)
    FQA_VFXTestReport TestBreathSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor)
    FQA_VFXTestReport TestVolcanicAtmosphereSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor)
    FQA_VFXTestReport TestFootstepImpactSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor)
    FQA_VFXTestReport TestVFXPerformanceThresholds();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor)
    TArray<FQA_VFXTestReport> RunAllVFXTests();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 CountVFXComponents();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 CountParticleSystemComponents();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 CountNiagaraComponents();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 CountAudioComponents();

    // Actor Discovery
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<AActor*> FindActorsByLabel(const FString& LabelSubstring);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<AActor*> FindDinosaurActors();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<AActor*> FindVFXActors();

protected:
    // Test Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxVFXComponents = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 ExpectedBreathSystems = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 ExpectedImpactZones = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 ExpectedVolcanicSystems = 1;

    // Internal Test Helpers
    bool ValidateActorComponents(AActor* Actor, bool bRequireParticles, bool bRequireAudio);
    EQA_VFXTestResult CalculateTestResult(int32 Found, int32 Expected, float PassThreshold = 0.8f);
    FString GenerateTestDetails(const FString& SystemName, int32 Found, int32 Expected);
};