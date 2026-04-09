#include "VFXPoolManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogVFXPool);

UVFXPoolManager::UVFXPoolManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for pool management
    
    // Initialize default settings
    PoolManagerSettings.MaxTotalVFXInstances = 200;
    PoolManagerSettings.MaxPoolsPerCategory = 5;
    PoolManagerSettings.PoolCleanupInterval = 10.0f;
    PoolManagerSettings.UnusedPoolTimeout = 60.0f;
    PoolManagerSettings.bEnablePoolStatistics = true;
    PoolManagerSettings.bEnableAutoCleanup = true;
    PoolManagerSettings.bEnablePoolGrowth = true;
    PoolManagerSettings.bPreloadCriticalPools = true;
    
    NextPoolID = 0;
    LastCleanupTime = 0.0f;
    TotalActiveVFXCount = 0;
}

void UVFXPoolManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogVFXPool, Log, TEXT("VFXPoolManager: Initializing..."));
    
    // Initialize pools based on settings
    InitializeVFXPoolManager(PoolManagerSettings);
    
    // Preload critical pools if enabled
    if (PoolManagerSettings.bPreloadCriticalPools)
    {
        PreloadCriticalPools();
    }
    
    UE_LOG(LogVFXPool, Log, TEXT("VFXPoolManager: Initialization complete"));
}

void UVFXPoolManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Perform periodic cleanup
    if (PoolManagerSettings.bEnableAutoCleanup && 
        (CurrentTime - LastCleanupTime) >= PoolManagerSettings.PoolCleanupInterval)
    {
        CleanupUnusedPools();
        LastCleanupTime = CurrentTime;
    }
    
    // Update VFX LOD based on distance
    UpdateVFXLODBasedOnDistance();
    
    // Check for expired VFX instances
    RecycleExpiredVFXInstances();
}

void UVFXPoolManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all pools
    for (auto& CategoryPair : VFXPools)
    {
        for (auto& PoolPair : CategoryPair.Value)
        {
            FVFXPool& Pool = PoolPair.Value;
            for (FVFXPoolEntry& Entry : Pool.PoolEntries)
            {
                if (Entry.NiagaraComponent && IsValid(Entry.NiagaraComponent))
                {
                    Entry.NiagaraComponent->DestroyComponent();
                }
            }
            Pool.PoolEntries.Empty();
        }
    }
    VFXPools.Empty();
    
    UE_LOG(LogVFXPool, Log, TEXT("VFXPoolManager: Cleanup complete"));
    
    Super::EndPlay(EndPlayReason);
}

void UVFXPoolManager::InitializeVFXPoolManager(const FVFXPoolManagerSettings& Settings)
{
    PoolManagerSettings = Settings;
    
    UE_LOG(LogVFXPool, Log, TEXT("VFXPoolManager: Initialized with MaxTotalVFXInstances=%d"), 
           Settings.MaxTotalVFXInstances);
}

