// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "VFXManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

// Constants
const float UVFXManager::VFX_TICK_INTERVAL = 0.1f; // 10 times per second
const float UVFXManager::PERFORMANCE_UPDATE_INTERVAL = 1.0f; // Once per second
const int32 UVFXManager::MAX_POOLED_COMPONENTS_PER_EFFECT = 10;
const float UVFXManager::POOL_CLEANUP_TIME = 30.0f; // 30 seconds

UVFXManager::UVFXManager()
{
    NextInstanceID = 1;
    LastPerformanceUpdate = 0.0f;
    CurrentPerformanceCost = 0.0f;
}

void UVFXManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize default configuration
    SystemConfig = FVFXSystemConfig();
    
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Subsystem initialized"));

    // Start VFX tick timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            VFXTickTimer,
            this,
            &UVFXManager::TickVFXSystem,
            VFX_TICK_INTERVAL,
            true
        );

        World->GetTimerManager().SetTimer(
            PerformanceTimer,
            this,
            &UVFXManager::ManagePerformance,
            PERFORMANCE_UPDATE_INTERVAL,
            true
        );
    }
}

void UVFXManager::Deinitialize()
{
    // Stop all active effects
    StopAllVFXEffects();

    // Clear pools
    ClearVFXPool();

    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(VFXTickTimer);
        World->GetTimerManager().ClearTimer(PerformanceTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("VFXManager: Subsystem deinitialized"));

    Super::Deinitialize();
}

int32 UVFXManager::SpawnVFXEffect(const FString& EffectID, const FVFXSpawnParams& SpawnParams)
{
    if (!SystemConfig.bEnableVFX)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: VFX system is disabled"));
        return -1;
    }

    // Find effect definition
    const FVFXEffectDefinition* EffectDef = EffectDefinitions.Find(EffectID);
    if (!EffectDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Effect ID '%s' not found"), *EffectID);
        return -1;
    }

    // Check performance limits
    if (ActiveInstances.Num() >= SystemConfig.MaxSimultaneousEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Maximum simultaneous effects reached (%d)"), SystemConfig.MaxSimultaneousEffects);
        return -1;
    }

    // Check distance culling
    if (ShouldCullVFXByDistance(SpawnParams.Location))
    {
        UE_LOG(LogTemp, Verbose, TEXT("VFXManager: Effect '%s' culled by distance"), *EffectID);
        return -1;
    }

    // Get appropriate Niagara system for current quality
    UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForQuality(*EffectDef, SystemConfig.GlobalQuality);
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: No Niagara system found for effect '%s' at quality level %d"), 
            *EffectID, (int32)SystemConfig.GlobalQuality);
        return -1;
    }

    // Try to get pooled component first
    UNiagaraComponent* NiagaraComp = GetPooledComponent(EffectID);
    
    // If no pooled component available, create new one
    if (!NiagaraComp)
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            UE_LOG(LogTemp, Error, TEXT("VFXManager: No valid world context"));
            return -1;
        }

        NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            NiagaraSystem,
            SpawnParams.Location,
            SpawnParams.Rotation,
            SpawnParams.Scale * SystemConfig.GlobalScale,
            true, // Auto destroy
            true, // Auto activate
            ENCPoolMethod::None,
            true // Pre cull check
        );
    }
    else
    {
        // Configure pooled component
        NiagaraComp->SetAsset(NiagaraSystem);
        NiagaraComp->SetWorldLocationAndRotation(SpawnParams.Location, SpawnParams.Rotation);
        NiagaraComp->SetWorldScale3D(SpawnParams.Scale * SystemConfig.GlobalScale);
        NiagaraComp->Activate(true);
    }

    if (!NiagaraComp)
    {
        UE_LOG(LogTemp, Error, TEXT("VFXManager: Failed to create Niagara component for effect '%s'"), *EffectID);
        return -1;
    }

    // Apply quality settings
    ApplyQualitySettings(NiagaraComp, SystemConfig.GlobalQuality);

    // Set custom parameters
    for (const auto& FloatParam : SpawnParams.CustomFloatParams)
    {
        NiagaraComp->SetFloatParameter(FName(*FloatParam.Key), FloatParam.Value);
    }

    for (const auto& VectorParam : SpawnParams.CustomVectorParams)
    {
        NiagaraComp->SetVectorParameter(FName(*VectorParam.Key), VectorParam.Value);
    }

    for (const auto& ColorParam : SpawnParams.CustomColorParams)
    {
        NiagaraComp->SetColorParameter(FName(*ColorParam.Key), ColorParam.Value);
    }

    // Handle attachment
    if (SpawnParams.AttachToActor.IsValid())
    {
        if (USceneComponent* AttachComponent = SpawnParams.AttachToActor->GetRootComponent())
        {
            NiagaraComp->AttachToComponent(
                AttachComponent,
                FAttachmentTransformRules::KeepWorldTransform,
                SpawnParams.AttachSocketName
            );
        }
    }

    // Create active instance
    int32 InstanceID = NextInstanceID++;
    FActiveVFXInstance& Instance = ActiveInstances.Add(InstanceID);
    Instance.InstanceID = InstanceID;
    Instance.EffectID = EffectID;
    Instance.NiagaraComponent = NiagaraComp;
    Instance.SpawnParams = SpawnParams;
    Instance.SpawnTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    Instance.bIsActive = true;

    // Play audio if specified
    if (EffectDef->AudioCue.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            UGameplayStatics::PlaySoundAtLocation(
                World,
                EffectDef->AudioCue.LoadSynchronous(),
                SpawnParams.Location,
                1.0f, // Volume
                1.0f, // Pitch
                0.0f, // Start time
                nullptr, // Attenuation
                nullptr, // Concurrency
                SpawnParams.AttachToActor.Get()
            );
        }
    }

    // Set duration if specified
    float Duration = SpawnParams.DurationOverride > 0.0f ? SpawnParams.DurationOverride : EffectDef->DefaultDuration;
    if (Duration > 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            FTimerHandle DurationTimer;
            World->GetTimerManager().SetTimer(
                DurationTimer,
                [this, InstanceID]()
                {
                    StopVFXEffect(InstanceID);
                },
                Duration,
                false
            );
        }
    }

    // Update performance cost
    CurrentPerformanceCost += EffectDef->PerformanceCost;

    // Broadcast event
    OnVFXSpawned.Broadcast(InstanceID, EffectID);

    UE_LOG(LogTemp, Verbose, TEXT("VFXManager: Spawned effect '%s' with instance ID %d"), *EffectID, InstanceID);

    return InstanceID;
}

