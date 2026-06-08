#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AmbienceType : uint8
{
    Forest UMETA(DisplayName = "Forest Ambience"),
    Danger UMETA(DisplayName = "Danger Zone"),
    Peaceful UMETA(DisplayName = "Peaceful Area"),
    Storm UMETA(DisplayName = "Storm Weather"),
    Night UMETA(DisplayName = "Night Time")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe UMETA(DisplayName = "Safe"),
    Caution UMETA(DisplayName = "Caution"),
    Danger UMETA(DisplayName = "Danger"),
    Extreme UMETA(DisplayName = "Extreme Danger")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    TSoftObjectPtr<UMetaSoundSource> MetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    bool bIsLooping = true;

    FAudio_SoundLayer()
    {
        Volume = 1.0f;
        FadeTime = 2.0f;
        bIsLooping = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbienceConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    EAudio_AmbienceType AmbienceType = EAudio_AmbienceType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    TArray<FAudio_SoundLayer> SoundLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    float CrossfadeTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    float MaxDistance = 5000.0f;

    FAudio_AmbienceConfig()
    {
        AmbienceType = EAudio_AmbienceType::Forest;
        CrossfadeTime = 3.0f;
        MaxDistance = 5000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_MetaSoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetAmbienceType(EAudio_AmbienceType NewAmbienceType, float FadeTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetThreatLevel(EAudio_ThreatLevel ThreatLevel, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayNarrationLine(const FString& NarrationKey, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetAmbienceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetVoiceVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "Audio Manager")
    EAudio_AmbienceType GetCurrentAmbienceType() const { return CurrentAmbienceType; }

    UFUNCTION(BlueprintPure, Category = "Audio Manager")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Config")
    TMap<EAudio_AmbienceType, FAudio_AmbienceConfig> AmbienceConfigs;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Config")
    TMap<FString, TSoftObjectPtr<UMetaSoundSource>> DinosaurSounds;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Config")
    TMap<FString, TSoftObjectPtr<USoundBase>> NarrationSounds;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_AmbienceType CurrentAmbienceType;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> AmbienceComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* NarrationComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Settings")
    float AmbienceVolume = 0.8f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Settings")
    float VoiceVolume = 1.0f;

private:
    void InitializeAmbienceConfigs();
    void InitializeDinosaurSounds();
    void InitializeNarrationSounds();
    void CrossfadeToNewAmbience(EAudio_AmbienceType NewType, float FadeTime);
    void UpdateThreatLevelMix(EAudio_ThreatLevel ThreatLevel, float FadeTime);
    UAudioComponent* CreateAudioComponent();
};

#include "Audio_MetaSoundManager.generated.h"