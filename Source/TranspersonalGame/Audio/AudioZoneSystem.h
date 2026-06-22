// AudioZoneSystem.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260622_003
// Adaptive ambient audio zones for prehistoric survival world.
// Zones: tribal camp (campfire), raptor patrol (distant calls), open plains (wind).
// NO spiritual/mystical content — pure environmental survival audio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneSystem.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    TribalCamp      UMETA(DisplayName = "Tribal Camp"),
    RaptorPatrol    UMETA(DisplayName = "Raptor Patrol"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float ThreatCallInterval = 12.0f;  // seconds between distant threat calls

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLoopAmbient = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bHasThreatCalls = false;
};

USTRUCT(BlueprintType)
struct FAudio_SoundscapeState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ZoneType ActiveZone = EAudio_ZoneType::OpenPlains;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float BlendAlpha = 0.0f;  // 0=previous zone, 1=current zone

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float TimeSinceLastThreatCall = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bPlayerNearCampfire = false;
};

// ─── Audio Zone Actor ─────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Zone"))
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    // Trigger sphere — player enters to activate zone
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    // Primary ambient audio component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComp;

    // Secondary threat/event audio component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* ThreatAudioComp;

    // Current soundscape state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudio_SoundscapeState CurrentState;

    // Called when player enters the zone
    UFUNCTION(BlueprintNativeEvent, Category = "Audio Zone")
    void OnPlayerEnterZone(AActor* PlayerActor);

    // Called when player exits the zone
    UFUNCTION(BlueprintNativeEvent, Category = "Audio Zone")
    void OnPlayerExitZone(AActor* PlayerActor);

    // Update threat level (called by DinosaurAI when raptors are nearby)
    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    // Get zone type
    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

    // Get current threat level
    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_ThreatLevel GetThreatLevel() const { return CurrentState.ThreatLevel; }

private:
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                               bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void TickThreatCallTimer(float DeltaTime);
    void PlayThreatCall();

    bool bPlayerInZone = false;
    float ThreatCallAccumulator = 0.0f;
};

// ─── Audio Manager Subsystem ──────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UAudio_WorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Register an audio zone with the manager
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void RegisterZone(AAudio_ZoneActor* Zone);

    // Unregister an audio zone
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UnregisterZone(AAudio_ZoneActor* Zone);

    // Get the active zone for a world location
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    AAudio_ZoneActor* GetActiveZoneForLocation(FVector WorldLocation) const;

    // Broadcast threat level change to all nearby zones
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void BroadcastThreatLevel(FVector ThreatOrigin, float Radius, EAudio_ThreatLevel Level);

    // Get global soundscape state
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    FAudio_SoundscapeState GetGlobalSoundscapeState() const { return GlobalState; }

private:
    UPROPERTY()
    TArray<AAudio_ZoneActor*> RegisteredZones;

    FAudio_SoundscapeState GlobalState;
};