bool UVFXManager::StopVFXEffect(int32 InstanceID)
{
    FActiveVFXInstance* Instance = ActiveInstances.Find(InstanceID);
    if (!Instance)
    {
        return false;
    }

    if (Instance->NiagaraComponent.IsValid())
    {
        UNiagaraComponent* NiagaraComp = Instance->NiagaraComponent.Get();
        
        // Deactivate the component
        NiagaraComp->Deactivate();

        // Return to pool if possible
        const FVFXEffectDefinition* EffectDef = EffectDefinitions.Find(Instance->EffectID);
        if (EffectDef && EffectDef->bCanBePooled)
        {
            ReturnComponentToPool(NiagaraComp, Instance->EffectID);
        }
        else
        {
            // Destroy component
            NiagaraComp->DestroyComponent();
        }

        // Update performance cost
        if (EffectDef)
        {
            CurrentPerformanceCost -= EffectDef->PerformanceCost;
            CurrentPerformanceCost = FMath::Max(0.0f, CurrentPerformanceCost);
        }
    }

    // Broadcast event
    OnVFXStopped.Broadcast(InstanceID, Instance->EffectID);

    // Remove from active instances
    ActiveInstances.Remove(InstanceID);

    UE_LOG(LogTemp, Verbose, TEXT("VFXManager: Stopped effect instance %d"), InstanceID);

    return true;
}

bool UVFXManager::IsVFXEffectActive(int32 InstanceID) const
{
    const FActiveVFXInstance* Instance = ActiveInstances.Find(InstanceID);
    return Instance && Instance->bIsActive && Instance->NiagaraComponent.IsValid();
}

void UVFXManager::StopAllVFXEffects()
{
    TArray<int32> InstanceIDs;
    ActiveInstances.GetKeys(InstanceIDs);

    for (int32 InstanceID : InstanceIDs)
    {
        StopVFXEffect(InstanceID);
    }

    UE_LOG(LogTemp, Log, TEXT("VFXManager: Stopped all VFX effects"));
}

