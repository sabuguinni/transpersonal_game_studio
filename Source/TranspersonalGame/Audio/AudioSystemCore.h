#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "AudioSystemCore.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemCore.h
// Adaptive music, ambient layers, voice line registry
// PROD_CYCLE_AUTO_20260630_011
// ============================================================

// ----------------------------------------------------------
// Enums — all prefixed Audio_ to avoid name collisions
// ----------------------------------------------------------

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn    UMETA(DisplayName = "Dawn"),
    Day     UMETA(DisplayName = "Day"),
    Dusk    UMETA(DisplayName = "Dusk"),
    Night   UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Distant     UMETA(DisplayName = "Distant"),
    Approaching UMETA(DisplayName = "Approaching"),
    Immediate   UMETA(DisplayName = "Immediate"),
    Combat      UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    OpenPlains  UMETA(DisplayName = "Open Plains"),
    DenseJungle UMETA(DisplayName = "Dense Jungle"),
    RiverBank   UMETA(DisplayName = "River Bank"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

UENUM(BlueprintType)
enum class EAudio_VoiceLineID : uint8
{
    TRexWarning     UMETA(DisplayName = "TRex Warning"),
    DawnAmbience    UMETA(DisplayName = "Dawn Ambience"),
    QuestFailed     UMETA(DisplayName = "Quest Failed"),
    HerdTracker     UMETA(DisplayName = "Herd Tracker"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    FireMaking      UMETA(DisplayName = "Fire Making")
};

// ----------------------------------------------------------
// Structs
// ----------------------------------------------------------

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome = EAudio_BiomeType::OpenPlains;
};

USTRUCT(BlueprintType)
struct FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float PlayerHealthNormalized = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    bool bPlayerInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    bool bNearCampfire = false;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    EAudio_VoiceLineID LineID = EAudio_VoiceLineID::TRexWarning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString TranscriptText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float DurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bHasBeenPlayed = false;
};

USTRUCT(BlueprintType)
struct FAudio_FreesoundRef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    int32 FreesoundID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    FString PreviewURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    float DurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    FString UsageContext;
};

// ----------------------------------------------------------
// Main Audio System Actor
// ----------------------------------------------------------

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio System Core"))
class TRANSPERSONALGAME_API AAudioSystemCore : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemCore();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- State ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    FAudio_MusicState CurrentMusicState;

    // --- Adaptive Music ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicTransitionSpeed = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float ThreatMusicIntensity = 0.0f;

    // --- Voice Lines Registry ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> VoiceLineRegistry;

    // --- Freesound SFX References ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|SFX")
    TArray<FAudio_FreesoundRef> FreesoundCatalog;

    // --- Audio Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* MusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* VoiceComponent;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    void UpdateMusicState(const FAudio_MusicState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    void SetThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceLine(EAudio_VoiceLineID LineID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void RegisterFreesoundRef(const FAudio_FreesoundRef& SFXRef);

    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    float GetCurrentThreatIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    FString GetCurrentAudioStateDebugString() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Debug")
    void PopulateVoiceLineRegistry();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Debug")
    void PopulateFreesoundCatalog();

private:
    float TickAccumulator = 0.0f;
    float ThreatIntensityTarget = 0.0f;

    void TickAdaptiveMusic(float DeltaTime);
    void ApplyMusicTransition();
};
