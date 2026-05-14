#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../SharedTypes.h"
#include "Audio_MetaSoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    class UMetaSoundSource* MetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    bool bIsActive;

    FAudio_SoundLayer()
    {
        LayerName = TEXT("DefaultLayer");
        MetaSound = nullptr;
        Volume = 1.0f;
        FadeTime = 2.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    EDinosaurSpecies TriggerSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    class UMetaSoundSource* ProximitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    class USoundCue* VoiceWarning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float LastTriggered;

    FAudio_ProximityTrigger()
    {
        TriggerSpecies = EDinosaurSpecies::TRex;
        TriggerDistance = 2000.0f;
        ProximitySound = nullptr;
        VoiceWarning = nullptr;
        CooldownTime = 30.0f;
        LastTriggered = 0.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_MetaSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Environmental Audio Layers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TArray<FAudio_SoundLayer> EnvironmentalLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    class UMetaSoundSource* BaseAmbientMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    class UMetaSoundSource* DayNightTransitionMetaSound;

    // Proximity Warning System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    TArray<FAudio_ProximityTrigger> ProximityTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    class UAudioComponent* ProximityAudioComponent;

    // Dynamic Music System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    class UMetaSoundSource* CombatMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    class UMetaSoundSource* ExplorationMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    class UMetaSoundSource* TensionMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    class UAudioComponent* MusicAudioComponent;

    // Audio State Management
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EGameplayState CurrentGameplayState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    ETimeOfDay CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float MasterVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float EnvironmentalVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float MusicVolume;

public:
    // Environmental Audio Control
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetEnvironmentalLayer(const FString& LayerName, bool bActive, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateTimeOfDayAudio(ETimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateWeatherAudio(EWeatherType WeatherType, float Intensity);

    // Proximity Warning System
    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void CheckProximityTriggers();

    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void TriggerProximityWarning(EDinosaurSpecies Species, float Distance);

    // Dynamic Music Control
    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void TransitionToGameplayState(EGameplayState NewState, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMusicIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void PlayStinger(class USoundCue* StingerSound);

    // Volume Control
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetEnvironmentalVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMusicVolume(float Volume);

    // Audio Event System
    UFUNCTION(BlueprintCallable, Category = "Audio Events")
    void PlayDinosaurCall(EDinosaurSpecies Species, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Events")
    void PlayFootstepAudio(EDinosaurSpecies Species, FVector Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Events")
    void PlayEnvironmentalEvent(const FString& EventName, FVector Location);

private:
    // Internal Audio Management
    void InitializeAudioComponents();
    void UpdateEnvironmentalAudio(float DeltaTime);
    void UpdateDynamicMusic(float DeltaTime);
    void ProcessAudioFades(float DeltaTime);

    // Proximity Detection
    TArray<class AActor*> GetNearbyDinosaurs(float SearchRadius);
    float CalculateProximityIntensity(float Distance, float MaxDistance);

    // Audio Component References
    UPROPERTY()
    class UAudioComponent* EnvironmentalAudioComponent;

    UPROPERTY()
    TArray<class UAudioComponent*> LayerAudioComponents;

    // Fade Management
    TMap<FString, float> LayerFadeTargets;
    TMap<FString, float> LayerFadeSpeeds;

    // Performance Optimization
    float LastProximityCheck;
    float ProximityCheckInterval;
    float LastAudioUpdate;
    float AudioUpdateInterval;
};