void UVFXManager::StopVFXEffectsByType(EVFXEffectType EffectType)
{
    TArray<int32> InstanceIDsToStop;

    for (const auto& InstancePair : ActiveInstances)
    {
        const FActiveVFXInstance& Instance = InstancePair.Value;
        const FVFXEffectDefinition* EffectDef = EffectDefinitions.Find(Instance.EffectID);
        
        if (EffectDef && EffectDef->EffectType == EffectType)
        {
            InstanceIDsToStop.Add(InstancePair.Key);
        }
    }

    for (int32 InstanceID : InstanceIDsToStop)
    {
        StopVFXEffect(InstanceID);
    }

    UE_LOG(LogTemp, Log, TEXT("VFXManager: Stopped %d effects of type %d"), InstanceIDsToStop.Num(), (int32)EffectType);
}

void UVFXManager::RegisterVFXEffect(const FVFXEffectDefinition& EffectDefinition)
{
    if (EffectDefinition.EffectID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Cannot register effect with empty ID"));
        return;
    }

    EffectDefinitions.Add(EffectDefinition.EffectID, EffectDefinition);
    
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Registered effect '%s' (%s)"), 
        *EffectDefinition.EffectID, *EffectDefinition.DisplayName);
}

bool UVFXManager::GetVFXEffectDefinition(const FString& EffectID, FVFXEffectDefinition& OutDefinition) const
{
    const FVFXEffectDefinition* EffectDef = EffectDefinitions.Find(EffectID);
    if (EffectDef)
    {
        OutDefinition = *EffectDef;
        return true;
    }
    return false;
}

TArray<FString> UVFXManager::GetAllEffectIDs() const
{
    TArray<FString> EffectIDs;
    EffectDefinitions.GetKeys(EffectIDs);
    return EffectIDs;
}

TArray<FString> UVFXManager::GetEffectIDsByType(EVFXEffectType EffectType) const
{
    TArray<FString> EffectIDs;
    
    for (const auto& EffectPair : EffectDefinitions)
    {
        if (EffectPair.Value.EffectType == EffectType)
        {
            EffectIDs.Add(EffectPair.Key);
        }
    }
    
    return EffectIDs;
}

void UVFXManager::SetGlobalVFXQuality(EVFXQualityLevel NewQuality)
{
    if (SystemConfig.GlobalQuality != NewQuality)
    {
        SystemConfig.GlobalQuality = NewQuality;
        
        // Update all active effects to new quality level
        UpdateLODLevels();
        
        OnVFXQualityChanged.Broadcast(NewQuality);
        
        UE_LOG(LogTemp, Log, TEXT("VFXManager: Global VFX quality changed to %d"), (int32)NewQuality);
    }
}

EVFXQualityLevel UVFXManager::GetGlobalVFXQuality() const
{
    return SystemConfig.GlobalQuality;
}

void UVFXManager::UpdateVFXSystemConfig(const FVFXSystemConfig& NewConfig)
{
    SystemConfig = NewConfig;
    UE_LOG(LogTemp, Log, TEXT("VFXManager: System configuration updated"));
}

FVFXSystemConfig UVFXManager::GetVFXSystemConfig() const
{
    return SystemConfig;
}

int32 UVFXManager::GetActiveVFXCount() const
{
    return ActiveInstances.Num();
}

int32 UVFXManager::GetActiveVFXCountByType(EVFXEffectType EffectType) const
{
    int32 Count = 0;
    
    for (const auto& InstancePair : ActiveInstances)
    {
        const FActiveVFXInstance& Instance = InstancePair.Value;
        const FVFXEffectDefinition* EffectDef = EffectDefinitions.Find(Instance.EffectID);
        
        if (EffectDef && EffectDef->EffectType == EffectType)
        {
            Count++;
        }
    }
    
    return Count;
}

float UVFXManager::GetCurrentVFXPerformanceCost() const
{
    return CurrentPerformanceCost;
}

void UVFXManager::PrewarmVFXPool(const FString& EffectID, int32 PoolSize)
{
    const FVFXEffectDefinition* EffectDef = EffectDefinitions.Find(EffectID);
    if (!EffectDef || !EffectDef->bCanBePooled)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Cannot prewarm pool for effect '%s' - not poolable"), *EffectID);
        return;
    }

    TArray<FVFXPoolEntry>& Pool = ComponentPools.FindOrAdd(EffectID);
    
    for (int32 i = Pool.Num(); i < PoolSize; ++i)
    {
        CreatePooledComponent(EffectID);
    }

    UE_LOG(LogTemp, Log, TEXT("VFXManager: Prewarmed pool for effect '%s' with %d components"), *EffectID, PoolSize);
}

