#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "VFXManager.generated.h"

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    Environmental,      // Chuva, vento, folhas caindo
    Creature,          // Efeitos de dinossauros (respiração, pegadas)
    Combat,            // Sangue, impactos, destruição
    Survival,          // Fogo, fumo, ferramentas
    Atmospheric,       // Neblina, raios de sol, partículas de ar
    Interaction        // Coleta de recursos, domesticação
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical = 0,      // Sempre renderiza (sangue, fogo)
    High = 1,          // Renderiza até 30m (pegadas, respiração)
    Medium = 2,        // Renderiza até 15m (folhas, poeira)
    Low = 3            // Renderiza até 5m (detalhes ambientais)
};

USTRUCT(BlueprintType)
struct FVFXPoolData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* EffectAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PoolSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EVFXPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance;

    FVFXPoolData()
    {
        EffectAsset = nullptr;
        PoolSize = 10;
        Priority = EVFXPriority::Medium;
        MaxDistance = 1500.0f;
    }
};

/**
 * Sistema centralizado de gestão de VFX com pooling automático e LOD chain
 * Garante 60fps PC / 30fps console através de culling inteligente
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Pool de efeitos por categoria
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Pools")
    TMap<EVFXCategory, TArray<FVFXPoolData>> VFXPools;

    // Componentes Niagara ativos
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    // Componentes em pool (inativas)
    UPROPERTY()
    TMap<UNiagaraSystem*, TArray<UNiagaraComponent*>> PooledEffects;

    // Performance tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceBudgetMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentActiveCount;

public:
    // Interface principal para spawnar efeitos
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFX(UNiagaraSystem* Effect, FVector Location, FRotator Rotation = FRotator::ZeroRotator, EVFXPriority Priority = EVFXPriority::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(UNiagaraSystem* Effect, USceneComponent* AttachComponent, FName AttachPoint = NAME_None, EVFXPriority Priority = EVFXPriority::Medium);

    // Gestão de performance
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(int32 Level); // 0=Ultra, 1=High, 2=Medium, 3=Low

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceCleanupLowPriorityEffects();

    // Debug e profiling
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowVFXDebugInfo(bool bShow);

private:
    void InitializePools();
    void UpdatePerformanceMetrics();
    void CullDistantEffects();
    UNiagaraComponent* GetPooledComponent(UNiagaraSystem* Effect);
    void ReturnToPool(UNiagaraComponent* Component);
    
    // Performance budgeting
    bool CanSpawnNewEffect(EVFXPriority Priority);
    void AdaptiveQualityAdjustment();

    // Singleton instance
    static AVFXManager* Instance;

public:
    static AVFXManager* GetInstance(UWorld* World);
};