#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetasoundSource.h"
#include "AudioComponent.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "GameplayTagContainer.h"
#include "AudioSystemManager.h"
#include "AdaptiveMusicSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAdaptiveMusic, Log, All);

/**
 * Adaptive Music System - The Emotional Conductor
 * 
 * This system orchestrates the game's musical narrative, creating seamless
 * transitions between emotional states that the player never consciously notices
 * but always emotionally feels.
 * 
 * Core Philosophy: Music is the subconscious narrator of emotional experience.
 * When perfect, the player thinks about the world, not the music.
 */

UENUM(BlueprintType)
enum class EMusicEmotionalState : uint8
{
    // Primary Emotional States
    Wonder_Pure         UMETA(DisplayName = "Pure Wonder"),
    Wonder_Cautious     UMETA(DisplayName = "Cautious Wonder"),
    Fear_Creeping       UMETA(DisplayName = "Creeping Dread"),
    Fear_Immediate      UMETA(DisplayName = "Immediate Terror"),
    
    // Survival States
    Survival_Calm       UMETA(DisplayName = "Calm Survival"),
    Survival_Urgent     UMETA(DisplayName = "Urgent Survival"),
    Survival_Desperate  UMETA(DisplayName = "Desperate Survival"),
    
    // Discovery States
    Discovery_Scientific UMETA(DisplayName = "Scientific Discovery"),
    Discovery_Mystical   UMETA(DisplayName = "Mystical Discovery"),
    Discovery_Dangerous  UMETA(DisplayName = "Dangerous Discovery"),
    
    // Connection States
    Connection_Nature    UMETA(DisplayName = "Nature Connection"),
    Connection_Creatures UMETA(DisplayName = "Creature Connection"),
    Connection_Time      UMETA(DisplayName = "Temporal Connection"),
    
    // Isolation States
    Isolation_Peaceful   UMETA(DisplayName = "Peaceful Solitude"),
    Isolation_Lonely     UMETA(DisplayName = "Loneliness"),
    Isolation_Existential UMETA(DisplayName = "Existential Isolation")
};

UENUM(BlueprintType)
enum class EMusicLayer : uint8
{
    Foundation      UMETA(DisplayName = "Foundation Layer"),
    Harmony         UMETA(DisplayName = "Harmony Layer"),
    Melody          UMETA(DisplayName = "Melody Layer"),
    Rhythm          UMETA(DisplayName = "Rhythm Layer"),
    Texture         UMETA(DisplayName = "Texture Layer"),
    Tension         UMETA(DisplayName = "Tension Layer"),
    Release         UMETA(DisplayName = "Release Layer"),
    Ethereal        UMETA(DisplayName = "Ethereal Layer")
};

USTRUCT(BlueprintType)
struct FMusicLayerState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State")
    EMusicLayer Layer = EMusicLayer::Foundation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State")
    float FilterCutoff = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State")
    float FadeTime = 2.0f;
};

USTRUCT(BlueprintType)
struct FMusicEmotionalContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    EMusicEmotionalState PrimaryState = EMusicEmotionalState::Wonder_Pure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    EMusicEmotionalState SecondaryState = EMusicEmotionalState::Wonder_Pure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendRatio = 0.0f; // 0.0 = Pure Primary, 1.0 = Pure Secondary

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TensionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WonderLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IsolationLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConnectionLevel = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TemporalDisplacement = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    float TimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    int32 DinosaurProximityCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    bool bInShelter = false;
};

