#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFXImpactManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float ImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EVFX_ImpactType ImpactType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float DustCloudSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FLinearColor BloodColor;

    FVFX_ImpactData()
    {
        ImpactLocation = FVector::ZeroVector;
        ImpactForce = 100.0f;
        ImpactType = EVFX_ImpactType::Footstep;
        DustCloudSize = 200.0f;
        BloodColor = FLinearColor::Red;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* DustParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* BloodParticleComponent;

    // VFX Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxImpactDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float DustLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float BloodLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFX_ImpactData> ActiveImpacts;

public:
    virtual void Tick(float DeltaTime) override;

    // Impact VFX Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateFootstepImpact(const FVector& Location, float DinosaurMass);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateBloodSplatter(const FVector& Location, const FVector& Direction, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateDustCloud(const FVector& Location, float CloudSize);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateCombatImpact(const FVector& Location, EVFX_ImpactType Type, float Force);

    // System Management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void UpdateVFXLOD(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupExpiredEffects();

    UFUNCTION(CallInEditor, Category = "VFX Testing")
    void TestFootstepVFX();

    UFUNCTION(CallInEditor, Category = "VFX Testing")
    void TestBloodVFX();

private:
    void InitializeParticleSystems();
    void UpdateImpactEffects(float DeltaTime);
    float CalculateVFXIntensity(float Distance) const;
};