#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "VFX_FootstepEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_TerrainType : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Sand        UMETA(DisplayName = "Sand"),
    Rock        UMETA(DisplayName = "Rock"),
    Mud         UMETA(DisplayName = "Mud"),
    Snow        UMETA(DisplayName = "Snow")
};

USTRUCT(BlueprintType)
struct FVFX_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_TerrainType TerrainType = EVFX_TerrainType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DustIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float CreatureWeight = 1000.0f;

    FVFX_FootstepData()
    {
        TerrainType = EVFX_TerrainType::Dirt;
        DustIntensity = 1.0f;
        ImpactLocation = FVector::ZeroVector;
        CreatureWeight = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_FootstepEffectManager : public UObject
{
    GENERATED_BODY()

public:
    UVFX_FootstepEffectManager();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerFootstepEffect(const FVFX_FootstepData& FootstepData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerDinosaurFootstep(FVector Location, float Weight, EVFX_TerrainType Terrain);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerPlayerFootstep(FVector Location, EVFX_TerrainType Terrain);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    EVFX_TerrainType DetectTerrainType(FVector Location);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float BaseDustScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxDustLifetime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float WeightMultiplier = 0.001f;

private:
    void SpawnDustParticles(const FVFX_FootstepData& Data);
    void CreateGroundCrack(const FVFX_FootstepData& Data);
    void PlayFootstepSound(const FVFX_FootstepData& Data);
};