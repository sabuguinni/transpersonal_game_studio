#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "VFX_ParticleSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    CampfireFire    UMETA(DisplayName = "Campfire Fire"),
    DinosaurDust    UMETA(DisplayName = "Dinosaur Dust"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    RainDroplets    UMETA(DisplayName = "Rain Droplets"),
    WindDebris      UMETA(DisplayName = "Wind Debris")
};

USTRUCT(BlueprintType)
struct FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::CampfireFire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bLooping = true;

    FVFX_ParticleConfig()
    {
        ParticleType = EVFX_ParticleType::CampfireFire;
        Intensity = 1.0f;
        Duration = 5.0f;
        SpawnLocation = FVector::ZeroVector;
        bLooping = true;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AVFX_ParticleSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TArray<UParticleSystemComponent*> ActiveParticleSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TArray<FVFX_ParticleConfig> ParticleConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxRenderDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableDistanceCulling = true;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopParticleEffect(EVFX_ParticleType ParticleType);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopAllParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetGlobalIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void CreateVolcanicAshCloud(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void CreateCampfireEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void CreateDinosaurDustImpact(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void CreateBloodSplatterEffect(FVector Location, float Intensity = 1.0f);

private:
    void UpdateParticleSystemLOD();
    void CullDistantParticles();
    UParticleSystemComponent* CreateParticleComponent(EVFX_ParticleType ParticleType);
    void ConfigureParticleSystem(UParticleSystemComponent* ParticleComp, const FVFX_ParticleConfig& Config);
};