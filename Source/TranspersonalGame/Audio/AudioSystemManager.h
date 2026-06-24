#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

/**
 * Audio zone type — defines what ambient soundscape plays in a region.
 */
UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    None            UMETA(DisplayName = "None"),
    TRexTerritory   UMETA(DisplayName = "T-Rex Territory"),
    RaptorPack      UMETA(DisplayName = "Raptor Pack Zone"),
    RiverAmbience   UMETA(DisplayName = "River Ambience"),
    ForestCanopy    UMETA(DisplayName = "Forest Canopy"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    Danger          UMETA(DisplayName = "Danger — Predator Near"),
};

/**
 * Struct describing a single audio zone in the world.
 */
USTRUCT(BlueprintType)
struct FAudio_ZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float Radius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bIsActive = true;
};

/**
 * Struct for a single narration/dialogue voice line.
 */
USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bHasBeenPlayed = false;
};

/**
 * AAudio_SystemManager — manages all ambient audio zones, voice lines,
 * and adaptive music state for the prehistoric survival world.
 *
 * Placed once in the level. Detects player proximity to zones and
 * triggers appropriate soundscapes. Integrates with ElevenLabs TTS
 * voice lines stored as FAudio_VoiceLine entries.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === Zone Management ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void RegisterAudioZone(const FAudio_ZoneData& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    EAudio_ZoneType GetActiveZoneForLocation(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void SetZoneActive(EAudio_ZoneType ZoneType, bool bActive);

    // === Voice Lines ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    bool TriggerVoiceLine(const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void TriggerRandomSurvivalHint();

    // === Adaptive Music ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetDangerLevel(float DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    float GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void OnDinosaurNearby(float DistanceMeters, bool bIsPredator);

    // === Screen Shake ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
    void TriggerFootstepShake(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
    void TriggerDamageFlash();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    TArray<FAudio_ZoneData> RegisteredZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> VoiceLines;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Music",
        meta = (AllowPrivateAccess = "true"))
    float CurrentDangerLevel = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zones",
        meta = (AllowPrivateAccess = "true"))
    EAudio_ZoneType CurrentActiveZone = EAudio_ZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float DangerDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float DangerThresholdHigh = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float DangerThresholdMedium = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    float ZoneCheckInterval = 1.0f;

private:
    float ZoneCheckTimer = 0.0f;
    float VoiceLineCooldown = 0.0f;
    static constexpr float VoiceLineCooldownDuration = 30.0f;

    void UpdateZoneDetection(const FVector& PlayerLocation);
    void UpdateDangerDecay(float DeltaTime);
};
