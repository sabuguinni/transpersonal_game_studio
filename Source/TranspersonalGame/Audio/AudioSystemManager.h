// AudioSystemManager.h
// Agent #16 — Audio Agent — PROD_CYCLE_AUTO_20260628_007
// Adaptive audio system: ambient layers, proximity triggers, survival state reactions
// NO spiritual/mystical content — pure survival/nature audio

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_AmbientLayer : uint8
{
    None            UMETA(DisplayName = "None"),
    DayForest       UMETA(DisplayName = "Day Forest"),
    NightJungle     UMETA(DisplayName = "Night Jungle"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    CaveInterior    UMETA(DisplayName = "Cave Interior"),
    StormApproach   UMETA(DisplayName = "Storm Approaching"),
    StormActive     UMETA(DisplayName = "Storm Active"),
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Uneasy      UMETA(DisplayName = "Uneasy"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat"),
    Flee        UMETA(DisplayName = "Flee"),
};

UENUM(BlueprintType)
enum class EAudio_SurvivalState : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Starving    UMETA(DisplayName = "Starving"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured"),
    Critical    UMETA(DisplayName = "Critical"),
};

UENUM(BlueprintType)
enum class EAudio_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Generic         UMETA(DisplayName = "Generic"),
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_AmbientLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_AmbientLayer LayerType = EAudio_AmbientLayer::DayForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* LoopSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLoops = true;
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    EAudio_DinoSpecies Species = EAudio_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<USoundBase*> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<USoundBase*> AlertSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<USoundBase*> AttackSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<USoundBase*> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<USoundBase*> DeathSounds;

    // Distance at which footsteps cause screen shake
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float ScreenShakeRadius = 1200.0f;

    // Volume multiplier based on distance (applied at runtime)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float MaxHearingDistance = 4000.0f;
};

USTRUCT(BlueprintType)
struct FAudio_SurvivalCue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    EAudio_SurvivalState State = EAudio_SurvivalState::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    USoundBase* StingerSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    float TriggerThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    float RepeatCooldown = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Survival")
    float LastPlayedTime = -999.0f;
};

USTRUCT(BlueprintType)
struct FAudio_FreesoundAsset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    int32 FreesoundID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    FString PreviewURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    FString Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    FString LocalAssetPath;
};

// ─── Component ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
    DisplayName = "Audio System Manager")
class TRANSPERSONALGAME_API UAudio_SystemManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SystemManagerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ── Ambient Layer Control ──────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetAmbientLayer(EAudio_AmbientLayer NewLayer);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void CrossfadeToLayer(EAudio_AmbientLayer NewLayer, float CrossfadeTime = 2.0f);

    UFUNCTION(BlueprintPure, Category = "Audio|Ambient")
    EAudio_AmbientLayer GetCurrentAmbientLayer() const { return CurrentAmbientLayer; }

    // ── Threat Level Music ─────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void NotifyDinoProximity(EAudio_DinoSpecies Species, float Distance);

    UFUNCTION(BlueprintPure, Category = "Audio|Threat")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // ── Dinosaur Audio ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinoSound(EAudio_DinoSpecies Species, FVector Location, bool bIsAttack = false);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinoFootstep(EAudio_DinoSpecies Species, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    FAudio_DinoSoundSet GetDinoSoundSet(EAudio_DinoSpecies Species) const;

    // ── Survival State Audio ───────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void UpdateSurvivalState(float Health, float Hunger, float Thirst, float Stamina);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlaySurvivalStinger(EAudio_SurvivalState State);

    // ── Screen Shake ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerFootstepShake(EAudio_DinoSpecies Species, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageFlash(float DamageAmount);

    // ── Day/Night Cycle Audio ──────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void UpdateTimeOfDay(float NormalizedTimeOfDay);

    UFUNCTION(BlueprintPure, Category = "Audio|DayNight")
    bool IsNighttime() const { return bIsNighttime; }

    // ── Asset Registry (Freesound catalogue) ──────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Assets")
    void RegisterFreesoundAsset(const FAudio_FreesoundAsset& Asset);

    UFUNCTION(BlueprintPure, Category = "Audio|Assets")
    TArray<FAudio_FreesoundAsset> GetRegisteredAssets() const { return RegisteredFreesoundAssets; }

    // ── Configuration ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayerConfig> AmbientLayerConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinoSoundSet> DinoSoundSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_SurvivalCue> SurvivalCues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float DinoVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.7f;

    // Minimum distance for T-Rex footstep screen shake
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float TRexShakeRadius = 1500.0f;

    // Threat level transition smoothing time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float ThreatTransitionTime = 4.0f;

private:
    UPROPERTY()
    EAudio_AmbientLayer CurrentAmbientLayer = EAudio_AmbientLayer::None;

    UPROPERTY()
    EAudio_AmbientLayer TargetAmbientLayer = EAudio_AmbientLayer::DayForest;

    UPROPERTY()
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY()
    EAudio_ThreatLevel TargetThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY()
    UAudioComponent* ActiveAmbientComponent = nullptr;

    UPROPERTY()
    UAudioComponent* FadingAmbientComponent = nullptr;

    UPROPERTY()
    TArray<FAudio_FreesoundAsset> RegisteredFreesoundAssets;

    bool bIsNighttime = false;
    float CurrentTimeOfDay = 0.5f;
    float ThreatTransitionTimer = 0.0f;
    float CrossfadeTimer = 0.0f;
    float CrossfadeDuration = 2.0f;
    bool bIsCrossfading = false;

    void TickAmbientCrossfade(float DeltaTime);
    void TickThreatTransition(float DeltaTime);
    void ApplyThreatLevelMusicChange(EAudio_ThreatLevel NewLevel);
    EAudio_SurvivalState EvaluateSurvivalState(float Health, float Hunger, float Thirst, float Stamina) const;
    const FAudio_DinoSoundSet* FindDinoSoundSet(EAudio_DinoSpecies Species) const;
    USoundBase* GetRandomSound(const TArray<USoundBase*>& Sounds) const;
};
