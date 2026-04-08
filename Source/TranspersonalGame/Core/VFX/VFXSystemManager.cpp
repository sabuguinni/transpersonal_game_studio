#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

UVFXSystemManager::UVFXSystemManager()
{
    CurrentQuality = EVFXQuality::High;
    MaxGlobalVFXInstances = 100;
    VFXCullingDistance = 5000.0f;
    VFXUpdateInterval = 0.1f;
}

void UVFXSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: Initializing VFX System"));
    
    InitializeDefaultVFXDefinitions();
    ApplyQualitySettings();
    
    // Setup timers for cleanup and culling
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            VFXCleanupTimer,
            this,
            &UVFXSystemManager::CleanupExpiredVFX,
            VFXUpdateInterval,
            true
        );
        
        World->GetTimerManager().SetTimer(
            VFXCullingTimer,
            this,
            &UVFXSystemManager::UpdateVFXCulling,
            VFXUpdateInterval * 2.0f,
            true
        );
    }
}

void UVFXSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: Deinitializing VFX System"));
    
    // Clear all timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(VFXCleanupTimer);
        World->GetTimerManager().ClearTimer(VFXCullingTimer);
    }
    
    // Destroy all active VFX
    DestroyAllVFX();
    
    Super::Deinitialize();
}

bool UVFXSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UVFXSystemManager::InitializeDefaultVFXDefinitions()
{
    // Environmental Effects
    FVFXDefinition FireDef;
    FireDef.VFXType = EVFXType::Fire;
    FireDef.DefaultScale = FVector(1.0f);
    FireDef.DefaultLifetime = 10.0f;
    FireDef.MaxInstances = 15;
    VFXDefinitions.Add(EVFXType::Fire, FireDef);
    
    FVFXDefinition SmokeDef;
    SmokeDef.VFXType = EVFXType::Smoke;
    SmokeDef.DefaultScale = FVector(1.2f);
    SmokeDef.DefaultLifetime = 8.0f;
    SmokeDef.MaxInstances = 20;
    VFXDefinitions.Add(EVFXType::Smoke, SmokeDef);
    
    FVFXDefinition SteamDef;
    SteamDef.VFXType = EVFXType::Steam;
    SteamDef.DefaultScale = FVector(0.8f);
    SteamDef.DefaultLifetime = 6.0f;
    SteamDef.MaxInstances = 10;
    VFXDefinitions.Add(EVFXType::Steam, SteamDef);
    
    FVFXDefinition DustDef;
    DustDef.VFXType = EVFXType::Dust;
    DustDef.DefaultScale = FVector(1.5f);
    DustDef.DefaultLifetime = 4.0f;
    DustDef.MaxInstances = 25;
    VFXDefinitions.Add(EVFXType::Dust, DustDef);
    
    // Combat Effects
    FVFXDefinition BloodDef;
    BloodDef.VFXType = EVFXType::Blood;
    BloodDef.DefaultScale = FVector(0.6f);
    BloodDef.DefaultLifetime = 3.0f;
    BloodDef.MaxInstances = 30;
    VFXDefinitions.Add(EVFXType::Blood, BloodDef);
    
    FVFXDefinition ImpactDef;
    ImpactDef.VFXType = EVFXType::Impact;
    ImpactDef.DefaultScale = FVector(0.8f);
    ImpactDef.DefaultLifetime = 1.5f;
    ImpactDef.MaxInstances = 40;
    VFXDefinitions.Add(EVFXType::Impact, ImpactDef);
    
    // Dinosaur Effects
    FVFXDefinition BreathDef;
    BreathDef.VFXType = EVFXType::Breath;
    BreathDef.DefaultScale = FVector(2.0f);
    BreathDef.DefaultLifetime = 2.0f;
    BreathDef.MaxInstances = 5;
    VFXDefinitions.Add(EVFXType::Breath, BreathDef);
    
    FVFXDefinition FootstepDef;
    FootstepDef.VFXType = EVFXType::Footstep;
    FootstepDef.DefaultScale = FVector(1.0f);
    FootstepDef.DefaultLifetime = 2.0f;
    FootstepDef.MaxInstances = 20;
    VFXDefinitions.Add(EVFXType::Footstep, FootstepDef);
    
    // Mystical Effects
    FVFXDefinition GemGlowDef;
    GemGlowDef.VFXType = EVFXType::GemGlow;
    GemGlowDef.DefaultScale = FVector(0.5f);
    GemGlowDef.DefaultLifetime = -1.0f; // Persistent
    GemGlowDef.bAutoDestroy = false;
    GemGlowDef.MaxInstances = 3;
    VFXDefinitions.Add(EVFXType::GemGlow, GemGlowDef);
    
    UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: Initialized %d default VFX definitions"), VFXDefinitions.Num());
}

