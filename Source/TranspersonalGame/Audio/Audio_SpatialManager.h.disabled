#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../Core/SharedTypes.h"
#include "Audio_SpatialManager.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - AUDIO SPATIAL MANAGER
 * Audio Agent #16
 * 
 * Manages 3D spatial audio for the prehistoric world.
 * Handles distance-based attenuation, environmental reverb,
 * and dynamic audio zones for immersive prehistoric soundscape.
 */

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Forest = 0          UMETA(DisplayName = "Forest"),
    Plains = 1          UMETA(DisplayName = "Plains"),
    Swamp = 2           UMETA(DisplayName = "Swamp"),
    Cave = 3            UMETA(DisplayName = "Cave"),
    River = 4           UMETA(DisplayName = "River"),
    Mountain = 5        UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient = 0         UMETA(DisplayName = "Ambient"),
    Dinosaur = 1        UMETA(DisplayName = "Dinosaur"),
    Weather = 2         UMETA(DisplayName = "Weather"),
    Foliage = 3         UMETA(DisplayName = "Foliage"),
    Water = 4           UMETA(DisplayName = "Water"),
    Footsteps = 5       UMETA(DisplayName = "Footsteps")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SpatialSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float AttenuationRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bEnableReverb = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float ReverbWetness = 0.3f;

    FAudio_SpatialSettings()
    {
        MaxDistance = 5000.0f;
        AttenuationRadius = 1000.0f;
        VolumeMultiplier = 1.0f;
        bEnableReverb = true;
        ReverbWetness = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_SpatialSettings SpatialSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TArray<TSoftObjectPtr<USoundCue>> AmbientSounds;

    FAudio_ZoneData()
    {
        ZoneType = EAudio_ZoneType::Forest;
        Center = FVector::ZeroVector;
        Radius = 2000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_SpatialManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SpatialManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Audio zones for different environments
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<FAudio_ZoneData> AudioZones;

    // Current active zone
    UPROPERTY(BlueprintReadOnly, Category = "Audio Zones")
    EAudio_ZoneType CurrentZone = EAudio_ZoneType::Forest;

    // Player reference for distance calculations
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    AActor* PlayerActor = nullptr;

    // Active ambient audio component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AmbientAudioComponent = nullptr;

    // Zone transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    float ZoneTransitionTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    float ZoneCheckInterval = 1.0f;

    // Timer for zone checking
    float ZoneCheckTimer = 0.0f;

public:
    // Initialize audio zones
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void InitializeAudioZones();

    // Play spatial sound at location
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySpatialSound(USoundCue* SoundCue, FVector Location, EAudio_SoundCategory Category, float VolumeMultiplier = 1.0f);

    // Update current audio zone based on player position
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateCurrentZone();

    // Get spatial settings for current zone
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio")
    FAudio_SpatialSettings GetCurrentSpatialSettings() const;

    // Set zone ambient sound
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetZoneAmbientSound(EAudio_ZoneType ZoneType, USoundCue* AmbientSound);

    // Calculate distance-based volume
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio")
    float CalculateDistanceVolume(FVector SoundLocation, float MaxDistance) const;

    // Check if player is in specific zone
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio")
    bool IsPlayerInZone(EAudio_ZoneType ZoneType) const;

    // Get zone by location
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio")
    EAudio_ZoneType GetZoneAtLocation(FVector Location) const;

private:
    // Transition between zones
    void TransitionToZone(EAudio_ZoneType NewZone);

    // Setup default audio zones
    void SetupDefaultZones();

    // Find player actor
    void FindPlayerActor();
};