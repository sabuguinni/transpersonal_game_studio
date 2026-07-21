#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "Arch_SoundArchitectureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_AcousticZoneType : uint8
{
    OpenAir         UMETA(DisplayName = "Open Air"),
    CaveSystem      UMETA(DisplayName = "Cave System"),
    ForestCanopy    UMETA(DisplayName = "Forest Canopy"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    UndergroundTunnel UMETA(DisplayName = "Underground Tunnel"),
    WaterCavern     UMETA(DisplayName = "Water Cavern")
};

USTRUCT(BlueprintType)
struct FArch_AcousticProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float ReverbAmount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float EchoDelay = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float SoundAbsorption = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    float MaxHearingDistance = 2000.0f;

    FArch_AcousticProperties()
    {
        ReverbAmount = 0.3f;
        EchoDelay = 0.2f;
        SoundAbsorption = 0.5f;
        AmbientVolume = 0.7f;
        MaxHearingDistance = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct FArch_SoundZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Zone")
    EArch_AcousticZoneType ZoneType = EArch_AcousticZoneType::OpenAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Zone")
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Zone")
    FArch_AcousticProperties AcousticSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Zone")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Zone")
    bool bIsActive = true;

    FArch_SoundZone()
    {
        ZoneType = EArch_AcousticZoneType::OpenAir;
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_SoundArchitectureSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_SoundArchitectureSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Sound zone management
    UFUNCTION(BlueprintCallable, Category = "Sound Architecture")
    void RegisterSoundZone(const FArch_SoundZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Sound Architecture")
    void UnregisterSoundZone(const FVector& ZoneCenter);

    UFUNCTION(BlueprintCallable, Category = "Sound Architecture")
    EArch_AcousticZoneType GetCurrentAcousticZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Sound Architecture")
    FArch_AcousticProperties GetAcousticPropertiesAtLocation(const FVector& Location) const;

    // Audio component management
    UFUNCTION(BlueprintCallable, Category = "Sound Architecture")
    void UpdatePlayerAudioSettings(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Sound Architecture")
    void PlayArchitecturalSound(USoundCue* SoundToPlay, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Sound Architecture")
    void SetGlobalReverbSettings(float ReverbAmount, float EchoDelay);

    // Cave-specific audio functions
    UFUNCTION(BlueprintCallable, Category = "Cave Audio")
    void EnableCaveReverb(const FVector& CaveEntrance, float CaveDepth);

    UFUNCTION(BlueprintCallable, Category = "Cave Audio")
    void DisableCaveReverb();

    UFUNCTION(BlueprintCallable, Category = "Cave Audio")
    void PlayCaveAmbient(const FVector& Location);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Zones")
    TArray<FArch_SoundZone> RegisteredSoundZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalReverbAmount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalEchoDelay = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Audio")
    bool bCaveReverbEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Audio")
    float CurrentCaveDepth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Audio")
    TSoftObjectPtr<USoundCue> CaveAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Audio")
    TSoftObjectPtr<USoundCue> CaveEchoSound;

private:
    // Internal helper functions
    FArch_SoundZone* FindSoundZoneAtLocation(const FVector& Location);
    void UpdateAudioComponentSettings(UAudioComponent* AudioComp, const FArch_AcousticProperties& Properties);
    void CleanupInactiveAudioComponents();
    float CalculateDistanceAttenuation(const FVector& SoundLocation, const FVector& ListenerLocation, float MaxDistance) const;
};