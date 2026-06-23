#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneSystem.generated.h"

// ============================================================
// Audio Agent #16 — AudioZoneSystem
// Ambient audio zones for prehistoric world soundscape.
// Triggers MetaSounds/audio cues when player enters radius.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    JungleAmbience  UMETA(DisplayName = "Jungle Ambience"),
    DinoRoar        UMETA(DisplayName = "Dinosaur Roar"),
    WaterHole       UMETA(DisplayName = "Water Hole"),
    WindCorridor    UMETA(DisplayName = "Wind Corridor"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::JungleAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float TriggerRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bRandomizeInterval = false;

    // Seconds between random one-shot triggers (e.g. distant dino calls)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float RandomIntervalMin = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float RandomIntervalMax = 30.0f;
};

USTRUCT(BlueprintType)
struct FAudio_FreesoundRef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reference")
    int32 FreesoundID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reference")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reference")
    FString PreviewURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reference")
    float DurationSeconds = 0.0f;
};

/**
 * AAudio_ZoneActor
 * Placed in the world to define an ambient audio zone.
 * When the player enters the sphere trigger, audio fades in.
 * When the player exits, audio fades out.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Zone Actor"))
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    // Freesound reference for the audio designer to import
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_FreesoundRef FreesoundReference;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeInAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeOutAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    bool IsPlayerInside() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const;

protected:
    UFUNCTION()
    void OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    bool bPlayerInside = false;
    float CurrentVolume = 0.0f;
    float RandomTriggerTimer = 0.0f;
};

/**
 * UAudio_ZoneManager
 * World subsystem that tracks all audio zones and manages
 * global audio state (e.g. danger level affects music intensity).
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_ZoneManager : public UObject
{
    GENERATED_BODY()

public:
    UAudio_ZoneManager();

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void RegisterZone(AAudio_ZoneActor* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void UnregisterZone(AAudio_ZoneActor* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    int32 GetActiveZoneCount() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    TArray<AAudio_ZoneActor*> GetZonesOfType(EAudio_ZoneType ZoneType) const;

    // 0.0 = calm, 1.0 = maximum danger (affects music layer intensity)
    UPROPERTY(BlueprintReadWrite, Category = "Audio|Manager")
    float GlobalDangerLevel = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetDangerLevel(float NewLevel);

private:
    UPROPERTY()
    TArray<AAudio_ZoneActor*> RegisteredZones;
};
