#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AVFXSystemManager::AVFXSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS tick for performance monitoring
}

void AVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize component pool
    for (int32 i = 0; i < MaxActiveEffects; i++)
    {
        UNiagaraComponent* Component = CreateDefaultSubobject<UNiagaraComponent>(FName(*FString::Printf(TEXT("PooledVFX_%d"), i)));
        Component->SetAutoDestroy(false);
        Component->SetVisibility(false);
        AvailableComponents.Add(Component);
    }

    // Start performance monitoring
    GetWorldTimerManager().SetTimer(PerformanceCheckTimer, this, &AVFXSystemManager::PerformanceCheck, PerformanceCheckInterval, true);

    UE_LOG(LogTemp, Log, TEXT("VFX System Manager initialized with %d pooled components"), MaxActiveEffects);
}

void AVFXSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Clean up finished effects
    for (int32 i = ActiveComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Component = ActiveComponents[i];
        if (!Component || !Component->IsActive() || Component->IsComplete())
        {
            ReturnComponentToPool(Component);
            ActiveComponents.RemoveAt(i);
        }
    }
}

UNiagaraComponent* AVFXSystemManager::SpawnVFXAtLocation(const FString& EffectName, const FVector& Location, const FRotator& Rotation)
{
    // Find effect definition
    FVFXDefinition* EffectDef = nullptr;
    for (FVFXDefinition& Def : VFXRegistry)
    {
        if (Def.EffectName == EffectName)
        {
            EffectDef = &Def;
            break;
        }
    }

    if (!EffectDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect not found: %s"), *EffectName);
        return nullptr;
    }

    // Check performance limits
    if (ActiveComponents.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum VFX limit reached, skipping effect: %s"), *EffectName);
        return nullptr;
    }

    // Get pooled component
    UNiagaraComponent* Component = GetPooledComponent();
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("No available VFX components in pool"));
        return nullptr;
    }

    // Configure component
    if (EffectDef->NiagaraSystem.LoadSynchronous())
    {
        Component->SetAsset(EffectDef->NiagaraSystem.Get());
        Component->SetWorldLocationAndRotation(Location, Rotation);
        Component->SetWorldScale3D(EffectDef->DefaultScale);
        Component->SetVisibility(true);
        Component->SetAutoDestroy(EffectDef->bAutoDestroy);
        
        // Performance settings
        Component->SetMaxSimTime(EffectDef->DefaultLifetime);
        
        // LOD settings based on current performance level
        float LODDistanceScale = 1.0f;
        switch (CurrentPerformanceLevel)
        {
            case 0: LODDistanceScale = 1.0f; break;   // Ultra
            case 1: LODDistanceScale = 0.8f; break;  // High
            case 2: LODDistanceScale = 0.6f; break;  // Medium
            case 3: LODDistanceScale = 0.4f; break;  // Low
        }
        
        Component->SetMaxSimTime(EffectDef->DefaultLifetime * LODDistanceScale);
        
        // Activate
        Component->Activate(true);
        ActiveComponents.Add(Component);

        UE_LOG(LogTemp, Log, TEXT("Spawned VFX: %s at location %s"), *EffectName, *Location.ToString());
        return Component;
    }

    ReturnComponentToPool(Component);
    return nullptr;
}

UNiagaraComponent* AVFXSystemManager::SpawnVFXAttached(const FString& EffectName, USceneComponent* AttachComponent, const FVector& LocationOffset)
{
    if (!AttachComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot attach VFX %s: AttachComponent is null"), *EffectName);
        return nullptr;
    }

    UNiagaraComponent* Component = SpawnVFXAtLocation(EffectName, AttachComponent->GetComponentLocation() + LocationOffset, AttachComponent->GetComponentRotation());
    
    if (Component)
    {
        Component->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepWorldTransform);
    }

    return Component;
}

void AVFXSystemManager::StopVFX(UNiagaraComponent* VFXComponent)
{
    if (VFXComponent && ActiveComponents.Contains(VFXComponent))
    {
        VFXComponent->DeactivateImmediate();
        ReturnComponentToPool(VFXComponent);
        ActiveComponents.Remove(VFXComponent);
    }
}

void AVFXSystemManager::StopAllVFXOfCategory(EVFXCategory Category)
{
    for (int32 i = ActiveComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Component = ActiveComponents[i];
        if (Component)
        {
            // Find the effect definition to check category
            for (const FVFXDefinition& Def : VFXRegistry)
            {
                if (Def.Category == Category && Component->GetAsset() == Def.NiagaraSystem.Get())
                {
                    StopVFX(Component);
                    break;
                }
            }
        }
    }
}

void AVFXSystemManager::SetPerformanceLevel(int32 Level)
{
    CurrentPerformanceLevel = FMath::Clamp(Level, 0, 3);
    
    // Adjust active effects based on performance level
    OptimizeActiveEffects();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Performance level set to: %d"), CurrentPerformanceLevel);
}

float AVFXSystemManager::GetCurrentGPUTime() const
{
    return LastGPUTime;
}

UNiagaraComponent* AVFXSystemManager::GetPooledComponent()
{
    if (AvailableComponents.Num() > 0)
    {
        UNiagaraComponent* Component = AvailableComponents.Pop();
        return Component;
    }
    return nullptr;
}

void AVFXSystemManager::ReturnComponentToPool(UNiagaraComponent* Component)
{
    if (Component)
    {
        Component->DeactivateImmediate();
        Component->SetVisibility(false);
        Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        Component->SetAsset(nullptr);
        
        if (!AvailableComponents.Contains(Component))
        {
            AvailableComponents.Add(Component);
        }
    }
}

void AVFXSystemManager::PerformanceCheck()
{
    // Simple GPU time estimation based on active effects
    LastGPUTime = ActiveComponents.Num() * 0.1f; // Rough estimate: 0.1ms per effect

    // If we're over budget, reduce quality
    if (LastGPUTime > MaxGPUTime && CurrentPerformanceLevel < 3)
    {
        SetPerformanceLevel(CurrentPerformanceLevel + 1);
        UE_LOG(LogTemp, Warning, TEXT("VFX Performance degraded due to high GPU time: %.2fms"), LastGPUTime);
    }
    // If we have headroom, increase quality
    else if (LastGPUTime < MaxGPUTime * 0.5f && CurrentPerformanceLevel > 0)
    {
        SetPerformanceLevel(CurrentPerformanceLevel - 1);
        UE_LOG(LogTemp, Log, TEXT("VFX Performance improved, GPU time: %.2fms"), LastGPUTime);
    }
}

void AVFXSystemManager::OptimizeActiveEffects()
{
    // Reduce particle counts and simulation complexity based on performance level
    for (UNiagaraComponent* Component : ActiveComponents)
    {
        if (Component && Component->GetAsset())
        {
            float QualityScale = 1.0f;
            switch (CurrentPerformanceLevel)
            {
                case 0: QualityScale = 1.0f; break;   // Ultra
                case 1: QualityScale = 0.8f; break;  // High
                case 2: QualityScale = 0.6f; break;  // Medium
                case 3: QualityScale = 0.4f; break;  // Low
            }
            
            // Apply quality scaling to particle systems
            Component->SetFloatParameter(TEXT("QualityScale"), QualityScale);
        }
    }
}