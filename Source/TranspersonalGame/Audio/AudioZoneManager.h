// AudioZoneManager.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260622_013
// Prehistoric survival game — ambient audio zone system
// NO spiritual/mystical content. Realistic survival soundscape only.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneManager.generated.h"

// ─── Enums (global scope — UHT requirement) ───────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Camp        UMETA(DisplayName = "Camp Ambient"),
    Jungle      UMETA(DisplayName = "Jungle Ambient"),
    River       UMETA(DisplayName = "River Ambient"),
    Danger      UMETA(DisplayName = "Danger / Predator Nearby"),
    Combat      UMETA(DisplayName = "Active Combat"),
    Cave        UMETA(DisplayName = "Cave Interior"),
    OpenPlain   UMETA(DisplayName = "Open Plain")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution — predator nearby"),
    Imminent    UMETA(DisplayName = "Imminent — predator attacking"),
    Combat      UMETA(DisplayName = "In Combat")
};

// ─── Structs ──────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float DangerStingVolume = 1.0f;

    // Freesound reference IDs for runtime asset loading
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    int32 FreesoundAmbientID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString AmbientAssetPath;
};

USTRUCT(BlueprintType)
struct FAudio_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float DustParticleThreshold = 300.0f;   // speed (cm/s) above which dust spawns

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float ScreenShakeRadiusTRex = 1500.0f;  // radius within which T-Rex stomp shakes camera

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float ScreenShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float StompIntervalSeconds = 0.8f;      // T-Rex step cadence
};

// ─── Main Actor ───────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Audio Zone Manager"))
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Zone configuration ────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_FootstepConfig FootstepConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* ZoneTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    // ── Danger state ──────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Danger")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float DangerCheckInterval = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float RaptorDangerRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float TRexDangerRadius = 1500.0f;

    // ── Day/Night audio blend ─────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float DayAmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float NightAmbientVolume = 0.9f;   // night is louder — insects, owls, predators

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float CurrentTimeOfDay = 0.5f;     // 0=midnight, 0.5=noon, 1=midnight

    // ── Blueprint events ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void UpdateTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void TriggerTRexStompShake(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void TriggerDamageFlash();

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    float GetAmbientVolumeForTime() const;

    // ── Overlap handlers ──────────────────────────────────────────────────

    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                           bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    float DangerCheckTimer = 0.0f;
    bool bPlayerInZone = false;

    void CheckNearbyPredators();
    void BlendAmbientVolume(float TargetVolume, float DeltaTime);
};
