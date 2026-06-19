#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneSystem.generated.h"

// ─────────────────────────────────────────────
// Enums — global scope (UHT requirement)
// ─────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Valley_Ambient      UMETA(DisplayName = "Valley Ambient"),
    RaptorTerritory     UMETA(DisplayName = "Raptor Territory"),
    CampFire            UMETA(DisplayName = "Camp Fire"),
    River_Ambient       UMETA(DisplayName = "River Ambient"),
    Forest_Dense        UMETA(DisplayName = "Dense Forest"),
    OpenPlain           UMETA(DisplayName = "Open Plain")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

// ─────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Valley_Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLoopAmbient = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutTime = 3.0f;
};

USTRUCT(BlueprintType)
struct FAudio_SoundAssetRef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    FString SoundCuePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    FString FreesoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    FString SupabaseURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    float DefaultVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    float DefaultPitch = 1.0f;
};

// ─────────────────────────────────────────────
// AAudio_ZoneTrigger — ambient audio zone actor
// ─────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Zone Trigger"))
class TRANSPERSONALGAME_API AAudio_ZoneTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneTrigger();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Zone configuration ──────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    // ── Trigger volume ──────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    // ── Audio component ─────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    // ── State ───────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    bool bPlayerInZone = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float CurrentBlendAlpha = 0.0f;

    // ── Sound references ────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Sounds")
    FAudio_SoundAssetRef AmbientSoundRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Sounds")
    FAudio_SoundAssetRef StingerSoundRef;

    // ── Events ───────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone(AActor* OverlappingActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone(AActor* OverlappingActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_DangerLevel GetDangerLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeAmbientIn();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeAmbientOut();

private:
    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    float FadeTimer = 0.0f;
    bool bFadingIn = false;
    bool bFadingOut = false;
};

// ─────────────────────────────────────────────
// AAudio_DinosaurSoundEmitter — per-dino audio
// ─────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dinosaur Sound Emitter"))
class TRANSPERSONALGAME_API AAudio_DinosaurSoundEmitter : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DinosaurSoundEmitter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FString DinosaurSpecies = "Raptor";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarCooldownMin = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarCooldownMax = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float MaxHearingDistance = 2000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dinosaur")
    float TimeUntilNextRoar = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* RoarAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* FootstepAudioComponent;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerFootstep();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void SetDistanceAttenuation(float MaxDistance);

private:
    void ScheduleNextRoar();
    float RoarTimer = 0.0f;
};
