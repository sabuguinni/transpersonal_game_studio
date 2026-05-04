#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Audio_AdaptiveAudioComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    class USoundBase* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    class USoundBase* MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeOutTime;

    FAudio_BiomeAudioData()
    {
        AmbientSound = nullptr;
        MusicTrack = nullptr;
        AmbientVolume = 0.7f;
        MusicVolume = 0.5f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    class USoundBase* ThreatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float ThreatVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    bool bLooping;

    FAudio_ThreatAudioData()
    {
        ThreatSound = nullptr;
        ThreatVolume = 0.8f;
        TriggerDistance = 1000.0f;
        bLooping = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AdaptiveAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveAudioComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configurações de áudio por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Audio")
    TMap<EBiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    // Configurações de áudio por threat level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Audio")
    TMap<EThreatLevel, FAudio_ThreatAudioData> ThreatAudioMap;

    // Volume master
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume;

    // Distância máxima para detecção de bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BiomeDetectionRadius;

    // Tempo de actualização do sistema
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float UpdateInterval;

    // Componentes de áudio
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* ThreatAudioComponent;

    // Estado actual
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EBiomeType CurrentBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EThreatLevel CurrentThreatLevel;

    // Métodos públicos
    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void SetBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void SetThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void PlayOneShot(class USoundBase* Sound, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void StopAllAudio();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void SetMasterVolume(float NewVolume);

private:
    // Métodos internos
    void UpdateAudioBasedOnLocation();
    void UpdateThreatAudio();
    EBiomeType DetectBiomeAtLocation(const FVector& Location);
    EThreatLevel DetectThreatLevel();
    void TransitionToBiomeAudio(EBiomeType NewBiome);
    void TransitionToThreatAudio(EThreatLevel NewThreatLevel);
    void InitializeAudioComponents();
    void SetupDefaultAudioData();

    // Timer handles
    FTimerHandle UpdateTimerHandle;

    // Estado de transição
    bool bIsTransitioning;
    float TransitionTimer;
    float TransitionDuration;
};