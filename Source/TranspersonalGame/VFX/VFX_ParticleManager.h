#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "VFX_ParticleManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ParticleType : uint8
{
    Fire_Campfire UMETA(DisplayName = "Campfire"),
    Dust_DinosaurFootstep UMETA(DisplayName = "Dinosaur Footstep"),
    Weather_Rain UMETA(DisplayName = "Rain"),
    Weather_Snow UMETA(DisplayName = "Snow"),
    Blood_Impact UMETA(DisplayName = "Blood Impact"),
    Water_Splash UMETA(DisplayName = "Water Splash"),
    Smoke_Cooking UMETA(DisplayName = "Cooking Smoke"),
    Sparks_Crafting UMETA(DisplayName = "Crafting Sparks")
};

USTRUCT(BlueprintType)
struct FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    EVFX_ParticleType ParticleType = EVFX_ParticleType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Scale = 1.0f;

    FVFX_ParticleConfig()
    {
        ParticleType = EVFX_ParticleType::Fire_Campfire;
        SpawnLocation = FVector::ZeroVector;
        Duration = 5.0f;
        Scale = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ParticleManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ParticleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFX_ParticleConfig> ParticleConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoStartParticles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalScale = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopAllParticles();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StartAllParticles();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    UNiagaraComponent* CreateNiagaraComponent(UNiagaraSystem* System, FVector Location, float Scale = 1.0f);

private:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveParticleComponents;

    void InitializeDefaultParticles();
    UNiagaraSystem* GetNiagaraSystemForType(EVFX_ParticleType ParticleType);
};