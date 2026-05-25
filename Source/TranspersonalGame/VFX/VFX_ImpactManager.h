#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystemComponent.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    Blood       UMETA(DisplayName = "Blood"),
    Dust        UMETA(DisplayName = "Dust"),
    Rock        UMETA(DisplayName = "Rock"),
    Water       UMETA(DisplayName = "Water"),
    Mud         UMETA(DisplayName = "Mud")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UParticleSystem* ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    FVFX_ImpactData()
    {
        ParticleEffect = nullptr;
        Scale = FVector(1.0f);
        Duration = 2.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ImpactManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float DefaultEffectDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    bool bAutoCleanupEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnBloodSplatter(FVector Location, FVector ImpactDirection, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnDustCloud(FVector Location, float Size = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void InitializeDefaultEffects();

private:
    UPROPERTY()
    TArray<UParticleSystemComponent*> ActiveEffects;

    void CleanupExpiredEffects();
};