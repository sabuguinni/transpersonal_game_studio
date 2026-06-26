#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "AudioZoneSystem.generated.h"

// ============================================================
// Audio Zone System — Agent #16 Audio Agent
// Adaptive ambient audio zones for prehistoric survival game.
// Zones respond to player proximity, time of day, and dino state.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Jungle          UMETA(DisplayName = "Jungle Ambience"),
    Campfire        UMETA(DisplayName = "Campfire Zone"),
    TRexDanger      UMETA(DisplayName = "T-Rex Danger Zone"),
    River           UMETA(DisplayName = "River Ambience"),
    Cave            UMETA(DisplayName = "Cave Interior"),
    OpenPlain       UMETA(DisplayName = "Open Plain"),
    NightTime       UMETA(DisplayName = "Night Ambience"),
    Storm           UMETA(DisplayName = "Thunderstorm")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe            UMETA(DisplayName = "Safe"),
    Caution         UMETA(DisplayName = "Caution"),
    Danger          UMETA(DisplayName = "Danger"),
    Critical        UMETA(DisplayName = "Critical — Flee Now")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;

    // Freesound reference IDs for this zone (from search results)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<int32> FreesoundIDs;

    // TTS audio URL for narration lines in this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString NarrationAudioURL;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float Duration = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    bool bHasBeenPlayed = false;
};

/**
 * UAudio_ZoneSystem
 * ActorComponent that manages adaptive audio zones for the prehistoric world.
 * Attach to a zone trigger actor. Blends ambient sounds based on player distance.
 * Integrates with Agent #15 DialogueSystem for triggered narration.
 */
UCLASS(ClassGroup = "TranspersonalGame|Audio", meta = (BlueprintSpawnableComponent), DisplayName = "Audio Zone System")
class TRANSPERSONALGAME_API UAudio_ZoneSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneSystem();

    // Zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    // Current danger level — set by AI/Combat agents
    UPROPERTY(BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    // Current volume (0-1), driven by player proximity
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    float CurrentVolume = 0.0f;

    // Is player currently inside this zone?
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    bool bPlayerInZone = false;

    // Dialogue entries registered for this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    TArray<FAudio_DialogueEntry> DialogueEntries;

    // Audio URLs from TTS generation (Agent #16 cycle outputs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString JungleNarrationURL;   // Narrator_Jungle_Ambience TTS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|TTS")
    FString CombatWarningURL;     // Kael_Combat_Warning TTS

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Called by proximity trigger when player enters zone
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone();

    // Called by proximity trigger when player exits zone
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone();

    // Update danger level from AI/Combat systems
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    // Get current zone type as string (for UI/debug)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio|Zone")
    FString GetZoneTypeString() const;

    // Get volume multiplier based on danger level
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio|Zone")
    float GetDangerVolumeMultiplier() const;

    // Register a dialogue entry for this zone
    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void RegisterDialogueLine(const FString& Speaker, const FString& Text, const FString& AudioURL, float Duration);

    // Get next unplayed dialogue entry (returns false if none available)
    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    bool GetNextDialogueLine(FAudio_DialogueEntry& OutEntry);

    // Mark a dialogue line as played
    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void MarkDialoguePlayed(const FString& Speaker);

    // Seed default TTS URLs from Agent #16 production cycle
    UFUNCTION(BlueprintCallable, Category = "Audio|TTS")
    void SeedDefaultAudioURLs();

private:
    float TargetVolume = 0.0f;
    float VolumeVelocity = 0.0f;
    float TickAccumulator = 0.0f;
    static constexpr float TickInterval = 0.1f; // 10Hz
};
