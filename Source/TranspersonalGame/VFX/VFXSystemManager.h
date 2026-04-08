#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "VFXSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Atmospheric     UMETA(DisplayName = "Atmospheric"),
    Combat          UMETA(DisplayName = "Combat"),
    Environmental   UMETA(DisplayName = "Environmental"),
    Creature        UMETA(DisplayName = "Creature"),
    Survival        UMETA(DisplayName = "Survival"),
    Weather         UMETA(DisplayName = "Weather")
};

UENUM(BlueprintType)
enum class EVFXIntensity : uint8
{
    Subtle      UMETA(DisplayName = "Subtle"),
    Moderate    UMETA(DisplayName = "Moderate"),
    Intense     UMETA(DisplayName = "Intense"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FVFXDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXIntensity Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxInstances = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAffectsGameplay = false;

    FVFXDefinition()
    {
        EffectName = NAME_None;
        Category = EVFXCategory::Environmental;
        Intensity = EVFXIntensity::Moderate;
        MaxDistance = 5000.0f;
        MaxInstances = 10;
        bRequiresLOD = true;
        bAffectsGameplay = false;
    }
};

/**
 * Sistema central de gestão de VFX para o jogo jurássico
 * Gere pools de efeitos, LOD automático e performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Sistema de Pool de Efeitos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    TMap<FName, FVFXDefinition> RegisteredEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pool")
    int32 MaxActiveEffects = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxVFXBudgetMS = 2.0f;

private:
    // Pool de componentes Niagara reutilizáveis
    UPROPERTY()
    TArray<UNiagaraComponent*> AvailableComponents;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveComponents;

    // Sistema LOD
    float LastLODUpdate = 0.0f;
    
    // Performance tracking
    float CurrentFrameVFXTime = 0.0f;

public:
    // Interface pública para spawnar efeitos
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXEffect(FName EffectName, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(UNiagaraComponent* Effect);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void RegisterEffect(const FVFXDefinition& EffectDefinition);

    // Sistema LOD automático
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVFXLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVFXQualityLevel(int32 QualityLevel);

    // Debug e profiling
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowVFXDebugInfo(bool bShow);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    int32 GetActiveEffectCount() const { return ActiveComponents.Num(); }

private:
    UNiagaraComponent* GetPooledComponent();
    void ReturnComponentToPool(UNiagaraComponent* Component);
    void CleanupFinishedEffects();
    float CalculateDistanceToPlayer(const FVector& EffectLocation) const;
    int32 GetLODLevelForDistance(float Distance) const;
};