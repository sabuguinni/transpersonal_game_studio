#include "Perf_PhysicsOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

UPerf_PhysicsOptimizer::UPerf_PhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    OptimizationUpdateInterval = 1.0f;
    bEnableAutomaticOptimization = true;
    LastOptimizationTime = 0.0f;
}

void UPerf_PhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    FindPhysicsActors();
    UpdatePhysicsStats();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsOptimizer: Initialized with %d physics actors"), TrackedPhysicsActors.Num());
}

void UPerf_PhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableAutomaticOptimization)
    {
        return;
    }
    
    LastOptimizationTime += DeltaTime;
    
    if (LastOptimizationTime >= OptimizationUpdateInterval)
    {
        OptimizePhysicsActors();
        UpdatePhysicsStats();
        LastOptimizationTime = 0.0f;
    }
}

void UPerf_PhysicsOptimizer::OptimizePhysicsActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Update tracked actors list
    FindPhysicsActors();
    
    // Apply distance-based LOD
    if (PhysicsSettings.bEnablePhysicsLOD)
    {
        for (auto& WeakActor : TrackedPhysicsActors)
        {
            if (AActor* Actor = WeakActor.Get())
            {
                float Distance = GetDistanceToPlayer(Actor);
                EPerf_PhysicsLOD LODLevel = CalculateLODLevel(Distance);
                SetPhysicsLOD(Actor, LODLevel);
            }
        }
    }
    
    // Cull distant actors
    if (PhysicsSettings.bEnableDistanceCulling)
    {
        CullDistantPhysicsActors();
    }
    
    // Enforce limits
    EnforcePhysicsLimits();
}

void UPerf_PhysicsOptimizer::SetPhysicsLOD(AActor* Actor, EPerf_PhysicsLOD LODLevel)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (!PrimComp)
        {
            continue;
        }
        
        switch (LODLevel)
        {
            case EPerf_PhysicsLOD::High:
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetSimulatePhysics(true);
                break;
                
            case EPerf_PhysicsLOD::Medium:
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                PrimComp->SetSimulatePhysics(false);
                break;
                
            case EPerf_PhysicsLOD::Low:
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                PrimComp->SetSimulatePhysics(false);
                break;
                
            case EPerf_PhysicsLOD::Disabled:
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                PrimComp->SetSimulatePhysics(false);
                break;
        }
    }
}

void UPerf_PhysicsOptimizer::CullDistantPhysicsActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Restore previously culled actors that are now close enough
    RestoreCulledActors();
    
    for (auto& WeakActor : TrackedPhysicsActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            float Distance = GetDistanceToPlayer(Actor);
            
            if (Distance > PhysicsSettings.LowLODDistance * 1.5f) // Cull beyond low LOD distance
            {
                DisablePhysicsForActor(Actor);
                CulledActors.AddUnique(WeakActor);
            }
        }
    }
}

void UPerf_PhysicsOptimizer::EnforcePhysicsLimits()
{
    UpdatePhysicsStats();
    
    // Enforce ragdoll limits
    if (CurrentStats.ActiveRagdolls > PhysicsSettings.MaxRagdolls)
    {
        int32 ExcessRagdolls = CurrentStats.ActiveRagdolls - PhysicsSettings.MaxRagdolls;
        UE_LOG(LogTemp, Warning, TEXT("PhysicsOptimizer: Excess ragdolls detected: %d"), ExcessRagdolls);
        
        // Disable oldest/furthest ragdolls
        for (auto& WeakActor : TrackedPhysicsActors)
        {
            if (AActor* Actor = WeakActor.Get())
            {
                if (USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
                {
                    if (SkelMesh->IsSimulatingPhysics())
                    {
                        SkelMesh->SetSimulatePhysics(false);
                        ExcessRagdolls--;
                        if (ExcessRagdolls <= 0) break;
                    }
                }
            }
        }
    }
    
    // Enforce destruction object limits
    if (CurrentStats.ActiveDestructionObjects > PhysicsSettings.MaxDestructionObjects)
    {
        int32 ExcessDestruction = CurrentStats.ActiveDestructionObjects - PhysicsSettings.MaxDestructionObjects;
        UE_LOG(LogTemp, Warning, TEXT("PhysicsOptimizer: Excess destruction objects: %d"), ExcessDestruction);
        
        // Disable furthest destruction objects
        for (auto& WeakActor : TrackedPhysicsActors)
        {
            if (AActor* Actor = WeakActor.Get())
            {
                FString ActorName = Actor->GetName();
                if (ActorName.Contains(TEXT("Destruction")) || ActorName.Contains(TEXT("Debris")))
                {
                    DisablePhysicsForActor(Actor);
                    ExcessDestruction--;
                    if (ExcessDestruction <= 0) break;
                }
            }
        }
    }
}

void UPerf_PhysicsOptimizer::UpdatePhysicsStats()
{
    CurrentStats = FPerf_PhysicsStats();
    
    if (!GetWorld())
    {
        return;
    }
    
    for (auto& WeakActor : TrackedPhysicsActors)
    {
        if (AActor* Actor = WeakActor.Get())
        {
            // Count ragdolls
            if (USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
            {
                if (SkelMesh->IsSimulatingPhysics())
                {
                    CurrentStats.ActiveRagdolls++;
                }
            }
            
            // Count destruction objects
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Destruction")) || ActorName.Contains(TEXT("Debris")))
            {
                CurrentStats.ActiveDestructionObjects++;
            }
            
            // Count simulating actors
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    CurrentStats.SimulatingActors++;
                    break;
                }
            }
        }
    }
    
    CurrentStats.CulledActors = CulledActors.Num();
    
    // Get physics frame time (simplified)
    CurrentStats.PhysicsFrameTime = GetWorld()->GetDeltaSeconds();
}

