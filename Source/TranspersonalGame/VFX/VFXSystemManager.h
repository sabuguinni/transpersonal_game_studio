#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "VFXSystemManager.generated.h"

// Forward declarations
class UNiagaraSystem;
class UNiagaraComponent;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Weather_Fog         UMETA(DisplayName = "Fog"),
    Impact_Footstep     UMETA(DisplayName = "Footstep"),
    Impact_DinoStep     UMETA(DisplayName = "Dinosaur Step"),
    Impact_RockHit      UMETA(DisplayName = "Rock Impact"),
    Blood_Splatter      UMETA(DisplayName = "Blood Splatter"),
    Dust_Cloud          UMETA(DisplayName = "Dust Cloud"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Smoke_General       UMETA(DisplayName = "Smoke"),
    Sparks_Tool         UMETA(DisplayName = "Tool Sparks"),
    Breath_Cold         UMETA(DisplayName = "Cold Breath")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor DefaultColor;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        DefaultScale = FVector(1.0f);
        DefaultLifetime = 5.0f;
        bAutoDestroy = true;
        DefaultColor = FLinearColor::White;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // VFX Effect Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Database")
    TArray<FVFX_EffectData> EffectDatabase;

    // Active VFX tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bEnableLODSystem;

public:
    // Core VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffectAttached(EVFX_EffectType EffectType, USceneComponent* AttachTo, FName SocketName = NAME_None, FVector RelativeLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* EffectComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    // Specific prehistoric VFX functions
    UFUNCTION(BlueprintCallable, Category = "VFX | Fire")
    UNiagaraComponent* CreateCampfire(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX | Impact")
    void CreateFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX | Weather")
    void StartRainEffect(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX | Weather")
    void StopRainEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX | Combat")
    void CreateBloodSplatter(FVector Location, FVector Direction, float Amount = 1.0f);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQualityLevel(int32 QualityLevel);

private:
    // Internal helper functions
    FVFX_EffectData* GetEffectData(EVFX_EffectType EffectType);
    void InitializeEffectDatabase();
    void RegisterActiveEffect(UNiagaraComponent* Effect);
    void UnregisterActiveEffect(UNiagaraComponent* Effect);
    bool ShouldCullEffect(UNiagaraComponent* Effect) const;

    // Performance tracking
    float LastCleanupTime;
    static const float CleanupInterval;
};