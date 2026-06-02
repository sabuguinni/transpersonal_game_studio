#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "VFX_NiagaraLibrary.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire UMETA(DisplayName = "Campfire"),
    Dust_FootstepImpact UMETA(DisplayName = "Footstep Impact"),
    Blood_Splatter UMETA(DisplayName = "Blood Splatter"),
    Weather_Rain UMETA(DisplayName = "Rain"),
    Weather_Snow UMETA(DisplayName = "Snow"),
    Volcanic_Ash UMETA(DisplayName = "Volcanic Ash"),
    Water_Splash UMETA(DisplayName = "Water Splash"),
    Breath_Vapor UMETA(DisplayName = "Breath Vapor"),
    Combat_Impact UMETA(DisplayName = "Combat Impact"),
    Crafting_Sparks UMETA(DisplayName = "Crafting Sparks")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        Duration = 5.0f;
        Intensity = 1.0f;
        bLooping = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnEffectWithData(const FVFX_EffectData& EffectData);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    // Prehistoric Specific Effects
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void CreateFootstepImpact(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void CreateBloodSplatter(const FVector& Location, const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateCampfire(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StartRain(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StopRain();

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void CreateVolcanicAsh(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Player")
    void CreateBreathVapor(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Crafting")
    void CreateCraftingSparks(const FVector& Location);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void CleanupExpiredEffects();

protected:
    // Effect Templates
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Templates")
    TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> EffectTemplates;

    // Active Effects Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float EffectCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 CurrentLODLevel;

    // Weather Effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Weather")
    UNiagaraComponent* RainEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Weather")
    bool bIsRaining;

private:
    void InitializeEffectTemplates();
    UNiagaraSystem* GetEffectTemplate(EVFX_EffectType EffectType);
    void RegisterActiveEffect(UNiagaraComponent* Effect);
    void UnregisterActiveEffect(UNiagaraComponent* Effect);
    bool ShouldCullEffect(const FVector& EffectLocation) const;
    void ApplyLODSettings(UNiagaraComponent* Effect, int32 LODLevel);
};