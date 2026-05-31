#include "Perf_PhysicsOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"

UPerf_PhysicsOptimizer::UPerf_PhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Default physics profile for 60fps target
    PhysicsProfile.MaxSimulationDistance = 5000.0f;
    PhysicsProfile.MaxActivePhysicsBodies = 100;
    PhysicsProfile.PhysicsTickRate = 60.0f;
    PhysicsProfile.bEnableAdaptiveLOD = true;
    PhysicsProfile.LODUpdateInterval = 1.0f;
}

void UPerf_PhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Optimizer initialized for performance target"));
    OptimizePhysicsForPerformance();
}

void UPerf_PhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CurrentFrameTime = DeltaTime;
    
    // Update LOD based on interval
    if (PhysicsProfile.bEnableAdaptiveLOD)
    {
        LastLODUpdateTime += DeltaTime;
        if (LastLODUpdateTime >= PhysicsProfile.LODUpdateInterval)
        {
            UpdatePhysicsLOD();
            LastLODUpdateTime = 0.0f;
        }
    }
    
    // Monitor frame time and adjust physics if needed
    if (DeltaTime > 0.0166f) // Above 60fps threshold
    {
        DisableDistantPhysics();
    }
}

void UPerf_PhysicsOptimizer::OptimizePhysicsForPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count current active physics bodies
    CountActivePhysicsBodies();
    
    // Apply LOD settings based on performance
    ApplyLODSettings();
    
    // Optimize collision complexity
    OptimizeCollisionComplexity();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics optimization complete. Active bodies: %d"), ActivePhysicsBodiesCount);
}

void UPerf_PhysicsOptimizer::SetPhysicsLODLevel(EPerf_PhysicsLODLevel NewLODLevel)
{
    CurrentLODLevel = NewLODLevel;
    ApplyLODSettings();
}

void UPerf_PhysicsOptimizer::UpdatePhysicsLOD()
{
    // Determine LOD level based on frame time
    if (CurrentFrameTime > 0.0333f) // Below 30fps
    {
        SetPhysicsLODLevel(EPerf_PhysicsLODLevel::Low);
    }
    else if (CurrentFrameTime > 0.0166f) // Below 60fps
    {
        SetPhysicsLODLevel(EPerf_PhysicsLODLevel::Medium);
    }
    else
    {
        SetPhysicsLODLevel(EPerf_PhysicsLODLevel::High);
    }
}

int32 UPerf_PhysicsOptimizer::GetActivePhysicsBodiesCount() const
{
    return ActivePhysicsBodiesCount;
}

void UPerf_PhysicsOptimizer::DisableDistantPhysics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == PlayerPawn)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        OptimizeActorPhysics(Actor, Distance);
    }
}

void UPerf_PhysicsOptimizer::OptimizeCollisionComplexity()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component && Component->GetBodyInstance())
            {
                // Simplify collision for distant objects
                float Distance = CalculateDistanceToPlayer(Actor);
                if (Distance > PhysicsProfile.MaxSimulationDistance * 0.5f)
                {
                    Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                }
            }
        }
    }
}

void UPerf_PhysicsOptimizer::CountActivePhysicsBodies()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        ActivePhysicsBodiesCount = 0;
        return;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component && Component->GetBodyInstance() && Component->IsSimulatingPhysics())
            {
                Count++;
            }
        }
    }
    
    ActivePhysicsBodiesCount = Count;
}

void UPerf_PhysicsOptimizer::ApplyLODSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Adjust physics settings based on LOD level
    switch (CurrentLODLevel)
    {
        case EPerf_PhysicsLODLevel::High:
            PhysicsProfile.PhysicsTickRate = 60.0f;
            PhysicsProfile.MaxActivePhysicsBodies = 100;
            break;
        case EPerf_PhysicsLODLevel::Medium:
            PhysicsProfile.PhysicsTickRate = 30.0f;
            PhysicsProfile.MaxActivePhysicsBodies = 50;
            break;
        case EPerf_PhysicsLODLevel::Low:
            PhysicsProfile.PhysicsTickRate = 20.0f;
            PhysicsProfile.MaxActivePhysicsBodies = 25;
            break;
        case EPerf_PhysicsLODLevel::Disabled:
            PhysicsProfile.PhysicsTickRate = 10.0f;
            PhysicsProfile.MaxActivePhysicsBodies = 10;
            break;
    }
}

float UPerf_PhysicsOptimizer::CalculateDistanceToPlayer(AActor* Actor)
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return 0.0f;
    }
    
    return FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
}

void UPerf_PhysicsOptimizer::OptimizeActorPhysics(AActor* Actor, float Distance)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (!Component)
        {
            continue;
        }
        
        if (Distance > PhysicsProfile.MaxSimulationDistance)
        {
            // Disable physics simulation for very distant objects
            Component->SetSimulatePhysics(false);
        }
        else if (Distance > PhysicsProfile.MaxSimulationDistance * 0.7f)
        {
            // Reduce physics complexity for distant objects
            if (Component->GetBodyInstance())
            {
                Component->GetBodyInstance()->SetCollisionEnabled(ECE_QueryOnly);
            }
        }
        else
        {
            // Enable full physics for nearby objects
            if (Component->GetBodyInstance())
            {
                Component->GetBodyInstance()->SetCollisionEnabled(ECE_QueryAndPhysics);
            }
        }
    }
}