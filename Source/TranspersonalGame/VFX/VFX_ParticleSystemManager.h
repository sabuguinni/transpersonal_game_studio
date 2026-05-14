#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "VFX_ParticleSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Dust_Footstep       UMETA(DisplayName = "Dinosaur Footstep Dust"),
    Blood_Impact        UMETA(DisplayName = "Blood Impact"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Rain_Weather        UMETA(DisplayName = "Rain Weather"),
    Smoke_Fire          UMETA(DisplayName = "Fire Smoke"),
    Sparks_Crafting     UMETA(DisplayName = "Crafting Sparks"),
    Breath_Cold         UMETA(DisplayName = "Cold Breath Vapor")
};

USTRUCT(BlueprintType)
struct FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float LifeTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy = true;

    FVFX_ParticleConfig()
    {
        ParticleType = EVFX_ParticleType::Fire_Campfire;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        LifeTime = 5.0f;
        Scale = 1.0f;
        bAutoDestroy = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ParticleSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ParticleSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Spawn particle effect at location
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnParticleEffect(const FVFX_ParticleConfig& Config);

    // Spawn particle effect attached to actor
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnParticleEffectAttached(const FVFX_ParticleConfig& Config, AActor* AttachActor, FName SocketName = NAME_None);

    // Stop all active particle effects
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllParticleEffects();

    // Get particle system for type
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraSystem* GetParticleSystemForType(EVFX_ParticleType ParticleType);

    // Spawn dinosaur footstep effect
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstepEffect(FVector FootLocation, float DinosaurSize = 1.0f);

    // Spawn campfire effect
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnCampfireEffect(FVector FireLocation);

    // Spawn blood impact effect
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodImpactEffect(FVector ImpactLocation, FVector ImpactNormal);

protected:
    // Particle system assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    TMap<EVFX_ParticleType, TSoftObjectPtr<UNiagaraSystem>> ParticleSystemMap;

    // Active particle components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveParticleComponents;

    // Initialize particle system map
    void InitializeParticleSystemMap();

    // Clean up finished particle effects
    void CleanupFinishedEffects();

    // Load particle system asset
    UNiagaraSystem* LoadParticleSystemAsset(const FString& AssetPath);
};