bool UVFXPoolManager::CreateVFXPool(const FVFXPoolData& PoolData)
{
    if (!PoolData.NiagaraSystemAsset.IsValid())
    {
        UE_LOG(LogVFXPool, Warning, TEXT("VFXPoolManager: Cannot create pool - invalid Niagara system asset"));
        return false;
    }
    
    // Check if we've reached the maximum pools per category
    if (VFXPools.Contains(PoolData.Category) && 
        VFXPools[PoolData.Category].Num() >= PoolManagerSettings.MaxPoolsPerCategory)
    {
        UE_LOG(LogVFXPool, Warning, TEXT("VFXPoolManager: Maximum pools per category reached for %s"), 
               *UEnum::GetValueAsString(PoolData.Category));
        return false;
    }
    
    // Create new pool
    FVFXPool NewPool;
    NewPool.NiagaraSystemAsset = PoolData.NiagaraSystemAsset;
    NewPool.Category = PoolData.Category;
    NewPool.Priority = PoolData.Priority;
    NewPool.MaxPoolSize = PoolData.MaxPoolSize;
    NewPool.InitialPoolSize = PoolData.InitialPoolSize;
    NewPool.bCanGrow = PoolData.bCanGrow;
    NewPool.bAutoRecycle = PoolData.bAutoRecycle;
    NewPool.RecycleTimeout = PoolData.RecycleTimeout;
    NewPool.PoolName = PoolData.PoolName;
    
    // Pre-allocate initial pool entries
    for (int32 i = 0; i < NewPool.InitialPoolSize; ++i)
    {
        FVFXPoolEntry NewEntry;
        NewEntry.PoolID = NextPoolID++;
        NewEntry.Category = PoolData.Category;
        NewEntry.Priority = PoolData.Priority;
        NewEntry.bIsInUse = false;
        NewEntry.LastUsedTime = 0.0f;
        
        // Create Niagara component but don't activate it yet
        NewEntry.NiagaraComponent = CreateNiagaraComponent(PoolData.NiagaraSystemAsset.Get());
        if (NewEntry.NiagaraComponent)
        {
            NewEntry.NiagaraComponent->SetAutoDestroy(false);
            NewEntry.NiagaraComponent->SetVisibility(false);
            NewPool.PoolEntries.Add(NewEntry);
        }
    }
    
    // Add pool to the manager
    if (!VFXPools.Contains(PoolData.Category))
    {
        VFXPools.Add(PoolData.Category, TMap<FString, FVFXPool>());
    }
    
    VFXPools[PoolData.Category].Add(PoolData.PoolName, NewPool);
    
    UE_LOG(LogVFXPool, Log, TEXT("VFXPoolManager: Created pool '%s' for category %s with %d initial entries"), 
           *PoolData.PoolName, *UEnum::GetValueAsString(PoolData.Category), NewPool.InitialPoolSize);
    
    return true;
}

UNiagaraComponent* UVFXPoolManager::CreateNiagaraComponent(UNiagaraSystem* NiagaraSystem)
{
    if (!NiagaraSystem || !GetOwner())
    {
        return nullptr;
    }
    
    UNiagaraComponent* NewComponent = NewObject<UNiagaraComponent>(GetOwner());
    if (NewComponent)
    {
        NewComponent->SetAsset(NiagaraSystem);
        NewComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                                      FAttachmentTransformRules::KeepWorldTransform);
        NewComponent->RegisterComponent();
    }
    
    return NewComponent;
}

UNiagaraComponent* UVFXPoolManager::GetVFXFromPool(EVFXCategory Category, EVFXPriority Priority, const FString& PoolName)
{
    if (!VFXPools.Contains(Category))
    {
        UE_LOG(LogVFXPool, Warning, TEXT("VFXPoolManager: No pools found for category %s"), 
               *UEnum::GetValueAsString(Category));
        return nullptr;
    }
    
    // Find the appropriate pool
    FVFXPool* TargetPool = nullptr;
    FString TargetPoolName = PoolName;
    
    if (PoolName.IsEmpty())
    {
        // Find the first available pool with matching priority
        for (auto& PoolPair : VFXPools[Category])
        {
            if (PoolPair.Value.Priority == Priority)
            {
                TargetPool = &PoolPair.Value;
                TargetPoolName = PoolPair.Key;
                break;
            }
        }
    }
    else
    {
        if (VFXPools[Category].Contains(PoolName))
        {
            TargetPool = &VFXPools[Category][PoolName];
        }
    }
    
    if (!TargetPool)
    {
        UE_LOG(LogVFXPool, Warning, TEXT("VFXPoolManager: No suitable pool found"));
        return nullptr;
    }
    
    // Find an available entry in the pool
    for (FVFXPoolEntry& Entry : TargetPool->PoolEntries)
    {
        if (!Entry.bIsInUse && Entry.NiagaraComponent && IsValid(Entry.NiagaraComponent))
        {
            Entry.bIsInUse = true;
            Entry.LastUsedTime = GetWorld()->GetTimeSeconds();
            TotalActiveVFXCount++;
            
            UE_LOG(LogVFXPool, VeryVerbose, TEXT("VFXPoolManager: Retrieved VFX from pool '%s'"), *TargetPoolName);
            return Entry.NiagaraComponent;
        }
    }
    
    // If no available entry and pool can grow, create a new one
    if (TargetPool->bCanGrow && TargetPool->PoolEntries.Num() < TargetPool->MaxPoolSize)
    {
        FVFXPoolEntry NewEntry;
        NewEntry.PoolID = NextPoolID++;
        NewEntry.Category = Category;
        NewEntry.Priority = Priority;
        NewEntry.bIsInUse = true;
        NewEntry.LastUsedTime = GetWorld()->GetTimeSeconds();
        
        NewEntry.NiagaraComponent = CreateNiagaraComponent(TargetPool->NiagaraSystemAsset.Get());
        if (NewEntry.NiagaraComponent)
        {
            NewEntry.NiagaraComponent->SetAutoDestroy(false);
            TargetPool->PoolEntries.Add(NewEntry);
            TotalActiveVFXCount++;
            
            UE_LOG(LogVFXPool, Log, TEXT("VFXPoolManager: Grew pool '%s' to %d entries"), 
                   *TargetPoolName, TargetPool->PoolEntries.Num());
            
            return NewEntry.NiagaraComponent;
        }
    }
    
    UE_LOG(LogVFXPool, Warning, TEXT("VFXPoolManager: Pool '%s' is full and cannot grow"), *TargetPoolName);
    return nullptr;
}

