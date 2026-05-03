#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Audio_MetaSoundManager.generated.h"

/**
 * Estrutura para configuração de MetaSound por bioma
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeMetaSoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<UMetaSoundSource> AmbientMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BasePitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ReverbWetness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float LowPassFrequency = 8000.0f;

    FAudio_BiomeMetaSoundConfig()
    {
        AmbientMetaSound = nullptr;
        BaseVolume = 0.7f;
        BasePitch = 1.0f;
        ReverbWetness = 0.3f;
        LowPassFrequency = 8000.0f;
    }
};

/**
 * Estrutura para configuração de MetaSound de dinossauros
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurMetaSoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<UMetaSoundSource> FootstepMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<UMetaSoundSource> RoarMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<UMetaSoundSource> BreathingMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RoarVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BreathingVolume = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ThreatMultiplier = 1.0f;

    FAudio_DinosaurMetaSoundConfig()
    {
        FootstepMetaSound = nullptr;
        RoarMetaSound = nullptr;
        BreathingMetaSound = nullptr;
        FootstepVolume = 0.8f;
        RoarVolume = 1.0f;
        BreathingVolume = 0.4f;
        ThreatMultiplier = 1.0f;
    }
};

/**
 * Enum para tipos de eventos de áudio
 */
UENUM(BlueprintType)
enum class EAudio_EventType : uint8
{
    BiomeEnter,
    BiomeExit,
    DinosaurFootstep,
    DinosaurRoar,
    DinosaurBreathing,
    PlayerFootstep,
    PlayerHeartbeat,
    EnvironmentalEffect
};

/**
 * Estrutura para eventos de áudio em tempo real
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_Event
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_EventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    AActor* SourceActor = nullptr;

    FAudio_Event()
    {
        EventType = EAudio_EventType::EnvironmentalEffect;
        Location = FVector::ZeroVector;
        Intensity = 1.0f;
        Duration = 1.0f;
        SourceActor = nullptr;
    }
};

/**
 * Sistema de gestão de MetaSounds para áudio dinâmico e adaptativo
 * Responsável por:
 * - Configuração de MetaSounds por bioma
 * - Gestão de áudio de dinossauros em tempo real
 * - Sistema de eventos de áudio baseado em localização
 * - Integração com sistema de ameaças e sobrevivência
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_MetaSoundManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Gestão de biomas
    UFUNCTION(BlueprintCallable, Category = "Audio|Biome")
    void SetCurrentBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Biome")
    void UpdateBiomeAudio(EBiomeType BiomeType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Biome")
    FAudio_BiomeMetaSoundConfig GetBiomeAudioConfig(EBiomeType BiomeType) const;

    // Gestão de dinossauros
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void RegisterDinosaurActor(AActor* DinosaurActor, EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void UnregisterDinosaurActor(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurFootstep(AActor* DinosaurActor, const FVector& FootstepLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurRoar(AActor* DinosaurActor, float ThreatLevel = 1.0f);

    // Sistema de eventos
    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void TriggerAudioEvent(const FAudio_Event& AudioEvent);

    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void ProcessAudioEvents(float DeltaTime);

    // Configuração master
    UFUNCTION(BlueprintCallable, Category = "Audio|Master")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Master")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Master")
    void SetEffectsVolume(float Volume);

    // Análise de proximidade
    UFUNCTION(BlueprintCallable, Category = "Audio|Analysis")
    float CalculateProximityThreat(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Analysis")
    TArray<AActor*> GetNearbyDinosaurs(const FVector& PlayerLocation, float Radius = 5000.0f) const;

protected:
    // Configurações de bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TMap<EBiomeType, FAudio_BiomeMetaSoundConfig> BiomeConfigs;

    // Configurações de dinossauros
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TMap<EDinosaurSpecies, FAudio_DinosaurMetaSoundConfig> DinosaurConfigs;

    // Estado actual
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    TMap<AActor*, EDinosaurSpecies> RegisteredDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    TArray<FAudio_Event> PendingAudioEvents;

    // Componentes de áudio activos
    UPROPERTY()
    TMap<EBiomeType, UAudioComponent*> BiomeAudioComponents;

    UPROPERTY()
    TMap<AActor*, TArray<UAudioComponent*>> DinosaurAudioComponents;

    // Configurações master
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Master")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Master")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Master")
    float EffectsVolume = 0.9f;

private:
    // Métodos internos
    void InitializeBiomeConfigs();
    void InitializeDinosaurConfigs();
    void CreateBiomeAudioComponent(EBiomeType BiomeType);
    void CreateDinosaurAudioComponents(AActor* DinosaurActor, EDinosaurSpecies Species);
    void UpdateAudioComponentSettings(UAudioComponent* AudioComp, const FAudio_BiomeMetaSoundConfig& Config);
    void CleanupInvalidActors();

    // Timer para limpeza periódica
    FTimerHandle CleanupTimerHandle;
};