#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    Footstep,
    DustCloud,
    Campfire,
    BloodSplatter,
    RockImpact,
    WaterSplash,
    BreathVapor,
    DinosaurRoar
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::None;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 2.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_Manager : public AActor
{
    GENERATED_BODY()

public:
    UVFX_Manager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Active")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepEffect(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDustCloud(FVector Location, FVector Velocity = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodEffect(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupFinishedEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void InitializeEffectDatabase();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

private:
    void CreateBasicEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation);
    void SetupDefaultEffects();
    
    UPROPERTY()
    float EffectCleanupTimer = 0.0f;
    
    static constexpr float CLEANUP_INTERVAL = 5.0f;
};