#include "VFXManager.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

// Static instance
AVFXManager* AVFXManager::Instance = nullptr;

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Default performance settings
    MaxActiveEffects = 50;
    PerformanceBudgetMS = 2.0f; // 2ms budget for VFX per frame
    CurrentFrameTimeMS = 0.0f;
    CurrentActiveCount = 0;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set singleton instance
    Instance = this;
    
    // Initialize VFX pools
    InitializePools();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Manager initialized with %d max effects"), MaxActiveEffects);
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Cull distant effects
    CullDistantEffects();
    
    // Adaptive quality adjustment
    AdaptiveQualityAdjustment();
}

UNiagaraComponent* AVFXManager::SpawnVFX(UNiagaraSystem* Effect, FVector Location, FRotator Rotation, EVFXPriority Priority)
{
    if (!Effect || !CanSpawnNewEffect(Priority))
    {
        return nullptr;
    }

    // Try to get pooled component first
    UNiagaraComponent* VFXComponent = GetPooledComponent(Effect);
    
    if (!VFXComponent)
    {
        // Create new component if pool is empty
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            Effect,
            Location,
            Rotation,
            FVector::OneVector,
            true,
            true,
            ENCPoolMethod::None,
            true
        );
    }
    else
    {
        // Reuse pooled component
        VFXComponent->SetWorldLocationAndRotation(Location, Rotation);
        VFXComponent->Activate(true);
    }

    if (VFXComponent)
    {
        ActiveEffects.Add(VFXComponent);
        CurrentActiveCount = ActiveEffects.Num();
        
        UE_LOG(LogTemp, Log, TEXT("VFX spawned at location: %s, Priority: %d"), 
               *Location.ToString(), (int32)Priority);
    }

    return VFXComponent;
}

UNiagaraComponent* AVFXManager::SpawnVFXAttached(UNiagaraSystem* Effect, USceneComponent* AttachComponent, FName AttachPoint, EVFXPriority Priority)
{
    if (!Effect || !AttachComponent || !CanSpawnNewEffect(Priority))
    {
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        Effect,
        AttachComponent,
        AttachPoint,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true
    );

    if (VFXComponent)
    {
        ActiveEffects.Add(VFXComponent);
        CurrentActiveCount = ActiveEffects.Num();
        
        UE_LOG(LogTemp, Log, TEXT("VFX attached to component: %s, Priority: %d"), 
               *AttachComponent->GetName(), (int32)Priority);
    }

    return VFXComponent;
}

void AVFXManager::SetPerformanceLevel(int32 Level)
{
    // 0=Ultra, 1=High, 2=Medium, 3=Low
    Level = FMath::Clamp(Level, 0, 3);
    
    switch (Level)
    {
        case 0: // Ultra
            MaxActiveEffects = 100;
            PerformanceBudgetMS = 4.0f;
            break;
        case 1: // High
            MaxActiveEffects = 75;
            PerformanceBudgetMS = 3.0f;
            break;
        case 2: // Medium
            MaxActiveEffects = 50;
            PerformanceBudgetMS = 2.0f;
            break;
        case 3: // Low
            MaxActiveEffects = 25;
            PerformanceBudgetMS = 1.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Performance Level set to %d (Max Effects: %d)"), Level, MaxActiveEffects);
}

void AVFXManager::ForceCleanupLowPriorityEffects()
{
    int32 RemovedCount = 0;
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Component = ActiveEffects[i];
        if (Component && IsValid(Component))
        {
            // Remove low priority effects first
            // This is a simplified version - in practice you'd check the priority stored with each effect
            if (RemovedCount < 10) // Remove up to 10 effects
            {
                ReturnToPool(Component);
                ActiveEffects.RemoveAt(i);
                RemovedCount++;
            }
        }
        else
        {
            // Remove invalid components
            ActiveEffects.RemoveAt(i);
        }
    }
    
    CurrentActiveCount = ActiveEffects.Num();
    UE_LOG(LogTemp, Log, TEXT("Cleaned up %d low priority VFX effects"), RemovedCount);
}

