#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Audio_FootstepController.generated.h"

UENUM(BlueprintType)
enum class EAudio_FootstepSurface : uint8
{
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Mud         UMETA(DisplayName = "Mud"),
    Sand        UMETA(DisplayName = "Sand"),
    Water       UMETA(DisplayName = "Water"),
    Wood        UMETA(DisplayName = "Wood")
};

USTRUCT(BlueprintType)
struct FAudio_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    class USoundCue* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float VolumeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float PitchVariation;

    FAudio_FootstepData()
    {
        FootstepSound = nullptr;
        VolumeScale = 1.0f;
        PitchVariation = 0.2f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_FootstepController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_FootstepController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Settings")
    TMap<EAudio_FootstepSurface, FAudio_FootstepData> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Settings")
    float FootstepVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Settings")
    float MinFootstepInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Settings")
    bool bEnableFootstepParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Settings")
    class UParticleSystem* DustParticleEffect;

private:
    float LastFootstepTime;
    EAudio_FootstepSurface CurrentSurface;

public:
    UFUNCTION(BlueprintCallable, Category = "Footstep Audio")
    void PlayFootstep(FVector FootLocation, EAudio_FootstepSurface Surface = EAudio_FootstepSurface::Grass);

    UFUNCTION(BlueprintCallable, Category = "Footstep Audio")
    void SetCurrentSurface(EAudio_FootstepSurface NewSurface);

    UFUNCTION(BlueprintCallable, Category = "Footstep Audio")
    EAudio_FootstepSurface DetectSurfaceAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Footstep Audio")
    void SetFootstepVolumeMultiplier(float NewMultiplier);

private:
    void InitializeFootstepSounds();
    void SpawnFootstepParticles(FVector Location, EAudio_FootstepSurface Surface);
};