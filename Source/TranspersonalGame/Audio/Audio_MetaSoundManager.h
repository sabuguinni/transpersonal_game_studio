#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundLayer : uint8
{
    Ambient = 0,
    Music = 1,
    SFX = 2,
    Dialogue = 3,
    UI = 4
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest = 0,
    Plains = 1,
    Desert = 2,
    Swamp = 3,
    Mountains = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    float Pitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    bool bIsPlaying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    EAudio_SoundLayer Layer;

    FAudio_SoundState()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        bIsPlaying = false;
        Layer = EAudio_SoundLayer::Ambient;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<TSoftObjectPtr<USoundCue>> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float RandomSoundChance;

    FAudio_BiomeAudioData()
    {
        AmbientVolume = 0.7f;
        RandomSoundChance = 0.1f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_MetaSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core audio management
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlaySound(USoundCue* SoundCue, EAudio_SoundLayer Layer, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopSound(EAudio_SoundLayer Layer);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetLayerVolume(EAudio_SoundLayer Layer, float Volume);

    // Biome-specific audio
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EAudio_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdateBiomeAudio(float DeltaTime);

    // Narrative integration
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayDialogue(USoundCue* DialogueCue, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarration(USoundCue* NarrationCue, float Volume = 0.8f);

    // Survival audio cues
    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void PlayDangerWarning(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void PlayHeartbeat(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void UpdateSurvivalAudio(float Health, float Hunger, float Thirst, float Fear);

protected:
    // Audio components for different layers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TMap<EAudio_SoundLayer, UAudioComponent*> LayerComponents;

    // Current audio state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    TMap<EAudio_SoundLayer, FAudio_SoundState> SoundStates;

    // Biome audio data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioData> BiomeAudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeType CurrentBiome;

    // Survival audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundCue> HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundCue> DangerWarningSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    float HeartbeatBaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    float DangerWarningVolume;

    // Internal state
    float RandomSoundTimer;
    float HeartbeatTimer;
    bool bIsInDanger;

private:
    void InitializeAudioComponents();
    void UpdateRandomSounds(float DeltaTime);
    void UpdateHeartbeatAudio(float DeltaTime, float FearLevel);
    UAudioComponent* GetOrCreateAudioComponent(EAudio_SoundLayer Layer);
};