USTRUCT(BlueprintType)
struct FMusicTransitionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float CrossfadeTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float LayerFadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float ParameterInterpolationTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bUseSmartTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bSyncToMusicalBeat = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BPM = 80.0f;
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAdaptiveMusicSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAdaptiveMusicSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Music Control
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void InitializeMusicSystem(UAudioSystemManager* AudioManager);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TransitionToEmotionalState(EMusicEmotionalState TargetState, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void BlendEmotionalStates(EMusicEmotionalState PrimaryState, EMusicEmotionalState SecondaryState, float BlendRatio);

    // Layer Management
    UFUNCTION(BlueprintCallable, Category = "Music Layers")
    void SetLayerIntensity(EMusicLayer Layer, float Intensity, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music Layers")
    void ActivateLayer(EMusicLayer Layer, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music Layers")
    void DeactivateLayer(EMusicLayer Layer, float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music Layers")
    void SetAllLayersIntensity(float Intensity, float FadeTime = 2.0f);

    // Emotional Context Updates
    UFUNCTION(BlueprintCallable, Category = "Emotional Context")
    void UpdateEmotionalContext(const FMusicEmotionalContext& NewContext);

    UFUNCTION(BlueprintCallable, Category = "Emotional Context")
    void SetTensionLevel(float TensionLevel, float TransitionTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Context")
    void SetWonderLevel(float WonderLevel, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Context")
    void SetIsolationLevel(float IsolationLevel, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Context")
    void SetConnectionLevel(float ConnectionLevel, float TransitionTime = 2.0f);

    // Environmental Influence
    UFUNCTION(BlueprintCallable, Category = "Environmental Music")
    void UpdateTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environmental Music")
    void UpdateWeatherIntensity(float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental Music")
    void UpdateDinosaurProximity(int32 DinosaurCount, float ClosestDistance);

    UFUNCTION(BlueprintCallable, Category = "Environmental Music")
    void SetShelterState(bool bInShelter);

    // Event-Driven Music
    UFUNCTION(BlueprintCallable, Category = "Music Events")
    void TriggerDiscoveryMoment(bool bPositiveDiscovery, float IntensityLevel);

    UFUNCTION(BlueprintCallable, Category = "Music Events")
    void TriggerDangerMoment(float DangerLevel, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Music Events")
    void TriggerConnectionMoment(float ConnectionIntensity);

    UFUNCTION(BlueprintCallable, Category = "Music Events")
    void TriggerMemoryEcho(float TemporalDistance);

    // Advanced Controls
    UFUNCTION(BlueprintCallable, Category = "Advanced Music")
    void SetMasterMusicVolume(float Volume, float FadeTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Advanced Music")
    void SetMusicBPM(float BPM);

    UFUNCTION(BlueprintCallable, Category = "Advanced Music")
    void SyncToGameplayBeat(bool bEnable);

    // Query Functions
    UFUNCTION(BlueprintPure, Category = "Music State")
    EMusicEmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalContext.PrimaryState; }

    UFUNCTION(BlueprintPure, Category = "Music State")
    FMusicEmotionalContext GetCurrentEmotionalContext() const { return CurrentEmotionalContext; }

    UFUNCTION(BlueprintPure, Category = "Music State")
    float GetLayerIntensity(EMusicLayer Layer) const;

    UFUNCTION(BlueprintPure, Category = "Music State")
    bool IsLayerActive(EMusicLayer Layer) const;

protected:
    // Core System References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System References")
    TObjectPtr<UAudioSystemManager> AudioSystemManager;

    // MetaSound Assets
    UPROPERTY(EditDefaultsOnly, Category = "MetaSound Assets")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "MetaSound Assets")
    TMap<EMusicEmotionalState, UMetaSoundSource*> EmotionalStateMetaSounds;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> PrimaryMusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> SecondaryMusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> LayerAudioComponents;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FMusicEmotionalContext CurrentEmotionalContext;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    TMap<EMusicLayer, FMusicLayerState> LayerStates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FMusicTransitionSettings TransitionSettings;

    // Transition Management
    UPROPERTY()
    FTimerHandle EmotionalTransitionTimer;

    UPROPERTY()
    TMap<EMusicLayer, FTimerHandle> LayerTransitionTimers;

private:
    // Internal State Management
    void UpdateMetaSoundParameters();
    void ProcessEmotionalTransition(float DeltaTime);
    void ProcessLayerTransitions(float DeltaTime);
    
    // Parameter Interpolation
    struct FParameterInterpolation
    {
        float StartValue;
        float TargetValue;
        float CurrentTime;
        float Duration;
        FString ParameterName;
    };
    
    TMap<FString, FParameterInterpolation> ActiveParameterInterpolations;
    void UpdateParameterInterpolations(float DeltaTime);
    
    // Smart Transition Logic
    void CalculateOptimalTransition(EMusicEmotionalState FromState, EMusicEmotionalState ToState);
    float CalculateEmotionalDistance(EMusicEmotionalState StateA, EMusicEmotionalState StateB);
    
    // Beat Synchronization
    float BeatTimer;
    float CurrentBPM;
    bool bSyncToGameplayBeat;
    void UpdateBeatSync(float DeltaTime);
    
    // Performance Optimization
    void OptimizeLayerPerformance();
    int32 MaxActiveLayers;
    float CPUBudgetPercentage;
};