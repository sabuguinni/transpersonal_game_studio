#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Hilltop     UMETA(DisplayName = "Hilltop"),
    Cave        UMETA(DisplayName = "Cave"),
    Clearing    UMETA(DisplayName = "Clearing"),
    HuntingGround UMETA(DisplayName = "HuntingGround"),
    RockShelter UMETA(DisplayName = "RockShelter"),
    WaterHole   UMETA(DisplayName = "WaterHole")
};

UENUM(BlueprintType)
enum class EAudio_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "TRex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Unknown     UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    EAudio_DinoSpecies Species = EAudio_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FString RoarSoundCuePath = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FString FootstepSoundCuePath = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FString IdleBreathPath = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float RoarVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FootstepVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float RoarCooldownSeconds = 8.0f;
};

USTRUCT(BlueprintType)
struct FAudio_ZoneAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Clearing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString AmbienceSoundCuePath = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString VoiceBriefURL = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbienceVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FAudio_DangerCue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    EAudio_DinoSpecies TriggerSpecies = EAudio_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    FString VoiceWarningURL = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float TriggerRadiusMeters = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    bool bHasTriggered = false;
};

/**
 * AAudio_ZoneManager
 * Manages ambient audio zones and dinosaur sound profiles in MinPlayableMap.
 * Tracks player proximity to dino placeholders and triggers danger voice cues.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === Zone Ambience ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    TArray<FAudio_ZoneAmbience> ZoneAmbienceProfiles;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void InitialiseZoneAmbience();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void ActivateZone(EAudio_ZoneType ZoneType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zones")
    void DeactivateZone(EAudio_ZoneType ZoneType);

    // === Dino Sound Profiles ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinos")
    TArray<FAudio_DinoSoundProfile> DinoSoundProfiles;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinos")
    void InitialiseDinoProfiles();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinos")
    FAudio_DinoSoundProfile GetProfileForSpecies(EAudio_DinoSpecies Species) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinos")
    void TriggerDinoRoar(EAudio_DinoSpecies Species, FVector Location);

    // === Danger Voice Cues ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    TArray<FAudio_DangerCue> DangerCues;

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void InitialiseDangerCues();

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void CheckProximityDangerCues(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void ResetAllDangerCues();

    // === Audio State ===

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    EAudio_ZoneType CurrentActiveZone = EAudio_ZoneType::Clearing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    float ProximityCheckInterval = 2.0f;

private:
    float TimeSinceLastProximityCheck = 0.0f;

    UPROPERTY()
    UAudioComponent* ActiveAmbienceComponent = nullptr;
};
