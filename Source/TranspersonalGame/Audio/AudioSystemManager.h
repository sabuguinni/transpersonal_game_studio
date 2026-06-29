#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Adaptive audio system for prehistoric survival game
// Manages ambient zones, danger music, dialogue audio, SFX
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Camp            UMETA(DisplayName = "Camp — Safe Zone"),
    Forest          UMETA(DisplayName = "Forest — Neutral"),
    DangerNear      UMETA(DisplayName = "Danger — Predator Nearby"),
    Combat          UMETA(DisplayName = "Combat — Active Threat"),
    Night           UMETA(DisplayName = "Night — High Tension"),
    Water           UMETA(DisplayName = "Water — River/Lake"),
    OpenPlain       UMETA(DisplayName = "Open Plain — Exposed")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    None        UMETA(DisplayName = "No Danger"),
    Low         UMETA(DisplayName = "Low — Distant predator"),
    Medium      UMETA(DisplayName = "Medium — Predator in range"),
    High        UMETA(DisplayName = "High — Predator charging"),
    Critical    UMETA(DisplayName = "Critical — Under attack")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundCue* SoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    USoundCue* SoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    bool bSubtitleEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString SubtitleText;
};

USTRUCT(BlueprintType)
struct FAudio_SFXEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    FName SFXId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    USoundCue* SoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    float PitchVariationMin = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    float PitchVariationMax = 1.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    bool bIs3D = true;
};

// ============================================================
// Main Audio System Manager Actor
// Place one in the level — manages all audio state transitions
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Audio")
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Zone & Danger State ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_ZoneType CurrentZone = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    float DangerProximityRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    float MusicTransitionSpeed = 1.5f;

    // ---- Ambient Layers ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float NightAmbientVolumeMultiplier = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float DayAmbientVolumeMultiplier = 1.0f;

    // ---- Dialogue Audio ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    TArray<FAudio_DialogueLine> RegisteredDialogueLines;

    // Pre-wired TTS URLs from Agent #15 Narrative
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue|TTS_URLs")
    FString TTS_HunterLeader_MainQuest = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782763482834_HunterLeader_MainQuest.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue|TTS_URLs")
    FString TTS_CampElder_Lore = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782763487267_CampElder_Lore.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue|TTS_URLs")
    FString TTS_Scout_NPC_Warning = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782763514029_Scout_NPC_Warning.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue|TTS_URLs")
    FString TTS_TribalElder_StoneValley = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782763518862_TribalElder_StoneValleyLore.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue|TTS_URLs")
    FString TTS_CampNarrator_NightWarning = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782763617187_CampNarrator_NightWarning.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue|TTS_URLs")
    FString TTS_HunterLeader_TRexWarning = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782763636699_HunterLeader_TRexWarning.mp3");

    // ---- SFX Registry ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    TArray<FAudio_SFXEntry> SFXRegistry;

    // Freesound asset IDs catalogued this cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX|Freesound")
    int32 FreesoundID_Campfire_Pos1 = 681366;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX|Freesound")
    int32 FreesoundID_Campfire_Pos2 = 681367;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX|Freesound")
    int32 FreesoundID_Campfire_Dusk = 688992;

    // ---- Audio Component ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    // ---- Blueprint-callable API ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetAudioZone(EAudio_ZoneType NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void SetDangerLevel(EAudio_DangerLevel NewDanger);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void UpdateDangerFromProximity(float ClosestPredatorDistance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayDialogueLine(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlaySFX(FName SFXId, FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlayFootstepSound(bool bIsRunning, bool bIsOnStone);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlayCraftingSound(FName ToolType);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    EAudio_ZoneType GetCurrentZone() const { return CurrentZone; }

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(CallInEditor, Category = "Audio|Debug")
    void LogAudioState();

private:
    float CurrentMusicVolume = 1.0f;
    float TargetMusicVolume = 1.0f;
    EAudio_ZoneType PreviousZone = EAudio_ZoneType::Forest;
    EAudio_DangerLevel PreviousDangerLevel = EAudio_DangerLevel::None;

    void TransitionAmbientLayer(EAudio_ZoneType FromZone, EAudio_ZoneType ToZone);
    void ApplyDangerMusicIntensity(EAudio_DangerLevel DangerLevel);
};
