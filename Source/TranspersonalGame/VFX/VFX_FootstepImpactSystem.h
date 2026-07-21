#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "VFX_FootstepImpactSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float ImpactForce = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float DustCloudScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    FVector DebrisDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    bool bCreateGroundCracks = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    ESurfaceType SurfaceType = ESurfaceType::Default;
};

UENUM(BlueprintType)
enum class EVFX_FootstepType : uint8
{
    Light       UMETA(DisplayName = "Light Step"),
    Medium      UMETA(DisplayName = "Medium Step"),
    Heavy       UMETA(DisplayName = "Heavy Step"),
    Dinosaur    UMETA(DisplayName = "Dinosaur Step"),
    Giant       UMETA(DisplayName = "Giant Step")
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_FootstepImpactSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_FootstepImpactSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX Footstep")
    void TriggerFootstepImpact(const FVector& Location, EVFX_FootstepType FootstepType, const FVFX_FootstepData& FootstepData);

    UFUNCTION(BlueprintCallable, Category = "VFX Footstep")
    void CreateDustCloud(const FVector& Location, float Scale, ESurfaceType Surface);

    UFUNCTION(BlueprintCallable, Category = "VFX Footstep")
    void CreateDebrisEffect(const FVector& Location, const FVector& Direction, float Force);

    UFUNCTION(BlueprintCallable, Category = "VFX Footstep")
    void CreateGroundCracks(const FVector& Location, float CrackRadius);

    UFUNCTION(BlueprintCallable, Category = "VFX Footstep")
    void PlayFootstepSound(const FVector& Location, EVFX_FootstepType FootstepType, ESurfaceType Surface);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxImpactDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableDustClouds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableDebrisEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableGroundCracks = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableFootstepSounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TMap<ESurfaceType, UParticleSystem*> DustCloudParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TMap<ESurfaceType, UParticleSystem*> DebrisParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TMap<EVFX_FootstepType, USoundCue*> FootstepSounds;

private:
    void InitializeParticleSystems();
    void InitializeAudioSystems();
    ESurfaceType GetSurfaceTypeAtLocation(const FVector& Location);
    float CalculateImpactIntensity(EVFX_FootstepType FootstepType, float ImpactForce);
};