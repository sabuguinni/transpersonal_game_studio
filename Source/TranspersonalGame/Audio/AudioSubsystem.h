// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameplayTags.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Components/AudioComponent.h"
#include "AudioSubsystem.generated.h"

class UNarrativeSubsystem;
class UAudioStateComponent;
class UMetaSoundSource;
class USoundAttenuation;
class USoundSubmix;

UENUM(BlueprintType)
enum class EAudioState : uint8
{
    Silence,            // Complete silence (rare)
    Ambient,            // Natural forest sounds
    Tension,            // Building suspense
    Danger,             // Immediate threat
    Combat,             // Active fighting
    Exploration,        // Curious discovery
    Wonder,             // Awe and beauty
    Fear,               // Terror and panic
    Relief,             // Safety after danger
    Melancholy,         // Sadness and loss
    Mystery,            // Unknown and enigmatic
    Triumph,            // Victory and achievement
    Contemplation,      // Peaceful reflection
    Urgency,            // Time pressure
    Mystical            // Transpersonal experience
};

UENUM(BlueprintType)
enum class EAudioLayer : uint8
{
    Ambient,            // Environmental background
    Music,              // Adaptive musical score
    SFX,                // Sound effects
    Dialogue,           // Voice and speech
    UI,                 // Interface sounds
    Foley,              // Movement and interaction
    Creature,           // Dinosaur vocalizations
    Weather,            // Rain, wind, thunder
    Vegetation,         // Rustling leaves, branches
    Water,              // Rivers, rain, droplets
    Geological,         // Rock falls, earthquakes
    Temporal            // Time-related effects
};

UENUM(BlueprintType)
enum class EAudioIntensity : uint8
{
    Minimal,            // 0-20% intensity
    Low,                // 21-40% intensity
    Medium,             // 41-60% intensity
    High,               // 61-80% intensity
    Maximum             // 81-100% intensity
};

UENUM(BlueprintType)
enum class EAudioTransition : uint8
{
    Instant,            // Immediate change
    Quick,              // 0.5-1 second fade
    Smooth,             // 2-3 second transition
    Gradual,            // 5-8 second evolution
    Slow                // 10+ second transformation
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    EAudioState State = EAudioState::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    EAudioIntensity Intensity = EAudioIntensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    float Duration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    TArray<FGameplayTag> TriggerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    TArray<FGameplayTag> RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    TArray<FGameplayTag> ForbiddenTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    EAudioTransition TransitionType = EAudioTransition::Smooth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    float TransitionDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    bool bCanInterrupt = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    FVector SourceLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    float SourceRadius = 2000.0f;

    FAudioStateData()
    {
        State = EAudioState::Ambient;
        Intensity = EAudioIntensity::Medium;
        Duration = 30.0f;
        Priority = 1.0f;
        TransitionType = EAudioTransition::Smooth;
        TransitionDuration = 3.0f;
        bCanInterrupt = true;
        bLooping = true;
        SourceLocation = FVector::ZeroVector;
        SourceRadius = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudioLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    EAudioLayer LayerType = EAudioLayer::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TSoftObjectPtr<UMetaSoundSource> MetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TSoftObjectPtr<USoundClass> SoundClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TSoftObjectPtr<USoundSubmix> Submix;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TSoftObjectPtr<USoundAttenuation> Attenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float BasePitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    int32 Priority = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TMap<FString, float> Parameters;

    FAudioLayerConfig()
    {
        LayerType = EAudioLayer::Ambient;
        BaseVolume = 1.0f;
        BasePitch = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bIs3D = true;
        bLooping = true;
        Priority = 100;
    }
};

USTRUCT(BlueprintType)
struct FAdaptiveMusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    FString StateID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudioState AudioState = EAudioState::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    TArray<FAudioLayerConfig> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float Tempo = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    FString Key = TEXT("C Major");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float Tension = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float Wonder = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    TArray<FString> ValidTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float MinDuration = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float MaxDuration = 120.0f;