UNiagaraComponent* UVFXSystemManager::SpawnVFX(EVFXType VFXType, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    FVFXDefinition* Definition = VFXDefinitions.Find(VFXType);
    if (!Definition)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: No definition found for VFX type %d"), (int32)VFXType);
        return nullptr;
    }
    
    // Check instance limits
    int32 CurrentTypeCount = GetActiveVFXCountOfType(VFXType);
    if (CurrentTypeCount >= Definition->MaxInstances)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: Max instances reached for VFX type %d (%d/%d)"), 
               (int32)VFXType, CurrentTypeCount, Definition->MaxInstances);
        return nullptr;
    }
    
    if (GetActiveVFXCount() >= MaxGlobalVFXInstances)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: Max global VFX instances reached (%d/%d)"), 
               GetActiveVFXCount(), MaxGlobalVFXInstances);
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = CreateVFXComponent(*Definition);
    if (!VFXComponent)
    {
        return nullptr;
    }
    
    // Set transform
    VFXComponent->SetWorldLocation(Location);
    VFXComponent->SetWorldRotation(Rotation);
    VFXComponent->SetWorldScale3D(Scale * Definition->DefaultScale);
    
    // Activate the effect
    VFXComponent->Activate();
    
    // Register the instance
    RegisterVFXInstance(VFXComponent, VFXType, Definition->DefaultLifetime);
    
    // Fire event
    OnVFXSpawned.Broadcast(VFXType, VFXComponent);
    
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Spawned VFX type %d at location %s"), 
           (int32)VFXType, *Location.ToString());
    
    return VFXComponent;
}

UNiagaraComponent* UVFXSystemManager::SpawnVFXAttached(EVFXType VFXType, USceneComponent* AttachToComponent, FName AttachPointName, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    if (!AttachToComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: Cannot attach VFX - AttachToComponent is null"));
        return nullptr;
    }
    
    FVFXDefinition* Definition = VFXDefinitions.Find(VFXType);
    if (!Definition)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: No definition found for VFX type %d"), (int32)VFXType);
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = CreateVFXComponent(*Definition);
    if (!VFXComponent)
    {
        return nullptr;
    }
    
    // Attach to component
    VFXComponent->AttachToComponent(
        AttachToComponent,
        FAttachmentTransformRules::KeepRelativeTransform,
        AttachPointName
    );
    
    // Set relative transform
    VFXComponent->SetRelativeLocation(Location);
    VFXComponent->SetRelativeRotation(Rotation);
    VFXComponent->SetRelativeScale3D(Scale * Definition->DefaultScale);
    
    // Activate the effect
    VFXComponent->Activate();
    
    // Register the instance
    RegisterVFXInstance(VFXComponent, VFXType, Definition->DefaultLifetime);
    
    // Fire event
    OnVFXSpawned.Broadcast(VFXType, VFXComponent);
    
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Spawned attached VFX type %d"), (int32)VFXType);
    
    return VFXComponent;
}

void UVFXSystemManager::DestroyVFX(UNiagaraComponent* VFXComponent)
{
    if (!VFXComponent)
    {
        return;
    }
    
    // Find and remove from active instances
    for (int32 i = ActiveVFXInstances.Num() - 1; i >= 0; --i)
    {
        if (ActiveVFXInstances[i].Component == VFXComponent)
        {
            EVFXType VFXType = ActiveVFXInstances[i].VFXType;
            ActiveVFXInstances.RemoveAt(i);
            
            // Fire event
            OnVFXDestroyed.Broadcast(VFXType, VFXComponent);
            break;
        }
    }
    
    // Deactivate and destroy
    VFXComponent->Deactivate();
    VFXComponent->DestroyComponent();
    
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Destroyed VFX component"));
}

