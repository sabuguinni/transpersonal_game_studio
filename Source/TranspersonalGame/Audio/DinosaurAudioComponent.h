#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "DinosaurAudioComponent.generated.h"

/**
 * Enum for dinosaur audio state — drives MetaSound parameter selection.
 * Prefix: EAudio_ to avoid collision with other agents.
 */
UENUM(BlueprintType)
enum class EAudio_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Dead        UMETA(DisplayName = "Dead")
};

/**
 * Enum for dinosaur species — each species has a distinct audio profile.
 */
UENUM(BlueprintType)
enum class EAudio_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Generic             UMETA(DisplayName = "Generic")
};

/**
 * Struct holding the full audio profile for a dinosaur species.
 * Assigned per-species in Blueprint or DataAsset.
 */
USTRUCT(BlueprintType)
struct FAudio_DinoAudioProfile
{
    GENERATED_BODY()

    /** Ambient idle vocalisation — looping, low volume */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundCue* IdleVocalisation = nullptr;

    /** Alert call — played once when dinosaur detects a threat */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundCue* AlertCall = nullptr;

    /** Attack roar — played at the moment of charge/strike */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundCue* AttackRoar = nullptr;

    /** Death sound — played once on death */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundCue* DeathSound = nullptr;

    /** Footstep — played per-step via animation notify */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    USoundCue* Footstep = nullptr;

    /** Ground shake radius for heavy species (T-Rex, Brachio) in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float GroundShakeRadius = 1500.0f;

    /** Volume multiplier for this species */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float VolumeMultiplier = 1.0f;

    /** Attenuation distance — how far the player can hear this dino */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino", meta = (ClampMin = "500.0", ClampMax = "20000.0"))
    float AttenuationMaxDistance = 8000.0f;
};

/**
 * UAudio_DinosaurAudioComponent
 *
 * Attached to every dinosaur pawn. Manages:
 * - State-driven vocalisation (idle/alert/attack/death)
 * - Footstep audio via animation notify
 * - Ground shake propagation to nearby players
 * - Proximity-based audio intensity (fear factor)
 *
 * Designed to work with MetaSounds in UE5.3+.
 * Falls back to SoundCue if MetaSound asset not assigned.
 */
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Audio Component")
class TRANSPERSONALGAME_API UAudio_DinosaurAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DinosaurAudioComponent();

    // ─── Species & Profile ────────────────────────────────────────────────────

    /** Species determines which audio profile is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino|Config")
    EAudio_DinoSpecies Species = EAudio_DinoSpecies::Generic;

    /** Full audio profile for this dinosaur — assign in Blueprint or DataAsset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino|Config")
    FAudio_DinoAudioProfile AudioProfile;

    // ─── Runtime State ────────────────────────────────────────────────────────

    /** Current behavioural state — drives which audio plays */
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dino|State")
    EAudio_DinoState CurrentState = EAudio_DinoState::Idle;

    /** Distance to nearest player — updated every tick (throttled) */
    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dino|State")
    float DistanceToPlayer = 99999.0f;

    // ─── Public Interface ─────────────────────────────────────────────────────

    /**
     * Called by AI Behavior Tree when dino state changes.
     * Transitions audio from current state to new state.
     */
    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void SetDinoState(EAudio_DinoState NewState);

    /**
     * Called by Animation Notify on each footstep.
     * Plays footstep sound and triggers ground shake if applicable.
     */
    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void PlayFootstep();

    /**
     * Called by Combat system on death.
     * Stops all looping audio, plays death sound once.
     */
    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void PlayDeathAudio();

    /**
     * Returns a 0-1 fear intensity value based on species, state, and distance.
     * Used by the HUD and player character to modulate heartbeat/breathing audio.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio|Dino")
    float GetFearIntensity() const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    /** Active audio component for looping vocalisations */
    UPROPERTY()
    UAudioComponent* LoopingAudioComp = nullptr;

    /** Accumulated time since last player distance update */
    float DistanceUpdateAccumulator = 0.0f;

    /** How often to update player distance (seconds) */
    static constexpr float DistanceUpdateInterval = 0.25f;

    /** Stops any currently playing looping vocalisation */
    void StopLoopingAudio();

    /** Starts a new looping vocalisation for the given state */
    void StartLoopingAudio(USoundCue* SoundCue);

    /** Finds the nearest player pawn in the world */
    APawn* FindNearestPlayer() const;

    /** Propagates ground shake to all players within GroundShakeRadius */
    void PropagateGroundShake();
};
