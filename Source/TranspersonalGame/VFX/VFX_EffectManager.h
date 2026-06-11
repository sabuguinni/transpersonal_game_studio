#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "VFX_EffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Dust_Footstep      UMETA(DisplayName = "Dinosaur Footstep"),
    Weather_Rain       UMETA(DisplayName = "Rain"),
    Combat_BloodImpact UMETA(DisplayName = "Blood Impact"),
    Water_Splash       UMETA(DisplayName = "Water Splash"),
    Smoke_Cooking      UMETA(DisplayName = "Cooking Smoke"),
    Sparks_Crafting    UMETA(DisplayName = "Crafting Sparks")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    FVFX_EffectData()
    {
        NiagaraSystem = nullptr;
        Duration = 5.0f;
        Scale = 1.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EffectManager();

protected:
    virtual void BeginPlay() override;

    // VFX Effect Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Database")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectDatabase;

    // Active effect tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float CustomScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAtActor(EVFX_EffectType EffectType, AActor* TargetActor, FVector Offset = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    // Prehistoric environment effects
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void CreateCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void CreateDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void CreateRainEffect(FVector Location, float Intensity = 1.0f);

    // Combat effects
    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void CreateBloodImpact(FVector Location, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void CreateWaterSplash(FVector Location, float SplashSize = 1.0f);

    // Survival crafting effects
    UFUNCTION(BlueprintCallable, Category = "VFX Survival")
    void CreateCraftingSparks(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Survival")
    void CreateCookingSmoke(FVector Location);

    // Effect management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupFinishedEffects();

private:
    void InitializeEffectDatabase();
    void CleanupEffect(UNiagaraComponent* Effect);

    // Timer for automatic cleanup
    FTimerHandle CleanupTimerHandle;
    void PerformPeriodicCleanup();
};