void UVFXSystemManager::DestroyAllVFXOfType(EVFXType VFXType)
{
    for (int32 i = ActiveVFXInstances.Num() - 1; i >= 0; --i)
    {
        if (ActiveVFXInstances[i].VFXType == VFXType)
        {
            UNiagaraComponent* Component = ActiveVFXInstances[i].Component;
            if (Component)
            {
                Component->Deactivate();
                Component->DestroyComponent();
            }
            
            OnVFXDestroyed.Broadcast(VFXType, Component);
            ActiveVFXInstances.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Destroyed all VFX of type %d"), (int32)VFXType);
}

void UVFXSystemManager::DestroyAllVFX()
{
    for (const FVFXInstance& Instance : ActiveVFXInstances)
    {
        if (Instance.Component)
        {
            Instance.Component->Deactivate();
            Instance.Component->DestroyComponent();
            OnVFXDestroyed.Broadcast(Instance.VFXType, Instance.Component);
        }
    }
    
    ActiveVFXInstances.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Destroyed all VFX"));
}

void UVFXSystemManager::SetVFXQuality(EVFXQuality Quality)
{
    CurrentQuality = Quality;
    ApplyQualitySettings();
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Set VFX quality to %d"), (int32)Quality);
}

void UVFXSystemManager::SetMaxVFXInstances(int32 MaxInstances)
{
    MaxGlobalVFXInstances = FMath::Max(1, MaxInstances);
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Set max VFX instances to %d"), MaxGlobalVFXInstances);
}

int32 UVFXSystemManager::GetActiveVFXCount() const
{
    return ActiveVFXInstances.Num();
}

int32 UVFXSystemManager::GetActiveVFXCountOfType(EVFXType VFXType) const
{
    int32 Count = 0;
    for (const FVFXInstance& Instance : ActiveVFXInstances)
    {
        if (Instance.VFXType == VFXType && Instance.bIsActive)
        {
            Count++;
        }
    }
    return Count;
}

void UVFXSystemManager::RegisterVFXDefinition(const FVFXDefinition& Definition)
{
    VFXDefinitions.Add(Definition.VFXType, Definition);
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Registered VFX definition for type %d"), (int32)Definition.VFXType);
}

bool UVFXSystemManager::GetVFXDefinition(EVFXType VFXType, FVFXDefinition& OutDefinition) const
{
    if (const FVFXDefinition* Definition = VFXDefinitions.Find(VFXType))
    {
        OutDefinition = *Definition;
        return true;
    }
    return false;
}

UNiagaraComponent* UVFXSystemManager::CreateVFXComponent(const FVFXDefinition& Definition)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // For now, create a basic Niagara component
    // In a real implementation, you would load the actual Niagara system asset
    UNiagaraComponent* Component = NewObject<UNiagaraComponent>(World);
    if (!Component)
    {
        return nullptr;
    }
    
    // Set up the component
    Component->SetAutoDestroy(Definition.bAutoDestroy);
    
    // Add to world's root component for now
    if (AActor* WorldActor = World->GetWorldSettings())
    {
        Component->AttachToComponent(
            WorldActor->GetRootComponent(),
            FAttachmentTransformRules::KeepWorldTransform
        );
    }
    
    return Component;
}

void UVFXSystemManager::RegisterVFXInstance(UNiagaraComponent* Component, EVFXType VFXType, float Lifetime)
{
    FVFXInstance Instance;
    Instance.Component = Component;
    Instance.VFXType = VFXType;
    Instance.SpawnTime = GetWorld()->GetTimeSeconds();
    Instance.Lifetime = Lifetime;
    Instance.bIsActive = true;
    
    ActiveVFXInstances.Add(Instance);
}

void UVFXSystemManager::UnregisterVFXInstance(UNiagaraComponent* Component)
{
    for (int32 i = ActiveVFXInstances.Num() - 1; i >= 0; --i)
    {
        if (ActiveVFXInstances[i].Component == Component)
        {
            ActiveVFXInstances.RemoveAt(i);
            break;
        }
    }
}

void UVFXSystemManager::CleanupExpiredVFX()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveVFXInstances.Num() - 1; i >= 0; --i)
    {
        FVFXInstance& Instance = ActiveVFXInstances[i];
        
        // Skip persistent effects
        if (Instance.Lifetime <= 0.0f)
        {
            continue;
        }
        
        // Check if expired
        if (CurrentTime - Instance.SpawnTime >= Instance.Lifetime)
        {
            if (Instance.Component)
            {
                Instance.Component->Deactivate();
                Instance.Component->DestroyComponent();
                OnVFXDestroyed.Broadcast(Instance.VFXType, Instance.Component);
            }
            
            ActiveVFXInstances.RemoveAt(i);
        }
        // Check if component is still valid
        else if (!IsValid(Instance.Component))
        {
            ActiveVFXInstances.RemoveAt(i);
        }
    }
}