    FAdaptiveMusicState()
    {
        StateID = TEXT("");
        AudioState = EAudioState::Ambient;
        Tempo = 120.0f;
        Key = TEXT("C Major");
        Intensity = 0.5f;
        Tension = 0.0f;
        Wonder = 0.0f;
        Fear = 0.0f;
        MinDuration = 15.0f;
        MaxDuration = 120.0f;
    }
};

USTRUCT(BlueprintType)
struct FAmbientSoundscape
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    FString SoundscapeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    FText SoundscapeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    TArray<FAudioLayerConfig> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float TimeOfDayInfluence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float WeatherInfluence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float SeasonInfluence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float FalloffDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    TArray<FGameplayTag> BiomeTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    TArray<FGameplayTag> EnvironmentTags;

    FAmbientSoundscape()
    {
        SoundscapeID = TEXT("");
        SoundscapeName = FText::GetEmpty();
        TimeOfDayInfluence = 1.0f;
        WeatherInfluence = 1.0f;
        SeasonInfluence = 0.5f;
        CenterLocation = FVector::ZeroVector;
        Radius = 5000.0f;
        FalloffDistance = 1000.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioStateChanged, EAudioState, OldState, EAudioState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMusicTransition, const FString&, FromState, const FString&, ToState, float, TransitionTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSoundscapeChanged, const FString&, OldSoundscape, const FString&, NewSoundscape);

