// AudioSystemManager.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260627_008
// Adaptive audio system: ambient zones, dinosaur proximity audio, dialogue voice lines
// Integrates with ANarr_DialogueTrigger from DialogueSystem.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — global scope (RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_AmbientZoneType : uint8
{
    None            UMETA(DisplayName = "None"),
    JungleDense     UMETA(DisplayName = "Jungle Dense"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    DangerZone      UMETA(DisplayName = "Danger Zone"),
};

UENUM(BlueprintType)
enum class EAudio_DinoSpecies : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
};

UENUM(BlueprintType)
enum class EAudio_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Alerted         UMETA(DisplayName = "Alerted"),
    Charging        UMETA(DisplayName = "Charging"),
    Feeding         UMETA(DisplayName = "Feeding"),
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS — global scope (RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_DinoSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    EAudio_DinoSpecies Species = EAudio_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundBase* IdleSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundBase* AlertSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundBase* ChargeRoarSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundBase* FootstepSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FootstepVolumeMultiplier = 1.0f;

    // Radius at which footsteps trigger screen shake (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float ScreenShakeRadius = 800.0f;

    // Shake intensity (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float ShakeIntensity = 0.5f;
};

USTRUCT(BlueprintType)
struct FAudio_AmbientZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_AmbientZoneType ZoneType = EAudio_AmbientZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* DayAmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* NightAmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FName CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    USoundBase* VoiceSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FText SubtitleText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bHasBeenPlayed = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// AMBIENT ZONE ACTOR
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Ambient Zone"))
class TRANSPERSONALGAME_API AAudio_AmbientZone : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZone();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FAudio_AmbientZoneConfig ZoneConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Ambient",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* ZoneRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Ambient",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetDayNightBlend(float DayFraction);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetZoneVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    EAudio_AmbientZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

private:
    float CurrentDayFraction = 0.5f;
    bool bPlayerInZone = false;

    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                           bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

// ─────────────────────────────────────────────────────────────────────────────
// DINOSAUR AUDIO COMPONENT
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dino Audio Component"))
class TRANSPERSONALGAME_API UAudio_DinoAudioComponent : public UAudioComponent
{
    GENERATED_BODY()

public:
    UAudio_DinoAudioComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FAudio_DinoSoundProfile SoundProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    EAudio_DinoState CurrentState = EAudio_DinoState::Idle;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void PlayStateSound(EAudio_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void PlayFootstep();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void TriggerScreenShakeIfNearPlayer();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void SetDinoState(EAudio_DinoState NewState);

private:
    float FootstepCooldown = 0.0f;
    static constexpr float FootstepMinInterval = 0.4f;
};

// ─────────────────────────────────────────────────────────────────────────────
// AUDIO SYSTEM MANAGER — World Subsystem-style singleton actor
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Day/night cycle fraction (0=midnight, 0.5=noon, 1=midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float DayFraction = 0.5f;

    // Speed of day cycle (full cycle per real second — set very low, e.g. 0.0001)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float DayCycleSpeed = 0.0001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Master")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Master")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Master")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Master")
    float VoiceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    TArray<FAudio_DinoSoundProfile> DinoSoundProfiles;

    UFUNCTION(BlueprintCallable, Category = "Audio|Master")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Master")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void UpdateDayNightAudio(float NewDayFraction);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceLine(const FAudio_VoiceLineEntry& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void RegisterDinoAudio(UAudio_DinoAudioComponent* DinoAudio);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void NotifyDinoStateChange(EAudio_DinoSpecies Species, EAudio_DinoState NewState);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Debug")
    void LogAudioSystemStatus();

private:
    TArray<TWeakObjectPtr<UAudio_DinoAudioComponent>> RegisteredDinoAudio;

    UPROPERTY()
    UAudioComponent* ActiveVoiceComponent;

    void TickDayCycle(float DeltaTime);
    void BroadcastDayFractionToZones();
};
