#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_BloodEffectManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_BloodSplatterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    FVector ImpactNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    float SplatterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    float BloodAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    FLinearColor BloodColor;

    FVFX_BloodSplatterData()
    {
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        SplatterRadius = 100.0f;
        BloodAmount = 1.0f;
        BloodColor = FLinearColor::Red;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_BloodEffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_BloodEffectManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    float MaxBloodDecals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    float DecalLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood VFX")
    bool bEnableBloodEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "Blood VFX")
    void CreateBloodSplatter(const FVFX_BloodSplatterData& SplatterData);

    UFUNCTION(BlueprintCallable, Category = "Blood VFX")
    void CreateDinosaurBloodHit(FVector HitLocation, FVector HitNormal, float Damage);

    UFUNCTION(BlueprintCallable, Category = "Blood VFX")
    void CreatePlayerBloodHit(FVector HitLocation, FVector HitNormal, float Damage);

    UFUNCTION(BlueprintCallable, Category = "Blood VFX")
    void ClearAllBloodDecals();

private:
    TArray<AActor*> ActiveBloodDecals;

    void CleanupOldDecals();
    void SpawnBloodParticles(const FVFX_BloodSplatterData& SplatterData);
};