/**
 * Audio Subsystem - Manages adaptive audio, music, and soundscapes
 * Provides sample-accurate audio control using MetaSounds
 * Integrates with narrative system for emotional audio responses
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAudioSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Audio State Management
    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetAudioState(EAudioState NewState, EAudioIntensity Intensity = EAudioIntensity::Medium, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    EAudioState GetCurrentAudioState() const { return CurrentAudioState; }

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void TransitionToState(const FAudioStateData& StateData);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    bool CanTransitionToState(EAudioState NewState) const;

    // Adaptive Music System
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StartAdaptiveMusic(const FString& InitialState = TEXT("Ambient"));

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StopAdaptiveMusic(float FadeOutTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void TransitionMusicTo(const FString& NewState, float TransitionTime = 8.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetMusicParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    FString GetCurrentMusicState() const { return CurrentMusicState; }

    // Ambient Soundscape System
    UFUNCTION(BlueprintCallable, Category = "Audio|Soundscape")
    void SetActiveSoundscape(const FString& SoundscapeID, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Soundscape")
    void UpdateSoundscapeForLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Soundscape")
    void RegisterSoundscape(const FAmbientSoundscape& Soundscape);

    UFUNCTION(BlueprintCallable, Category = "Audio|Soundscape")
    FString GetActiveSoundscape() const { return ActiveSoundscapeID; }

    // Audio Layer Management
    UFUNCTION(BlueprintCallable, Category = "Audio|Layers")
    void PlayAudioLayer(EAudioLayer LayerType, const FAudioLayerConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio|Layers")
    void StopAudioLayer(EAudioLayer LayerType, float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Layers")
    void SetLayerVolume(EAudioLayer LayerType, float Volume, float FadeTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Layers")
    void SetLayerParameter(EAudioLayer LayerType, const FString& ParameterName, float Value);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void UpdateEnvironmentalAudio(float TimeOfDay, float WeatherIntensity, const FGameplayTag& BiomeTag);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void PlayEnvironmentalEvent(const FString& EventID, const FVector& Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SetGlobalReverb(const FString& ReverbPreset, float Intensity = 1.0f);

    // Creature Audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Creatures")
    void PlayCreatureVocalization(const FGameplayTag& CreatureType, const FString& VocalizationType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Creatures")
    void UpdateCreatureProximity(const FGameplayTag& CreatureType, float Distance, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Creatures")
    void RegisterCreatureAudioProfile(const FGameplayTag& CreatureType, const TArray<FAudioLayerConfig>& AudioProfile);

    // Narrative Integration
    UFUNCTION(BlueprintCallable, Category = "Audio|Narrative")
    void OnNarrativeEvent(const FString& EventID, const FGameplayTag& EventType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narrative")
    void PlayDialogueAudio(const FString& DialogueID, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narrative")
    void SetEmotionalState(float Fear, float Wonder, float Tension, float Calm);

    // Performance and Optimization
    UFUNCTION(BlueprintCallable, Category = "Audio|Performance")
    void SetAudioQuality(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Performance")
    void CullDistantAudio(const FVector& ListenerLocation, float MaxDistance = 10000.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Performance")
    int32 GetActiveAudioSourceCount() const;

    // Debug and Analysis
    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    void DebugPrintAudioState() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    TArray<FString> GetActiveAudioLayers() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    void SetDebugVisualization(bool bEnabled);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnAudioStateChanged OnAudioStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnMusicTransition OnMusicTransition;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnSoundscapeChanged OnSoundscapeChanged;

protected:
    // Internal state management
    void UpdateAudioSystem(float DeltaTime);
    void ProcessAudioTransitions(float DeltaTime);
    void UpdateAdaptiveMusic(float DeltaTime);
    void UpdateAmbientSoundscape(float DeltaTime);
    void CalculateEmotionalInfluence();

    // Audio component management
    UAudioComponent* CreateAudioComponent(const FAudioLayerConfig& Config);
    void UpdateAudioComponent(UAudioComponent* Component, const FAudioLayerConfig& Config);
    void ReleaseAudioComponent(UAudioComponent* Component);

    // MetaSound parameter management
    void UpdateMetaSoundParameters(UAudioComponent* Component, const TMap<FString, float>& Parameters);
    void SetMetaSoundTrigger(UAudioComponent* Component, const FString& TriggerName);

private:
    // Current audio state
    UPROPERTY()
    EAudioState CurrentAudioState = EAudioState::Ambient;

    UPROPERTY()
    EAudioIntensity CurrentIntensity = EAudioIntensity::Medium;

    UPROPERTY()
    float StateTransitionProgress = 0.0f;

    UPROPERTY()
    float StateTransitionDuration = 3.0f;

    UPROPERTY()
    EAudioState TargetAudioState = EAudioState::Ambient;

    // Music system
    UPROPERTY()
    FString CurrentMusicState = TEXT("Ambient");

    UPROPERTY()
    FString TargetMusicState = TEXT("");

    UPROPERTY()
    float MusicTransitionProgress = 0.0f;

    UPROPERTY()
    float MusicTransitionDuration = 8.0f;

    UPROPERTY()
    TMap<FString, FAdaptiveMusicState> MusicStates;

    // Soundscape system
    UPROPERTY()
    FString ActiveSoundscapeID = TEXT("");

    UPROPERTY()
    FString TargetSoundscapeID = TEXT("");

    UPROPERTY()
    float SoundscapeTransitionProgress = 0.0f;

    UPROPERTY()
    TMap<FString, FAmbientSoundscape> RegisteredSoundscapes;

    // Audio components
    UPROPERTY()
    TMap<EAudioLayer, UAudioComponent*> ActiveAudioLayers;

    UPROPERTY()
    TArray<UAudioComponent*> PooledAudioComponents;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveEnvironmentalSources;

    // Emotional state
    UPROPERTY()
    float EmotionalFear = 0.0f;

    UPROPERTY()
    float EmotionalWonder = 0.0f;

    UPROPERTY()
    float EmotionalTension = 0.0f;

    UPROPERTY()
    float EmotionalCalm = 1.0f;

    // Performance tracking
    UPROPERTY()
    int32 MaxConcurrentSources = 64;

    UPROPERTY()
    int32 CurrentAudioSourceCount = 0;

    UPROPERTY()
    bool bDebugVisualizationEnabled = false;

    // Subsystem references
    UPROPERTY()
    UNarrativeSubsystem* NarrativeSubsystem = nullptr;

    // Timer handles
    FTimerHandle AudioUpdateTimer;
    FTimerHandle PerformanceUpdateTimer;
};