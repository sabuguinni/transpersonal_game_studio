#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "../Narrative/Narr_EmotionalResponseSystem.h"
#include "../SharedTypes.h"
#include "Audio_AdaptiveAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Ambient     UMETA(DisplayName = "Ambient"),
    Tension     UMETA(DisplayName = "Tension"),
    Combat      UMETA(DisplayName = "Combat"),
    Exploration UMETA(DisplayName = "Exploration"),
    Calm        UMETA(DisplayName = "Calm")
};

UENUM(BlueprintType)
enum class EAudio_EnvironmentType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Desert      UMETA(DisplayName = "Desert")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EAudio_MusicLayer CurrentLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    float IntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EAudio_EnvironmentType EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    bool bInDanger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    float EmotionalIntensity;

    FAudio_MusicState()
    {
        CurrentLayer = EAudio_MusicLayer::Ambient;
        IntensityLevel = 0.5f;
        EnvironmentType = EAudio_EnvironmentType::Forest;
        bInDanger = false;
        EmotionalIntensity = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AdaptiveAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AdaptiveAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* EffectsAudioComponent;

    // Music State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    FAudio_MusicState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    float TransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    float MaxAudioDistance;

    // Sound Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Assets")
    TMap<EAudio_MusicLayer, class USoundCue*> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Assets")
    TMap<EAudio_EnvironmentType, class USoundCue*> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Assets")
    TArray<class USoundCue*> DangerStingers;

public:
    // Music Control Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMusicLayer(EAudio_MusicLayer NewLayer, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetEnvironmentType(EAudio_EnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetDangerState(bool bDangerous, float IntensityLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateEmotionalState(ENarr_EmotionalState EmotionalState, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDangerStinger();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateMusicIntensity(float NewIntensity);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void TriggerEnvironmentalAudio(FVector Location, float Radius, class USoundCue* SoundToPlay);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetGlobalAudioVolume(float VolumeMultiplier);

protected:
    // Internal Functions
    void UpdateAudioBasedOnGameState();
    void HandleMusicTransition(float DeltaTime);
    void UpdateAmbientAudio();
    float CalculateDistanceToPlayer();
    bool IsPlayerInDanger();

private:
    float CurrentVolumeMultiplier;
    float TargetMusicVolume;
    float CurrentMusicVolume;
    bool bIsTransitioning;
    EAudio_MusicLayer TargetMusicLayer;
};