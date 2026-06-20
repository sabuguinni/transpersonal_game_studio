#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Camp        UMETA(DisplayName = "Camp"),
    Forest      UMETA(DisplayName = "Forest"),
    DangerZone  UMETA(DisplayName = "Danger Zone"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    OpenPlain   UMETA(DisplayName = "Open Plain")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float DangerIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bIsActive = true;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public USphereComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    USoundBase* AmbientSound = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void DeactivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetBlendWeight(const FVector& ListenerLocation) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float UpdateInterval = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableDynamicMixing = true;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterZone(UAudio_ZoneComponent* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UnregisterZone(UAudio_ZoneComponent* Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_ZoneType GetDominantZoneForListener(const FVector& ListenerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetDangerLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetDangerLevel(float NewDangerLevel);

private:
    UPROPERTY()
    TArray<UAudio_ZoneComponent*> RegisteredZones;

    UPROPERTY()
    UAudioComponent* ActiveAmbientComponent = nullptr;

    float CurrentDangerLevel = 0.0f;
    float TimeSinceLastUpdate = 0.0f;

    void UpdateAudioMix(const FVector& ListenerLocation);
};
