#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Audio Agent #16 — AudioZoneSystem.h
// Proximity-based ambient audio zone system for prehistoric survival game.
// Each zone plays looping ambient audio when the player is within radius.
// Supports crossfade blending between zones and intensity scaling by distance.
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    RaptorValley    UMETA(DisplayName = "Raptor Valley"),
    RiverCrossing   UMETA(DisplayName = "River Crossing"),
    VolcanicCave    UMETA(DisplayName = "Volcanic Cave"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    TRexTerritory   UMETA(DisplayName = "T-Rex Territory"),
    Forest          UMETA(DisplayName = "Forest"),
    Custom          UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Dangerous   UMETA(DisplayName = "Dangerous"),
    Lethal      UMETA(DisplayName = "Lethal")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Safe;

    /** Radius in cm within which ambient audio plays at full volume */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FullVolumeRadius = 400.0f;

    /** Outer radius — audio fades to zero at this distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeRadius = 800.0f;

    /** Volume multiplier for this zone (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VolumeMultiplier = 1.0f;

    /** Crossfade blend time in seconds when entering/leaving zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendTime = 2.0f;

    /** If true, plays creature vocalisations at random intervals */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bPlayCreatureVocalisations = false;

    /** Min/max interval in seconds between creature vocalisation events */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float VocalisationIntervalMin = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float VocalisationIntervalMax = 25.0f;
};

USTRUCT(BlueprintType)
struct FAudio_SoundReference
{
    GENERATED_BODY()

    /** Freesound.org asset ID for reference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Reference")
    int32 FreesoundID = 0;

    /** Human-readable description of the sound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Reference")
    FString Description;

    /** Preview URL from Freesound */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Reference")
    FString PreviewURL;

    /** Duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Reference")
    float Duration = 0.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_AmbientZone — proximity-triggered ambient audio zone actor
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Ambient Audio Zone"))
class TRANSPERSONALGAME_API AAudio_AmbientZone : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZone();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Zone configuration ─────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    /** Sound asset to loop when player is in zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    USoundBase* AmbientSound = nullptr;

    /** Optional creature vocalisation sound (one-shot) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    USoundBase* CreatureVocalisationSound = nullptr;

    // ── Freesound references (for asset pipeline) ──────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio References")
    TArray<FAudio_SoundReference> SoundReferences;

    // ── Runtime state ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone", meta = (AllowPrivateAccess = "true"))
    bool bPlayerInZone = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone", meta = (AllowPrivateAccess = "true"))
    float CurrentVolume = 0.0f;

    // ── Functions ──────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetZoneActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_DangerLevel GetDangerLevel() const;

    UFUNCTION(BlueprintPure, Category = "Audio Zone")
    bool IsPlayerInZone() const { return bPlayerInZone; }

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* TriggerSphere = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UAudioComponent* AudioComp = nullptr;

    float VocalisationTimer = 0.0f;
    float NextVocalisationTime = 0.0f;

    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void UpdateVolumeByDistance(float DeltaTime);
    void ScheduleNextVocalisation();
};
