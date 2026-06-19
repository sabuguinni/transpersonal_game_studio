// AudioZoneManager.h
// Agent #16 — Audio Agent — PROD_CYCLE_AUTO_20260619_007
// Adaptive ambient audio zone system for prehistoric survival game.
// Manages per-biome sound layers: base ambience, danger proximity, weather overlay.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "SharedTypes.h"
#include "AudioZoneManager.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    None        UMETA(DisplayName = "None"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"),
    River       UMETA(DisplayName = "River"),
    Valley      UMETA(DisplayName = "Valley"),
    Volcano     UMETA(DisplayName = "Volcano"),
    Cave        UMETA(DisplayName = "Cave"),
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical"),
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> Sound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay ActiveTimeOfDay = EAudio_TimeOfDay::Day;
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bUseSpatialBlend = true;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TSoftObjectPtr<USoundBase> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float CooldownSeconds = 120.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Voice")
    float LastPlayedTime = -9999.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bSpatial = true;
};

// ─── AudioZoneManager Actor ───────────────────────────────────────────────────

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

    // ── Zone Configuration ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    // ── Voice Lines ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> VoiceLines;

    // ── Components ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* VoiceAudioComponent;

    // ── Danger Proximity ────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float TRexProximityRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float RaptorProximityRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float DangerMusicFadeTime = 2.5f;

    // ── Screen Shake ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TRexFootstepShakeRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TRexFootstepShakeIntensity = 1.0f;

    // ── Public API ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    bool TryPlayVoiceLine(int32 VoiceLineIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceLineImmediate(int32 VoiceLineIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeInAmbience(float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeOutAmbience(float FadeTime = 3.0f);

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    float GetDistanceToNearestDino() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void UpdateDangerFromDinoProximity();

private:
    float DangerCheckInterval = 1.0f;
    float DangerCheckTimer = 0.0f;
    float WorldTimeSeconds = 0.0f;

    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
