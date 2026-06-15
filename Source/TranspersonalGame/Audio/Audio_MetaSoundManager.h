#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Sound/SoundCue.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AmbienceType : uint8
{
    Forest UMETA(DisplayName = "Forest"),
    Valley UMETA(DisplayName = "Valley"),
    River UMETA(DisplayName = "River"),
    Cave UMETA(DisplayName = "Cave"),
    Plains UMETA(DisplayName = "Plains")
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Calm UMETA(DisplayName = "Calm"),
    Tense UMETA(DisplayName = "Tense"),
    Danger UMETA(DisplayName = "Danger"),
    Combat UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbienceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    EAudio_AmbienceType AmbienceType = EAudio_AmbienceType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    float Volume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    bool bLooping = true;

    FAudio_AmbienceSettings()
    {
        AmbienceType = EAudio_AmbienceType::Forest;
        Volume = 0.5f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bLooping = true;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SFXAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_AmbienceSettings CurrentAmbienceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<EAudio_AmbienceType, class USoundBase*> AmbienceSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<EAudio_IntensityLevel, class USoundBase*> MusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    class UMetaSoundSource* DynamicAmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    class UMetaSoundSource* AdaptiveMusicMetaSound;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetAmbienceType(EAudio_AmbienceType NewAmbienceType);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMusicIntensity(EAudio_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlaySFX(class USoundBase* SFXSound, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void FadeToAmbience(EAudio_AmbienceType NewAmbienceType, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void StopAllAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "MetaSound Control")
    void UpdateMetaSoundParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound Control")
    void TriggerMetaSoundEvent(const FString& EventName);

protected:
    UFUNCTION()
    void OnAmbienceFadeComplete();

    UFUNCTION()
    void OnMusicFadeComplete();

private:
    float CurrentMasterVolume = 1.0f;
    EAudio_IntensityLevel CurrentMusicIntensity = EAudio_IntensityLevel::Calm;
    bool bIsFadingAmbience = false;
    bool bIsFadingMusic = false;

    void InitializeAudioComponents();
    void LoadDefaultAudioAssets();
    void UpdateDynamicParameters();
};