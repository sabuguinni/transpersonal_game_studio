#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerSphere.h"
#include "AudioZoneSystem.generated.h"

// ============================================================
// Audio Zone System — Agent #16 Audio Agent
// Prehistoric survival ambient audio: proximity-based zone
// triggers for camp, predator, water, cave, herd environments.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Camp            UMETA(DisplayName = "Camp"),
    PredatorNear    UMETA(DisplayName = "PredatorNear"),
    Water           UMETA(DisplayName = "Water"),
    Cave            UMETA(DisplayName = "Cave"),
    OpenPlains      UMETA(DisplayName = "OpenPlains"),
    Forest          UMETA(DisplayName = "Forest"),
    None            UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ProximityRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float CrossfadeDuration = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString FreesoundAssetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString FreesoundPreviewURL;
};

USTRUCT(BlueprintType)
struct FAudio_DinosaурSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepInterval = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float BreathingRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarCooldown = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    bool bCausesScreenShake = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float ScreenShakeIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_ZoneType AssociatedZone = EAudio_ZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float NightMultiplier = 1.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float RainMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtNight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtDay = true;
};

/**
 * UAudio_ZoneComponent
 * Attach to any actor to define its ambient audio zone.
 * Handles proximity detection, volume crossfade, and
 * threat-level-driven music intensity changes.
 */
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), DisplayName = "Audio Zone Component")
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    // Zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    // Current threat level — set by AI/combat system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    // Ambient layers active in this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    // Is player currently inside this zone?
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone", meta = (AllowPrivateAccess = "true"))
    bool bPlayerInZone = false;

    // Distance to player (updated each tick)
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone", meta = (AllowPrivateAccess = "true"))
    float DistanceToPlayer = 99999.0f;

    // Current blended volume (0-1)
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone", meta = (AllowPrivateAccess = "true"))
    float CurrentVolume = 0.0f;

    // ---- UActorComponent overrides ----
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Blueprint callable ----
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetZoneType(EAudio_ZoneType NewZoneType);

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    bool IsPlayerInZone() const;

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    float GetNormalizedProximity() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ForceEnterZone();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ForceExitZone();

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_ThreatLevel GetCurrentThreatLevel() const;

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    FString GetZoneDebugInfo() const;

private:
    void UpdateProximity(float DeltaTime);
    void CrossfadeVolume(float TargetVolume, float DeltaTime);
    float ComputeTargetVolume() const;

    FTimerHandle ThreatLevelCooldownHandle;
};

/**
 * UAudio_DinosaurSoundManager
 * Manages per-species dinosaur audio profiles.
 * Drives footstep rumble, roar radius, and screen shake
 * based on distance to player.
 */
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Sound Manager")
class TRANSPERSONALGAME_API UAudio_DinosaurSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DinosaurSoundManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FAudio_DinosaурSoundProfile SoundProfile;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dinosaur", meta = (AllowPrivateAccess = "true"))
    float CurrentDistanceToPlayer = 99999.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dinosaur", meta = (AllowPrivateAccess = "true"))
    bool bPlayerInRoarRadius = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dinosaur", meta = (AllowPrivateAccess = "true"))
    bool bPlayerInFootstepRadius = false;

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerFootstep();

    UFUNCTION(BlueprintPure, Category = "Audio|Dinosaur")
    float GetFootstepScreenShakeIntensity() const;

    UFUNCTION(BlueprintPure, Category = "Audio|Dinosaur")
    bool ShouldTriggerScreenShake() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void SetDinosaurSpecies(FName Species);

private:
    float FootstepTimer = 0.0f;
    float RoarCooldownTimer = 0.0f;
    bool bRoarOnCooldown = false;

    void UpdateDistanceToPlayer();
    void InitDefaultProfiles();
};
