#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AVFXManager* AVFXManager::Instance = nullptr;

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update performance metrics 10x per second
    
    MaxActiveEffects = 150; // Conservative limit for 60fps
    PerformanceBudgetMS = 2.0f; // 2ms budget for VFX per frame
    CurrentFrameTimeMS = 0.0f;
    CurrentActiveCount = 0;
    
    Instance = this;
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    InitializePools();
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdatePerformanceMetrics();
    CullDistantEffects();
    AdaptiveQualityAdjustment();
}

void AVFXManager::InitializePools()
{
    // Pre-allocate pools for common effects
    for (auto& CategoryPair : VFXPools)
    {
        EVFXCategory Category = CategoryPair.Key;
        TArray<FVFXPoolData>& PoolDataArray = CategoryPair.Value;
        
        for (FVFXPoolData& PoolData : PoolDataArray)
        {
            if (PoolData.EffectAsset)
            {
                TArray<UNiagaraComponent*> Pool;
                for (int32 i = 0; i < PoolData.PoolSize; ++i)
                {
                    UNiagaraComponent* Component = CreateDefaultSubobject<UNiagaraComponent>(
                        *FString::Printf(TEXT("PooledVFX_%s_%d"), *PoolData.EffectAsset->GetName(), i)
                    );
                    Component->SetAsset(PoolData.EffectAsset);
                    Component->SetAutoDestroy(false);
                    Component->SetVisibility(false);
                    Pool.Add(Component);
                }
                PooledEffects.Add(PoolData.EffectAsset, Pool);
            }
        }
    }
}

UNiagaraComponent* AVFXManager::SpawnVFX(UNiagaraSystem* Effect, FVector Location, FRotator Rotation, EVFXPriority Priority)
{
    if (!Effect || !CanSpawnNewEffect(Priority))
    {
        return nullptr;
    }
    
    UNiagaraComponent* Component = GetPooledComponent(Effect);
    if (!Component)
    {
        // Create new component if pool is empty
        Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), Effect, Location, Rotation
        );
    }
    else
    {
        Component->SetWorldLocationAndRotation(Location, Rotation);
        Component->SetVisibility(true);
        Component->Activate(true);
    }
    
    if (Component)
    {
        ActiveEffects.Add(Component);
        CurrentActiveCount++;
    }
    
    return Component;
}

UNiagaraComponent* AVFXManager::SpawnVFXAttached(UNiagaraSystem* Effect, USceneComponent* AttachComponent, FName AttachPoint, EVFXPriority Priority)
{
    if (!Effect || !AttachComponent || !CanSpawnNewEffect(Priority))
    {
        return nullptr;
    }
    
    UNiagaraComponent* Component = GetPooledComponent(Effect);
    if (!Component)
    {
        Component = UNiagaraFunctionLibrary::SpawnSystemAttached(
            Effect, AttachComponent, AttachPoint, FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset, true
        );
    }
    else
    {
        Component->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
        Component->SetVisibility(true);
        Component->Activate(true);
    }
    
    if (Component)
    {
        ActiveEffects.Add(Component);
        CurrentActiveCount++;
    }
    
    return Component;
}

void AVFXManager::UpdatePerformanceMetrics()
{
    // Simple frame time tracking
    CurrentFrameTimeMS = FApp::GetDeltaTime() * 1000.0f;
    
    // Clean up finished effects
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!Effect || !Effect->IsActive() || Effect->IsComplete())
        {
            ReturnToPool(Effect);
            ActiveEffects.RemoveAt(i);
            CurrentActiveCount--;
        }
    }
}

void AVFXManager::CullDistantEffects()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (Effect)
        {
            float Distance = FVector::Dist(Effect->GetComponentLocation(), PlayerLocation);
            
            // Determine max distance based on priority (simplified)
            float MaxDistance = 1500.0f; // Default medium priority
            
            if (Distance > MaxDistance)
            {
                Effect->Deactivate();
                ReturnToPool(Effect);
                ActiveEffects.RemoveAt(i);
                CurrentActiveCount--;
            }
        }
    }
}

bool AVFXManager::CanSpawnNewEffect(EVFXPriority Priority)
{
    // Always allow critical effects
    if (Priority == EVFXPriority::Critical)
    {
        return true;
    }
    
    // Check active count limit
    if (CurrentActiveCount >= MaxActiveEffects)
    {
        return false;
    }
    
    // Check frame time budget
    if (CurrentFrameTimeMS > PerformanceBudgetMS && Priority != EVFXPriority::High)
    {
        return false;
    }
    
    return true;
}

void AVFXManager::AdaptiveQualityAdjustment()
{
    // If we're consistently over budget, reduce quality
    if (CurrentFrameTimeMS > PerformanceBudgetMS * 1.5f)
    {
        ForceCleanupLowPriorityEffects();
    }
}

void AVFXManager::ForceCleanupLowPriorityEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (Effect)
        {
            // Simplified priority check - in real implementation, 
            // we'd store priority with each effect
            Effect->Deactivate();
            ReturnToPool(Effect);
            ActiveEffects.RemoveAt(i);
            CurrentActiveCount--;
        }
    }
}

UNiagaraComponent* AVFXManager::GetPooledComponent(UNiagaraSystem* Effect)
{
    TArray<UNiagaraComponent*>* Pool = PooledEffects.Find(Effect);
    if (Pool && Pool->Num() > 0)
    {
        return Pool->Pop();
    }
    return nullptr;
}

void AVFXManager::ReturnToPool(UNiagaraComponent* Component)
{
    if (!Component)
    {
        return;
    }
    
    Component->SetVisibility(false);
    Component->Deactivate();
    Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    
    UNiagaraSystem* Asset = Component->GetAsset();
    if (Asset)
    {
        TArray<UNiagaraComponent*>* Pool = PooledEffects.Find(Asset);
        if (Pool)
        {
            Pool->Add(Component);
        }
    }
}

void AVFXManager::SetPerformanceLevel(int32 Level)
{
    switch (Level)
    {
        case 0: // Ultra
            MaxActiveEffects = 200;
            PerformanceBudgetMS = 3.0f;
            break;
        case 1: // High
            MaxActiveEffects = 150;
            PerformanceBudgetMS = 2.0f;
            break;
        case 2: // Medium
            MaxActiveEffects = 100;
            PerformanceBudgetMS = 1.5f;
            break;
        case 3: // Low
            MaxActiveEffects = 50;
            PerformanceBudgetMS = 1.0f;
            break;
    }
}

void AVFXManager::ShowVFXDebugInfo(bool bShow)
{
    // Implementation for debug visualization
    // Would show active effect count, performance metrics, etc.
}

AVFXManager* AVFXManager::GetInstance(UWorld* World)
{
    if (!Instance && World)
    {
        // Find existing instance or create new one
        Instance = World->SpawnActor<AVFXManager>();
    }
    return Instance;
}