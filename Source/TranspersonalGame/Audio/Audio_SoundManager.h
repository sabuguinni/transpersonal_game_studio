#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Pântano"),
    Forest      UMETA(DisplayName = "Floresta"),
    Savanna     UMETA(DisplayName = "Savana"),
    Desert      UMETA(DisplayName = "Deserto"),
    Mountain    UMETA(DisplayName = "Montanha Nevada")
};

UENUM(BlueprintType)
enum class EAudio_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Céu Limpo"),
    Cloudy      UMETA(DisplayName = "Nublado"),
    Rain        UMETA(DisplayName = "Chuva"),
    Storm       UMETA(DisplayName = "Tempestade"),
    Fog         UMETA(DisplayName = "Nevoeiro")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Amanhecer"),
    Morning     UMETA(DisplayName = "Manhã"),
    Noon        UMETA(DisplayName = "Meio-dia"),
    Afternoon   UMETA(DisplayName = "Tarde"),
    Dusk        UMETA(DisplayName = "Entardecer"),
    Night       UMETA(DisplayName = "Noite")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbientProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    EAudio_WeatherType WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    EAudio_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float WindIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    TSoftObjectPtr<USoundCue> MusicTrack;

    FAudio_AmbientProfile()
    {
        BiomeType = EAudio_BiomeType::Savanna;
        WeatherType = EAudio_WeatherType::Clear;
        TimeOfDay = EAudio_TimeOfDay::Morning;
        BaseVolume = 0.7f;
        WindIntensity = 0.3f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SoundManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateAmbientProfile(EAudio_BiomeType NewBiome, EAudio_WeatherType NewWeather, EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayFootstepSound(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void FadeToNewAmbient(const FAudio_AmbientProfile& NewProfile, float FadeTime = 2.0f);

    UFUNCTION(BlueprintPure, Category = "Audio Manager")
    FAudio_AmbientProfile GetCurrentProfile() const { return CurrentProfile; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudio_BiomeType, FAudio_AmbientProfile> BiomeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudioDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudio_AmbientProfile CurrentProfile;

    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* MusicAudioComponent;

private:
    void InitializeBiomeProfiles();
    void UpdateAmbientAudio();
    float CalculateVolumeByDistance(const FVector& SoundLocation, const FVector& ListenerLocation) const;
};