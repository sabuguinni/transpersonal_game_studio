#include "Core_PhysicsConsolidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "Kismet/GameplayStatics.h"

ACore_PhysicsConsolidator::ACore_PhysicsConsolidator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz update rate
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Initialize system state
    bIsSystemActive = true;
    bEnablePhysicsOptimization = true;
    bEnableCollisionOptimization = true;
    bEnableRagdollOptimization = true;
    bEnableDestructionOptimization = true;
    
    // Performance thresholds
    MaxPhysicsActors = 500;
    MaxCollisionChecks = 1000;
    MaxRagdollBodies = 50;
    PerformanceUpdateInterval = 1.0f;
    
    // Optimization parameters
    PhysicsLODDistance = 2000.0f;
    CollisionLODDistance = 1500.0f;
    RagdollLODDistance = 1000.0f;
    
    // Initialize counters
    CurrentPhysicsActors = 0;
    CurrentCollisionChecks = 0;
    CurrentRagdollBodies = 0;
    SystemPerformanceScore = 100.0f;
    
    LastPerformanceUpdate = 0.0f;
}

void ACore_PhysicsConsolidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsConsolidator: System initialized"));
    
    // Register with physics subsystems
    RegisterWithPhysicsSubsystems();
    
    // Start performance monitoring
    GetWorldTimerManager().SetTimer(
        PerformanceTimerHandle,
        this,
        &ACore_PhysicsConsolidator::UpdatePerformanceMetrics,
        PerformanceUpdateInterval,
        true
    );
}

void ACore_PhysicsConsolidator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsSystemActive) return;
    
    // Update system counters
    UpdateSystemCounters();
    
    // Apply optimizations based on performance
    if (SystemPerformanceScore < 60.0f)
    {
        ApplyEmergencyOptimizations();
    }
    else if (SystemPerformanceScore < 80.0f)
    {
        ApplyStandardOptimizations();
    }
}

void ACore_PhysicsConsolidator::RegisterWithPhysicsSubsystems()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find and register physics system actors
    TArray<AActor*> PhysicsActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), PhysicsActors);
    
    int32 RegisteredSystems = 0;
    for (AActor* Actor : PhysicsActors)
    {
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Physics")) || ActorName.Contains(TEXT("Core_")))
        {
            RegisteredPhysicsSystems.Add(Actor);
            RegisteredSystems++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsConsolidator: Registered %d physics systems"), RegisteredSystems);
}

void ACore_PhysicsConsolidator::UpdateSystemCounters()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Count physics actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    CurrentPhysicsActors = 0;
    CurrentCollisionChecks = 0;
    CurrentRagdollBodies = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetRootComponent() && Actor->GetRootComponent()->IsSimulatingPhysics())
        {
            CurrentPhysicsActors++;
        }
        
        // Count collision components
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        CurrentCollisionChecks += PrimitiveComponents.Num();
    }
    
    // Update performance score based on load
    float PhysicsLoad = (float)CurrentPhysicsActors / (float)MaxPhysicsActors;
    float CollisionLoad = (float)CurrentCollisionChecks / (float)MaxCollisionChecks;
    float RagdollLoad = (float)CurrentRagdollBodies / (float)MaxRagdollBodies;
    
    float AverageLoad = (PhysicsLoad + CollisionLoad + RagdollLoad) / 3.0f;
    SystemPerformanceScore = FMath::Clamp(100.0f - (AverageLoad * 100.0f), 0.0f, 100.0f);
}

void ACore_PhysicsConsolidator::UpdatePerformanceMetrics()
{
    LastPerformanceUpdate = GetWorld()->GetTimeSeconds();
    
    // Log performance metrics
    UE_LOG(LogTemp, Log, TEXT("PhysicsConsolidator Performance:"));
    UE_LOG(LogTemp, Log, TEXT("  Physics Actors: %d/%d"), CurrentPhysicsActors, MaxPhysicsActors);
    UE_LOG(LogTemp, Log, TEXT("  Collision Checks: %d/%d"), CurrentCollisionChecks, MaxCollisionChecks);
    UE_LOG(LogTemp, Log, TEXT("  Ragdoll Bodies: %d/%d"), CurrentRagdollBodies, MaxRagdollBodies);
    UE_LOG(LogTemp, Log, TEXT("  Performance Score: %.1f"), SystemPerformanceScore);
    
    // Broadcast performance update to registered systems
    OnPerformanceUpdated.Broadcast(SystemPerformanceScore);
}

