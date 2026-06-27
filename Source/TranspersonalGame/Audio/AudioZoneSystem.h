// AudioZoneSystem.h
// Agent #16 — Audio Agent
// Cycle: PROD_CYCLE_AUTO_20260627_007
// Ambient audio zone system for prehistoric survival game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioZoneSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    JungleAmbient     UMETA(DisplayName = "Jungle Ambient"),
    RiverAmbient      UMETA(DisplayName = "River Ambient"),
    DangerProximity   UMETA(DisplayName = "Danger Proximity"),
    NightAmbient      UMETA(DisplayName = "Night Ambient"),
    CampfireAmbient   UMETA(DisplayName = "Campfire Ambient"),
    CaveAmbient       UMETA(DisplayName = "Cave Ambient"),
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe      UMETA(DisplayName = "Safe"),
    Caution   UMETA(DisplayName = "Caution"),
    Danger    UMETA(DisplayName = "Danger"),
    Critical  UMETA(DisplayName = "Critical"),
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::JungleAmbient;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bSpatialBlend = true;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float AudioDuration = 0.0f;
};

// ─── Audio Zone Actor ─────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Zone"))
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

    // ── Components ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    // ── Config ──────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    USoundBase* AmbientSound;

    // ── State ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    bool bPlayerInZone;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float CurrentVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    EAudio_DangerLevel CurrentDangerLevel;

    // ── Functions ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeAudioIn();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeAudioOut();

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    bool IsPlayerInZone() const { return bPlayerInZone; }

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

private:
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                               bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    float FadeTimer;
    bool bFadingIn;
    bool bFadingOut;
};

// ─── Adaptive Music Manager ───────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Adaptive Music Manager"))
class TRANSPERSONALGAME_API AAudio_AdaptiveMusicManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AdaptiveMusicManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* ExplorationMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* DangerMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* NightMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* CombatMusic;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Music")
    EAudio_DangerLevel GlobalDangerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicTransitionTime = 3.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetGlobalDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void TransitionToTrack(USoundBase* NewTrack);

    UFUNCTION(BlueprintPure, Category = "Audio|Music")
    EAudio_DangerLevel GetGlobalDangerLevel() const { return GlobalDangerLevel; }

private:
    UPROPERTY()
    UAudioComponent* MusicComponent;

    EAudio_DangerLevel PreviousDangerLevel;
    float TransitionTimer;
    bool bTransitioning;
};
