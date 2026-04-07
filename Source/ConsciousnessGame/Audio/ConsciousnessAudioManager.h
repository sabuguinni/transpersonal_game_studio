#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "ConsciousnessAudioManager.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    Questioning     UMETA(DisplayName = "Questioning"), 
    Resistance      UMETA(DisplayName = "Resistance"),
    Acceptance      UMETA(DisplayName = "Acceptance"),
    Integration     UMETA(DisplayName = "Integration"),
    Transcendence   UMETA(DisplayName = "Transcendence")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Fear            UMETA(DisplayName = "Fear"),
    Curiosity       UMETA(DisplayName = "Curiosity"),
    Wonder          UMETA(DisplayName = "Wonder"),
    Confusion       UMETA(DisplayName = "Confusion"),
    Clarity         UMETA(DisplayName = "Clarity"),
    Peace           UMETA(DisplayName = "Peace")
};

USTRUCT(BlueprintType)
struct FAudioLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState RequiredState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState RequiredEmotion;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CONSCIOUSNESSGAME_API UConsciousnessAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessAudioManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                              FActorComponentTickFunction* ThisTickFunction) override;

    // Core Audio Management
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void UpdateConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void UpdateEmotionalState(EEmotionalState NewEmotion);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void SetConsciousnessLevel(float Level); // 0.0 to 1.0

    // Dynamic Audio Layers
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void PlayNarrativeAudio(const FString& DialogueID, bool bInterruptCurrent = false);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void PlayInnerVoice(const FString& ThoughtID, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void TriggerMemoryEcho(const FString& MemoryID, float FadeInTime = 3.0f);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void SetAmbientIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void PlayQuestAudio(const FString& QuestID, bool bIsCompletion = false);

protected:
    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* InnerVoiceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MemoryEchoComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* QuestAudioComponent;

    // Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layers")
    TArray<FAudioLayer> ConsciousnessLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layers")
    TArray<FAudioLayer> EmotionalLayers;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<FString, USoundCue*> DialogueLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<FString, USoundCue*> InnerVoiceLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<FString, USoundCue*> MemoryLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<FString, USoundCue*> QuestAudioLibrary;

    // State Management
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessState CurrentConsciousnessState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EEmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float ConsciousnessLevel;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float AmbientIntensity;

private:
    void UpdateAudioLayers();
    void FadeAudioLayer(UAudioComponent* Component, float TargetVolume, float FadeTime);
    USoundCue* GetAudioForState(EConsciousnessState State, EEmotionalState Emotion);
    
    // Fade management
    TMap<UAudioComponent*, float> FadeTargets;
    TMap<UAudioComponent*, float> FadeSpeeds;
};