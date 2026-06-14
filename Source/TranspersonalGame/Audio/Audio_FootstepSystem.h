#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "Audio_FootstepSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float MinDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    float MaxDistance = 2000.0f;

    FAudio_FootstepData()
    {
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        MinDistance = 100.0f;
        MaxDistance = 2000.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_FootstepSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_FootstepSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Footstep configuration per surface type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
    TMap<ESurfaceType, FAudio_FootstepData> FootstepSounds;

    // Player footstep settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Footsteps")
    FAudio_FootstepData PlayerFootsteps;

    // Dinosaur footstep settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Footsteps")
    FAudio_FootstepData DinosaurFootsteps;

    // Heavy creature (T-Rex) footstep settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heavy Footsteps")
    FAudio_FootstepData HeavyFootsteps;

    // Footstep timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float FootstepInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LastFootstepTime = 0.0f;

    // Audio component for playing footsteps
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AudioComponent;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Footsteps")
    void PlayFootstep(ESurfaceType SurfaceType, FVector Location, float VelocityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Footsteps")
    void PlayPlayerFootstep(FVector Location, float MovementSpeed);

    UFUNCTION(BlueprintCallable, Category = "Footsteps")
    void PlayDinosaurFootstep(FVector Location, ECreatureSize CreatureSize);

    UFUNCTION(BlueprintCallable, Category = "Footsteps")
    ESurfaceType GetSurfaceTypeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Footsteps")
    void SetFootstepVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Footsteps")
    void EnableFootsteps(bool bEnable);

private:
    bool bFootstepsEnabled = true;
    float CurrentVolume = 1.0f;
    
    // Helper functions
    void InitializeFootstepSounds();
    FAudio_FootstepData GetFootstepDataForSurface(ESurfaceType SurfaceType);
};