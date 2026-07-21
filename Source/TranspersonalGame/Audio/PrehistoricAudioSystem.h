#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "PrehistoricAudioSystem.generated.h"

// ============================================================
// Prehistoric Audio System — Agent #16 Audio Agent
// Adaptive audio manager for dinosaur survival game.
// Controls ambient layers, danger music, and environmental SFX.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Combat      UMETA(DisplayName = "Combat"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome = EAudio_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;
};

USTRUCT(BlueprintType)
struct FAudio_DangerCue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    USoundBase* StingerSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    EAudio_DangerLevel TriggerLevel = EAudio_DangerLevel::Threatened;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float CooldownSeconds = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    bool bLooping = false;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString Subtitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bPlayed = false;
};

/**
 * APrehistoricAudioSystem
 * 
 * Adaptive audio manager placed in the level.
 * Monitors player state, proximity to dinosaurs, time of day,
 * and biome to blend ambient layers and trigger danger stingers.
 * 
 * Designed around Walter Murch's principle: the sound that does
 * not exist is often more powerful than the sound that does.
 * Silence before a T-Rex attack is the most terrifying sound.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Prehistoric Audio System"))
class TRANSPERSONALGAME_API APrehistoricAudioSystem : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricAudioSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Danger State ──────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::Forest;

    // ── Ambient Layers ────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    TArray<FAudio_DangerCue> DangerCues;

    // ── Voice Lines (loaded from ElevenLabs TTS URLs) ─────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> SurvivorVoiceLines;

    // ── Audio Components ──────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* DangerAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* VoiceAudioComponent;

    // ── Adaptive Music Settings ───────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float DangerDetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicTransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float SilenceBeforeAttackSeconds = 3.0f;

    // ── Dinosaur Proximity Audio ──────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    float TRexFootstepRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    float RaptorCallRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    float HerdAmbientRadius = 8000.0f;

    // ── Runtime State ─────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float NearestDinosaurDistance = 99999.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float DangerIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    bool bInSilenceWindow = false;

    // ── Public API ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerDangerStinger(EAudio_DangerLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayVoiceLine(const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerSilenceWindow(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetDangerIntensity() const { return DangerIntensity; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool IsInSilenceWindow() const { return bInSilenceWindow; }

    UFUNCTION(CallInEditor, Category = "Audio|Debug")
    void DebugPrintAudioState();

private:
    void UpdateDangerFromProximity(float DeltaTime);
    void UpdateAmbientLayers(float DeltaTime);
    void BlendMusicLayers(float DeltaTime);
    float CalculateDangerIntensity() const;

    float DangerStingerCooldown = 0.0f;
    float SilenceWindowTimer = 0.0f;
    float AmbientUpdateTimer = 0.0f;
    static constexpr float AmbientUpdateInterval = 0.5f;
};