void UPerf_PhysicsOptimizer::DisablePhysicsForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            PrimComp->SetSimulatePhysics(false);
        }
    }
}

void UPerf_PhysicsOptimizer::EnablePhysicsForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetSimulatePhysics(true);
        }
    }
}

bool UPerf_PhysicsOptimizer::IsPhysicsPerformanceGood() const
{
    bool bGoodPerformance = true;
    
    if (CurrentStats.ActiveRagdolls > PhysicsSettings.MaxRagdolls * 0.8f)
    {
        bGoodPerformance = false;
    }
    
    if (CurrentStats.ActiveDestructionObjects > PhysicsSettings.MaxDestructionObjects * 0.8f)
    {
        bGoodPerformance = false;
    }
    
    if (CurrentStats.SimulatingActors > PhysicsSettings.MaxSimulatingActors * 0.8f)
    {
        bGoodPerformance = false;
    }
    
    return bGoodPerformance;
}

void UPerf_PhysicsOptimizer::ApplyPerformancePreset(const FString& PresetName)
{
    if (PresetName == TEXT("High"))
    {
        PhysicsSettings.MaxRagdolls = 15;
        PhysicsSettings.MaxDestructionObjects = 75;
        PhysicsSettings.MaxSimulatingActors = 150;
        PhysicsSettings.HighLODDistance = 1500.0f;
        PhysicsSettings.MediumLODDistance = 3000.0f;
        PhysicsSettings.LowLODDistance = 6000.0f;
    }
    else if (PresetName == TEXT("Medium"))
    {
        PhysicsSettings.MaxRagdolls = 10;
        PhysicsSettings.MaxDestructionObjects = 50;
        PhysicsSettings.MaxSimulatingActors = 100;
        PhysicsSettings.HighLODDistance = 1000.0f;
        PhysicsSettings.MediumLODDistance = 2500.0f;
        PhysicsSettings.LowLODDistance = 5000.0f;
    }
    else if (PresetName == TEXT("Low"))
    {
        PhysicsSettings.MaxRagdolls = 5;
        PhysicsSettings.MaxDestructionObjects = 25;
        PhysicsSettings.MaxSimulatingActors = 50;
        PhysicsSettings.HighLODDistance = 500.0f;
        PhysicsSettings.MediumLODDistance = 1500.0f;
        PhysicsSettings.LowLODDistance = 3000.0f;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsOptimizer: Applied %s performance preset"), *PresetName);
}

void UPerf_PhysicsOptimizer::FindPhysicsActors()
{
    TrackedPhysicsActors.Empty();
    
    if (!GetWorld())
    {
        return;
    }
    
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }
        
        // Check if actor has physics components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp && (PrimComp->IsSimulatingPhysics() || 
                PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision))
            {
                TrackedPhysicsActors.AddUnique(Actor);
                break;
            }
        }
    }
}

float UPerf_PhysicsOptimizer::GetDistanceToPlayer(AActor* Actor) const
{
    if (!Actor || !GetWorld())
    {
        return 99999.0f;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return 99999.0f;
    }
    
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void UPerf_PhysicsOptimizer::RestoreCulledActors()
{
    for (int32 i = CulledActors.Num() - 1; i >= 0; i--)
    {
        if (AActor* Actor = CulledActors[i].Get())
        {
            float Distance = GetDistanceToPlayer(Actor);
            
            if (Distance <= PhysicsSettings.LowLODDistance)
            {
                EnablePhysicsForActor(Actor);
                CulledActors.RemoveAt(i);
            }
        }
        else
        {
            // Remove invalid weak pointer
            CulledActors.RemoveAt(i);
        }
    }
}

EPerf_PhysicsLOD UPerf_PhysicsOptimizer::CalculateLODLevel(float Distance) const
{
    if (Distance <= PhysicsSettings.HighLODDistance)
    {
        return EPerf_PhysicsLOD::High;
    }
    else if (Distance <= PhysicsSettings.MediumLODDistance)
    {
        return EPerf_PhysicsLOD::Medium;
    }
    else if (Distance <= PhysicsSettings.LowLODDistance)
    {
        return EPerf_PhysicsLOD::Low;
    }
    else
    {
        return EPerf_PhysicsLOD::Disabled;
    }
}