#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "SharedTypes.h"
#include "VFX_DinosaurImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_DinosaurImpactType : uint8
{
    Footstep_Small      UMETA(DisplayName = "Small Dinosaur Footstep"),
    Footstep_Medium     UMETA(DisplayName = "Medium Dinosaur Footstep"),
    Footstep_Large      UMETA(DisplayName = "Large Dinosaur Footstep"),
    Footstep_Massive    UMETA(DisplayName = "Massive Dinosaur Footstep"),
    TailSlam            UMETA(DisplayName = "Tail Slam Impact"),
    BodyFall            UMETA(DisplayName = "Body Fall Impact")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_DinosaurImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> DustCloudEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> DebrisEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ImpactRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float DustIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float DebrisCount;

    FVFX_DinosaurImpactData()
    {
        ImpactRadius = 200.0f;
        DustIntensity = 1.0f;
        DebrisCount = 10.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_DinosaurImpactManager : public UObject
{
    GENERATED_BODY()

public:
    UVFX_DinosaurImpactManager();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerDinosaurImpact(EVFX_DinosaurImpactType ImpactType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerFootstepImpact(FVector Location, float DinosaurMass, ETerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerTailSlamImpact(FVector Location, FVector Direction, float Force);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetupImpactData();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Data")
    TMap<EVFX_DinosaurImpactType, FVFX_DinosaurImpactData> ImpactDataMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effects")
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float EffectLifetime;

private:
    void SpawnDustCloud(const FVFX_DinosaurImpactData& ImpactData, FVector Location, FRotator Rotation);
    void SpawnDebris(const FVFX_DinosaurImpactData& ImpactData, FVector Location, FRotator Rotation);
    void PlayImpactAudio(const FVFX_DinosaurImpactData& ImpactData, FVector Location);
    float CalculateImpactIntensity(float DinosaurMass, ETerrainType TerrainType);
    UWorld* GetWorld() const;
};