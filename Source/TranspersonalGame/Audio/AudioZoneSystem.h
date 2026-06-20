#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "AudioZoneSystem.generated.h"

// ============================================================
// Audio Zone System — Agent #16 Audio Agent
// Adaptive ambient soundscape zones for MinPlayableMap
// Zones: ForestDay, RiverAmbient, NightPredator, CampFire
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    None            UMETA(DisplayName = "None"),
    ForestDay       UMETA(DisplayName = "Forest Day"),
    ForestNight     UMETA(DisplayName = "Forest Night"),
    RiverAmbient    UMETA(DisplayName = "River Ambient"),
    NightPredator   UMETA(DisplayName = "Night Predator Zone"),
    CampFireSafe    UMETA(DisplayName = "Camp Fire Safe Zone"),
    CaveInterior    UMETA(DisplayName = "Cave Interior"),
    OpenPlain       UMETA(DisplayName = "Open Plain"),
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Extreme     UMETA(DisplayName = "Extreme"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MusicIntensity = 0.0f;

    // Freesound IDs for reference (actual assets loaded via content browser)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString FreesoundInsectNightID = "523435";  // "Insects at Night High Frequency"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString FreesoundCricketsCloseID = "523438"; // "Crickets Close and Distant Night"

    // TTS audio URLs for narrative lines triggered in this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<FString> NarrativeAudioURLs;
};

USTRUCT(BlueprintType)
struct FAudio_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    FString TerrainType = "Grass";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float PitchVariance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    bool bTriggersDustParticle = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAudio_OnZoneChanged, EAudio_ZoneType, NewZone, EAudio_DangerLevel, DangerLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudio_OnDangerLevelChanged, EAudio_DangerLevel, NewDangerLevel);

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Audio Zone System")
class TRANSPERSONALGAME_API UAudioZoneSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioZoneSystemComponent();

    // === Zone Configuration ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    TArray<FAudio_ZoneConfig> ZoneConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    float ZoneTransitionSpeed = 2.0f;

    // === Current State ===
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    EAudio_ZoneType CurrentZone = EAudio_ZoneType::ForestDay;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    float CurrentAmbientVolume = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    float CurrentMusicIntensity = 0.0f;

    // === Footstep Config ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footsteps")
    TArray<FAudio_FootstepConfig> FootstepConfigs;

    // === Delegates ===
    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnZoneChanged OnZoneChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnDangerLevelChanged OnDangerLevelChanged;

    // === TTS Audio URLs (from Agent #15 + #16 production) ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString URL_ForestSilenceWarning = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916883891_QuestNarrator_ForestSilence.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString URL_FireSurvival = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916914702_TribalElder_FireSurvival.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString URL_FirstTools = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916736095_QuestNarrator_FirstTools.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString URL_WaterFirst = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916749037_QuestNarrator_WaterFirst.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString URL_DinoWarningBrachio = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916738386_TribalElder_DinoWarning.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString URL_RaptorTactic = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781916746449_TribalElder_RaptorTactic.mp3";

    // === Functions ===
    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void SetActiveZone(EAudio_ZoneType NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void SetDangerLevel(EAudio_DangerLevel NewDanger);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    EAudio_ZoneType GetCurrentZone() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    EAudio_DangerLevel GetCurrentDangerLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    FAudio_FootstepConfig GetFootstepConfig(const FString& TerrainType) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void UpdateZoneFromPlayerLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    float GetAmbientVolumeForZone(EAudio_ZoneType Zone) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    float GetMusicIntensityForZone(EAudio_ZoneType Zone) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Narrative")
    FString GetNarrativeURLForZone(EAudio_ZoneType Zone) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void InitializeDefaultZoneConfigs();
    void InitializeDefaultFootstepConfigs();
    void BlendAudioParameters(float DeltaTime);

    float TargetAmbientVolume = 1.0f;
    float TargetMusicIntensity = 0.0f;
};
