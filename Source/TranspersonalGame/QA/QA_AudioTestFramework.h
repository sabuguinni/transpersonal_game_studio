#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "QA_AudioTestFramework.generated.h"

/**
 * QA Audio Test Framework for Prehistoric Game Audio Validation
 * Tests ambient sounds, dinosaur audio, environmental effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_AudioTestFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_AudioTestFramework();

protected:
    virtual void BeginPlay() override;

    // Audio Test Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Audio Testing")
    class UAudioComponent* AmbientTestComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Audio Testing")
    class UAudioComponent* DinosaurTestComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Audio Testing")
    class UAudioComponent* EnvironmentTestComponent;

    // Test Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Audio Assets")
    class USoundCue* ForestAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Audio Assets")
    class USoundCue* TRexRoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Audio Assets")
    class USoundCue* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Audio Assets")
    class USoundCue* WindSound;

public:
    // Test Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Audio Testing")
    void RunAmbientAudioTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Audio Testing")
    void RunDinosaurAudioTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Audio Testing")
    void RunEnvironmentAudioTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Audio Testing")
    void RunAllAudioTests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Audio Testing")
    void ValidateAudioPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Audio Testing")
    void TestAudioOcclusion();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Audio Testing")
    void TestAudio3DPositioning();

    // Validation Results
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bAmbientTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bDinosaurTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bEnvironmentTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bPerformanceTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bOcclusionTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool b3DPositioningTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    float AudioMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    int32 ActiveAudioSources;

private:
    void LogAudioTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
    void MeasureAudioPerformance();
    bool ValidateAudioAsset(class USoundBase* Sound, const FString& AssetName);
    void TestAudioAttenuation(class UAudioComponent* Component, const FString& TestName);
};