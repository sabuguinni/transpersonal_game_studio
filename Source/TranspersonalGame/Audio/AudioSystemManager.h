#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Prehistoric survival ambient + reactive audio system
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    DangerZone  UMETA(DisplayName = "DangerZone"),
    Camp        UMETA(DisplayName = "Camp"),
    OpenPlains  UMETA(DisplayName = "OpenPlains")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Uneasy      UMETA(DisplayName = "Uneasy"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BlendRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float NightVolumeMultiplier = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_ImpactEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Magnitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SourceTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Timestamp = 0.0f;
};

// ============================================================
// UAudio_ZoneComponent — attach to any actor to define audio zone
// ============================================================
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Calm;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_ThreatLevel GetThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetBlendedVolume(float TimeOfDay) const;

protected:
    virtual void BeginPlay() override;
};

// ============================================================
// AAudio_AmbientZoneActor — spatial ambient audio source
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AmbientZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZoneActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio",
        meta = (AllowPrivateAccess = "true"))
    UAudio_ZoneComponent* AudioZoneComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ActivationRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bActiveAtNight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bActiveAtDay = true;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void DeactivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsPlayerInRange() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    bool bIsActive = false;
    float LastCheckTime = 0.0f;
};

// ============================================================
// AAudio_SystemManager — world-level audio orchestrator
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ThreatLevel GlobalThreatLevel = EAudio_ThreatLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TimeOfDay = 0.5f;  // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ThreatTransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DinosaurProximityRadius = 1500.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterImpactEvent(const FAudio_ImpactEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetGlobalThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_ThreatLevel GetGlobalThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerDinosaurProximityAlert(FVector DinosaurLocation, float DinosaurSize);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    TArray<FAudio_ImpactEvent> RecentImpacts;
    float CurrentThreatBlend = 0.0f;
    float TargetThreatBlend = 0.0f;

    void UpdateThreatBlend(float DeltaTime);
    void AuditDinosaurProximity();
};
