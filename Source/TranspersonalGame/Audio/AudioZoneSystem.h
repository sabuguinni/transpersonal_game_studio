// AudioZoneSystem.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260626_008
// Proximity-based audio zone system for prehistoric soundscape
// Wires MetaSounds ambient cues to dialogue trigger zones

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneSystem.generated.h"

// ============================================================
// ENUMS — Audio_* prefix (RULE 2: unique names across project)
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    JungleAmbience      UMETA(DisplayName = "Jungle Ambience"),
    TRexProximity       UMETA(DisplayName = "T-Rex Proximity"),
    RaptorStalk         UMETA(DisplayName = "Raptor Stalk"),
    RiverAmbience       UMETA(DisplayName = "River Ambience"),
    CampfireSafe        UMETA(DisplayName = "Campfire Safe Zone"),
    DangerAlert         UMETA(DisplayName = "Danger Alert")
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Silent      UMETA(DisplayName = "Silent"),
    Whisper     UMETA(DisplayName = "Whisper"),
    Ambient     UMETA(DisplayName = "Ambient"),
    Tense       UMETA(DisplayName = "Tense"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::JungleAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float TriggerRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BaseVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bOneShot = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FString FreesoundAssetID = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FString VoiceLineURL = TEXT("");
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    FName DinosaurSpecies = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    float RoarRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    float FootstepRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    float BreathingRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    float RoarCooldown = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    float FootstepInterval = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    EAudio_IntensityLevel ThreatLevel = EAudio_IntensityLevel::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    int32 FreesoundRoarID = 586546;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino Audio")
    int32 FreesoundGrowlID = 586547;
};

// ============================================================
// MAIN CLASS — Audio Zone Actor
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Zone Actor"))
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Zone Configuration ──────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_DinosaurSoundProfile DinosaurProfile;

    // ── Components ──────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    // ── State ────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone State")
    bool bPlayerInZone = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone State")
    float CurrentVolume = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone State")
    EAudio_IntensityLevel CurrentIntensity = EAudio_IntensityLevel::Silent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone State")
    float TimeSinceLastRoar = 0.0f;

    // ── Functions ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerEnterZone(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerExitZone(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetIntensityLevel(EAudio_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void TriggerDinosaurRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Audio Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

    UFUNCTION(BlueprintPure, Category = "Audio Zone")
    bool IsPlayerInZone() const { return bPlayerInZone; }

    UFUNCTION(CallInEditor, Category = "Audio Zone")
    void PreviewZoneInEditor();

private:
    void UpdateVolumeBlend(float DeltaTime);
    float TargetVolume = 0.0f;
    bool bHasTriggeredOnce = false;
};

// ============================================================
// AUDIO MANAGER SUBSYSTEM
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_PrehistoricSoundManager : public UObject
{
    GENERATED_BODY()

public:
    UAudio_PrehistoricSoundManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Manager")
    TArray<FAudio_DinosaurSoundProfile> RegisteredDinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Manager")
    TArray<FAudio_ZoneConfig> ActiveZoneConfigs;

    UPROPERTY(BlueprintReadOnly, Category = "Sound Manager")
    EAudio_IntensityLevel GlobalThreatLevel = EAudio_IntensityLevel::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Manager")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Manager")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Manager")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Manager")
    float VoiceVolume = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Sound Manager")
    void UpdateGlobalThreatLevel(EAudio_IntensityLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Sound Manager")
    void RegisterDinosaurProfile(FAudio_DinosaurSoundProfile Profile);

    UFUNCTION(BlueprintPure, Category = "Sound Manager")
    EAudio_IntensityLevel GetGlobalThreatLevel() const { return GlobalThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Sound Manager")
    float GetEffectiveMasterVolume() const { return MasterVolume; }
};
