#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioZoneSystem.generated.h"

// Audio zone type — controls which ambient layer is active
UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    None        UMETA(DisplayName = "None"),
    Camp        UMETA(DisplayName = "Camp"),       // Campfire, voices, safety
    Forest      UMETA(DisplayName = "Forest"),     // Wind, insects, birds
    Danger      UMETA(DisplayName = "Danger"),     // Tension sting, heartbeat
    Combat      UMETA(DisplayName = "Combat"),     // Percussion, chaos
    Night       UMETA(DisplayName = "Night"),      // Crickets, distant howls
    TRexProximity UMETA(DisplayName = "TRexProximity") // Ground rumble, silence of prey
};

// Describes a single ambient audio layer
USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName LayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::None;

    // Volume 0.0 - 1.0
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Volume = 1.0f;

    // Fade-in time in seconds when zone becomes active
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInDuration = 2.0f;

    // Fade-out time in seconds when zone becomes inactive
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutDuration = 3.0f;

    // Whether this layer loops
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    // Freesound asset reference ID (for documentation)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString FreesoundID;
};

// Describes a one-shot audio event (dialogue sting, danger alert)
USTRUCT(BlueprintType)
struct FAudio_EventCue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName CueID;

    // Game event that triggers this cue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName TriggerEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Volume = 0.8f;

    // Delay after trigger event fires (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerDelay = 0.0f;

    // Duration of the cue (seconds) — should match dialogue line DisplayDuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration = 4.0f;
};

/**
 * UAudio_ZoneComponent
 * ActorComponent that manages ambient audio zone transitions and event cues.
 * Attach to AmbientSound actors or zone trigger volumes.
 * Drives MetaSound parameter blocks when MetaSounds are available.
 */
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), DisplayName = "Audio Zone Component")
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    // Current active zone type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ActiveZone = EAudio_ZoneType::Forest;

    // Registered ambient layers for this zone actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layers")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    // Registered one-shot event cues
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Events")
    TArray<FAudio_EventCue> EventCues;

    // Radius (cm) within which this zone is audible
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ZoneRadius = 1500.0f;

    // Whether zone transitions are currently enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bZoneActive = true;

    // Transition to a new zone type with crossfade
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void TransitionToZone(EAudio_ZoneType NewZone, float CrossfadeDuration = 2.0f);

    // Fire a named event cue immediately
    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void FireEventCue(FName CueID);

    // Get current zone type
    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_ZoneType GetActiveZone() const { return ActiveZone; }

    // Check if player is within zone radius
    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    bool IsPlayerInZone() const;

    // Register a new ambient layer at runtime
    UFUNCTION(BlueprintCallable, Category = "Audio|Layers")
    void RegisterAmbientLayer(FAudio_AmbientLayer NewLayer);

    // Register a new event cue at runtime
    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void RegisterEventCue(FAudio_EventCue NewCue);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Time elapsed in current zone
    float ZoneActiveTime = 0.0f;

    // Previous zone for crossfade tracking
    EAudio_ZoneType PreviousZone = EAudio_ZoneType::None;

    // Crossfade progress (0.0 = full previous, 1.0 = full new)
    float CrossfadeProgress = 1.0f;
    float CrossfadeDurationRemaining = 0.0f;

    // Initialize default layers based on zone type
    void InitializeDefaultLayers();
};
