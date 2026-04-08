#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AVFXSystemManager::AVFXSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update LOD 10 times per second
    
    CurrentFrameVFXTime = 0.0f;
    CurrentActiveCount = 0;
}

void AVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default VFX systems
    InitializeDefaultSystems();
}

void AVFXSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdatePerformanceMetrics();
    UpdateLODForAllEffects();
    
    // Cull effects if over budget
    if (CurrentFrameVFXTime > PerformanceBudgetMS || CurrentActiveCount > MaxActiveEffects)
    {
        CullLowPriorityEffects();
    }
}

UNiagaraComponent* AVFXSystemManager::SpawnVFXSystem(const FString& SystemName, FVector Location, FRotator Rotation, AActor* AttachToActor)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System %s not found in registry"), *SystemName);
        return nullptr;
    }

    const FVFXSystemData& SystemData = RegisteredSystems[SystemName];
    
    // Check performance budget
    if (CurrentActiveCount >= MaxActiveEffects && SystemData.Priority == EVFXPriority::Low)
    {
        return nullptr; // Skip low priority effects when at capacity
    }

    UNiagaraSystem* NiagaraSystem = SystemData.NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Niagara System for %s"), *SystemName);
        return nullptr;
    }

    UNiagaraComponent* NiagaraComponent = nullptr;
    
    if (AttachToActor)
    {
        NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraSystem,
            AttachToActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepWorldPosition,
            true
        );
    }
    else
    {
        NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            NiagaraSystem,
            Location,
            Rotation
        );
    }

    if (NiagaraComponent)
    {
        // Set initial LOD based on distance
        APawn* PlayerPawn = GetLocalPlayerPawn();
        if (PlayerPawn)
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
            int32 LODLevel = GetLODLevelForDistance(Distance);
            NiagaraComponent->SetNiagaraVariableInt("LOD_Level", LODLevel);
        }

        // Track active effect
        ActiveEffects.Add(NiagaraComponent);
        CurrentActiveCount++;
    }

    return NiagaraComponent;
}

void AVFXSystemManager::RegisterVFXSystem(const FString& SystemName, const FVFXSystemData& SystemData)
{
    RegisteredSystems.Add(SystemName, SystemData);
}

void AVFXSystemManager::SetPerformanceMode(bool bHighPerformance)
{
    if (bHighPerformance)
    {
        PerformanceBudgetMS = 1.0f;
        MaxActiveEffects = 25;
        LODDistanceHigh = 500.0f;
        LODDistanceMedium = 1500.0f;
        LODDistanceLow = 2500.0f;
    }
    else
    {
        PerformanceBudgetMS = 2.0f;
        MaxActiveEffects = 50;
        LODDistanceHigh = 1000.0f;
        LODDistanceMedium = 3000.0f;
        LODDistanceLow = 5000.0f;
    }
}

void AVFXSystemManager::UpdateLODForAllEffects()
{
    APawn* PlayerPawn = GetLocalPlayerPawn();
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Clean up invalid components
    ActiveEffects.RemoveAll([](UNiagaraComponent* Component) {
        return !IsValid(Component) || !Component->IsActive();
    });

    CurrentActiveCount = ActiveEffects.Num();

    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (!IsValid(Effect)) continue;

        float Distance = FVector::Dist(PlayerLocation, Effect->GetComponentLocation());
        int32 LODLevel = GetLODLevelForDistance(Distance);
        
        Effect->SetNiagaraVariableInt("LOD_Level", LODLevel);

        // Cull distant effects
        if (Distance > LODDistanceLow)
        {
            Effect->SetVisibility(false);
        }
        else
        {
            Effect->SetVisibility(true);
        }
    }
}

int32 AVFXSystemManager::GetLODLevelForDistance(float Distance) const
{
    if (Distance <= LODDistanceHigh)
    {
        return 0; // High quality
    }
    else if (Distance <= LODDistanceMedium)
    {
        return 1; // Medium quality
    }
    else if (Distance <= LODDistanceLow)
    {
        return 2; // Low quality
    }
    
    return 3; // Culled
}

void AVFXSystemManager::CullLowPriorityEffects()
{
    // Sort by priority and distance, remove lowest priority first
    APawn* PlayerPawn = GetLocalPlayerPawn();
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    ActiveEffects.Sort([PlayerLocation](const UNiagaraComponent& A, const UNiagaraComponent& B) {
        float DistA = FVector::Dist(PlayerLocation, A.GetComponentLocation());
        float DistB = FVector::Dist(PlayerLocation, B.GetComponentLocation());
        
        // Prioritize by distance for now (could be enhanced with priority system)
        return DistA < DistB;
    });

    // Remove furthest effects until under budget
    while (CurrentActiveCount > MaxActiveEffects && ActiveEffects.Num() > 0)
    {
        UNiagaraComponent* EffectToRemove = ActiveEffects.Last();
        if (IsValid(EffectToRemove))
        {
            EffectToRemove->DestroyComponent();
        }
        ActiveEffects.RemoveAt(ActiveEffects.Num() - 1);
        CurrentActiveCount--;
    }
}

void AVFXSystemManager::UpdatePerformanceMetrics()
{
    // Simple performance tracking - in production this would use more sophisticated profiling
    CurrentFrameVFXTime = ActiveEffects.Num() * 0.04f; // Rough estimate: 0.04ms per effect
}

APawn* AVFXSystemManager::GetLocalPlayerPawn() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->GetPawn();
        }
    }
    return nullptr;
}

void AVFXSystemManager::InitializeDefaultSystems()
{
    // Environment VFX
    FVFXSystemData DustParticles;
    DustParticles.Category = EVFXCategory::Environment;
    DustParticles.Priority = EVFXPriority::Low;
    DustParticles.MaxDrawDistance = 2000.0f;
    RegisterVFXSystem("Environment_DustParticles", DustParticles);

    FVFXSystemData LeafFall;
    LeafFall.Category = EVFXCategory::Environment;
    LeafFall.Priority = EVFXPriority::Medium;
    LeafFall.MaxDrawDistance = 3000.0f;
    RegisterVFXSystem("Environment_LeafFall", LeafFall);

    // Creature VFX
    FVFXSystemData DinosaurBreath;
    DinosaurBreath.Category = EVFXCategory::Creature;
    DinosaurBreath.Priority = EVFXPriority::High;
    DinosaurBreath.MaxDrawDistance = 5000.0f;
    RegisterVFXSystem("Creature_DinosaurBreath", DinosaurBreath);

    // Combat VFX
    FVFXSystemData BloodSplatter;
    BloodSplatter.Category = EVFXCategory::Combat;
    BloodSplatter.Priority = EVFXPriority::Critical;
    BloodSplatter.MaxDrawDistance = 1000.0f;
    RegisterVFXSystem("Combat_BloodSplatter", BloodSplatter);

    // Survival VFX
    FVFXSystemData CraftingSparkles;
    CraftingSparkles.Category = EVFXCategory::Survival;
    CraftingSparkles.Priority = EVFXPriority::Medium;
    CraftingSparkles.MaxDrawDistance = 500.0f;
    RegisterVFXSystem("Survival_CraftingSparkles", CraftingSparkles);
}