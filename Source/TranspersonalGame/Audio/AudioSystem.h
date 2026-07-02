#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "AudioSystem.generated.h"

// ============================================================
// ENUMS — Audio_* prefix to avoid cross-agent collisions
// ============================================================

UENUM(BlueprintType)
enum class EAudio_SurfaceType : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Rock        UMETA(DisplayName = "Rock"),
    Water       UMETA(DisplayName = "Water"),
    Grass       UMETA(DisplayName = "Grass"),
    Mud         UMETA(DisplayName = "Mud"),
    Gravel      UMETA(DisplayName = "Gravel")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),       // No predator nearby
    Aware       UMETA(DisplayName = "Aware"),      // Predator 80-150m
    Tense       UMETA(DisplayName = "Tense"),      // Predator 40-80m
    Critical    UMETA(DisplayName = "Critical")    // Predator <40m
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_FootstepSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    EAudio_SurfaceType SurfaceType = EAudio_SurfaceType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    TArray<USoundBase*> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    float PitchVariationRange = 0.1f;
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float TargetVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeSpeed = 2.0f;

    // Which danger levels activate this layer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<EAudio_DangerLevel> ActiveOnDangerLevels;

    // Which times of day activate this layer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<EAudio_TimeOfDay> ActiveOnTimeOfDay;
};

USTRUCT(BlueprintType)
struct FAudio_SoundCueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Registry")
    FName CueName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Registry")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Registry")
    float DefaultVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Registry")
    float DefaultPitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Registry")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Registry")
    float MaxDistance = 2000.0f;
};

// ============================================================
// COMPONENT — Footstep System (attach to TranspersonalCharacter)
// ============================================================

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Audio Footstep Component")
class TRANSPERSONALGAME_API UAudio_FootstepComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_FootstepComponent();

    // Called from AnimNotify_Footstep in character animation
    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    void PlayFootstep(EAudio_SurfaceType Surface, float VolumeScale = 1.0f);

    // Detect surface type under the character using line trace
    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    EAudio_SurfaceType DetectSurfaceUnderFoot() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    TArray<FAudio_FootstepSet> FootstepSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    float FootstepVolumeWalk = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    float FootstepVolumeRun = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    bool bIsRunning = false;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    UAudioComponent* ActiveFootstepAudio = nullptr;
};

// ============================================================
// COMPONENT — Adaptive Ambient System
// ============================================================

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Audio Adaptive Ambient Component")
class TRANSPERSONALGAME_API UAudio_AdaptiveAmbientComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveAmbientComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Called by DinosaurAI or GameState when predator proximity changes
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintPure, Category = "Audio|Ambient")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    // Proximity thresholds for automatic danger level detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float DangerRadius_Aware = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float DangerRadius_Tense = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float DangerRadius_Critical = 40.0f;

    // How often to scan for nearby predators (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float ProximityScanInterval = 2.0f;

private:
    UPROPERTY()
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY()
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY()
    TArray<UAudioComponent*> LayerAudioComponents;

    float TimeSinceLastScan = 0.0f;

    void ScanForNearbyPredators();
    void UpdateLayerVolumes(float DeltaTime);
    void InitializeAudioComponents();
};

// ============================================================
// ACTOR — Audio Manager (singleton, placed in level)
// ============================================================

UCLASS(BlueprintType, DisplayName = "Audio Manager")
class TRANSPERSONALGAME_API AAudio_Manager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_Manager();

    virtual void BeginPlay() override;

    // Global sound cue registry — play any registered sound by name
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void PlaySoundCue(FName CueName, FVector Location, float VolumeScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void PlaySoundCue2D(FName CueName, float VolumeScale = 1.0f);

    // Register a voice line URL (from ElevenLabs TTS) to a cue name
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void RegisterSoundCue(FAudio_SoundCueEntry Entry);

    // Screen shake when T-Rex walks nearby
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void TriggerTRexProximityShake(float Intensity, FVector TRexLocation);

    // Damage audio feedback — plays on player hit
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void PlayDamageAudioFeedback(float DamageAmount);

    // Campfire ambient — toggle
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetCampfireAmbientActive(bool bActive, FVector CampfireLocation);

    // River ambient — toggle
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetRiverAmbientActive(bool bActive, FVector RiverLocation);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Registry")
    TArray<FAudio_SoundCueEntry> SoundRegistry;

    // Voice line assets — assigned from TTS imports
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceLines")
    USoundBase* VoiceLine_NarratorHerdSilence = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceLines")
    USoundBase* VoiceLine_NarratorBleedingWarning = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceLines")
    USoundBase* VoiceLine_NarratorTRexProximity = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceLines")
    USoundBase* VoiceLine_ChiefHunterDawn = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceLines")
    USoundBase* VoiceLine_TribalElderSpino = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceLines")
    USoundBase* VoiceLine_ScoutRunnerRiver = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceLines")
    USoundBase* VoiceLine_CampBuilderMemorial = nullptr;

    // Freesound references (campfire, river, roar)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* AmbientSound_Campfire = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* AmbientSound_River = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* AmbientSound_DinosaurRoar = nullptr;

protected:
    UPROPERTY()
    UAudio_AdaptiveAmbientComponent* AdaptiveAmbient = nullptr;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAmbientComponents;

private:
    FAudio_SoundCueEntry* FindCueByName(FName CueName);
};
