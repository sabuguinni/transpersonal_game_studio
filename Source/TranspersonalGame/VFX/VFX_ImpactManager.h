#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
    RockImpact      UMETA(DisplayName = "Rock Impact"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    FireSparks      UMETA(DisplayName = "Fire Sparks")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    EVFX_ImpactType ImpactType = EVFX_ImpactType::FootstepDust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float ImpactIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float Duration = 2.0f;

    FVFX_ImpactData()
    {
        ImpactType = EVFX_ImpactType::FootstepDust;
        ImpactLocation = FVector::ZeroVector;
        ImpactIntensity = 1.0f;
        Duration = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    UVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UStaticMeshComponent* RootMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UParticleSystemComponent* DustParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UParticleSystemComponent* BloodParticleComponent;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerImpact(const FVFX_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerFootstepDust(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerBloodSplatter(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void CreateDustCloud(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void StopAllEffects();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float DefaultDustScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float DefaultBloodScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoCleanupEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectLifetime = 5.0f;

private:
    TArray<FVFX_ImpactData> ActiveImpacts;
    float CleanupTimer = 0.0f;

    void UpdateActiveEffects(float DeltaTime);
    void CleanupExpiredEffects();
};