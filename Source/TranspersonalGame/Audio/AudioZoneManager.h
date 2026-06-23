#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "AudioZoneManager.generated.h"

// ============================================================
// Audio Zone Manager — Agent #16 Audio Agent
// Manages ambient sound zones tied to narrative trigger zones
// from NarrativeDialogueManager (Agent #15).
// Freesound references:
//   Camp fire:    #729395 (Campfire 01, 109s loop)
//   Dino roar:   #586545 (Dinosaur Roars Pack 2, 39s)
//   Dino growl:  #586547 (Dinosaur Growls Pack 2, 34s)
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Camp        UMETA(DisplayName = "Camp Fire"),
    River       UMETA(DisplayName = "River Flow"),
    Predator    UMETA(DisplayName = "Predator Alert"),
    Wind        UMETA(DisplayName = "Open Wind"),
    Combat      UMETA(DisplayName = "Combat"),
    Silence     UMETA(DisplayName = "Silence")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Silence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    // Freesound asset ID for reference (not runtime — used for asset pipeline)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Reference")
    int32 FreesoundAssetID = 0;
};

USTRUCT(BlueprintType)
struct FAudio_DayNightLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float DawnStartHour = 5.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float DuskStartHour = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float NightStartHour = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float CurrentHour = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float DayNightCycleDuration = 1200.0f; // 20 real minutes = full day
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public USphereComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    bool bPlayerInZone = false;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCurrentVolume() const;
};

UCLASS()
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Day/Night cycle state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    FAudio_DayNightLayer DayNightState;

    // Active zone tracking
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EAudio_ZoneType ActiveZoneType = EAudio_ZoneType::Silence;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    float GlobalAmbientVolume = 1.0f;

    // Predator proximity — drives danger music layer intensity
    UPROPERTY(BlueprintReadWrite, Category = "Audio")
    float PredatorProximityFactor = 0.0f; // 0.0 = safe, 1.0 = imminent

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetActiveZone(EAudio_ZoneType NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetDayNightBlend() const; // 0.0=day, 1.0=night

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsDawnOrDusk() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetPredatorProximity(float Factor);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FString GetCurrentAudioState() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Debug")
    void LogAudioState();

private:
    float ZoneTransitionTimer = 0.0f;
    EAudio_ZoneType PreviousZoneType = EAudio_ZoneType::Silence;
};
