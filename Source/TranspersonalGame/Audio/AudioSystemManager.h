#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Enums — Audio_* prefix to avoid global name collisions
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Coastal         UMETA(DisplayName = "Coastal"),
    Cave            UMETA(DisplayName = "Cave")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn    UMETA(DisplayName = "Dawn"),
    Day     UMETA(DisplayName = "Day"),
    Dusk    UMETA(DisplayName = "Dusk"),
    Night   UMETA(DisplayName = "Night")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString FreesoundID = TEXT("");
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSFX
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName DinosaurSpecies = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RoarRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BreathRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bCausesScreenShake = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ScreenShakeIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bCombatActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bNightTime = false;
};

// ============================================================
// AAudio_SystemManager — main audio orchestrator
// ============================================================

UCLASS(ClassGroup = "TranspersonalAudio", BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Threat / Music ─────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void UpdateMusicIntensity(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Audio|Music")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentMusicState.ThreatLevel; }

    // ── Ambient ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiomeZone(EAudio_BiomeZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void CrossfadeAmbientLayers(float DeltaTime);

    // ── Dinosaur SFX ───────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void RegisterDinosaurSFX(const FAudio_DinosaurSFX& SFXConfig);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerDinosaurRoar(FName Species, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerDinosaurFootstep(FName Species, FVector Location, float Weight);

    // ── Screen Shake ───────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|FX")
    void TriggerScreenShake(float Intensity, FVector SourceLocation);

    // ── Survival SFX ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayCraftingSound(FName ItemCrafted);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayFireSound(FVector FireLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayDamageFlash(float DamageAmount);

    // ── Narration ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void QueueNarrationLine(const FString& AudioURL, float Delay = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void StopNarration();

    // ── State ─────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    FAudio_MusicState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    EAudio_BiomeZone CurrentBiome = EAudio_BiomeZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float NarrationVolume = 1.0f;

    // Freesound IDs for campfire (id: 157187 — Campfire Crackle 3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString CampfireFreesoundID = TEXT("157187");

    // Narration audio URLs from ElevenLabs TTS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString NarrationDanger_URL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884755262_Narrator_Danger.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString NarrationSurvival_URL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782884772397_Narrator_Survival.mp3");

private:
    UPROPERTY()
    TArray<FAudio_DinosaurSFX> RegisteredDinosaurSFX;

    UPROPERTY()
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY()
    UAudioComponent* MusicComponent;

    UPROPERTY()
    UAudioComponent* AmbientComponent;

    UPROPERTY()
    UAudioComponent* NarrationComponent;

    float ThreatTransitionTimer = 0.0f;
    float NarrationQueueTimer = 0.0f;
    bool bNarrationQueued = false;
    FString QueuedNarrationURL;
};
