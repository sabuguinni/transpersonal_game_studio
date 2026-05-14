#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "SharedTypes.h"
#include "Audio_ProximityAudioManager.generated.h"

class ATranspersonalCharacter;
class AActor;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximityAudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float InnerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float OuterRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TSoftObjectPtr<UMetaSoundSource> MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EBiomeType BiomeType = EBiomeType::Forest;

    FAudio_ProximityAudioZone()
    {
        ZoneCenter = FVector::ZeroVector;
        InnerRadius = 500.0f;
        OuterRadius = 1000.0f;
        BaseVolume = 0.7f;
        bIsActive = true;
        BiomeType = EBiomeType::Forest;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EDinosaurSpecies Species = EDinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> MovementSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundBase> AggressiveSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float FootstepInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float ProximityThreatDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VolumeMultiplier = 1.0f;

    FAudio_DinosaurAudioProfile()
    {
        Species = EDinosaurSpecies::TRex;
        FootstepInterval = 2.0f;
        ProximityThreatDistance = 2000.0f;
        VolumeMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_ProximityAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_ProximityAudioManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core proximity audio functions
    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void RegisterAudioZone(const FAudio_ProximityAudioZone& AudioZone);

    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void UnregisterAudioZone(FVector ZoneCenter, float Tolerance = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void UpdatePlayerProximity(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void RegisterDinosaurActor(AActor* DinosaurActor, EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Proximity Audio")
    void UpdateDinosaurProximity();

    // Environmental audio control
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetGlobalAudioMood(EAudioMood NewMood);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayNarrativeAudio(const FString& AudioURL, FVector WorldLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StopAllProximityAudio();

    // Dinosaur-specific audio
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void TriggerDinosaurFootstep(AActor* DinosaurActor, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurThreatSound(EDinosaurSpecies Species, FVector Location);

    // Debug and utility
    UFUNCTION(BlueprintCallable, Category = "Audio Debug", CallInEditor)
    void DebugPrintAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Audio Debug")
    int32 GetActiveAudioZoneCount() const;

protected:
    // Audio zone management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zones")
    TArray<FAudio_ProximityAudioZone> RegisteredAudioZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> ActiveAudioComponents;

    // Dinosaur audio tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur Audio")
    TMap<AActor*, FAudio_DinosaurAudioProfile> DinosaurAudioMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    FVector LastPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    EAudioMood CurrentAudioMood = EAudioMood::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float GlobalVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float ProximityUpdateInterval = 0.1f;

private:
    // Internal audio management
    void UpdateAudioZoneVolumes(FVector PlayerLocation);
    void CleanupInactiveAudioComponents();
    UAudioComponent* CreateAudioComponentForZone(const FAudio_ProximityAudioZone& AudioZone);
    float CalculateProximityVolume(FVector PlayerLocation, const FAudio_ProximityAudioZone& AudioZone);
    
    // Dinosaur audio helpers
    void ProcessDinosaurProximityAudio(AActor* DinosaurActor, const FAudio_DinosaurAudioProfile& AudioProfile);
    float CalculateDinosaurThreatLevel(FVector PlayerLocation, FVector DinosaurLocation, float ThreatDistance);

    // Timer handles
    FTimerHandle ProximityUpdateTimer;
    FTimerHandle DinosaurAudioTimer;

    // Audio component pool for optimization
    TArray<UAudioComponent*> PooledAudioComponents;
    int32 MaxPooledComponents = 20;
};