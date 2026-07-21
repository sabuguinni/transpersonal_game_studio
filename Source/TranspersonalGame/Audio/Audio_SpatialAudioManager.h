#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "Audio_SpatialAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    FString DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* MovementSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundCue* AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxHearingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VolumeMultiplier;

    FAudio_DinosaurSoundProfile()
    {
        DinosaurType = TEXT("Unknown");
        IdleSound = nullptr;
        MovementSound = nullptr;
        AttackSound = nullptr;
        MaxHearingDistance = 5000.0f;
        VolumeMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SpatialAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SpatialAudioManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core spatial audio functions
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void PlaySoundAtLocation(USoundCue* Sound, FVector Location, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void RegisterDinosaurSoundProfile(const FAudio_DinosaurSoundProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void SetMasterSpatialVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    float CalculateDistanceAttenuation(FVector SoundLocation, FVector ListenerLocation, float MaxDistance);

    // Screen shake integration
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerFootstepShake(FVector FootstepLocation, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void PlayFootstepWithShake(FVector Location, const FString& DinosaurType);

protected:
    // Audio components for spatial sound management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* SpatialAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* EnvironmentalAudioComponent;

    // Dinosaur sound profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_DinosaurSoundProfile> DinosaurSoundProfiles;

    // Master volume controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float MasterSpatialVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float MaxAudioDistance;

    // Screen shake settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float FootstepShakeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float FootstepShakeRadius;

private:
    // Internal audio management
    void InitializeDinosaurSoundProfiles();
    void UpdateSpatialAudio(float DeltaTime);
    FAudio_DinosaurSoundProfile* FindDinosaurProfile(const FString& DinosaurType);
    
    // Player reference for distance calculations
    APawn* PlayerPawn;
    
    // Audio timing management
    float LastAudioUpdateTime;
    float AudioUpdateInterval;
};