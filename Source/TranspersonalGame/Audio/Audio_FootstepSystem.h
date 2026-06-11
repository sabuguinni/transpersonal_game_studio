#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "Audio_FootstepSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SurfaceSound
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Audio")
    ESurfaceType SurfaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Audio")
    USoundCue* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Audio")
    USoundCue* LandingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Audio")
    float PitchVariation;

    FAudio_SurfaceSound()
    {
        SurfaceType = ESurfaceType::Grass;
        FootstepSound = nullptr;
        LandingSound = nullptr;
        VolumeMultiplier = 1.0f;
        PitchVariation = 0.2f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Settings")
    TArray<FAudio_SurfaceSound> SurfaceSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Settings")
    float FootstepVolumeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Settings")
    float MinTimeBetweenSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Settings")
    bool bEnableFootstepParticles;

private:
    float LastFootstepTime;
    ESurfaceType LastSurfaceType;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstep(const FVector& Location, ESurfaceType Surface, float MovementSpeed = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayLanding(const FVector& Location, ESurfaceType Surface, float ImpactForce = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    ESurfaceType DetectSurfaceType(const FVector& Location);

private:
    FAudio_SurfaceSound* GetSurfaceSound(ESurfaceType SurfaceType);
    float CalculateVolumeFromSpeed(float MovementSpeed);
    float CalculatePitchVariation();
};