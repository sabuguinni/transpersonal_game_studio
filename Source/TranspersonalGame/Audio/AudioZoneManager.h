// AudioZoneManager.h
// Agent #16 — Audio Agent
// Manages adaptive audio zones for prehistoric survival game
// Each zone blends ambient sounds based on player proximity and environment state

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Forest          UMETA(DisplayName = "Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    Riverbank       UMETA(DisplayName = "Riverbank"),
    Cave            UMETA(DisplayName = "Cave"),
    Campfire        UMETA(DisplayName = "Campfire"),
    DinosaurNest    UMETA(DisplayName = "Dinosaur Nest"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    NightTime       UMETA(DisplayName = "Night Time")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe            UMETA(DisplayName = "Safe"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Danger          UMETA(DisplayName = "Danger"),
    Extreme         UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bPlayAtNight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bPlayAtDay = true;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public USphereComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetBlendWeight(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintPure, Category = "Audio")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Zone detection radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    float ZoneDetectionRadius = 5000.0f;

    // Current dominant zone type
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zones")
    EAudio_ZoneType ActiveZoneType = EAudio_ZoneType::Forest;

    // Current threat level affecting music
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Threat")
    EAudio_ThreatLevel GlobalThreatLevel = EAudio_ThreatLevel::Safe;

    // Time of day (0-24) for day/night audio transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay = 12.0f;

    // Whether it is currently night (affects ambient mix)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|DayNight")
    bool bIsNight = false;

    // Campfire sound reference path (populated from Freesound assets)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString CampfireSoundPath = TEXT("/Game/Audio/Ambient/Campfire_Loop");

    // Forest ambient sound reference path
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString ForestAmbientPath = TEXT("/Game/Audio/Ambient/Forest_Day_Loop");

    // Night ambient sound reference path
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString NightAmbientPath = TEXT("/Game/Audio/Ambient/Night_Insects_Loop");

    // Danger music stinger reference path
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString DangerMusicPath = TEXT("/Game/Audio/Music/Danger_Stinger");

    // Registered audio zones in the world
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zones")
    TArray<UAudio_ZoneComponent*> RegisteredZones;

    // Update threat level globally
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateThreatLevel(EAudio_ThreatLevel NewLevel);

    // Update time of day and trigger day/night audio transitions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateTimeOfDay(float NewTime);

    // Register a zone component with this manager
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterZone(UAudio_ZoneComponent* Zone);

    // Get the dominant zone at a given world location
    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_ZoneType GetDominantZoneAtLocation(const FVector& Location) const;

    // Called when a dinosaur enters proximity — escalates threat
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnDinosaurProximityEnter(float DinosaurDangerRating);

    // Called when dinosaur leaves proximity — de-escalates threat
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnDinosaurProximityExit();

private:
    // Internal tick counter for audio updates (not every frame)
    float AudioUpdateTimer = 0.0f;
    float AudioUpdateInterval = 0.5f;

    // Cached player location
    FVector CachedPlayerLocation = FVector::ZeroVector;

    void UpdateActiveZone();
    void EvaluateDayNightTransition();
};
