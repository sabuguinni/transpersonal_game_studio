// AudioSystemManager.h
// Agent #16 — Audio Agent
// PROD_CYCLE_AUTO_20260628_009
// Adaptive audio system: ambient zones, dinosaur proximity audio, survival feedback sounds
// Wires to ANarr_DialogueTriggerVolume for voice line playback

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ─── ENUMS ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    SwampLowlands   UMETA(DisplayName = "Swamp Lowlands"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe — No Predators"),
    Cautious    UMETA(DisplayName = "Cautious — Predator Nearby"),
    Danger      UMETA(DisplayName = "Danger — Predator Detected"),
    Critical    UMETA(DisplayName = "Critical — Under Attack")
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
enum class EAudio_FootstepSurface : uint8
{
    DryGround   UMETA(DisplayName = "Dry Ground"),
    MudSoft     UMETA(DisplayName = "Soft Mud"),
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Water       UMETA(DisplayName = "Shallow Water"),
    DeadLeaves  UMETA(DisplayName = "Dead Leaves")
};

// ─── STRUCTS ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> BaseLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> DangerStinger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome = EAudio_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float DangerVolumeMult = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> IdleVocalisation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> AlertCall;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> ChargeRoar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> FootstepHeavy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> FootstepLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float ProximityAlertRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float GroundShakeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float GroundShakeIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bHasBeenPlayed = false;
};

// ─── AMBIENT ZONE ACTOR ───────────────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Audio Ambient Zone"))
class TRANSPERSONALGAME_API AAudio_AmbientZone : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZone();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_AmbientLayer AmbientLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ZoneRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeToSilence(float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USphereComponent> ZoneSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    bool bPlayerInZone = false;
};

// ─── SCREEN SHAKE / GROUND RUMBLE COMPONENT ──────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Audio Ground Rumble"))
class TRANSPERSONALGAME_API UAudio_GroundRumbleComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_GroundRumbleComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Rumble")
    float RumbleRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Rumble")
    float RumbleIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Rumble")
    float StepInterval = 1.2f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Rumble")
    void TriggerFootstepRumble();

    UFUNCTION(BlueprintCallable, Category = "Audio|Rumble")
    void SetRumbleIntensity(float NewIntensity);

private:
    float TimeSinceLastStep = 0.0f;
    bool bIsActive = false;
};

// ─── AUDIO SYSTEM MANAGER (SINGLETON ACTOR) ───────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Ambient ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> BiomeAmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    // ── Dinosaur Profiles ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    // ── Voice Lines ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> RegisteredVoiceLines;

    // ── Danger ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    EAudio_DangerLevel GlobalDangerLevel = EAudio_DangerLevel::Safe;

    // ── Functions ──
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetGlobalDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void PlayVoiceLine(FName LineID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void RegisterVoiceLine(FAudio_VoiceLine VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void TriggerDinosaurProximityAlert(FName Species, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    float GetDangerMusicIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    bool IsVoiceLinePlayed(FName LineID) const;

    // Static accessor
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    static AAudio_SystemManager* GetInstance(UObject* WorldContext);

private:
    UPROPERTY()
    TObjectPtr<UAudioComponent> MusicComponent;

    float DangerIntensity = 0.0f;
    float DangerFadeSpeed = 0.5f;

    void UpdateDangerIntensity(float DeltaTime);
    void BroadcastDangerToAmbientZones();
};
