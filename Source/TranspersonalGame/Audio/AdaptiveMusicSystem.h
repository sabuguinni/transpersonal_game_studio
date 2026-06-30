#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AdaptiveMusicSystem.generated.h"

/**
 * Adaptive music states for the prehistoric survival game.
 * Transitions are driven by gameplay events: dinosaur proximity, combat, exploration.
 */
UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Exploration     UMETA(DisplayName = "Exploration"),      // Calm, ambient — player is safe
    Tension         UMETA(DisplayName = "Tension"),          // Dinosaur detected within 300m
    Danger          UMETA(DisplayName = "Danger"),           // Dinosaur within 80m or combat active
    Combat          UMETA(DisplayName = "Combat"),           // Active attack — player or ally targeted
    Night           UMETA(DisplayName = "Night"),            // Night phase — heightened predator ambience
    Camp            UMETA(DisplayName = "Camp"),             // Player at campfire — safe zone music
    Death           UMETA(DisplayName = "Death"),            // Player death sting
    Discovery       UMETA(DisplayName = "Discovery")         // New area / resource found — brief swell
};

UENUM(BlueprintType)
enum class EAudio_DinoThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Distant     UMETA(DisplayName = "Distant"),    // >300m
    Near        UMETA(DisplayName = "Near"),        // 80-300m
    Immediate   UMETA(DisplayName = "Immediate"),  // <80m
    Attacking   UMETA(DisplayName = "Attacking")   // Active aggression
};

USTRUCT(BlueprintType)
struct FAudio_MusicLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* SoundAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float TargetVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    bool bLoop = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    FString AssetPath;  // Path to the sound asset in Content Browser
};

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float MaxDistance = 2000.0f;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;  // ElevenLabs TTS URL — loaded at runtime

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    EAudio_MusicState TriggerState = EAudio_MusicState::Exploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float CooldownSeconds = 120.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Voice")
    float LastPlayedTime = -999.0f;
};

/**
 * UAudio_AdaptiveMusicSystem
 *
 * Manages the adaptive music and audio feedback for the prehistoric survival game.
 * Transitions between music states based on gameplay events.
 * Integrates with NarrativeDialogueManager for voice line playback.
 *
 * Architecture:
 * - Music layers fade in/out based on EAudio_MusicState
 * - Screen shake triggered by T-Rex footsteps (distance-based)
 * - Damage flash audio cue on player hit
 * - Voice lines registered from TTS URLs and played on trigger
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === Music State Control ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetMusicState(EAudio_MusicState NewState, float CrossfadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    EAudio_MusicState GetCurrentMusicState() const { return CurrentMusicState; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnDinosaurThreatChanged(EAudio_DinoThreatLevel NewThreatLevel, FName DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnPlayerDamaged(float DamageAmount, FVector DamageSourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnPlayerEnteredCamp();

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnPlayerLeftCamp();

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnNightfallBegin();

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnDawnBegin();

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnPlayerDeath();

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnDiscovery();

    // === Screen Shake ===

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerTRexFootstepShake(FVector TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerDamageShake(float DamageAmount);

    // === Voice Lines ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FAudio_VoiceLineEntry& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceLine(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterTTSVoiceLines();

    // === Ambient Audio ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetAmbientLayer(const FString& LayerName, float TargetVolume, float FadeTime = 1.5f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void UpdateAmbientForTimeOfDay(float NormalizedTimeOfDay);

    // === Freesound Asset Registration ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Assets")
    void RegisterFreesoundAsset(const FString& AssetName, const FString& PreviewURL, const FString& Tags);

    // === Debug ===

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Debug")
    void LogAudioSystemState();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_MusicState CurrentMusicState = EAudio_MusicState::Exploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_MusicState PreviousMusicState = EAudio_MusicState::Exploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeParams TRexShakeParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeParams DamageShakeParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLineEntry> RegisteredVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float StateCooldown = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MinStateHoldTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_DinoThreatLevel CurrentThreatLevel = EAudio_DinoThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    FName DominantDinoSpecies = NAME_None;

    // Registered Freesound assets (preview URLs for runtime streaming)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    TMap<FString, FString> FreesoundAssetURLs;

    // TTS voice line URLs from ElevenLabs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TMap<FString, FString> TTSVoiceLineURLs;

private:
    void EvaluateMusicStateTransition();
    void ApplyScreenShake(float Scale, float Duration);
    float GetCurrentWorldTime() const;
};
