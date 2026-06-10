#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFX_NiagaraLibrary.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Dust_Footstep      UMETA(DisplayName = "Footstep Dust"),
    Weather_Rain       UMETA(DisplayName = "Rain"),
    Weather_Snow       UMETA(DisplayName = "Snow"),
    Impact_Blood       UMETA(DisplayName = "Blood Impact"),
    Impact_Rock        UMETA(DisplayName = "Rock Impact"),
    Breath_Steam       UMETA(DisplayName = "Breath Steam"),
    Water_Splash       UMETA(DisplayName = "Water Splash")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Scale = FVector(1.0f);
        Duration = 5.0f;
        bAutoDestroy = true;
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

    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(EVFX_EffectType EffectType, USceneComponent* AttachToComponent, FName AttachPointName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFX();

    // Prehistoric-specific VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayDinosaurFootstepVFX(FVector ImpactLocation, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void PlayCampfireVFX(FVector FireLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StartRainVFX(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StopRainVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void PlayImpactVFX(FVector ImpactLocation, EVFX_EffectType ImpactType);

protected:
    // VFX effect library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Library")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectLibrary;

    // Active VFX components
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    // Weather VFX state
    UPROPERTY()
    UNiagaraComponent* CurrentWeatherVFX;

    // Initialize default effects
    void InitializeEffectLibrary();

    // Helper functions
    UNiagaraSystem* GetEffectSystem(EVFX_EffectType EffectType);
    void CleanupFinishedVFX();
};