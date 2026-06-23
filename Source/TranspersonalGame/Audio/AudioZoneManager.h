#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "AudioZoneManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioZoneManager
// Manages proximity-based ambient audio zones tied to narrative
// trigger locations. Each zone blends in/out based on player
// distance using a linear attenuation model.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    TRexTerritory   UMETA(DisplayName = "TRex Territory"),
    RiverCrossing   UMETA(DisplayName = "River Crossing"),
    TribeCamp       UMETA(DisplayName = "Tribe Camp"),
    FlintDeposit    UMETA(DisplayName = "Flint Deposit"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    Custom          UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Custom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString ZoneName = TEXT("DefaultZone");

    /** Radius in cm at which audio begins fading in */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float InnerRadius = 800.0f;

    /** Radius in cm at which audio is fully silent */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float OuterRadius = 2400.0f;

    /** Master volume for this zone (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MasterVolume = 1.0f;

    /** Low-pass filter frequency when player is at outer edge (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float DistanceLPFHz = 4000.0f;

    /** Whether this zone suppresses other zones when active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bDominant = false;
};

USTRUCT(BlueprintType)
struct FAudio_ZoneState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    FString ZoneName = TEXT("");

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    float CurrentVolume = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    float PlayerDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    bool bPlayerInside = false;
};

// ============================================================
// UAudio_ZoneComponent — attaches to any actor to define an
// audio zone. Handles volume interpolation per tick.
// ============================================================
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), DisplayName = "Audio Zone Component")
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Zone")
    FAudio_ZoneState ZoneState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    UAudioComponent* PrimaryAudioComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    UAudioComponent* SecondaryAudioComponent = nullptr;

    /** Volume interpolation speed (units/sec) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float VolumeInterpSpeed = 2.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float ComputeVolumeForDistance(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    FAudio_ZoneState GetCurrentState() const { return ZoneState; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ForceVolume(float Volume);

private:
    float TargetVolume = 0.0f;
    APawn* CachedPlayerPawn = nullptr;
};

// ============================================================
// AAudio_ZoneActor — standalone actor that owns a ZoneComponent
// and can be placed directly in the level.
// ============================================================
UCLASS(BlueprintType, Blueprintable, DisplayName = "Audio Zone Actor")
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudio_ZoneComponent* AudioZoneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* PrimarySound;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* SecondarySound;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetZoneType(EAudio_ZoneType NewType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const;

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    bool IsPlayerInside() const;
};

// ============================================================
// UAudio_ZoneManagerSubsystem — world subsystem that tracks
// all active audio zones and resolves priority/dominance.
// ============================================================
UCLASS(DisplayName = "Audio Zone Manager Subsystem")
class TRANSPERSONALGAME_API UAudio_ZoneManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void RegisterZone(UAudio_ZoneComponent* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void UnregisterZone(UAudio_ZoneComponent* Zone);

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    TArray<FAudio_ZoneState> GetAllZoneStates() const;

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    bool HasDominantZoneActive() const;

private:
    UPROPERTY()
    TArray<UAudio_ZoneComponent*> RegisteredZones;
};
