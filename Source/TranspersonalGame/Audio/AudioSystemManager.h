// AudioSystemManager.h
// Agent #16 — Audio Agent
// Adaptive audio system for prehistoric survival game
// Manages ambient zones, danger proximity audio, and MetaSound integration

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// Audio zone types for the prehistoric world
UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    OpenPlains       UMETA(DisplayName = "Open Plains"),
    DenseForest      UMETA(DisplayName = "Dense Forest"),
    RiverBank        UMETA(DisplayName = "River Bank"),
    CaveEntrance     UMETA(DisplayName = "Cave Entrance"),
    CaveInterior     UMETA(DisplayName = "Cave Interior"),
    DinosaurTerritory UMETA(DisplayName = "Dinosaur Territory"),
    CampSite         UMETA(DisplayName = "Camp Site"),
    Danger           UMETA(DisplayName = "Danger Zone")
};

// Danger level that drives music intensity
UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe             UMETA(DisplayName = "Safe"),
    Aware            UMETA(DisplayName = "Aware"),
    Stalked          UMETA(DisplayName = "Stalked"),
    Imminent         UMETA(DisplayName = "Imminent"),
    Combat           UMETA(DisplayName = "Combat")
};

// Dinosaur species for species-specific audio
UENUM(BlueprintType)
enum class EAudio_DinoSpecies : uint8
{
    TRex             UMETA(DisplayName = "T-Rex"),
    Raptor           UMETA(DisplayName = "Raptor"),
    Brachiosaurus    UMETA(DisplayName = "Brachiosaurus"),
    Triceratops      UMETA(DisplayName = "Triceratops"),
    Pterodactyl      UMETA(DisplayName = "Pterodactyl"),
    Generic          UMETA(DisplayName = "Generic")
};

// Ambient audio layer data
USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::OpenPlains;
};

// Dinosaur proximity audio event
USTRUCT(BlueprintType)
struct FAudio_DinoProximityEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_DinoSpecies Species = EAudio_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ProximityRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DangerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ScreenShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bCausesGroundRumble = false;
};

// NPC voice line entry
USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SubtitleText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIsPlayed = false;
};

/**
 * UAudio_SystemManager
 * Central audio manager for the prehistoric survival game.
 * Handles adaptive music, ambient zones, dinosaur proximity audio,
 * and NPC voice line playback.
 */
UCLASS(ClassGroup = "TranspersonalGame", meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API UAudio_SystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    // Current danger level driving music intensity
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    // Current ambient zone
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    EAudio_ZoneType CurrentZone = EAudio_ZoneType::OpenPlains;

    // Active ambient layers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    // Registered dinosaur proximity events
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    TArray<FAudio_DinoProximityEvent> DinoProximityEvents;

    // Voice line registry (loaded from ElevenLabs URLs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> VoiceLineRegistry;

    // Master volume multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume = 1.0f;

    // Music volume multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume = 0.7f;

    // SFX volume multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SFXVolume = 1.0f;

    // Ambient volume multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume = 0.8f;

    // Update danger level based on nearest predator distance
    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void UpdateDangerLevel(EAudio_DangerLevel NewLevel);

    // Transition ambient zone with crossfade
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void TransitionToZone(EAudio_ZoneType NewZone, float CrossfadeTime = 2.0f);

    // Trigger dinosaur proximity audio event
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaurs")
    void TriggerDinoProximity(EAudio_DinoSpecies Species, float Distance);

    // Play a voice line by ID
    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    bool PlayVoiceLine(const FString& LineID);

    // Register a voice line from ElevenLabs URL
    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine);

    // Get current music intensity (0.0 - 1.0) based on danger
    UFUNCTION(BlueprintPure, Category = "Audio|Music")
    float GetMusicIntensity() const;

    // Check if a specific voice line has been played
    UFUNCTION(BlueprintPure, Category = "Audio|Voice")
    bool HasVoiceLinePlayed(const FString& LineID) const;

    // Reset all played voice lines (e.g., on new game)
    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void ResetVoiceLines();

    // Get danger level as float for MetaSound parameter
    UFUNCTION(BlueprintPure, Category = "Audio|Danger")
    float GetDangerLevelFloat() const;

private:
    // Internal danger level to intensity mapping
    float DangerLevelToIntensity(EAudio_DangerLevel Level) const;
};

/**
 * AAudio_AmbientZoneActor
 * Placed in the world to define ambient audio zones.
 * When the player enters, triggers zone transition.
 */
UCLASS(ClassGroup = "TranspersonalGame", meta = (DisplayName = "Ambient Audio Zone"))
class TRANSPERSONALGAME_API AAudio_AmbientZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZoneActor();

    // Zone type this actor represents
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::OpenPlains;

    // Radius of this audio zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone", meta = (ClampMin = "100.0"))
    float ZoneRadius = 3000.0f;

    // Crossfade time when entering/leaving this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone", meta = (ClampMin = "0.1"))
    float CrossfadeTime = 2.0f;

    // Priority (higher = overrides lower priority zones)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    int32 Priority = 0;

    // Ambient sound to play in this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    USoundBase* ZoneAmbientSound = nullptr;

    // Audio component for this zone
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    UAudioComponent* AudioComponent = nullptr;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Called when player enters zone radius
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone();

    // Called when player exits zone radius
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone();

    // Is player currently inside this zone?
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    bool bPlayerInZone = false;

private:
    float DistanceToPlayer = 0.0f;
    void CheckPlayerDistance();
};
