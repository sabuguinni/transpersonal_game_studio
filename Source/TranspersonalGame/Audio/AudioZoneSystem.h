// AudioZoneSystem.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260622_011
// Prehistoric survival game ambient audio zone system.
// Drives adaptive soundscape: wind, campfire, river, distant dinosaur calls.
// NO spiritual/meditation content — pure survival/nature audio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioZoneSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — Audio_* prefix to avoid collisions with other agents
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Ambient_Wind        UMETA(DisplayName = "Ambient Wind"),
    Campfire            UMETA(DisplayName = "Campfire"),
    River               UMETA(DisplayName = "River Stream"),
    DinoTerritory       UMETA(DisplayName = "Dinosaur Territory"),
    CaveInterior        UMETA(DisplayName = "Cave Interior"),
    OpenPlain           UMETA(DisplayName = "Open Plain")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Ambient_Wind;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BaseVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;

    // Freesound reference IDs for designer reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString FreesoundReferenceID;
};

USTRUCT(BlueprintType)
struct FAudio_DinoCallConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FString DinoSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float MinCallInterval = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float MaxCallInterval = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float CallRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Cautious;
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_AmbientZone — Placed in level to define audio regions
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Audio", meta = (DisplayName = "Ambient Audio Zone"))
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

    // ── Components ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* ZoneSphere;

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
    bool bPlayerInZone = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float CurrentVolume = 0.0f;

    // ── Functions ────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(EAudio_DangerLevel NewDanger);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetDistanceBlend(AActor* Player) const;

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

    EAudio_DangerLevel CurrentDanger = EAudio_DangerLevel::Safe;
    float FadeTimer = 0.0f;
    bool bFadingIn = false;
    bool bFadingOut = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_DinoCallEmitter — Randomised distant dinosaur vocalisations
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Audio", meta = (DisplayName = "Dino Call Emitter"))
class TRANSPERSONALGAME_API AAudio_DinoCallEmitter : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DinoCallEmitter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FAudio_DinoCallConfig DinoCallConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    TArray<USoundBase*> CallSounds;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dino")
    float TimeUntilNextCall = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void TriggerCall();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void ScheduleNextCall();

    UFUNCTION(BlueprintPure, Category = "Audio|Dino")
    EAudio_DangerLevel GetDangerLevel() const { return DinoCallConfig.DangerLevel; }

private:
    UPROPERTY()
    UAudioComponent* CallAudioComponent;
};

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_SoundscapeManager — GameInstance subsystem coordinating all zones
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Audio", meta = (DisplayName = "Soundscape Manager Component"))
class TRANSPERSONALGAME_API UAudio_SoundscapeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SoundscapeManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // Registered zones in the world
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Manager")
    TArray<AAudio_AmbientZone*> RegisteredZones;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Manager")
    TArray<AAudio_DinoCallEmitter*> RegisteredEmitters;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Manager")
    EAudio_DangerLevel GlobalDangerLevel = EAudio_DangerLevel::Safe;

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void RegisterZone(AAudio_AmbientZone* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void RegisterEmitter(AAudio_DinoCallEmitter* Emitter);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetGlobalDanger(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void NotifyTRexProximity(float Distance);

    UFUNCTION(BlueprintPure, Category = "Audio|Manager")
    EAudio_DangerLevel GetGlobalDangerLevel() const { return GlobalDangerLevel; }

    // Freesound asset catalogue (populated by designer)
    // Campfire loop:   Freesound #681366 (83s, campfire crackling, CC0)
    // River stream:    Freesound #819768 (87s, small brook, CC0)
    // River alt:       Freesound #808370 (58s, flowing river, CC0)
    // Campfire night:  Freesound #802195 (247s, fire + nature, CC0)
};
