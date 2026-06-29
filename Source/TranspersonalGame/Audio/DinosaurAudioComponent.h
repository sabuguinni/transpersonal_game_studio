// DinosaurAudioComponent.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260629_002
// Adaptive audio component for dinosaur entities — roars, footsteps, breathing, alerts
// Attaches to dinosaur pawns and drives MetaSound parameters based on AI state

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "DinosaurAudioComponent.generated.h"

// ─── Enums (global scope, Audio_ prefix) ───────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Unknown         UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EAudio_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class EAudio_FootstepWeight : uint8
{
    Light   UMETA(DisplayName = "Light"),   // Raptor, Pterodactyl
    Medium  UMETA(DisplayName = "Medium"),  // Triceratops
    Heavy   UMETA(DisplayName = "Heavy"),   // T-Rex, Brachiosaurus
    Massive UMETA(DisplayName = "Massive")  // Brachiosaurus full weight
};

// ─── Structs (global scope) ────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_DinoSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    EAudio_DinoSpecies Species = EAudio_DinoSpecies::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    EAudio_FootstepWeight FootstepWeight = EAudio_FootstepWeight::Medium;

    // Roar range — how far the roar carries (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float RoarRadius = 5000.0f;

    // Footstep interval in seconds (faster = more frequent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FootstepInterval = 0.8f;

    // Ground shake radius when footstep lands (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float GroundShakeRadius = 2000.0f;

    // Breathing rate — breaths per minute
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float BreathingRate = 12.0f;

    // Volume multiplier for this species
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float VolumeMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_ProximityThreat
{
    GENERATED_BODY()

    // Distance to nearest predator (cm)
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Threat")
    float NearestPredatorDistance = 99999.0f;

    // Is the predator actively hunting the player?
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Threat")
    bool bIsBeingHunted = false;

    // Threat level 0.0–1.0 (drives music intensity)
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Threat")
    float ThreatLevel = 0.0f;

    // Species of nearest threat
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Threat")
    EAudio_DinoSpecies NearestThreatSpecies = EAudio_DinoSpecies::Unknown;
};

// ─── Component ─────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Audio Component")
class TRANSPERSONALGAME_API UDinosaurAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioComponent();

    // ── Configuration ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_DinoSoundProfile SoundProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    EAudio_DinoState CurrentState = EAudio_DinoState::Idle;

    // Distance at which player starts hearing ambient breathing (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float BreathingAudibleRadius = 800.0f;

    // Distance at which footsteps cause camera shake for player (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float FootstepShakeRadius = 1500.0f;

    // Enable low-frequency rumble effect for heavy dinosaurs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    bool bEnableLFRumble = true;

    // ── Runtime State ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float TimeSinceLastRoar = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float TimeSinceLastFootstep = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    bool bPlayerInBreathingRange = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float DistanceToPlayer = 99999.0f;

    // ── Audio Assets (assigned in Blueprint) ──────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* RoarSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* FootstepSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* BreathingSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* AlertSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* FeedingSound = nullptr;

    // ── Public API ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void SetDinoState(EAudio_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void TriggerFootstep();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void TriggerAlert();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void SetSpecies(EAudio_DinoSpecies Species);

    UFUNCTION(BlueprintPure, Category = "Audio|Dino")
    float GetThreatLevelForPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Audio|Dino")
    bool IsPlayerInDanger() const;

    // Returns the sound profile defaults for a given species
    UFUNCTION(BlueprintPure, Category = "Audio|Dino")
    static FAudio_DinoSoundProfile GetDefaultProfileForSpecies(EAudio_DinoSpecies Species);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Internal audio component for continuous sounds (breathing, ambient)
    UPROPERTY()
    UAudioComponent* BreathingAudioComp = nullptr;

    // Cooldown tracking
    float RoarCooldown = 8.0f;
    float FootstepCooldown = 0.0f;

    // Update distance to player each tick
    void UpdatePlayerDistance();

    // Apply state-specific audio behaviour
    void ApplyStateAudio(EAudio_DinoState OldState, EAudio_DinoState NewState);

    // Trigger ground shake via camera shake at player location
    void TriggerGroundShake(float Intensity);

    // Cached player pawn reference
    UPROPERTY()
    APawn* CachedPlayerPawn = nullptr;
};
