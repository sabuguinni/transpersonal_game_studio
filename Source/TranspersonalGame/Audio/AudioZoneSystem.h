// AudioZoneSystem.h
// Audio Agent #16 — PROD_CYCLE_AUTO_20260622_009
// Ambient audio zone system for prehistoric survival game.
// Drives MetaSounds-compatible audio zones with distance-based blending.
// NO spiritual/mystical content — pure environmental audio design.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneSystem.generated.h"

// ─── Enums (global scope — UHT rule) ────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Camp        UMETA(DisplayName = "Camp"),
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"),
    DangerZone  UMETA(DisplayName = "DangerZone"),
    Cave        UMETA(DisplayName = "Cave"),
    OpenPlain   UMETA(DisplayName = "OpenPlain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Panic       UMETA(DisplayName = "Panic")
};

// ─── Structs (global scope — UHT rule) ──────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;

    FAudio_ZoneConfig()
        : ZoneType(EAudio_ZoneType::Forest)
        , BlendRadius(500.0f)
        , MaxVolume(1.0f)
        , FadeInTime(2.0f)
        , FadeOutTime(3.0f)
        , bLooping(true)
    {}
};

USTRUCT(BlueprintType)
struct FAudio_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    FString SurfaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float PitchVarianceMin = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float PitchVarianceMax = 1.1f;

    FAudio_FootstepConfig()
        : SurfaceType(TEXT("Dirt"))
        , VolumeMultiplier(1.0f)
        , PitchVarianceMin(0.9f)
        , PitchVarianceMax(1.1f)
    {}
};

// ─── AAudio_AmbientZone ──────────────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), BlueprintType)
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

    // ── Zone configuration ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    // ── Components ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* BlendSphere = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent = nullptr;

    // ── Runtime state ───────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State",
              meta = (AllowPrivateAccess = "true"))
    bool bPlayerInZone = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State",
              meta = (AllowPrivateAccess = "true"))
    float CurrentBlendAlpha = 0.0f;

    // ── UFUNCTIONs ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeIn();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeOut();

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    bool IsPlayerInZone() const { return bPlayerInZone; }

private:
    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                           bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    float FadeTimer = 0.0f;
    bool bFadingIn = false;
    bool bFadingOut = false;
};

// ─── UAudio_FootstepComponent ────────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_FootstepComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_FootstepComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    TArray<FAudio_FootstepConfig> FootstepConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float StepInterval = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float RunStepInterval = 0.28f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Footstep")
    void PlayFootstep(const FString& SurfaceType, bool bIsRunning);

    UFUNCTION(BlueprintCallable, Category = "Audio|Footstep")
    void PlayDinosaurFootstep(float DinoMassKg);

    UFUNCTION(BlueprintPure, Category = "Audio|Footstep")
    FAudio_FootstepConfig GetConfigForSurface(const FString& SurfaceType) const;

private:
    float LastStepTime = 0.0f;
};
