#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "AudioZoneSystem.generated.h"

// === Audio Zone Types ===
UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    River       UMETA(DisplayName = "River"),
    Forest      UMETA(DisplayName = "Forest"),
    OpenPlain   UMETA(DisplayName = "Open Plain"),
    DangerZone  UMETA(DisplayName = "Danger Zone"),
    Storm       UMETA(DisplayName = "Storm"),
    Cave        UMETA(DisplayName = "Cave"),
    Campfire    UMETA(DisplayName = "Campfire"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MusicIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bDangerMusic = false;
};

/**
 * UAudio_ZoneComponent — attaches to any actor to define an audio zone.
 * The AudioZoneManager queries these to blend ambient layers.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetBlendWeightForPlayer(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    bool IsDangerZone() const { return ZoneConfig.bDangerMusic; }
};

/**
 * AAudio_ZoneActor — placeable actor that defines an audio zone in the world.
 * Place in the level and configure ZoneConfig to control ambient audio blending.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
        meta = (AllowPrivateAccess = "true"))
    UAudio_ZoneComponent* AudioZoneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Zone")
    void PreviewZoneConfig();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;
};

/**
 * UAudio_ZoneManager — GameInstance subsystem that tracks all audio zones
 * and drives adaptive music/ambient blending based on player proximity.
 */
UCLASS()
class TRANSPERSONALGAME_API UAudio_ZoneManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void RegisterZone(AAudio_ZoneActor* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void UnregisterZone(AAudio_ZoneActor* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetDominantZoneForPlayer(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetDangerLevel(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    TArray<AAudio_ZoneActor*> GetAllZones() const { return RegisteredZones; }

private:
    UPROPERTY()
    TArray<AAudio_ZoneActor*> RegisteredZones;
};
