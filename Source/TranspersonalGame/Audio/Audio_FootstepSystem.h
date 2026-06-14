#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Components/AudioComponent.h"
#include "Audio_FootstepSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_FootstepType : uint8
{
    Light       UMETA(DisplayName = "Light Step"),
    Medium      UMETA(DisplayName = "Medium Step"), 
    Heavy       UMETA(DisplayName = "Heavy Step"),
    Dinosaur    UMETA(DisplayName = "Dinosaur Step")
};

UENUM(BlueprintType)
enum class EAudio_SurfaceType : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Mud         UMETA(DisplayName = "Mud"),
    Sand        UMETA(DisplayName = "Sand"),
    Water       UMETA(DisplayName = "Water")
};

USTRUCT(BlueprintType)
struct FAudio_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UParticleSystem* DustParticle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier;

    FAudio_FootstepData()
    {
        FootstepSound = nullptr;
        DustParticle = nullptr;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
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

    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    void PlayFootstep(EAudio_FootstepType FootstepType, EAudio_SurfaceType SurfaceType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    void SetFootstepEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    void SetVolumeMultiplier(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Audio|Footsteps")
    EAudio_SurfaceType DetectSurfaceType(FVector Location);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    bool bFootstepsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float GlobalVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Settings")
    float FootstepCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep Data")
    TMap<EAudio_FootstepType, TMap<EAudio_SurfaceType, FAudio_FootstepData>> FootstepDataMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components")
    UAudioComponent* AudioComponent;

private:
    float LastFootstepTime;
    
    void InitializeFootstepData();
    void SpawnDustParticle(UParticleSystem* ParticleSystem, FVector Location);
};