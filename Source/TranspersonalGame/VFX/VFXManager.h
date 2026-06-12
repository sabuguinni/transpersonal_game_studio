#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SharedTypes.h"
#include "VFXManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector DefaultScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectData()
    {
        NiagaraSystem = nullptr;
        DefaultScale = FVector(1.0f);
        DefaultLifetime = 5.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // VFX Effect Library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Library")
    TMap<FName, FVFX_EffectData> VFXLibrary;

    // Active VFX tracking
    UPROPERTY(BlueprintReadOnly, Category = "VFX Runtime")
    TArray<UNiagaraComponent*> ActiveVFXComponents;

public:
    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(FName EffectName, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(FName EffectName, USceneComponent* AttachComponent, FName AttachPointName = NAME_None, FVector RelativeLocation = FVector::ZeroVector, FRotator RelativeRotation = FRotator::ZeroRotator, FVector Scale = FVector(1.0f));

    // Preset effect functions for common gameplay scenarios
    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void SpawnFootstepDust(FVector Location, float IntensityScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void SpawnBloodSplatter(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX Presets")
    void SpawnRainEffect(FVector Location, float Intensity = 1.0f);

    // VFX management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXComponent(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterVFXEffect(FName EffectName, UNiagaraSystem* NiagaraSystem, float Lifetime = 5.0f, bool bAutoDestroy = true);

    // Cleanup and optimization
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupFinishedVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount() const { return ActiveVFXComponents.Num(); }

private:
    void InitializeVFXLibrary();
    void CleanupVFXComponent(UNiagaraComponent* Component);
    
    // Performance tracking
    float LastCleanupTime;
    static constexpr float CleanupInterval = 1.0f;
};