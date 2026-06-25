#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"),
    Plains      UMETA(DisplayName = "Plains"),
    Danger      UMETA(DisplayName = "Danger Zone"),
    Cave        UMETA(DisplayName = "Cave"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;
};

/**
 * UAudio_ZoneManager — manages ambient audio zones in the prehistoric world.
 * Each zone blends in/out based on player proximity.
 * Zones: Forest (insects/birds), River (water), Plains (wind), Danger (low rumble).
 */
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float CurrentBlendAlpha = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetZoneVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetBlendAlpha() const { return CurrentBlendAlpha; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone(float DistanceToCenter);

private:
    UPROPERTY()
    UAudioComponent* AudioComp = nullptr;

    float TargetVolume = 0.0f;
    float BlendSpeed = 2.0f;
};

/**
 * AAudio_ZoneActor — placeable actor that wraps UAudio_ZoneManager.
 * Drop into level to define an ambient audio region.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    UAudio_ZoneManager* ZoneManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
