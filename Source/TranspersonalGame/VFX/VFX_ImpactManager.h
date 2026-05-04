#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep,
    WeaponHit,
    RockFall,
    TreeFall,
    BloodSplatter,
    DustCloud
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EVFX_ImpactType ImpactType = EVFX_ImpactType::DinosaurFootstep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float ImpactForce = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactNormal = FVector::UpVector;

    FVFX_ImpactData()
    {
        ImpactType = EVFX_ImpactType::DinosaurFootstep;
        ImpactLocation = FVector::ZeroVector;
        ImpactForce = 1.0f;
        ImpactNormal = FVector::UpVector;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UVFX_ImpactManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float ParticleLifetime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxParticleDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveParticles = 50;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    int32 CurrentActiveParticles = 0;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerImpact(const FVFX_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateFootstepVFX(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateBloodVFX(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateDustCloudVFX(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CleanupOldParticles();

protected:
    UFUNCTION()
    void SpawnParticleSystem(FVector Location, EVFX_ImpactType Type);

    UFUNCTION()
    bool IsLocationValid(FVector Location) const;

private:
    TArray<AActor*> ActiveParticleActors;
    float LastCleanupTime = 0.0f;
};