bool UVFXPoolManager::ReturnVFXToPool(UNiagaraComponent* VFXComponent)
{
    if (!VFXComponent || !IsValid(VFXComponent))
    {
        return false;
    }
    
    // Find the pool entry for this component
    for (auto& CategoryPair : VFXPools)
    {
        for (auto& PoolPair : CategoryPair.Value)
        {
            FVFXPool& Pool = PoolPair.Value;
            for (FVFXPoolEntry& Entry : Pool.PoolEntries)
            {
                if (Entry.NiagaraComponent == VFXComponent && Entry.bIsInUse)
                {
                    // Return to pool
                    Entry.bIsInUse = false;
                    Entry.LastUsedTime = GetWorld()->GetTimeSeconds();
                    
                    // Reset component state
                    VFXComponent->Deactivate();
                    VFXComponent->SetVisibility(false);
                    VFXComponent->SetWorldLocation(FVector::ZeroVector);
                    
                    TotalActiveVFXCount = FMath::Max(0, TotalActiveVFXCount - 1);
                    
                    UE_LOG(LogVFXPool, VeryVerbose, TEXT("VFXPoolManager: Returned VFX to pool '%s'"), 
                           *PoolPair.Key);
                    return true;
                }
            }
        }
    }
    
    UE_LOG(LogVFXPool, Warning, TEXT("VFXPoolManager: Could not find pool entry for VFX component"));
    return false;
}

UNiagaraComponent* UVFXPoolManager::SpawnVFXFromPool(const FVFXEffectData& EffectData, FVector Location, FRotator Rotation, AActor* AttachActor)
{
    UNiagaraComponent* VFXComponent = GetVFXFromPool(EffectData.Category, EffectData.Priority, EffectData.PoolName);
    
    if (!VFXComponent)
    {
        return nullptr;
    }
    
    // Configure the VFX component
    VFXComponent->SetWorldLocationAndRotation(Location, Rotation);
    VFXComponent->SetVisibility(true);
    
    // Attach to actor if specified
    if (AttachActor && AttachActor->GetRootComponent())
    {
        VFXComponent->AttachToComponent(AttachActor->GetRootComponent(), 
                                      FAttachmentTransformRules::KeepWorldTransform);
    }
    
    // Apply effect data parameters
    ApplyEffectDataToComponent(VFXComponent, EffectData);
    
    // Activate the effect
    VFXComponent->Activate(true);
    
    UE_LOG(LogVFXPool, VeryVerbose, TEXT("VFXPoolManager: Spawned VFX at location %s"), 
           *Location.ToString());
    
    return VFXComponent;
}

