#include "PhysicsSystemManager.h"
#include "AdvancedRagdollComponent.h"
#include "DestructionComponent.h"
#include "VehiclePhysicsComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UPhysicsSystemManager::UPhysicsSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    MaxSimulatedBodies = 1000;
    PhysicsLODDistance = 5000.0f;
    bPhysicsEnabled = true;
    PhysicsTimeBudget = 16.67f; // Target 60 FPS
    LastPerformanceUpdateTime = 0.0f;
}

void UPhysicsSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Initializing physics systems"));
    
    // Initialize with default parameters
    InitializePhysicsSystem(MaxSimulatedBodies, PhysicsLODDistance);
}

void UPhysicsSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPhysicsEnabled)
    {
        return;
    }
    
    // Manage physics performance budget
    ManagePhysicsBudget(DeltaTime);
    
    // Update physics LOD based on performance
    ApplyPhysicsLOD();
    
    // Clean up null references
    RegisteredRagdolls.RemoveAll([](UAdvancedRagdollComponent* Comp) { return !IsValid(Comp); });
    RegisteredDestructionComponents.RemoveAll([](UDestructionComponent* Comp) { return !IsValid(Comp); });
    RegisteredVehicleComponents.RemoveAll([](UVehiclePhysicsComponent* Comp) { return !IsValid(Comp); });
}

void UPhysicsSystemManager::InitializePhysicsSystem(int32 MaxSimulatedBodies, float PhysicsLODDistance)
{
    this->MaxSimulatedBodies = MaxSimulatedBodies;
    this->PhysicsLODDistance = PhysicsLODDistance;
    
    // Configure physics settings
    if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
    {
        // Optimize for performance while maintaining quality
        PhysicsSettings->DefaultGravityZ = -980.0f; // Realistic gravity
        PhysicsSettings->SolverIterationCount = 8; // Balance between stability and performance
        PhysicsSettings->SolverPushOutIterationCount = 3;
        
        // Enable CCD for fast-moving objects
        PhysicsSettings->bEnablePCM = true;
        PhysicsSettings->bEnableStabilization = true;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Initialized with MaxBodies=%d, LODDistance=%.1f"), 
           MaxSimulatedBodies, PhysicsLODDistance);
}

void UPhysicsSystemManager::RegisterRagdollComponent(UAdvancedRagdollComponent* RagdollComponent)
{
    if (IsValid(RagdollComponent) && !RegisteredRagdolls.Contains(RagdollComponent))
    {
        RegisteredRagdolls.Add(RagdollComponent);
        UE_LOG(LogTemp, Verbose, TEXT("PhysicsSystemManager: Registered ragdoll component"));
    }
}

void UPhysicsSystemManager::RegisterDestructionComponent(UDestructionComponent* DestructionComponent)
{
    if (IsValid(DestructionComponent) && !RegisteredDestructionComponents.Contains(DestructionComponent))
    {
        RegisteredDestructionComponents.Add(DestructionComponent);
        UE_LOG(LogTemp, Verbose, TEXT("PhysicsSystemManager: Registered destruction component"));
    }
}

void UPhysicsSystemManager::RegisterVehicleComponent(UVehiclePhysicsComponent* VehicleComponent)
{
    if (IsValid(VehicleComponent) && !RegisteredVehicleComponents.Contains(VehicleComponent))
    {
        RegisteredVehicleComponents.Add(VehicleComponent);
        UE_LOG(LogTemp, Verbose, TEXT("PhysicsSystemManager: Registered vehicle component"));
    }
}

FPhysicsPerformanceData UPhysicsSystemManager::GetPhysicsPerformanceData() const
{
    // Update cached data if needed
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPerformanceUpdateTime > PERFORMANCE_CACHE_DURATION)
    {
        UpdatePerformanceMetrics();
        LastPerformanceUpdateTime = CurrentTime;
    }
    
    return CachedPerformanceData;
}

void UPhysicsSystemManager::UpdatePhysicsLOD()
{
    ApplyPhysicsLOD();
}