void ACore_PhysicsConsolidator::ApplyStandardOptimizations()
{
    if (!bEnablePhysicsOptimization) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Apply LOD-based optimizations
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !Actor->GetRootComponent()) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        // Physics LOD optimization
        if (Distance > PhysicsLODDistance && Actor->GetRootComponent()->IsSimulatingPhysics())
        {
            Actor->GetRootComponent()->SetSimulatePhysics(false);
        }
        else if (Distance <= PhysicsLODDistance && !Actor->GetRootComponent()->IsSimulatingPhysics())
        {
            Actor->GetRootComponent()->SetSimulatePhysics(true);
        }
        
        // Collision LOD optimization
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (Distance > CollisionLODDistance)
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
            else
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsConsolidator: Applied standard optimizations"));
}

void ACore_PhysicsConsolidator::ApplyEmergencyOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("PhysicsConsolidator: Applying emergency optimizations!"));
    
    // Disable non-essential physics
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    int32 DisabledActors = 0;
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !Actor->GetRootComponent()) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        // Aggressive distance-based disabling
        if (Distance > (PhysicsLODDistance * 0.5f))
        {
            Actor->GetRootComponent()->SetSimulatePhysics(false);
            Actor->SetActorTickEnabled(false);
            DisabledActors++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsConsolidator: Disabled %d actors for emergency optimization"), DisabledActors);
}

bool ACore_PhysicsConsolidator::ValidatePhysicsSystem()
{
    // Validate all registered physics systems
    bool bAllSystemsValid = true;
    
    for (AActor* System : RegisteredPhysicsSystems)
    {
        if (!IsValid(System))
        {
            bAllSystemsValid = false;
            UE_LOG(LogTemp, Error, TEXT("PhysicsConsolidator: Invalid physics system detected"));
        }
    }
    
    // Check performance thresholds
    if (CurrentPhysicsActors > MaxPhysicsActors)
    {
        bAllSystemsValid = false;
        UE_LOG(LogTemp, Warning, TEXT("PhysicsConsolidator: Physics actor count exceeded threshold"));
    }
    
    if (CurrentCollisionChecks > MaxCollisionChecks)
    {
        bAllSystemsValid = false;
        UE_LOG(LogTemp, Warning, TEXT("PhysicsConsolidator: Collision check count exceeded threshold"));
    }
    
    return bAllSystemsValid;
}

void ACore_PhysicsConsolidator::OptimizePhysicsPerformance()
{
    if (SystemPerformanceScore > 80.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsConsolidator: System performance is optimal"));
        return;
    }
    
    // Apply appropriate optimization level
    if (SystemPerformanceScore < 60.0f)
    {
        ApplyEmergencyOptimizations();
    }
    else
    {
        ApplyStandardOptimizations();
    }
    
    // Force garbage collection if performance is critical
    if (SystemPerformanceScore < 40.0f)
    {
        GEngine->ForceGarbageCollection(true);
        UE_LOG(LogTemp, Warning, TEXT("PhysicsConsolidator: Forced garbage collection"));
    }
}

void ACore_PhysicsConsolidator::ResetPhysicsOptimizations()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Re-enable all physics and collision
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !Actor->GetRootComponent()) continue;
        
        Actor->SetActorTickEnabled(true);
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsConsolidator: Reset all physics optimizations"));
}

FString ACore_PhysicsConsolidator::GetSystemStatus()
{
    return FString::Printf(TEXT("Physics Systems Status:\n")
                          TEXT("Active: %s\n")
                          TEXT("Physics Actors: %d/%d\n")
                          TEXT("Collision Checks: %d/%d\n")
                          TEXT("Ragdoll Bodies: %d/%d\n")
                          TEXT("Performance Score: %.1f\n")
                          TEXT("Registered Systems: %d"),
                          bIsSystemActive ? TEXT("YES") : TEXT("NO"),
                          CurrentPhysicsActors, MaxPhysicsActors,
                          CurrentCollisionChecks, MaxCollisionChecks,
                          CurrentRagdollBodies, MaxRagdollBodies,
                          SystemPerformanceScore,
                          RegisteredPhysicsSystems.Num());
}