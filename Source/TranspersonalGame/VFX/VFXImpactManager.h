#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFXImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    BloodSplatter      UMETA(DisplayName = "Blood Splatter"),
    RockImpact         UMETA(DisplayName = "Rock Impact"),
    WaterSplash        UMETA(DisplayName = "Water Splash"),
    DustCloud          UMETA(DisplayName = "Dust Cloud")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_ImpactType ImpactType = EVFX_ImpactType::DinosaurFootstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float ImpactForce = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float ParticleScale = 1.0f;

    FVFX_ImpactData()
    {
        ImpactType = EVFX_ImpactType::DinosaurFootstep;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactForce = 1.0f;
        ParticleScale = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ImpactManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main VFX creation functions
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateImpactEffect(const FVFX_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateDinosaurFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateBloodSplatterEffect(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateDustCloudEffect(FVector Location, float Intensity = 1.0f);

    // Particle system references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UParticleSystem* FootstepDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UParticleSystem* BloodSplatterSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UParticleSystem* DustCloudSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UParticleSystem* RockDebrisSystem;

protected:
    // Internal helper functions
    void SpawnParticleEffect(UParticleSystem* ParticleSystem, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    // VFX pools for performance
    UPROPERTY()
    TArray<UParticleSystemComponent*> ActiveParticleComponents;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveParticles = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config", meta = (AllowPrivateAccess = "true"))
    float ParticleLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config", meta = (AllowPrivateAccess = "true"))
    bool bEnableVFXLOD = true;
};