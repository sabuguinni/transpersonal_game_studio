#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "AudioSystemManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe — No Predators"),
    Aware       UMETA(DisplayName = "Aware — Predator Detected"),
    Stalked     UMETA(DisplayName = "Stalked — Predator Approaching"),
    Imminent    UMETA(DisplayName = "Imminent — Predator Attacking"),
    Combat      UMETA(DisplayName = "Combat — Active Fight")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    DenseJungle UMETA(DisplayName = "Dense Jungle"),
    OpenPlains  UMETA(DisplayName = "Open Plains"),
    RiverDelta  UMETA(DisplayName = "River Delta"),
    Volcanic    UMETA(DisplayName = "Volcanic Region"),
    Cave        UMETA(DisplayName = "Cave / Underground")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundBase* IdleVocalisation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundBase* AlertRoar = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundBase* AttackRoar = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundBase* FootstepHeavy = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundBase* DeathSound = nullptr;

    // Distance at which footsteps cause camera shake (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float ShakeRadius = 2000.0f;

    // Shake intensity multiplier (1.0 = full shake at ShakeRadius)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float ShakeIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* AmbientLoop = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 2.0f;
};

USTRUCT(BlueprintType)
struct FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* MusicTrack = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float CrossfadeDuration = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float Volume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    bool bLooping = true;
};

// ─── Main Class ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Threat System ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintPure, Category = "Audio|Threat")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // ─── Ambient System ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void UpdateAmbientLayers();

    // ─── Dinosaur Audio ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurFootstep(FName Species, FVector Location, float Mass);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurRoar(FName Species, FVector Location, bool bIsAlert);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerProximityShake(FVector DinosaurLocation, float DinosaurMass);

    // ─── Player Feedback ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void PlayFootstep(bool bIsRunning, FName SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void TriggerDamageFlashAudio(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|Player")
    void PlayCraftingSound(FName ItemCrafted);

    // ─── Music System ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void CrossfadeToMusicState(EAudio_ThreatLevel TargetState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StopAllMusic(float FadeTime = 2.0f);

    // ─── Day/Night Audio Cycle ────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void UpdateDayNightAudio(float TimeOfDayNormalized);

    // ─── Screen Shake ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerScreenShake(float Intensity, float Duration, FVector SourceLocation);

    // ─── Properties ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_MusicState> MusicStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.8f;

    // Minimum time between threat level changes (prevents rapid flickering)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float ThreatTransitionCooldown = 3.0f;

    // Distance threshold for T-Rex proximity shake (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float TRexShakeDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TSubclassOf<UCameraShakeBase> LightShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TSubclassOf<UCameraShakeBase> HeavyShakeClass;

private:
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    EAudio_BiomeType   CurrentBiome       = EAudio_BiomeType::DenseJungle;
    EAudio_TimeOfDay   CurrentTimeOfDay   = EAudio_TimeOfDay::Day;

    UPROPERTY()
    UAudioComponent* ActiveMusicComponent = nullptr;

    UPROPERTY()
    UAudioComponent* ActiveAmbientComponent = nullptr;

    float TimeSinceLastThreatChange = 0.0f;
    float LastDayNightNormalized    = 0.5f;

    void CrossfadeAudioComponent(UAudioComponent*& Component, USoundBase* NewSound,
        float FadeOut, float FadeIn, float Volume);

    FAudio_DinosaurSoundProfile* FindDinosaurProfile(FName Species);
    FAudio_MusicState*           FindMusicState(EAudio_ThreatLevel Level);
    FAudio_AmbientLayer*         FindAmbientLayer(EAudio_BiomeType Biome, EAudio_TimeOfDay Time);

    EAudio_TimeOfDay NormalizedTimeToEnum(float Normalized) const;
};
