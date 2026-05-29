#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    Footstep_Light      UMETA(DisplayName = "Light Footstep"),
    Footstep_Heavy      UMETA(DisplayName = "Heavy Footstep"), 
    Footstep_Giant      UMETA(DisplayName = "Giant Footstep"),
    Combat_Hit          UMETA(DisplayName = "Combat Hit"),
    Combat_Bite         UMETA(DisplayName = "Bite Impact"),
    Environment_Splash  UMETA(DisplayName = "Water Splash"),
    Environment_Dust    UMETA(DisplayName = "Dust Cloud")
};

UENUM(BlueprintType)
enum class EVFX_TerrainType : uint8
{
    Grass       UMETA(DisplayName = "Grass"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Rock        UMETA(DisplayName = "Rock"),
    Water       UMETA(DisplayName = "Water"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    class UNiagaraSystem* NiagaraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    class UParticleSystem* LegacyParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float EffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact")
    bool bAttachToGround;

    FVFX_ImpactEffect()
    {
        NiagaraEffect = nullptr;
        LegacyParticleEffect = nullptr;
        EffectScale = 1.0f;
        Duration = 2.0f;
        bAttachToGround = true;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact Effects")
    TMap<EVFX_ImpactType, FVFX_ImpactEffect> ImpactEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Terrain Effects")
    TMap<EVFX_TerrainType, FVFX_ImpactEffect> TerrainEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxEffectDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableScreenShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float ScreenShakeIntensity;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnTerrainEffect(EVFX_TerrainType TerrainType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    EVFX_TerrainType DetectTerrainType(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnDinosaurFootstep(FVector Location, float DinosaurMass, EVFX_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SpawnCombatImpact(FVector Location, float Damage, bool bIsBite = false);

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    void InitializeDefaultEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    bool IsWithinEffectRange(FVector Location);

private:
    void LoadDefaultParticleSystems();
    void CreateScreenShakeEffect(FVector Location, float Intensity);
    
    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveNiagaraComponents;
    
    UPROPERTY()
    TArray<class UParticleSystemComponent*> ActiveParticleComponents;
};