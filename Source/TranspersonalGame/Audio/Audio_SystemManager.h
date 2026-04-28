#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../Core/SharedTypes.h"
#include "Audio_SystemManager.generated.h"

// Audio layer types for mixing and priority
UENUM(BlueprintType)
enum class EAudio_LayerType : uint8
{
    Ambient = 0     UMETA(DisplayName = "Ambient"),
    Music = 1       UMETA(DisplayName = "Music"),
    SFX = 2         UMETA(DisplayName = "Sound Effects"),
    Voice = 3       UMETA(DisplayName = "Voice/Dialogue"),
    UI = 4          UMETA(DisplayName = "User Interface")
};

// Audio state for adaptive music system
UENUM(BlueprintType)
enum class EAudio_GameState : uint8
{
    Peaceful = 0    UMETA(DisplayName = "Peaceful"),
    Tense = 1       UMETA(DisplayName = "Tense"),
    Combat = 2      UMETA(DisplayName = "Combat"),
    Danger = 3      UMETA(DisplayName = "Danger"),
    Exploration = 4 UMETA(DisplayName = "Exploration")
};

// Audio source configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SourceConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_LayerType LayerType = EAudio_LayerType::SFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    FAudio_SourceConfig()
    {
        SoundAsset = nullptr;
        LayerType = EAudio_LayerType::SFX;
        Volume = 1.0f;
        Pitch = 1.0f;
        bIs3D = true;
        bLooping = false;
    }
};

/**
 * Central audio system manager for prehistoric survival game
 * Handles adaptive music, environmental audio, and 3D positioned sound effects
 * Integrates with MetaSounds for dynamic audio generation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE AUDIO MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetGameState(EAudio_GameState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetLayerVolume(EAudio_LayerType LayerType, float Volume);

    // === ADAPTIVE MUSIC SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void PlayAdaptiveMusic(EAudio_GameState MusicState);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TransitionToMusicState(EAudio_GameState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StopAdaptiveMusic(float FadeOutTime = 1.0f);

    // === ENVIRONMENTAL AUDIO ===
    
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayAmbientSound(const FAudio_SourceConfig& Config, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StopAmbientSound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateTimeOfDayAudio(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeatherAudio(EWeatherType WeatherType);

    // === 3D POSITIONED AUDIO ===
    
    UFUNCTION(BlueprintCallable, Category = "3D Audio")
    UAudioComponent* Play3DSound(const FAudio_SourceConfig& Config, FVector Location, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "3D Audio")
    void PlayFootstepSound(FVector Location, ESurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "3D Audio")
    void PlayDinosaurRoar(FVector Location, EDinosaurSpecies Species, float IntensityMultiplier = 1.0f);

    // === SURVIVAL AUDIO FEEDBACK ===
    
    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void PlayHeartbeatIntensity(float IntensityLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void PlayBreathingPattern(float StaminaLevel, float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void PlayHungerSound(float HungerLevel);

protected:
    // === CORE COMPONENTS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    // === AUDIO STATE ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    EAudio_GameState CurrentGameState = EAudio_GameState::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    TMap<EAudio_LayerType, float> LayerVolumes;

    // === ADAPTIVE MUSIC ASSETS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Assets")
    TMap<EAudio_GameState, TSoftObjectPtr<UMetaSoundSource>> AdaptiveMusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Assets")
    float MusicTransitionTime = 2.0f;

    // === ENVIRONMENTAL AUDIO ASSETS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Assets")
    TMap<EWeatherType, TSoftObjectPtr<USoundCue>> WeatherSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Assets")
    TMap<ESurfaceType, TSoftObjectPtr<USoundCue>> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Assets")
    TMap<EDinosaurSpecies, TSoftObjectPtr<USoundCue>> DinosaurRoars;

    // === SURVIVAL AUDIO ASSETS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Assets")
    TSoftObjectPtr<USoundCue> HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Assets")
    TSoftObjectPtr<USoundCue> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Assets")
    TSoftObjectPtr<USoundCue> HungerSound;

    // === ACTIVE AUDIO TRACKING ===
    
    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    TMap<FString, UAudioComponent*> NamedAudioSources;

    // === TIMER HANDLES ===
    
    FTimerHandle MusicTransitionTimer;
    FTimerHandle AmbientUpdateTimer;

private:
    // === INTERNAL METHODS ===
    
    void InitializeLayerVolumes();
    void UpdateAmbientAudio();
    void CleanupFinishedAudioComponents();
    UAudioComponent* CreateAudioComponent(const FAudio_SourceConfig& Config);
    void ApplyLayerVolumeToComponent(UAudioComponent* Component, EAudio_LayerType LayerType);
};