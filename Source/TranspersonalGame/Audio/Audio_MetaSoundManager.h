#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Engine/World.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Tense       UMETA(DisplayName = "Tense"), 
    Combat      UMETA(DisplayName = "Combat"),
    Exploration UMETA(DisplayName = "Exploration"),
    Danger      UMETA(DisplayName = "Danger")
};

UENUM(BlueprintType)
enum class EAudio_AmbientZone : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Plains      UMETA(DisplayName = "Plains"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIsLooping = true;

    FAudio_SoundLayer()
    {
        SoundAsset = nullptr;
        Volume = 1.0f;
        FadeTime = 2.0f;
        bIsLooping = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_MetaSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_MetaSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SFXAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    TMap<EAudio_MusicState, FAudio_SoundLayer> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient System")
    TMap<EAudio_AmbientZone, FAudio_SoundLayer> AmbientLayers;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_AmbientZone CurrentAmbientZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 1.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Ambient Control")
    void SetAmbientZone(EAudio_AmbientZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlaySFX(class USoundBase* SoundEffect, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void FadeToMusicState(EAudio_MusicState NewState, float FadeTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void FadeToAmbientZone(EAudio_AmbientZone NewZone, float FadeTime = 2.0f);

private:
    void InitializeAudioComponents();
    void LoadDefaultSoundLayers();
    void UpdateVolumeSettings();

    FTimerHandle MusicFadeTimer;
    FTimerHandle AmbientFadeTimer;
    
    bool bIsFadingMusic = false;
    bool bIsFadingAmbient = false;
    
    float MusicFadeStartVolume = 0.0f;
    float MusicFadeTargetVolume = 0.0f;
    float MusicFadeCurrentTime = 0.0f;
    float MusicFadeDuration = 0.0f;
    
    float AmbientFadeStartVolume = 0.0f;
    float AmbientFadeTargetVolume = 0.0f;
    float AmbientFadeCurrentTime = 0.0f;
    float AmbientFadeDuration = 0.0f;

    void ProcessMusicFade();
    void ProcessAmbientFade();
};