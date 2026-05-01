#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "VFX_AudioSyncController.generated.h"

UENUM(BlueprintType)
enum class EVFX_AudioSyncType : uint8
{
    None            UMETA(DisplayName = "None"),
    ForestAmbience  UMETA(DisplayName = "Forest Ambience"),
    DangerZone      UMETA(DisplayName = "Danger Zone"),
    WaterSource     UMETA(DisplayName = "Water Source"),
    CraftingArea    UMETA(DisplayName = "Crafting Area"),
    WindEffect      UMETA(DisplayName = "Wind Effect")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_AudioSyncData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Sync")
    EVFX_AudioSyncType SyncType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Sync")
    float IntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Sync")
    float ResponseTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Sync")
    bool bAutoActivate;

    FVFX_AudioSyncData()
    {
        SyncType = EVFX_AudioSyncType::None;
        IntensityMultiplier = 1.0f;
        ResponseTime = 0.5f;
        bAutoActivate = true;
    }
};

/**
 * VFX Audio Sync Controller - Synchronizes visual effects with audio zones
 * Creates atmospheric VFX that respond to narrative audio triggers
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_AudioSyncController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_AudioSyncController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Audio sync configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Sync")
    FVFX_AudioSyncData AudioSyncData;

    // VFX components to control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Components")
    TArray<UParticleSystemComponent*> ParticleComponents;

    // Audio monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Monitoring")
    UAudioComponent* MonitoredAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Monitoring")
    float AudioThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Monitoring")
    float MaxDistance;

    // VFX intensity control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Control")
    float BaseIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Control")
    float CurrentIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Control")
    float TargetIntensity;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Audio Sync")
    void SetSyncType(EVFX_AudioSyncType NewSyncType);

    UFUNCTION(BlueprintCallable, Category = "VFX Audio Sync")
    void TriggerVFXResponse(float AudioIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Audio Sync")
    void UpdateVFXIntensity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "VFX Audio Sync")
    void RegisterParticleComponent(UParticleSystemComponent* ParticleComp);

    UFUNCTION(BlueprintCallable, Category = "VFX Audio Sync")
    void SetAudioMonitoring(UAudioComponent* AudioComp, float Threshold, float Distance);

    // Narrative integration
    UFUNCTION(BlueprintCallable, Category = "Narrative VFX")
    void OnNarrativeAudioStart(const FString& AudioContext);

    UFUNCTION(BlueprintCallable, Category = "Narrative VFX")
    void OnNarrativeAudioEnd(const FString& AudioContext);

    UFUNCTION(BlueprintCallable, Category = "Narrative VFX")
    void CreateContextualVFX(const FString& Context, FVector Location);

private:
    // Internal state
    float LastAudioLevel;
    float SmoothingFactor;
    bool bIsActive;
    FTimerHandle IntensityUpdateTimer;

    // Helper functions
    void InitializeVFXComponents();
    void UpdateParticleIntensity(UParticleSystemComponent* ParticleComp, float Intensity);
    float CalculateAudioLevel();
    void SmoothIntensityTransition(float DeltaTime);
};

#include "VFX_AudioSyncController.generated.h"