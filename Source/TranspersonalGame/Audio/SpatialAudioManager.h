#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "SpatialAudioManager.generated.h"

// Audio zone types for different environments and situations
UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Predator        UMETA(DisplayName = "Predator Zone"),
    Ambient         UMETA(DisplayName = "Ambient Zone"), 
    Water           UMETA(DisplayName = "Water Source"),
    Danger          UMETA(DisplayName = "Danger Zone"),
    Safe            UMETA(DisplayName = "Safe Zone"),
    Narrative       UMETA(DisplayName = "Narrative Trigger")
};

// Audio intensity levels based on threat and proximity
UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Silent          UMETA(DisplayName = "Silent"),
    Low             UMETA(DisplayName = "Low Intensity"),
    Medium          UMETA(DisplayName = "Medium Intensity"),
    High            UMETA(DisplayName = "High Intensity"),
    Critical        UMETA(DisplayName = "Critical Intensity")
};

// Spatial audio zone configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SpatialZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_IntensityLevel IntensityLevel = EAudio_IntensityLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FString TTSAudioURL;

    FAudio_SpatialZone()
    {
        ZoneType = EAudio_ZoneType::Ambient;
        Location = FVector::ZeroVector;
        Radius = 500.0f;
        IntensityLevel = EAudio_IntensityLevel::Medium;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        bIsActive = true;
    }
};

// Audio feedback data for player actions and events
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FeedbackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float AttenuationDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    FString AudioFilePath;

    FAudio_FeedbackData()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        bIs3D = true;
        AttenuationDistance = 1000.0f;
    }
};

/**
 * Spatial Audio Manager - Manages 3D positioned audio zones and environmental audio feedback
 * Integrates with narrative system for contextual audio triggers
 * Handles proximity-based audio intensity and spatial positioning
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USpatialAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    USpatialAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core spatial audio management
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void CreateAudioZone(const FAudio_SpatialZone& ZoneConfig);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void RemoveAudioZone(const FVector& Location, float SearchRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void UpdateZoneIntensity(const FVector& Location, EAudio_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void SetZoneActive(const FVector& Location, bool bActive);

    // Player proximity and audio feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void UpdatePlayerProximity(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void TriggerAudioFeedback(const FAudio_FeedbackData& FeedbackData, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Feedback")
    void PlayNarrativeAudio(const FString& TTSAudioURL, const FVector& TriggerLocation);

    // Zone queries and management
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    TArray<FAudio_SpatialZone> GetActiveZones() const;

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    FAudio_SpatialZone GetNearestZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    float GetZoneInfluence(const FVector& Location, const FAudio_SpatialZone& Zone) const;

    // Integration with survival systems
    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void UpdateThreatLevel(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void OnPlayerHealthChanged(float HealthPercentage);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void OnPlayerStaminaChanged(float StaminaPercentage);

    // Dinosaur proximity audio
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void RegisterDinosaurPresence(const FVector& DinosaurLocation, const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UpdateDinosaurThreat(const FVector& DinosaurLocation, float ThreatLevel);

protected:
    // Internal audio zone management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spatial Audio")
    TArray<FAudio_SpatialZone> AudioZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    FVector LastPlayerLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    float CurrentThreatLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    EAudio_IntensityLevel CurrentIntensity;

    // Audio component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxAudioDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float IntensityUpdateRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ProximityCheckInterval = 0.2f;

    // Timers
    float LastIntensityUpdate;
    float LastProximityCheck;

private:
    // Internal helper methods
    void UpdateAudioIntensityBasedOnProximity();
    void ProcessZoneAudio(const FAudio_SpatialZone& Zone, float DeltaTime);
    UAudioComponent* CreateAudioComponentForZone(const FAudio_SpatialZone& Zone);
    void CleanupInactiveAudioComponents();
    float CalculateDistanceAttenuation(float Distance, float MaxDistance) const;
    EAudio_IntensityLevel CalculateIntensityFromThreat(float ThreatLevel) const;
};