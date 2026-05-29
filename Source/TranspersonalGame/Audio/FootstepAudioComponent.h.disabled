#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "FootstepAudioComponent.generated.h"

UENUM(BlueprintType)
enum class EAudio_SurfaceType : uint8
{
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Water       UMETA(DisplayName = "Water"),
    Wood        UMETA(DisplayName = "Wood"),
    Metal       UMETA(DisplayName = "Metal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    USoundBase* FootstepSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    UParticleSystem* FootstepParticles = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float ParticleScale = 1.0f;

    FAudio_FootstepData()
    {
        FootstepSound = nullptr;
        FootstepParticles = nullptr;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        ParticleScale = 1.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UFootstepAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFootstepAudioComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main footstep trigger function
    UFUNCTION(BlueprintCallable, Category = "Footstep Audio")
    void TriggerFootstep(FVector FootstepLocation, EAudio_SurfaceType SurfaceType, float IntensityMultiplier = 1.0f);

    // Auto-detect surface type from hit result
    UFUNCTION(BlueprintCallable, Category = "Footstep Audio")
    void TriggerFootstepAutoDetect(FVector FootstepLocation, float IntensityMultiplier = 1.0f);

    // Set footstep data for different surface types
    UFUNCTION(BlueprintCallable, Category = "Footstep Audio")
    void SetSurfaceFootstepData(EAudio_SurfaceType SurfaceType, const FAudio_FootstepData& FootstepData);

    // Enable/disable automatic footstep detection
    UFUNCTION(BlueprintCallable, Category = "Footstep Audio")
    void SetAutoFootstepDetection(bool bEnabled);

    // Set the character mass (affects footstep intensity)
    UFUNCTION(BlueprintCallable, Category = "Footstep Audio")
    void SetCharacterMass(float Mass);

protected:
    // Surface-specific footstep data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Data")
    TMap<EAudio_SurfaceType, FAudio_FootstepData> SurfaceFootstepMap;

    // Default footstep data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Data")
    FAudio_FootstepData DefaultFootstepData;

    // Auto-detection settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Detection")
    bool bAutoDetectFootsteps = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Detection")
    float FootstepDetectionRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Detection")
    float MinFootstepInterval = 0.3f;

    // Character properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float CharacterMass = 70.0f; // Human mass in kg

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float MovementSpeedThreshold = 100.0f; // Minimum speed to trigger footsteps

private:
    // Internal state
    float LastFootstepTime = 0.0f;
    FVector LastFootstepLocation = FVector::ZeroVector;
    
    // Helper functions
    EAudio_SurfaceType DetectSurfaceType(FVector Location) const;
    bool ShouldTriggerFootstep(FVector CurrentLocation, float DeltaTime) const;
    void PlayFootstepAudio(const FAudio_FootstepData& FootstepData, FVector Location, float IntensityMultiplier) const;
    void SpawnFootstepParticles(const FAudio_FootstepData& FootstepData, FVector Location, float IntensityMultiplier) const;
    float CalculateFootstepIntensity(float CharacterMass, float MovementSpeed) const;
};