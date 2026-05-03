#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Core/SharedTypes.h"
#include "AudioManager.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - AUDIO MANAGER
 * Audio Agent #16
 * 
 * Sistema central de gestão de áudio para o jogo pré-histórico.
 * Gere música adaptativa, efeitos sonoros ambientais e audio espacial 3D.
 * Integra-se com MetaSounds do UE5 para reverberação dinâmica por bioma.
 */

UENUM(BlueprintType)
enum class EAudio_BiomeAmbience : uint8
{
    Swamp = 0       UMETA(DisplayName = "Pantano"),
    Forest = 1      UMETA(DisplayName = "Floresta"), 
    Savanna = 2     UMETA(DisplayName = "Savana"),
    Desert = 3      UMETA(DisplayName = "Deserto"),
    Mountain = 4    UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe = 0        UMETA(DisplayName = "Seguro"),
    Caution = 1     UMETA(DisplayName = "Cuidado"),
    Danger = 2      UMETA(DisplayName = "Perigo"),
    Critical = 3    UMETA(DisplayName = "Crítico")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeAmbience BiomeType = EAudio_BiomeAmbience::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbienceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float ReverbWetness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float ReverbDecayTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float WindIntensity = 0.5f;

    FAudio_BiomeSettings()
    {
        BiomeType = EAudio_BiomeAmbience::Savanna;
        ReverbWetness = 0.3f;
        ReverbDecayTime = 2.0f;
        WindIntensity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatAudio
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    TSoftObjectPtr<USoundCue> ThreatMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float HeartbeatIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float MusicVolume = 0.7f;

    FAudio_ThreatAudio()
    {
        ThreatLevel = EAudio_ThreatLevel::Safe;
        HeartbeatIntensity = 0.0f;
        MusicVolume = 0.7f;
    }
};

/**
 * Subsistema de gestão de áudio global do jogo
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ═══════════════════════════════════════════════════════════════
    // BIOME AUDIO MANAGEMENT
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetCurrentBiome(EAudio_BiomeAmbience NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    EAudio_BiomeAmbience GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateBiomeAmbience(const FVector& PlayerLocation);

    // ═══════════════════════════════════════════════════════════════
    // THREAT LEVEL AUDIO
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    EAudio_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurFootsteps(const FVector& DinosaurLocation, float DinosaurMass);

    // ═══════════════════════════════════════════════════════════════
    // SPATIAL AUDIO
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlaySpatialSound(USoundCue* Sound, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetListenerLocation(const FVector& Location);

    // ═══════════════════════════════════════════════════════════════
    // MASTER VOLUME CONTROLS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetAmbienceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetSFXVolume(float Volume);

protected:
    // ═══════════════════════════════════════════════════════════════
    // INTERNAL STATE
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeAmbience CurrentBiome = EAudio_BiomeAmbience::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio State")
    FVector ListenerLocation = FVector::ZeroVector;

    // ═══════════════════════════════════════════════════════════════
    // AUDIO COMPONENTS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbienceAudioComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* HeartbeatAudioComponent = nullptr;

    // ═══════════════════════════════════════════════════════════════
    // BIOME CONFIGURATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Configuration")
    TArray<FAudio_BiomeSettings> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threat Configuration")
    TArray<FAudio_ThreatAudio> ThreatAudioSettings;

    // ═══════════════════════════════════════════════════════════════
    // VOLUME SETTINGS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbienceVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SFXVolume = 0.8f;

private:
    // ═══════════════════════════════════════════════════════════════
    // INTERNAL METHODS
    // ═══════════════════════════════════════════════════════════════

    void InitializeBiomeSettings();
    void InitializeThreatSettings();
    void TransitionToNewBiome(EAudio_BiomeAmbience NewBiome);
    void UpdateReverbSettings(const FAudio_BiomeSettings& Settings);
    EAudio_BiomeAmbience DetermineBiomeFromLocation(const FVector& Location);
};