void AVFXManager::ShowVFXDebugInfo(bool bShow)
{
    if (bShow)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== VFX DEBUG INFO ==="));
        UE_LOG(LogTemp, Warning, TEXT("Active Effects: %d / %d"), CurrentActiveCount, MaxActiveEffects);
        UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms (Budget: %.2f ms)"), CurrentFrameTimeMS, PerformanceBudgetMS);
        UE_LOG(LogTemp, Warning, TEXT("Pool Status:"));
        
        for (auto& Pool : PooledEffects)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s: %d pooled"), 
                   Pool.Key ? *Pool.Key->GetName() : TEXT("NULL"), 
                   Pool.Value.Num());
        }
    }
}

void AVFXManager::InitializePools()
{
    // Initialize pools for common VFX categories
    // This would typically be configured via data assets or config files
    
    UE_LOG(LogTemp, Log, TEXT("VFX Pools initialized"));
}

void AVFXManager::UpdatePerformanceMetrics()
{
    // Simple frame time tracking
    static float LastFrameTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (LastFrameTime > 0.0f)
    {
        float DeltaTime = CurrentTime - LastFrameTime;
        CurrentFrameTimeMS = DeltaTime * 1000.0f;
    }
    
    LastFrameTime = CurrentTime;
}

void AVFXManager::CullDistantEffects()
{
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Component = ActiveEffects[i];
        if (!Component || !IsValid(Component))
        {
            ActiveEffects.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(Component->GetComponentLocation(), PlayerLocation);
        
        // Cull effects beyond 5000 units (50 meters)
        if (Distance > 5000.0f)
        {
            ReturnToPool(Component);
            ActiveEffects.RemoveAt(i);
        }
    }
    
    CurrentActiveCount = ActiveEffects.Num();
}

UNiagaraComponent* AVFXManager::GetPooledComponent(UNiagaraSystem* Effect)
{
    if (!Effect)
        return nullptr;
    
    TArray<UNiagaraComponent*>* Pool = PooledEffects.Find(Effect);
    if (Pool && Pool->Num() > 0)
    {
        UNiagaraComponent* Component = (*Pool)[0];
        Pool->RemoveAt(0);
        return Component;
    }
    
    return nullptr;
}

void AVFXManager::ReturnToPool(UNiagaraComponent* Component)
{
    if (!Component || !IsValid(Component))
        return;
    
    // Deactivate the component
    Component->Deactivate();
    
    // Get the system asset
    UNiagaraSystem* System = Component->GetAsset();
    if (System)
    {
        // Add to appropriate pool
        TArray<UNiagaraComponent*>& Pool = PooledEffects.FindOrAdd(System);
        Pool.Add(Component);
        
        // Limit pool size to prevent memory bloat
        if (Pool.Num() > 20)
        {
            UNiagaraComponent* OldComponent = Pool[0];
            Pool.RemoveAt(0);
            if (OldComponent && IsValid(OldComponent))
            {
                OldComponent->DestroyComponent();
            }
        }
    }
    else
    {
        // No system asset, just destroy
        Component->DestroyComponent();
    }
}

bool AVFXManager::CanSpawnNewEffect(EVFXPriority Priority)
{
    // Always allow critical effects
    if (Priority == EVFXPriority::Critical)
        return true;
    
    // Check active count limit
    if (CurrentActiveCount >= MaxActiveEffects)
        return false;
    
    // Check performance budget
    if (CurrentFrameTimeMS > PerformanceBudgetMS * 1.5f)
        return false;
    
    return true;
}

void AVFXManager::AdaptiveQualityAdjustment()
{
    // If we're over budget consistently, reduce quality
    static int32 OverBudgetFrames = 0;
    
    if (CurrentFrameTimeMS > PerformanceBudgetMS)
    {
        OverBudgetFrames++;
        
        if (OverBudgetFrames > 60) // 1 second at 60fps
        {
            // Force cleanup of some effects
            ForceCleanupLowPriorityEffects();
            OverBudgetFrames = 0;
        }
    }
    else
    {
        OverBudgetFrames = FMath::Max(0, OverBudgetFrames - 1);
    }
}

AVFXManager* AVFXManager::GetInstance(UWorld* World)
{
    if (!Instance && World)
    {
        // Try to find existing instance
        for (TActorIterator<AVFXManager> ActorItr(World); ActorItr; ++ActorItr)
        {
            Instance = *ActorItr;
            break;
        }
        
        // Create new instance if none found
        if (!Instance)
        {
            Instance = World->SpawnActor<AVFXManager>();
        }
    }
    
    return Instance;
}