void UVFXManager::ClearVFXPool()
{
    for (auto& PoolPair : ComponentPools)
    {
        for (FVFXPoolEntry& Entry : PoolPair.Value)
        {
            if (Entry.Component.IsValid())
            {
                Entry.Component->DestroyComponent();
            }
        }
    }
    
    ComponentPools.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFXManager: VFX pool cleared"));
}

UNiagaraComponent* UVFXManager::GetVFXComponent(int32 InstanceID) const
{
    const FActiveVFXInstance* Instance = ActiveInstances.Find(InstanceID);
    return Instance ? Instance->NiagaraComponent.Get() : nullptr;
}

bool UVFXManager::SetVFXParameter(int32 InstanceID, const FString& ParameterName, float Value)
{
    UNiagaraComponent* Component = GetVFXComponent(InstanceID);
    if (Component)
    {
        Component->SetFloatParameter(FName(*ParameterName), Value);
        return true;
    }
    return false;
}

bool UVFXManager::SetVFXVectorParameter(int32 InstanceID, const FString& ParameterName, const FVector& Value)
{
    UNiagaraComponent* Component = GetVFXComponent(InstanceID);
    if (Component)
    {
        Component->SetVectorParameter(FName(*ParameterName), Value);
        return true;
    }
    return false;
}

bool UVFXManager::SetVFXColorParameter(int32 InstanceID, const FString& ParameterName, const FLinearColor& Value)
{
    UNiagaraComponent* Component = GetVFXComponent(InstanceID);
    if (Component)
    {
        Component->SetColorParameter(FName(*ParameterName), Value);
        return true;
    }
    return false;
}

// Protected Methods

void UVFXManager::TickVFXSystem()
{
    CleanupFinishedEffects();
}

void UVFXManager::CleanupFinishedEffects()
{
    TArray<int32> FinishedInstances;
    
    for (const auto& InstancePair : ActiveInstances)
    {
        const FActiveVFXInstance& Instance = InstancePair.Value;
        
        if (!Instance.NiagaraComponent.IsValid() || 
            !Instance.NiagaraComponent->IsActive())
        {
            FinishedInstances.Add(InstancePair.Key);
        }
    }
    
    for (int32 InstanceID : FinishedInstances)
    {
        StopVFXEffect(InstanceID);
    }
}

void UVFXManager::UpdateLODLevels()
{
    for (auto& InstancePair : ActiveInstances)
    {
        FActiveVFXInstance& Instance = InstancePair.Value;
        
        if (Instance.NiagaraComponent.IsValid())
        {
            const FVFXEffectDefinition* EffectDef = EffectDefinitions.Find(Instance.EffectID);
            if (EffectDef)
            {
                UNiagaraSystem* NewSystem = GetNiagaraSystemForQuality(*EffectDef, SystemConfig.GlobalQuality);
                if (NewSystem)
                {
                    Instance.NiagaraComponent->SetAsset(NewSystem);
                    ApplyQualitySettings(Instance.NiagaraComponent.Get(), SystemConfig.GlobalQuality);
                }
            }
        }
    }
}

void UVFXManager::ManagePerformance()
{
    // Update performance cost calculation
    CurrentPerformanceCost = 0.0f;
    
    for (const auto& InstancePair : ActiveInstances)
    {
        const FActiveVFXInstance& Instance = InstancePair.Value;
        const FVFXEffectDefinition* EffectDef = EffectDefinitions.Find(Instance.EffectID);
        
        if (EffectDef)
        {
            CurrentPerformanceCost += EffectDef->PerformanceCost;
        }
    }
    
    // Clean up old pooled components
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    for (auto& PoolPair : ComponentPools)
    {
        TArray<FVFXPoolEntry>& Pool = PoolPair.Value;
        
        for (int32 i = Pool.Num() - 1; i >= 0; --i)
        {
            FVFXPoolEntry& Entry = Pool[i];
            
            if (!Entry.bInUse && 
                (CurrentTime - Entry.LastUsedTime) > POOL_CLEANUP_TIME)
            {
                if (Entry.Component.IsValid())
                {
                    Entry.Component->DestroyComponent();
                }
                Pool.RemoveAt(i);
            }
        }
    }
}