void UPhysicsSystemManager::SetPhysicsEnabled(bool bEnabled)
{
    bPhysicsEnabled = bEnabled;
    
    if (!bEnabled)
    {
        // Pause all physics simulation
        if (UWorld* World = GetWorld())
        {
            World->GetPhysicsScene()->SetIsStaticLoading(true);
        }
    }
    else
    {
        // Resume physics simulation
        if (UWorld* World = GetWorld())
        {
            World->GetPhysicsScene()->SetIsStaticLoading(false);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsSystemManager: Physics simulation %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPhysicsSystemManager::UpdatePerformanceMetrics() const
{
    CachedPerformanceData = FPhysicsPerformanceData();
    
    // Count active physics bodies in the world
    if (UWorld* World = GetWorld())
    {
        // This is a simplified count - in production, you'd want to query the physics scene directly
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (IsValid(Actor))
            {
                TArray<UPrimitiveComponent*> PrimitiveComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                
                for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                {
                    if (PrimComp && PrimComp->IsSimulatingPhysics())
                    {
                        CachedPerformanceData.ActivePhysicsBodies++;
                    }
                }
            }
        }
    }
    
    // Count active specialized components
    CachedPerformanceData.ActiveRagdolls = RegisteredRagdolls.Num();
    CachedPerformanceData.ActiveDestructions = RegisteredDestructionComponents.Num();
    CachedPerformanceData.ActiveVehicles = RegisteredVehicleComponents.Num();
    
    // Calculate current LOD level based on performance
    if (CachedPerformanceData.ActivePhysicsBodies > MaxSimulatedBodies * 0.8f)
    {
        CachedPerformanceData.CurrentLODLevel = 2; // High LOD
    }
    else if (CachedPerformanceData.ActivePhysicsBodies > MaxSimulatedBodies * 0.6f)
    {
        CachedPerformanceData.CurrentLODLevel = 1; // Medium LOD
    }
    else
    {
        CachedPerformanceData.CurrentLODLevel = 0; // Full detail
    }
    
    // Estimate physics simulation time (simplified)
    CachedPerformanceData.PhysicsSimulationTime = CachedPerformanceData.ActivePhysicsBodies * 0.01f; // Rough estimate
}

void UPhysicsSystemManager::ApplyPhysicsLOD()
{
    if (!bPhysicsEnabled)
    {
        return;
    }
    
    // Get player location for distance calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                PlayerLocation = PlayerPawn->GetActorLocation();
            }
        }
    }
    
    // Apply LOD to ragdoll components
    for (UAdvancedRagdollComponent* RagdollComp : RegisteredRagdolls)
    {
        if (IsValid(RagdollComp))
        {
            float Distance = FVector::Dist(PlayerLocation, RagdollComp->GetOwner()->GetActorLocation());
            
            if (Distance > PhysicsLODDistance)
            {
                // Reduce ragdoll fidelity or disable
                RagdollComp->SetPhysicsLODLevel(2);
            }
            else if (Distance > PhysicsLODDistance * 0.5f)
            {
                // Medium LOD
                RagdollComp->SetPhysicsLODLevel(1);
            }
            else
            {
                // Full detail
                RagdollComp->SetPhysicsLODLevel(0);
            }
        }
    }
    
    // Apply LOD to destruction components
    for (UDestructionComponent* DestructionComp : RegisteredDestructionComponents)
    {
        if (IsValid(DestructionComp))
        {
            float Distance = FVector::Dist(PlayerLocation, DestructionComp->GetOwner()->GetActorLocation());
            
            if (Distance > PhysicsLODDistance)
            {
                DestructionComp->SetDestructionLODLevel(2);
            }
            else if (Distance > PhysicsLODDistance * 0.5f)
            {
                DestructionComp->SetDestructionLODLevel(1);
            }
            else
            {
                DestructionComp->SetDestructionLODLevel(0);
            }
        }
    }
}

void UPhysicsSystemManager::ManagePhysicsBudget(float DeltaTime)
{
    FPhysicsPerformanceData PerfData = GetPhysicsPerformanceData();
    
    // If we're exceeding our physics budget, start reducing quality
    if (PerfData.PhysicsSimulationTime > PhysicsTimeBudget)
    {
        // Reduce physics quality globally
        if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
        {
            // Reduce solver iterations temporarily
            PhysicsSettings->SolverIterationCount = FMath::Max(4, PhysicsSettings->SolverIterationCount - 1);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("PhysicsSystemManager: Reducing physics quality due to budget overrun"));
    }
    else if (PerfData.PhysicsSimulationTime < PhysicsTimeBudget * 0.7f)
    {
        // We have budget to spare, increase quality
        if (UPhysicsSettings* PhysicsSettings = GetMutableDefault<UPhysicsSettings>())
        {
            // Increase solver iterations back to optimal
            PhysicsSettings->SolverIterationCount = FMath::Min(8, PhysicsSettings->SolverIterationCount + 1);
        }
    }
}