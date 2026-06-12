#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Audio_FootstepSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_SurfaceType : uint8
{
    Grass,
    Dirt,
    Rock,
    Water,
    Sand,
    Wood,
    Mud,
    Snow
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FootstepSound
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SurfaceType SurfaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PitchMultiplier = 1.0f;

    FAudio_FootstepSound()
    {
        SurfaceType = EAudio_SurfaceType::Grass;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_CreatureFootstepProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CreatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAudio_FootstepSound> SurfaceSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StepInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxAudibleDistance = 2000.0f;

    FAudio_CreatureFootstepProfile()
    {
        CreatureName = TEXT("Default");
        BaseVolume = 1.0f;
        StepInterval = 0.5f;
        MaxAudibleDistance = 2000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_FootstepSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_FootstepSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Footstep triggering
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerFootstep(const FVector& Location, float MovementSpeed = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCreatureProfile(const FString& ProfileName);

    // Surface detection
    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_SurfaceType DetectSurfaceType(const FVector& Location);

    // Movement tracking
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateMovement(const FVector& NewLocation, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMovementEnabled(bool bEnabled) { bIsMovementEnabled = bEnabled; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Profiles")
    TArray<FAudio_CreatureFootstepProfile> CreatureProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Component")
    UAudioComponent* FootstepAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Profile")
    FString CurrentProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Tracking")
    FVector LastLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Tracking")
    float TimeSinceLastStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Tracking")
    float MovementThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Tracking")
    bool bIsMovementEnabled = true;

    // Surface material mapping
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Detection")
    TMap<UPhysicalMaterial*, EAudio_SurfaceType> PhysicalMaterialMap;

private:
    void InitializeFootstepAudio();
    void InitializeCreatureProfiles();
    void InitializePhysicalMaterialMap();
    FAudio_CreatureFootstepProfile* GetCurrentProfile();
    USoundCue* GetFootstepSoundForSurface(EAudio_SurfaceType SurfaceType);
    float CalculateVolumeFromDistance(const FVector& FootstepLocation);
};