#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_NiagaraManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Dust_Footstep       UMETA(DisplayName = "Footstep Dust"),
    Blood_Splatter      UMETA(DisplayName = "Blood Impact"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Volcanic_Ash        UMETA(DisplayName = "Volcanic Ash"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Combat_Impact       UMETA(DisplayName = "Combat Impact"),
    Breath_Steam        UMETA(DisplayName = "Breath Steam"),
    Crafting_Sparks     UMETA(DisplayName = "Crafting Sparks")
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
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    int32 LODLevel = 0;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Duration = 5.0f;
        Scale = 1.0f;
        bAutoDestroy = true;
        LODLevel = 0;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachToComponent, FName AttachPointName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    // Prehistoric-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void PlayDinosaurFootstepVFX(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void PlayBloodImpactVFX(FVector Location, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void PlayCampfireVFX(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StartWeatherVFX(EVFX_EffectType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StopWeatherVFX();

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void SetVFXLODLevel(int32 NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void CleanupFinishedEffects();

protected:
    // VFX data registry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
    TMap<EVFX_EffectType, FVFX_EffectData> VFXRegistry;

    // Active effect tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Weather effect tracking
    UPROPERTY()
    UNiagaraComponent* CurrentWeatherEffect;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance")
    int32 CurrentLODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Performance")
    float EffectCleanupInterval = 2.0f;

private:
    float LastCleanupTime = 0.0f;

    // Helper functions
    void InitializeVFXRegistry();
    void RegisterVFXEffect(EVFX_EffectType EffectType, const FString& SystemPath, float Duration = 5.0f, float Scale = 1.0f);
    UNiagaraComponent* CreateNiagaraComponent(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation);
    void ApplyLODSettings(UNiagaraComponent* Component, int32 LODLevel);
};