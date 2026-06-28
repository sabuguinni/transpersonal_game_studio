#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "AudioZoneManager.generated.h"

// ============================================================
//  Audio Zone Type — defines what ambient layer plays in zone
// ============================================================
UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"),
    Plains      UMETA(DisplayName = "Plains"),
    Canyon      UMETA(DisplayName = "Canyon"),
    Cave        UMETA(DisplayName = "Cave"),
    Camp        UMETA(DisplayName = "Camp"),
};

// ============================================================
//  Dinosaur Audio Event — triggered when dino enters range
// ============================================================
UENUM(BlueprintType)
enum class EAudio_DinoAudioEvent : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Roar        UMETA(DisplayName = "Roar"),
    Footstep    UMETA(DisplayName = "Footstep"),
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death"),
    Alert       UMETA(DisplayName = "Alert"),
};

// ============================================================
//  Audio Zone Config — data per zone instance
// ============================================================
USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutSeconds = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;

    // Path to the USoundBase asset for this zone's ambient layer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FSoftObjectPath AmbientSoundAsset;
};

// ============================================================
//  Dinosaur Audio Profile — per-species sound configuration
// ============================================================
USTRUCT(BlueprintType)
struct FAudio_DinoProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FName SpeciesName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float RoarRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FootstepRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FootstepInterval = 0.8f;

    // Ground shake intensity when dino footstep plays (0=none, 1=max)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float GroundShakeIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FSoftObjectPath RoarSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FSoftObjectPath FootstepSoundAsset;
};

// ============================================================
//  AAudio_ZoneActor — placed in level to define ambient zones
// ============================================================
UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Audio Zone Actor"))
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void DeactivateZone();

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    bool IsZoneActive() const { return bZoneActive; }

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

protected:
    UFUNCTION()
    void OnPlayerEnterZone(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnPlayerExitZone(AActor* OverlappedActor, AActor* OtherActor);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
              meta = (AllowPrivateAccess = "true"))
    UBoxComponent* ZoneBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone",
              meta = (AllowPrivateAccess = "true"))
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone",
              meta = (AllowPrivateAccess = "true"))
    bool bZoneActive = false;
};

// ============================================================
//  AAudio_DinoTrigger — placed near dino spawn to emit sounds
// ============================================================
UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Dino Audio Trigger"))
class TRANSPERSONALGAME_API AAudio_DinoTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DinoTrigger();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void TriggerFootstep();

    UFUNCTION(BlueprintPure, Category = "Audio|Dino")
    float GetDistanceToPlayer() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FAudio_DinoProfile DinoProfile;

    // Roar fires every RoarIntervalSeconds while player is within RoarRadius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float RoarIntervalSeconds = 12.0f;

private:
    float TimeSinceLastRoar = 0.0f;
    float TimeSinceLastFootstep = 0.0f;
};
