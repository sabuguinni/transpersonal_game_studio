#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ── Enums ─────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_AmbientZone : uint8
{
    None        UMETA(DisplayName = "None"),
    Jungle      UMETA(DisplayName = "Jungle"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Camp        UMETA(DisplayName = "Camp"),
    Cave        UMETA(DisplayName = "Cave"),
    OpenPlain   UMETA(DisplayName = "Open Plain"),
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical"),
};

// ── Structs ───────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_AmbientZone Zone = EAudio_AmbientZone::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_NarrativeLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    USoundBase* SoundAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    float DisplayDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString SubtitleText;
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FString DinoSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundBase* RoarSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundBase* FootstepSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundBase* IdleSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float RoarRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FootstepRadius = 2000.0f;
};

// ── Main Actor ────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Ambient Zone System ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Ambient")
    EAudio_AmbientZone CurrentZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float ZoneTransitionSpeed = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetAmbientZone(EAudio_AmbientZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    EAudio_AmbientZone GetCurrentZone() const { return CurrentZone; }

    // ── Threat Level Music System ────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* MusicSafe = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* MusicAware = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* MusicDanger = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* MusicCritical = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Music")
    EAudio_ThreatLevel CurrentThreatLevel;

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    EAudio_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    // ── Narrative Audio ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    TArray<FAudio_NarrativeLine> NarrativeLines;

    UFUNCTION(BlueprintCallable, Category = "Audio|Narrative")
    void PlayNarrativeLine(int32 LineIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narrative")
    void AddNarrativeLine(const FAudio_NarrativeLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narrative")
    int32 GetNarrativeLineCount() const { return NarrativeLines.Num(); }

    // ── Dino Sound Profiles ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    TArray<FAudio_DinoSoundProfile> DinoSoundProfiles;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void PlayDinoRoar(const FString& DinoSpecies, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void PlayDinoFootstep(const FString& DinoSpecies, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void RegisterDinoProfile(const FAudio_DinoSoundProfile& Profile);

    // ── Screen Feedback ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float ScreenShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float TRexShakeRadius = 3000.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerTRexProximityShake(FVector TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageAudioFeedback(float DamageAmount);

    // ── Freesound References (sourced this cycle) ────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Sources")
    FString FreesoundCampfireURL = "https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Sources")
    FString FreesoundRiverURL = "https://cdn.freesound.org/previews/446/446019_7241289-hq.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Sources")
    FString FreesoundForestRiverURL = "https://cdn.freesound.org/previews/442/442478_5902878-hq.mp3";

    // ── ElevenLabs TTS URLs (generated this cycle) ──────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString TTS_NarratorTRexProximity = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782387435843_Narrator_TRex_Proximity.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString TTS_ScoutMiraWater = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782387443164_Scout_Mira_Water.mp3";

private:
    UPROPERTY()
    UAudioComponent* MusicComponent;

    UPROPERTY()
    TArray<UAudioComponent*> AmbientComponents;

    float ThreatTransitionTimer;
    float ZoneTransitionTimer;

    void UpdateAmbientVolumes(float DeltaTime);
    void UpdateMusicTransition(float DeltaTime);
};
