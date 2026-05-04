#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Audio_EffectsManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EffectType : uint8
{
    Footsteps       UMETA(DisplayName = "Footsteps"),
    Environmental   UMETA(DisplayName = "Environmental"),
    Combat          UMETA(DisplayName = "Combat"),
    UI              UMETA(DisplayName = "UI"),
    Ambient         UMETA(DisplayName = "Ambient")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Pantano         UMETA(DisplayName = "Pantano"),
    Floresta        UMETA(DisplayName = "Floresta"), 
    Savana          UMETA(DisplayName = "Savana"),
    Deserto         UMETA(DisplayName = "Deserto"),
    Montanha        UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    float AttenuationRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effect")
    bool bLoop = false;

    FAudio_EffectData()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        AttenuationRadius = 1000.0f;
        bLoop = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FAudio_EffectData AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_EffectData> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float RandomSoundInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BiomeVolumeMultiplier = 1.0f;

    FAudio_BiomeConfig()
    {
        RandomSoundInterval = 30.0f;
        BiomeVolumeMultiplier = 1.0f;
    }
};

/**
 * Sistema de gestão de efeitos sonoros para o jogo pré-histórico.
 * Gere reprodução contextual de sons baseada em biomas, acções do jogador e eventos do mundo.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_EffectsManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_EffectsManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Reprodução de efeitos sonoros
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    UAudioComponent* PlayEffectAtLocation(EAudio_EffectType EffectType, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    UAudioComponent* PlayEffectAttached(EAudio_EffectType EffectType, USceneComponent* AttachComponent, float VolumeMultiplier = 1.0f);

    // Gestão de áudio por bioma
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    // Configuração de efeitos
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void RegisterEffect(EAudio_EffectType EffectType, const FAudio_EffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void SetEffectTypeVolume(EAudio_EffectType EffectType, float Volume);

    // Efeitos específicos do jogo
    UFUNCTION(BlueprintCallable, Category = "Gameplay Audio")
    void PlayFootstepSound(const FVector& Location, bool bIsRunning = false);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Audio")
    void PlayDinosaurRoar(const FVector& Location, float IntensityLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Gameplay Audio")
    void PlayWeatherEffect(const FString& WeatherType, float Intensity = 1.0f);

protected:
    // Configurações de efeitos por tipo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EAudio_EffectType, FAudio_EffectData> EffectConfigs;

    // Configurações por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TMap<EAudio_BiomeType, FAudio_BiomeConfig> BiomeConfigs;

    // Estado actual
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float MasterVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    TMap<EAudio_EffectType, float> EffectTypeVolumes;

    // Componentes de áudio activos
    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    // Timer para sons aleatórios de bioma
    FTimerHandle BiomeRandomSoundTimer;

private:
    void InitializeDefaultConfigs();
    void InitializeBiomeConfigs();
    void CleanupInactiveComponents();
    void PlayRandomBiomeSound();
    FAudio_EffectData* GetEffectConfig(EAudio_EffectType EffectType);
    float CalculateFinalVolume(EAudio_EffectType EffectType, float BaseVolume) const;
};