#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Audio_DynamicSoundscape.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundscapeZone : uint8
{
    OpenValley     UMETA(DisplayName = "Open Valley"),
    DenseForest    UMETA(DisplayName = "Dense Forest"),
    RiverSide      UMETA(DisplayName = "River Side"),
    CaveEntrance   UMETA(DisplayName = "Cave Entrance"),
    PredatorTerritory UMETA(DisplayName = "Predator Territory"),
    SafeZone       UMETA(DisplayName = "Safe Zone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundscapeLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float PitchVariation = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape")
    float FadeOutTime = 3.0f;

    FAudio_SoundscapeLayer()
    {
        SoundCue = nullptr;
        BaseVolume = 0.5f;
        PitchVariation = 0.1f;
        bLooping = true;
        FadeInTime = 2.0f;
        FadeOutTime = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ZoneConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Config")
    EAudio_SoundscapeZone ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Config")
    TArray<FAudio_SoundscapeLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Config")
    TArray<FAudio_SoundscapeLayer> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Config")
    float TransitionRadius = 1000.0f;

    FAudio_ZoneConfiguration()
    {
        ZoneType = EAudio_SoundscapeZone::OpenValley;
        DangerLevel = 0.0f;
        TransitionRadius = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_DynamicSoundscape : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DynamicSoundscape();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core soundscape management
    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void SetCurrentZone(EAudio_SoundscapeZone NewZone, bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void UpdateDangerLevel(float NewDangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void TriggerStoryMoment(const FString& StoryEventName, float IntensityLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void SetTimeOfDay(float TimeRatio); // 0.0 = dawn, 0.25 = noon, 0.5 = dusk, 0.75 = midnight

    // Zone detection and transitions
    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    EAudio_SoundscapeZone DetectPlayerZone(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void RegisterZoneConfiguration(const FAudio_ZoneConfiguration& ZoneConfig);

    // Audio layer management
    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void FadeInLayer(int32 LayerIndex, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void FadeOutLayer(int32 LayerIndex, float FadeTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void CrossfadeToZone(EAudio_SoundscapeZone TargetZone, float TransitionTime = 5.0f);

    // Story integration
    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void PlayNarrationClip(const FString& CharacterName, const FString& AudioFilePath);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Soundscape")
    void StopNarration(bool bFadeOut = true);

protected:
    // Audio components for layered soundscape
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> AmbientAudioComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> MusicAudioComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* NarrationAudioComponent;

    // Zone configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soundscape Config")
    TMap<EAudio_SoundscapeZone, FAudio_ZoneConfiguration> ZoneConfigurations;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_SoundscapeZone CurrentZone;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_SoundscapeZone TargetZone;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentDangerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float TimeOfDayRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bInTransition;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float TransitionProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float TransitionDuration;

    // Audio file paths for generated narration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generated Audio")
    TMap<FString, FString> NarrationAudioPaths;

private:
    void InitializeAudioComponents();
    void UpdateZoneTransition(float DeltaTime);
    void ApplyDangerLevelModulation();
    void ApplyTimeOfDayModulation();
    void LoadDefaultZoneConfigurations();
    void UpdateAudioComponentVolumes();

    // Transition helpers
    float CalculateLayerVolume(const FAudio_SoundscapeLayer& Layer, float ZoneBlend, float DangerMod, float TimeMod);
    void StartZoneTransition(EAudio_SoundscapeZone FromZone, EAudio_SoundscapeZone ToZone, float Duration);
    void CompleteZoneTransition();
};