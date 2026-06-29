#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Adaptive audio system: ambient layers, proximity triggers,
// dinosaur state-driven music, campfire/night ambience
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Aware       UMETA(DisplayName = "Aware"),
    Tense       UMETA(DisplayName = "Tense"),
    Chase       UMETA(DisplayName = "Chase"),
    Combat      UMETA(DisplayName = "Combat"),
    Safe        UMETA(DisplayName = "Safe")
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
    OpenPlains  UMETA(DisplayName = "Open Plains"),
    DenseForest UMETA(DisplayName = "Dense Forest"),
    RiverBank   UMETA(DisplayName = "River Bank"),
    RockyRidge  UMETA(DisplayName = "Rocky Ridge"),
    Swamp       UMETA(DisplayName = "Swamp")
};

USTRUCT(BlueprintType)
struct FAudio_ProximityTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float TriggerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float FadeInTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float FadeOutTime = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    bool bIs3D = true;
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome = EAudio_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float CrossfadeDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString FreesoundPreviewURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString SoundAssetPath;
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
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float PlaybackVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bSubtitlesEnabled = true;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ProximityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ProximityComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    FAudio_ProximityTrigger ProximitySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    USoundBase* ProximitySound = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Proximity", meta = (AllowPrivateAccess = "true"))
    bool bPlayerInRange = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Proximity", meta = (AllowPrivateAccess = "true"))
    float CurrentVolume = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Proximity")
    void UpdateProximityVolume(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Audio|Proximity")
    void SetProximitySound(USoundBase* NewSound);

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    UAudioComponent* AudioComp = nullptr;
};

UCLASS()
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    // Current audio state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::OpenPlains;

    // Registered voice lines (TTS-generated)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> RegisteredVoiceLines;

    // Ambient layers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    // Campfire sound (Freesound ID 681366)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Campfire")
    FString CampfireFreesoundURL = TEXT("https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3");

    // Threat level transition times (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicTransitionTime = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float ThreatEscalationDelay = 1.5f;

    // State change functions
    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceLine(const FString& CharacterName, const FString& TriggerCondition);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void CrossfadeToAmbientLayer(EAudio_BiomeType Biome, EAudio_TimeOfDay Time);

    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    void LogAudioState() const;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    void InitializeDefaultVoiceLines();
    void InitializeDefaultAmbientLayers();
    void UpdateMusicState(float DeltaTime);

private:
    UPROPERTY()
    UAudioComponent* MusicComponent = nullptr;

    UPROPERTY()
    UAudioComponent* AmbientComponent = nullptr;

    float ThreatTransitionTimer = 0.0f;
    EAudio_ThreatLevel PendingThreatLevel = EAudio_ThreatLevel::Calm;
    bool bTransitioningThreat = false;
};
