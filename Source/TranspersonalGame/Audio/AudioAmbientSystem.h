// AudioAmbientSystem.h — Audio Agent #16 — PROD_CYCLE_AUTO_20260701_002
// Adaptive ambient audio system for prehistoric survival game
// Manages zone-based ambient layers, danger state transitions, and MetaSound integration
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "AudioAmbientSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_AmbientZoneType : uint8
{
    JungleDense     UMETA(DisplayName = "Jungle Dense"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    OpenPlain       UMETA(DisplayName = "Open Plain"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    CampSite        UMETA(DisplayName = "Camp Site"),
    TRexTerritory   UMETA(DisplayName = "T-Rex Territory")
};

UENUM(BlueprintType)
enum class EAudio_DangerState : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_AmbientZoneType ZoneType = EAudio_AmbientZoneType::JungleDense;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float DangerMusicThreshold = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bSilenceInsectsOnDanger = true;
};

/**
 * UAudio_AmbientZoneComponent
 * Attach to any actor to define an ambient audio zone.
 * Handles crossfade between layers based on player proximity and danger state.
 */
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AmbientZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AmbientZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 1500.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetDangerState(EAudio_DangerState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_DangerState GetCurrentDangerState() const { return CurrentDangerState; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeInZone(float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeOutZone(float FadeTime = 3.0f);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    EAudio_DangerState CurrentDangerState = EAudio_DangerState::Safe;
    TArray<UAudioComponent*> ActiveAudioComponents;
    float CurrentBlendAlpha = 0.0f;
    bool bIsActive = false;

    void UpdateAudioBlend(float DeltaTime);
    void SpawnAudioComponents();
    void CleanupAudioComponents();
};

/**
 * AAudio_AmbientZoneActor
 * Placeable actor in the level that defines a spatial ambient audio zone.
 * Triggers crossfade when player enters/exits the box volume.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AmbientZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZoneActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* ZoneBounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone", meta = (AllowPrivateAccess = "true"))
    UAudio_AmbientZoneComponent* AmbientZoneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_AmbientZoneType ZoneType = EAudio_AmbientZoneType::JungleDense;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone(AActor* OverlappingActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone(AActor* OverlappingActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void BroadcastDangerState(EAudio_DangerState NewState);

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

/**
 * UAudio_DangerStateManager
 * World subsystem that tracks global danger state and broadcasts to all ambient zones.
 * Driven by dinosaur proximity, combat state, and time of day.
 */
UCLASS()
class TRANSPERSONALGAME_API UAudio_DangerStateManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void SetGlobalDangerState(EAudio_DangerState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    EAudio_DangerState GetGlobalDangerState() const { return GlobalDangerState; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void RegisterAmbientZone(AAudio_AmbientZoneActor* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void UnregisterAmbientZone(AAudio_AmbientZoneActor* Zone);

    // Called every tick to evaluate danger from dinosaur proximity
    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void EvaluateDangerFromDinosaurProximity(float TRexDistance, float RaptorDistance);

private:
    EAudio_DangerState GlobalDangerState = EAudio_DangerState::Safe;
    TArray<TWeakObjectPtr<AAudio_AmbientZoneActor>> RegisteredZones;

    float DangerCooldownTimer = 0.0f;
    static constexpr float DangerCooldownDuration = 8.0f;
    static constexpr float TRexDangerRadius = 2500.0f;
    static constexpr float RaptorDangerRadius = 1200.0f;
};