void UVFXSystemManager::UpdateVFXCulling()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get player camera location for distance culling
    FVector CameraLocation = FVector::ZeroVector;
    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
        {
            CameraLocation = CameraManager->GetCameraLocation();
        }
    }
    
    for (int32 i = ActiveVFXInstances.Num() - 1; i >= 0; --i)
    {
        FVFXInstance& Instance = ActiveVFXInstances[i];
        
        if (!Instance.Component || !IsValid(Instance.Component))
        {
            ActiveVFXInstances.RemoveAt(i);
            continue;
        }
        
        // Distance culling
        float Distance = FVector::Dist(Instance.Component->GetComponentLocation(), CameraLocation);
        bool bShouldCull = Distance > VFXCullingDistance;
        
        if (bShouldCull && Instance.bIsActive)
        {
            Instance.Component->SetVisibility(false);
            Instance.bIsActive = false;
        }
        else if (!bShouldCull && !Instance.bIsActive)
        {
            Instance.Component->SetVisibility(true);
            Instance.bIsActive = true;
        }
    }
}

void UVFXSystemManager::ApplyQualitySettings()
{
    switch (CurrentQuality)
    {
        case EVFXQuality::Low:
            MaxGlobalVFXInstances = 50;
            VFXCullingDistance = 2500.0f;
            VFXUpdateInterval = 0.2f;
            break;
            
        case EVFXQuality::Medium:
            MaxGlobalVFXInstances = 75;
            VFXCullingDistance = 3500.0f;
            VFXUpdateInterval = 0.15f;
            break;
            
        case EVFXQuality::High:
            MaxGlobalVFXInstances = 100;
            VFXCullingDistance = 5000.0f;
            VFXUpdateInterval = 0.1f;
            break;
            
        case EVFXQuality::Ultra:
            MaxGlobalVFXInstances = 150;
            VFXCullingDistance = 7500.0f;
            VFXUpdateInterval = 0.05f;
            break;
    }
    
    // Update timer intervals
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(VFXCleanupTimer);
        World->GetTimerManager().ClearTimer(VFXCullingTimer);
        
        World->GetTimerManager().SetTimer(
            VFXCleanupTimer,
            this,
            &UVFXSystemManager::CleanupExpiredVFX,
            VFXUpdateInterval,
            true
        );
        
        World->GetTimerManager().SetTimer(
            VFXCullingTimer,
            this,
            &UVFXSystemManager::UpdateVFXCulling,
            VFXUpdateInterval * 2.0f,
            true
        );
    }
}

bool UVFXSystemManager::ShouldCullVFX(const FVFXInstance& Instance) const
{
    if (!Instance.Component)
    {
        return true;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Get player camera location
    FVector CameraLocation = FVector::ZeroVector;
    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
        {
            CameraLocation = CameraManager->GetCameraLocation();
        }
    }
    
    float Distance = FVector::Dist(Instance.Component->GetComponentLocation(), CameraLocation);
    return Distance > VFXCullingDistance;
}