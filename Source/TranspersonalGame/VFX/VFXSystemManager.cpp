#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AVFXSystemManager::AVFXSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance
    
    InitializePriorityLimits();
}

void AVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default VFX registry
    RegisterDefaultEffects();
}

void AVFXSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CleanupFinishedEffects();
    UpdateVFXLOD();
}

void AVFXSystemManager::InitializePriorityLimits()
{
    PriorityLimits.Add(EVFXPriority::Critical, 10);
    PriorityLimits.Add(EVFXPriority::High, 15);
    PriorityLimits.Add(EVFXPriority::Medium, 20);
    PriorityLimits.Add(EVFXPriority::Low, 30);
    PriorityLimits.Add(EVFXPriority::Background, 50);
}

UNiagaraComponent* AVFXSystemManager::SpawnVFX(const FString& EffectName, const FVector& Location, const FRotator& Rotation, AActor* AttachToActor)
{
    if (!VFXRegistry.Contains(EffectName))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect not found in registry: %s"), *EffectName);
        return nullptr;
    }

    const FVFXDefinition& VFXDef = VFXRegistry[EffectName];
    
    // Check if we can spawn this effect based on priority limits
    if (!CanSpawnEffect(VFXDef.Priority))
    {
        ForceCleanupLowPriorityEffects();
        
        if (!CanSpawnEffect(VFXDef.Priority))
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot spawn VFX due to budget limits: %s"), *EffectName);
            return nullptr;
        }
    }

    // Load the Niagara system
    UNiagaraSystem* NiagaraSystem = VFXDef.NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Niagara system for: %s"), *EffectName);
        return nullptr;
    }

    UNiagaraComponent* EffectComponent = nullptr;

    if (AttachToActor)
    {
        EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraSystem,
            AttachToActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepWorldPosition,
            VFXDef.bAutoDestroy
        );
    }
    else
    {
        EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            NiagaraSystem,
            Location,
            Rotation,
            FVector(1.0f),
            VFXDef.bAutoDestroy
        );
    }

    if (EffectComponent)
    {
        // Set LOD based on distance
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            float Distance = FVector::Dist(Location, PC->GetPawn()->GetActorLocation());
            int32 LODLevel = GetLODLevel(Distance);
            
            // Apply LOD settings to Niagara component
            EffectComponent->SetVariableFloat(TEXT("LODLevel"), static_cast<float>(LODLevel));
        }

        ActiveEffects.Add(EffectComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX: %s at location: %s"), *EffectName, *Location.ToString());
    }

    return EffectComponent;
}

void AVFXSystemManager::StopVFX(UNiagaraComponent* EffectComponent)
{
    if (EffectComponent && IsValid(EffectComponent))
    {
        EffectComponent->Deactivate();
        ActiveEffects.Remove(EffectComponent);
    }
}

void AVFXSystemManager::StopAllVFXByCategory(EVFXCategory Category)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (IsValid(Effect))
        {
            // Check if effect belongs to category (would need additional tracking)
            // For now, stop all effects - can be refined later
            Effect->Deactivate();
            ActiveEffects.RemoveAt(i);
        }
    }
}

void AVFXSystemManager::RegisterVFX(const FString& EffectName, const FVFXDefinition& Definition)
{
    VFXRegistry.Add(EffectName, Definition);
    UE_LOG(LogTemp, Log, TEXT("Registered VFX: %s"), *EffectName);
}

void AVFXSystemManager::UpdateVFXLOD()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            float Distance = FVector::Dist(PlayerLocation, Effect->GetComponentLocation());
            int32 LODLevel = GetLODLevel(Distance);
            
            Effect->SetVariableFloat(TEXT("LODLevel"), static_cast<float>(LODLevel));
            
            // Disable effects that are too far away
            if (Distance > LODDistance3)
            {
                Effect->SetVisibility(false);
            }
            else
            {
                Effect->SetVisibility(true);
            }
        }
    }
}

int32 AVFXSystemManager::GetLODLevel(float Distance) const
{
    if (Distance <= LODDistance1) return 0; // High quality
    if (Distance <= LODDistance2) return 1; // Medium quality
    if (Distance <= LODDistance3) return 2; // Low quality
    return 3; // Disabled
}

float AVFXSystemManager::GetVFXBudgetUsage() const
{
    return static_cast<float>(ActiveEffects.Num()) / static_cast<float>(MaxActiveEffects);
}

void AVFXSystemManager::CleanupFinishedEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (!IsValid(Effect) || !Effect->IsActive())
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

bool AVFXSystemManager::CanSpawnEffect(EVFXPriority Priority) const
{
    int32 CurrentCount = 0;
    
    // Count effects of this priority and higher
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            CurrentCount++;
        }
    }

    const int32* Limit = PriorityLimits.Find(Priority);
    return Limit ? (CurrentCount < *Limit) : false;
}

void AVFXSystemManager::ForceCleanupLowPriorityEffects()
{
    // Remove low priority effects to make room
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Effect = ActiveEffects[i];
        if (IsValid(Effect))
        {
            // This would need additional metadata to determine priority
            // For now, remove oldest effects
            Effect->Deactivate();
            ActiveEffects.RemoveAt(i);
            
            if (ActiveEffects.Num() < MaxActiveEffects * 0.8f)
            {
                break;
            }
        }
    }
}

void AVFXSystemManager::RegisterDefaultEffects()
{
    // Environment Effects
    FVFXDefinition FootstepDust;
    FootstepDust.EffectName = TEXT("FootstepDust");
    FootstepDust.Category = EVFXCategory::Survival;
    FootstepDust.Priority = EVFXPriority::Low;
    FootstepDust.MaxDistance = 1000.0f;
    FootstepDust.bAutoDestroy = true;
    FootstepDust.LifeTime = 3.0f;
    RegisterVFX(TEXT("FootstepDust"), FootstepDust);

    // Creature Effects
    FVFXDefinition DinosaurBreath;
    DinosaurBreath.EffectName = TEXT("DinosaurBreath");
    DinosaurBreath.Category = EVFXCategory::Creatures;
    DinosaurBreath.Priority = EVFXPriority::High;
    DinosaurBreath.MaxDistance = 3000.0f;
    DinosaurBreath.bAutoDestroy = false;
    RegisterVFX(TEXT("DinosaurBreath"), DinosaurBreath);

    // Atmosphere Effects
    FVFXDefinition MistParticles;
    MistParticles.EffectName = TEXT("MistParticles");
    MistParticles.Category = EVFXCategory::Atmosphere;
    MistParticles.Priority = EVFXPriority::Background;
    MistParticles.MaxDistance = 5000.0f;
    MistParticles.bAutoDestroy = false;
    RegisterVFX(TEXT("MistParticles"), MistParticles);

    UE_LOG(LogTemp, Log, TEXT("VFX System Manager: Default effects registered"));
}