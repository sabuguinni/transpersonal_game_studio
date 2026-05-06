#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Audio_AudioManager.generated.h"

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
enum class EAudio_AlertLevel : uint8
{
    Calm        UMETA(DisplayName = "Calmo"),
    Caution     UMETA(DisplayName = "Precaução"),
    Danger      UMETA(DisplayName = "Perigo"),
    Critical    UMETA(DisplayName = "Crítico")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TemperatureModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float HumidityModifier = 1.0f;

    FAudio_BiomeProfile()
    {
        BiomeType = EAudio_BiomeType::Savanna;
        BaseVolume = 0.7f;
        TemperatureModifier = 1.0f;
        HumidityModifier = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityAlert
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString CreatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_AlertLevel AlertLevel = EAudio_AlertLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    FAudio_ProximityAlert()
    {
        CreatureName = TEXT("Unknown");
        Distance = 0.0f;
        AlertLevel = EAudio_AlertLevel::Calm;
        Volume = 1.0f;
    }
};

/**
 * Sistema de áudio adaptativo para o mundo pré-histórico
 * Gere música dinâmica, efeitos ambientais e alertas de proximidade
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_AudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_AudioManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Audio system management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioSystem(float DeltaTime);

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdateBiomeAudio(float Temperature, float Humidity);

    UFUNCTION(BlueprintPure, Category = "Biome Audio")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    // Proximity alert system
    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void TriggerProximityAlert(const FString& CreatureName, float Distance, EAudio_AlertLevel AlertLevel);

    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void ClearProximityAlert();

    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    bool IsProximityAlertActive() const { return bProximityAlertActive; }

    // TTS integration
    UFUNCTION(BlueprintCallable, Category = "TTS Audio")
    void PlayTTSNarration(const FString& NarrationText, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "TTS Audio")
    void PlaySystemAlert(const FString& AlertText, EAudio_AlertLevel AlertLevel);

    // Audio component management
    UFUNCTION(BlueprintCallable, Category = "Audio Components")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Components")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Components")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Components")
    void SetAlertVolume(float Volume);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeatherAudio(float WindStrength, float RainIntensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateTimeOfDayAudio(float TimeOfDay);

protected:
    // Current biome state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTemperature;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentHumidity;

    // Biome profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Profiles")
    TMap<EAudio_BiomeType, FAudio_BiomeProfile> BiomeProfiles;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AlertAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> TTSAudioComponent;

    // Proximity alert state
    UPROPERTY(BlueprintReadOnly, Category = "Proximity State")
    bool bProximityAlertActive;

    UPROPERTY(BlueprintReadOnly, Category = "Proximity State")
    FAudio_ProximityAlert CurrentProximityAlert;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float AlertVolume;

    // Environmental state
    UPROPERTY(BlueprintReadOnly, Category = "Environmental State")
    float WindStrength;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental State")
    float RainIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental State")
    float TimeOfDay;

private:
    // Internal methods
    void InitializeBiomeProfiles();
    void InitializeAudioComponents();
    void UpdateBiomeTransition(float DeltaTime);
    void UpdateProximityAlertAudio(float DeltaTime);
    void CalculateVolumeModifiers();

    // Transition state
    bool bTransitioningBiome;
    float BiomeTransitionTime;
    EAudio_BiomeType TargetBiome;
};