#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Audio_ProximityWarningSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    EAudio_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector ThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor;

    FAudio_ThreatData()
    {
        ThreatLevel = EAudio_ThreatLevel::None;
        Distance = 0.0f;
        ThreatLocation = FVector::ZeroVector;
        ThreatActor = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ProximityWarningSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ProximityWarningSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Proximity detection settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Detection")
    float MaxDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Detection")
    float CriticalRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Detection")
    float HighThreatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Detection")
    float MediumThreatRange;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* TRexApproachSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* RaptorPackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* BrachiosaurusSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* GenericThreatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxVolume;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* ProximityAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* HeartbeatAudioComponent;

    // Current threat tracking
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    FAudio_ThreatData CurrentThreat;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsPlayingWarning;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float LastWarningTime;

    // Warning cooldown
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float WarningCooldown;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Proximity Warning")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Proximity Warning")
    void PlayThreatWarning(const FAudio_ThreatData& ThreatData);

    UFUNCTION(BlueprintCallable, Category = "Proximity Warning")
    void StopThreatWarning();

    UFUNCTION(BlueprintCallable, Category = "Proximity Warning")
    EAudio_ThreatLevel CalculateThreatLevel(float Distance, AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Proximity Warning")
    USoundCue* GetSoundForActor(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Proximity Warning")
    float CalculateVolumeForDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Proximity Warning")
    void UpdateHeartbeatIntensity(EAudio_ThreatLevel ThreatLevel);

private:
    float LastScanTime;
    float ScanInterval;
};

#include "Audio_ProximityWarningSystem.generated.h"