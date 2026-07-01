#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Cave        UMETA(DisplayName = "Cave"),
    River       UMETA(DisplayName = "River"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FAudio_AmbienceLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* SoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType Biome = EAudio_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;
};

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Magnitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TriggerRadius = 2000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Ambience ──────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambience")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambience")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambience")
    void SetDangerLevel(EAudio_DangerLevel NewDanger);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambience")
    void CrossfadeAmbience(USoundCue* NewCue, float FadeTime = 2.0f);

    // ── Screen Shake ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerTRexScreenShake(FVector TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerImpactShake(FVector ImpactLocation, float Magnitude);

    // ── Danger Music ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void UpdateMusicTension(float TensionValue);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void PlayStingerOneShot(USoundCue* Stinger);

    // ── Footstep System ───────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Footstep")
    void PlayDinosaurFootstep(FVector FootLocation, float DinosaurMass);

    UFUNCTION(BlueprintCallable, Category = "Audio|Footstep")
    void PlayPlayerFootstep(FVector FootLocation, bool bRunning);

    // ── State ─────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    EAudio_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    EAudio_DangerLevel CurrentDangerLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    float CurrentTension;

    // ── Config ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_ScreenShakeConfig TRexShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbienceVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    // ── Audio URLs (ElevenLabs TTS — loaded at runtime) ───────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narration")
    FString NarratorSurvivalURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782901293064_Narrator_Survival.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narration")
    FString ElderTribeURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782901309057_Elder_Tribe.mp3");

    // ── Freesound references ──────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambience")
    FString JungleNightFreesoundID = TEXT("479553"); // R15-77-Jungle Night.wav

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambience")
    FString TropicalNightFreesoundID = TEXT("516157"); // Tropical Ambience Night Ecuador.wav

private:
    UPROPERTY()
    UAudioComponent* AmbienceComponentA;

    UPROPERTY()
    UAudioComponent* AmbienceComponentB;

    UPROPERTY()
    UAudioComponent* MusicComponent;

    bool bAmbienceOnA = true;
    float TensionBlendAlpha = 0.0f;

    void InitAudioComponents();
    void UpdateAmbienceTick(float DeltaTime);
    void ApplyVolumeSettings();
    float ComputeShakeMagnitude(FVector SourceLocation, FVector PlayerLocation, float TriggerRadius);
};
