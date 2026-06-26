#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    JungleAmbience      UMETA(DisplayName = "Jungle Ambience"),
    RiverAmbience       UMETA(DisplayName = "River Ambience"),
    WindAmbience        UMETA(DisplayName = "Wind Ambience"),
    DangerZone          UMETA(DisplayName = "Danger Zone"),
    CaveAmbience        UMETA(DisplayName = "Cave Ambience"),
    OpenPlain           UMETA(DisplayName = "Open Plain"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::JungleAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float AttenuationRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutDuration = 3.0f;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone", meta = (AllowPrivateAccess = "true"))
    bool bIsPlayerInZone = false;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerEnterZone();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerExitZone();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY()
    UAudioComponent* AudioComponent = nullptr;
};

UCLASS()
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Manager")
    TArray<FAudio_ZoneConfig> RegisteredZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Manager")
    float GlobalVolumeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Manager")
    bool bEnableDynamicMixing = true;

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void RegisterZone(FAudio_ZoneConfig ZoneConfig);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetGlobalVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void TriggerDangerStinger(FVector DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void TriggerNightTransition();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void TriggerDawnTransition();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float CurrentDangerLevel = 0.0f;
    float TargetDangerLevel = 0.0f;
    float DangerBlendSpeed = 1.5f;

    void UpdateDangerMix(float DeltaTime);
};
