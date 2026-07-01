#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// ENUMS — Audio Agent #16 — all prefixed EAudio_ / FAudio_
// ============================================================

UENUM(BlueprintType)
enum class EAudio_EnvironmentZone : uint8
{
    OpenPlains       UMETA(DisplayName = "Open Plains"),
    DenseForest      UMETA(DisplayName = "Dense Forest"),
    RiverBank        UMETA(DisplayName = "River Bank"),
    CampFireSafe     UMETA(DisplayName = "Campfire Safe Zone"),
    DinosaurTerritory UMETA(DisplayName = "Dinosaur Territory"),
    Cave             UMETA(DisplayName = "Cave"),
    NightDanger      UMETA(DisplayName = "Night Danger")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe      UMETA(DisplayName = "Safe"),
    Cautious  UMETA(DisplayName = "Cautious"),
    Tense     UMETA(DisplayName = "Tense"),
    Critical  UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_NarratorTrigger : uint8
{
    GameStart        UMETA(DisplayName = "Game Start"),
    DinosaurNearby   UMETA(DisplayName = "Dinosaur Nearby"),
    NightFalling     UMETA(DisplayName = "Night Falling"),
    PlayerLowHealth  UMETA(DisplayName = "Player Low Health"),
    QuestComplete    UMETA(DisplayName = "Quest Complete"),
    FirstCraft       UMETA(DisplayName = "First Craft"),
    PlayerDeath      UMETA(DisplayName = "Player Death")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxAudibleDistance = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    FAudio_SoundEntry() {}
};

USTRUCT(BlueprintType)
struct FAudio_NarratorLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    EAudio_NarratorTrigger TriggerType = EAudio_NarratorTrigger::GameStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString TranscriptText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    float CooldownSeconds = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    bool bHasPlayed = false;

    FAudio_NarratorLine() {}
};

USTRUCT(BlueprintType)
struct FAudio_EnvironmentState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    EAudio_EnvironmentZone CurrentZone = EAudio_EnvironmentZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    bool bIsNight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float NearestDinosaurDistance = 9999.0f;

    FAudio_EnvironmentState() {}
};

// ============================================================
// COMPONENT
// ============================================================

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Audio System Manager")
class TRANSPERSONALGAME_API UAudio_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    // --- Environment State ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    FAudio_EnvironmentState CurrentEnvironmentState;

    // --- Narrator Lines ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    TArray<FAudio_NarratorLine> NarratorLines;

    // --- Sound Library ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Library")
    TArray<FAudio_SoundEntry> SoundLibrary;

    // --- Campfire Sound ID ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Campfire")
    FName CampfireSoundID = FName("Campfire_Loop");

    // --- Danger Music Intensity (0-1) ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Music",
        meta = (AllowPrivateAccess = "true"))
    float MusicDangerIntensity = 0.0f;

    // --- Delegates ---
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudio_OnZoneChanged, EAudio_EnvironmentZone, NewZone);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudio_OnDangerLevelChanged, EAudio_DangerLevel, NewLevel);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudio_OnNarratorTriggered, EAudio_NarratorTrigger, TriggerType);

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnZoneChanged OnZoneChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnDangerLevelChanged OnDangerLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnNarratorTriggered OnNarratorTriggered;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateEnvironmentZone(EAudio_EnvironmentZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateNearestDinosaurDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerNarratorLine(EAudio_NarratorTrigger Trigger);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTimeOfDay(float TimeHours);

    UFUNCTION(BlueprintPure, Category = "Audio")
    float GetMusicDangerIntensity() const;

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool IsNight() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterNarratorLine(EAudio_NarratorTrigger Trigger, const FString& AudioURL, const FString& Transcript, float Cooldown);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(FName SoundID, float Volume, float MaxDistance, bool bLoop);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void RecalculateMusicIntensity();
    void InitializeDefaultNarratorLines();

    float NarratorCooldownTimer = 0.0f;
    static constexpr float NIGHT_START_HOUR = 19.0f;
    static constexpr float NIGHT_END_HOUR   = 6.0f;
    static constexpr float TREX_DANGER_RADIUS = 600.0f;
};