void UVFXPoolManager::ApplyEffectDataToComponent(UNiagaraComponent* VFXComponent, const FVFXEffectData& EffectData)
{
    if (!VFXComponent)
    {
        return;
    }
    
    // Apply scale
    VFXComponent->SetWorldScale3D(FVector(EffectData.Scale));
    
    // Apply color tint if available
    if (EffectData.ColorTint != FLinearColor::White)
    {
        VFXComponent->SetColorParameter(TEXT("ColorTint"), EffectData.ColorTint);
    }
    
    // Apply intensity
    VFXComponent->SetFloatParameter(TEXT("Intensity"), EffectData.Intensity);
    
    // Apply duration if it's not looping
    if (EffectData.Duration > 0.0f && !EffectData.bIsLooping)
    {
        VFXComponent->SetFloatParameter(TEXT("Duration"), EffectData.Duration);
        
        // Set up auto-return to pool
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, 
            FTimerDelegate::CreateUFunction(this, FName("ReturnVFXToPool"), VFXComponent), 
            EffectData.Duration, false);
    }
}

void UVFXPoolManager::CleanupUnusedPools()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    int32 CleanedEntries = 0;
    
    for (auto& CategoryPair : VFXPools)
    {
        for (auto& PoolPair : CategoryPair.Value)
        {
            FVFXPool& Pool = PoolPair.Value;
            
            // Clean up unused entries that have exceeded timeout
            for (int32 i = Pool.PoolEntries.Num() - 1; i >= 0; --i)
            {
                FVFXPoolEntry& Entry = Pool.PoolEntries[i];
                
                if (!Entry.bIsInUse && 
                    (CurrentTime - Entry.LastUsedTime) > PoolManagerSettings.UnusedPoolTimeout)
                {
                    if (Entry.NiagaraComponent && IsValid(Entry.NiagaraComponent))
                    {
                        Entry.NiagaraComponent->DestroyComponent();
                    }
                    Pool.PoolEntries.RemoveAt(i);
                    CleanedEntries++;
                }
            }
        }
    }
    
    if (CleanedEntries > 0)
    {
        UE_LOG(LogVFXPool, Log, TEXT("VFXPoolManager: Cleaned up %d unused pool entries"), CleanedEntries);
    }
}

void UVFXPoolManager::PreloadCriticalPools()
{
    // Define critical VFX pools that should be preloaded
    TArray<FVFXPoolData> CriticalPools;
    
    // Add critical pools based on game requirements
    // These would typically be loaded from data assets or configuration
    
    UE_LOG(LogVFXPool, Log, TEXT("VFXPoolManager: Preloading %d critical pools"), CriticalPools.Num());
    
    for (const FVFXPoolData& PoolData : CriticalPools)
    {
        CreateVFXPool(PoolData);
    }
}

void UVFXPoolManager::UpdateVFXLODBasedOnDistance()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Get player camera location for distance calculations
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->PlayerCameraManager)
    {
        return;
    }
    
    FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
    
    // Update LOD for all active VFX
    for (auto& CategoryPair : VFXPools)
    {
        for (auto& PoolPair : CategoryPair.Value)
        {
            FVFXPool& Pool = PoolPair.Value;
            for (FVFXPoolEntry& Entry : Pool.PoolEntries)
            {
                if (Entry.bIsInUse && Entry.NiagaraComponent && IsValid(Entry.NiagaraComponent))
                {
                    FVector VFXLocation = Entry.NiagaraComponent->GetComponentLocation();
                    EVFXLODLevel LODLevel = CalculateVFXLOD(VFXLocation, CameraLocation);
                    UpdateVFXLOD(Entry.NiagaraComponent, LODLevel);
                }
            }
        }
    }
}

