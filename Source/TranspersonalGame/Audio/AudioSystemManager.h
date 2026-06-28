// AudioSystemManager.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260628_001
// Adaptive audio system: ambient layers, dialogue ducking, danger-reactive music
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Aware       UMETA(DisplayName = "Aware"),
    Tense       UMETA(DisplayName = "Tense"),
    Combat      UMETA(DisplayName = "Combat"),
    Flee        UMETA(DisplayName = "Flee"),
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    NorthernMarsh   UMETA(DisplayName = "Northern Marsh"),
    VolcanicRidge   UMETA(DisplayName = "Volcanic Ridge"),
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* SoundAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome = EAudio_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;
};

USTRUCT(BlueprintType)
struct FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* MusicAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float TransitionTime = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float Volume = 0.8f;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueDuckConfig
{
    GENERATED_BODY()

    /** Volume multiplier applied to ambient/music layers during dialogue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float DuckVolume = 0.25f;

    /** Time in seconds to fade down to duck volume */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float DuckFadeTime = 0.5f;

    /** Time in seconds to restore volume after dialogue ends */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float RestoreFadeTime = 1.5f;
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_AmbientZone — placed in level to define biome audio region
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Ambient Zone"))
class TRANSPERSONALGAME_API AAudio_AmbientZone : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZone();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* PrimaryAudioComponent;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void DeactivateZone();

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    bool IsPlayerInZone() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    bool bZoneActive = false;
    float CurrentVolume = 0.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_SystemManager — singleton manager for all adaptive audio
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    // ── Danger / Music State ─────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    TArray<FAudio_MusicState> MusicStates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Music")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicTransitionSpeed = 2.0f;

    // ── Dialogue Ducking ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FAudio_DialogueDuckConfig DuckConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Dialogue")
    bool bDialogueActive = false;

    // ── Time of Day ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Time")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    // ── Public API ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void OnDialogueStart();

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void OnDialogueEnd();

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintPure, Category = "Audio|Manager")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|Manager")
    bool IsDialogueActive() const { return bDialogueActive; }

    /** Find or spawn the singleton manager in the current world */
    UFUNCTION(BlueprintCallable, Category = "Audio|Manager", meta = (WorldContext = "WorldContext"))
    static AAudio_SystemManager* GetInstance(UObject* WorldContext);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    UAudioComponent* MusicComponent;

    TArray<AAudio_AmbientZone*> RegisteredZones;

    void DiscoverAmbientZones();
    void UpdateMusicLayer(float DeltaTime);
    void ApplyDialogueDuck(bool bDuck);

    float TargetMusicVolume = 0.8f;
    float CurrentMusicVolume = 0.0f;
};
