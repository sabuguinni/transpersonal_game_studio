#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Audio_ProximityAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ProximityThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxAudioDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ProximityThreatLevel ThreatLevel = EAudio_ProximityThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> ProximitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<UMetaSoundSource> ProximityMetaSound;

    FAudio_ProximityAudioData()
    {
        DetectionRadius = 2000.0f;
        MaxAudioDistance = 5000.0f;
        VolumeMultiplier = 1.0f;
        ThreatLevel = EAudio_ProximityThreatLevel::None;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ProximityAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ProximityAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_ProximityAudioData TRexProximityData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_ProximityAudioData RaptorProximityData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_ProximityAudioData GeneralDinosaurData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float ProximityCheckInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float HeartbeatIntensityMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableProximityAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableHeartbeatEffect = true;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ProximityThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentProximityIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    AActor* NearestThreatActor;

private:
    UPROPERTY()
    UAudioComponent* ProximityAudioComponent;

    UPROPERTY()
    UAudioComponent* HeartbeatAudioComponent;

    UPROPERTY()
    TArray<AActor*> TrackedDinosaurs;

    float LastProximityCheck;
    float CurrentHeartbeatRate;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateProximityAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ProximityThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void AddTrackedDinosaur(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RemoveTrackedDinosaur(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float CalculateProximityIntensity(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayProximityWarning(EAudio_ProximityThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateHeartbeatEffect(float Intensity);

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool IsPlayerInDanger() const;

    UFUNCTION(BlueprintPure, Category = "Audio")
    float GetDistanceToNearestThreat() const;
};