// Private Methods

UNiagaraComponent* UVFXManager::GetPooledComponent(const FString& EffectID)
{
    TArray<FVFXPoolEntry>* Pool = ComponentPools.Find(EffectID);
    if (!Pool)
    {
        return nullptr;
    }
    
    for (FVFXPoolEntry& Entry : *Pool)
    {
        if (!Entry.bInUse && Entry.Component.IsValid())
        {
            Entry.bInUse = true;
            return Entry.Component.Get();
        }
    }
    
    return nullptr;
}

void UVFXManager::ReturnComponentToPool(UNiagaraComponent* Component, const FString& EffectID)
{
    if (!Component)
    {
        return;
    }
    
    TArray<FVFXPoolEntry>& Pool = ComponentPools.FindOrAdd(EffectID);
    
    // Find the entry for this component
    for (FVFXPoolEntry& Entry : Pool)
    {
        if (Entry.Component == Component)
        {
            Entry.bInUse = false;
            Entry.LastUsedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            
            // Reset component state
            Component->Deactivate();
            Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
            
            return;
        }
    }
}

void UVFXManager::CreatePooledComponent(const FString& EffectID)
{
    const FVFXEffectDefinition* EffectDef = EffectDefinitions.Find(EffectID);
    if (!EffectDef)
    {
        return;
    }
    
    UNiagaraSystem* NiagaraSystem = GetNiagaraSystemForQuality(*EffectDef, SystemConfig.GlobalQuality);
    if (!NiagaraSystem)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        NiagaraSystem,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        FVector::OneVector,
        false, // Auto destroy
        false, // Auto activate
        ENCPoolMethod::None,
        false // Pre cull check
    );
    
    if (Component)
    {
        TArray<FVFXPoolEntry>& Pool = ComponentPools.FindOrAdd(EffectID);
        
        FVFXPoolEntry& Entry = Pool.AddDefaulted();
        Entry.Component = Component;
        Entry.EffectID = EffectID;
        Entry.bInUse = false;
        Entry.LastUsedTime = World->GetTimeSeconds();
    }
}

UNiagaraSystem* UVFXManager::GetNiagaraSystemForQuality(const FVFXEffectDefinition& Definition, EVFXQualityLevel Quality) const
{
    switch (Quality)
    {
        case EVFXQualityLevel::Low:
            return Definition.LowQualitySystem.LoadSynchronous();
        case EVFXQualityLevel::Medium:
            return Definition.MediumQualitySystem.LoadSynchronous();
        case EVFXQualityLevel::High:
            return Definition.HighQualitySystem.LoadSynchronous();
        case EVFXQualityLevel::Cinematic:
            return Definition.CinematicQualitySystem.LoadSynchronous();
        default:
            return Definition.MediumQualitySystem.LoadSynchronous();
    }
}

void UVFXManager::ApplyQualitySettings(UNiagaraComponent* Component, EVFXQualityLevel Quality) const
{
    if (!Component)
    {
        return;
    }
    
    // Apply quality-specific settings to the component
    switch (Quality)
    {
        case EVFXQualityLevel::Low:
            Component->SetFloatParameter(TEXT("QualityMultiplier"), 0.5f);
            Component->SetFloatParameter(TEXT("ParticleCountMultiplier"), 0.3f);
            break;
        case EVFXQualityLevel::Medium:
            Component->SetFloatParameter(TEXT("QualityMultiplier"), 1.0f);
            Component->SetFloatParameter(TEXT("ParticleCountMultiplier"), 0.7f);
            break;
        case EVFXQualityLevel::High:
            Component->SetFloatParameter(TEXT("QualityMultiplier"), 1.5f);
            Component->SetFloatParameter(TEXT("ParticleCountMultiplier"), 1.0f);
            break;
        case EVFXQualityLevel::Cinematic:
            Component->SetFloatParameter(TEXT("QualityMultiplier"), 2.0f);
            Component->SetFloatParameter(TEXT("ParticleCountMultiplier"), 1.5f);
            break;
    }
}

bool UVFXManager::ShouldCullVFXByDistance(const FVector& EffectLocation) const
{
    float DistanceToPlayer = GetDistanceToPlayer(EffectLocation);
    return DistanceToPlayer > SystemConfig.CullingDistance;
}

float UVFXManager::GetDistanceToPlayer(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 0.0f;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    return FVector::Dist(Location, PlayerLocation);
}