EVFXLODLevel UVFXPoolManager::CalculateVFXLOD(FVector VFXLocation, FVector ViewerLocation) const
{
    float Distance = FVector::Dist(VFXLocation, ViewerLocation);
    
    // Define LOD distance thresholds
    const float HighLODDistance = 1000.0f;
    const float MediumLODDistance = 3000.0f;
    const float LowLODDistance = 6000.0f;
    
    if (Distance <= HighLODDistance)
    {
        return EVFXLODLevel::High;
    }
    else if (Distance <= MediumLODDistance)
    {
        return EVFXLODLevel::Medium;
    }
    else if (Distance <= LowLODDistance)
    {
        return EVFXLODLevel::Low;
    }
    else
    {
        return EVFXLODLevel::Disabled;
    }
}

void UVFXPoolManager::UpdateVFXLOD(UNiagaraComponent* VFXComponent, EVFXLODLevel LODLevel)
{
    if (!VFXComponent || !IsValid(VFXComponent))
    {
        return;
    }
    
    switch (LODLevel)
    {
        case EVFXLODLevel::High:
            VFXComponent->SetFloatParameter(TEXT("LODScale"), 1.0f);
            VFXComponent->SetIntParameter(TEXT("MaxParticles"), 1000);
            break;
            
        case EVFXLODLevel::Medium:
            VFXComponent->SetFloatParameter(TEXT("LODScale"), 0.7f);
            VFXComponent->SetIntParameter(TEXT("MaxParticles"), 500);
            break;
            
        case EVFXLODLevel::Low:
            VFXComponent->SetFloatParameter(TEXT("LODScale"), 0.4f);
            VFXComponent->SetIntParameter(TEXT("MaxParticles"), 200);
            break;
            
        case EVFXLODLevel::Disabled:
            VFXComponent->SetVisibility(false);
            break;
    }
}

void UVFXPoolManager::RecycleExpiredVFXInstances()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (auto& CategoryPair : VFXPools)
    {
        for (auto& PoolPair : CategoryPair.Value)
        {
            FVFXPool& Pool = PoolPair.Value;
            if (!Pool.bAutoRecycle)
            {
                continue;
            }
            
            for (FVFXPoolEntry& Entry : Pool.PoolEntries)
            {
                if (Entry.bIsInUse && Entry.NiagaraComponent && IsValid(Entry.NiagaraComponent))
                {
                    // Check if VFX has finished playing
                    if (!Entry.NiagaraComponent->IsActive() || 
                        (CurrentTime - Entry.LastUsedTime) > Pool.RecycleTimeout)
                    {
                        ReturnVFXToPool(Entry.NiagaraComponent);
                    }
                }
            }
        }
    }
}

int32 UVFXPoolManager::GetTotalActiveVFXCount() const
{
    return TotalActiveVFXCount;
}

void UVFXPoolManager::PrintPoolStatistics()
{
    if (!PoolManagerSettings.bEnablePoolStatistics)
    {
        return;
    }
    
    UE_LOG(LogVFXPool, Log, TEXT("=== VFX Pool Statistics ==="));
    UE_LOG(LogVFXPool, Log, TEXT("Total Active VFX: %d"), TotalActiveVFXCount);
    UE_LOG(LogVFXPool, Log, TEXT("Max Total VFX: %d"), PoolManagerSettings.MaxTotalVFXInstances);
    
    for (auto& CategoryPair : VFXPools)
    {
        UE_LOG(LogVFXPool, Log, TEXT("Category %s:"), *UEnum::GetValueAsString(CategoryPair.Key));
        
        for (auto& PoolPair : CategoryPair.Value)
        {
            const FVFXPool& Pool = PoolPair.Value;
            int32 ActiveCount = 0;
            int32 TotalCount = Pool.PoolEntries.Num();
            
            for (const FVFXPoolEntry& Entry : Pool.PoolEntries)
            {
                if (Entry.bIsInUse)
                {
                    ActiveCount++;
                }
            }
            
            UE_LOG(LogVFXPool, Log, TEXT("  Pool '%s': %d/%d active (Max: %d)"), 
                   *PoolPair.Key, ActiveCount, TotalCount, Pool.MaxPoolSize);
        }
    }
    UE_LOG(LogVFXPool, Log, TEXT("========================"));
}