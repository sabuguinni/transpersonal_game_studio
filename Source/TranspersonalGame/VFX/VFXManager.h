#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "../Core/SharedTypes.h"
#include "VFXManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy;

    FVFX_EffectData()
    {
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighQualityDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumQualityDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LowQualityDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects;

    FVFX_LODSettings()
    {
        HighQualityDistance = 500.0f;
        MediumQualityDistance = 1000.0f;
        LowQualityDistance = 2000.0f;
        MaxActiveEffects = 50;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_Manager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_Manager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TMap<EDir_VFXType, FVFX_EffectData> VFXLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVFX_LODSettings LODSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(EDir_VFXType VFXType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(EDir_VFXType VFXType, USceneComponent* AttachComponent, FName SocketName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFX();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLOD(FVector ViewerLocation);

protected:
    UFUNCTION()
    void CleanupFinishedEffects();

    UFUNCTION()
    float CalculateDistanceToViewer(FVector EffectLocation) const;

    UFUNCTION()
    void ApplyLODToEffect(UNiagaraComponent* Effect, float Distance);
};