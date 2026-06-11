#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Arch_StructuralAcousticsSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_AcousticZoneType : uint8
{
    OpenAir         UMETA(DisplayName = "Open Air"),
    Cave            UMETA(DisplayName = "Cave"),
    Forest          UMETA(DisplayName = "Forest"),
    Canyon          UMETA(DisplayName = "Canyon"),
    Ruins           UMETA(DisplayName = "Ruins"),
    Shelter         UMETA(DisplayName = "Shelter")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_AcousticProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float ReverbAmount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float EchoDelay = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float SoundDamping = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustics")
    float AmbientVolume = 1.0f;

    FArch_AcousticProperties()
    {
        ReverbAmount = 0.3f;
        EchoDelay = 0.1f;
        SoundDamping = 0.5f;
        AmbientVolume = 1.0f;
    }
};

UCLASS(ClassGroup=(Architecture), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralAcousticsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralAcousticsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Zone")
    EArch_AcousticZoneType ZoneType = EArch_AcousticZoneType::OpenAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Acoustic Properties")
    FArch_AcousticProperties AcousticSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TObjectPtr<USoundCue> CaveAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TObjectPtr<USoundCue> ForestAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TObjectPtr<USoundCue> RuinsAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Detection")
    float DetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Detection")
    bool bAutoDetectZoneType = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Acoustics")
    void SetAcousticZone(EArch_AcousticZoneType NewZoneType);

    UFUNCTION(BlueprintCallable, Category = "Acoustics")
    void UpdateAcousticProperties(const FArch_AcousticProperties& NewProperties);

    UFUNCTION(BlueprintCallable, Category = "Acoustics")
    FArch_AcousticProperties GetCurrentAcousticProperties() const;

    UFUNCTION(BlueprintCallable, Category = "Acoustics")
    void PlayStructuralSound(USoundCue* SoundToPlay, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintImplementableEvent, Category = "Acoustics")
    void OnAcousticZoneChanged(EArch_AcousticZoneType OldZone, EArch_AcousticZoneType NewZone);

private:
    void InitializeAudioComponents();
    void DetectNearbyStructures();
    EArch_AcousticZoneType DetermineZoneTypeFromEnvironment();
    void ApplyAcousticSettings();

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> NearbyStructures;

    float LastDetectionTime = 0.0f;
    const float DetectionInterval = 2.0f;
};