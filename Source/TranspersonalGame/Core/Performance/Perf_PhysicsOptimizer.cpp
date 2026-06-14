#include "Perf_PhysicsOptimizer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UPerf_PhysicsOptimizer::UPerf_PhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize LOD settings
    HighLODSettings.MaxDistance = 2000.0f;
    HighLODSettings.MaxSimulatingBodies = 50;
    HighLODSettings.UpdateFrequency = 60.0f;
    HighLODSettings.bEnableComplexCollision = true;

    MediumLODSettings.MaxDistance = 5000.0f;
    MediumLODSettings.MaxSimulatingBodies = 30;
    MediumLODSettings.UpdateFrequency = 30.0f;
    MediumLODSettings.bEnableComplexCollision = false;

    LowLODSettings.MaxDistance = 10000.0f;
    LowLODSettings.MaxSimulatingBodies = 10;
    LowLODSettings.UpdateFrequency = 10.0f;
    LowLODSettings.bEnableComplexCollision = false;

    PhysicsCullingDistance = 8000.0f;
    MaxPhysicsBodies = 150;
    OptimizationUpdateInterval = 1.0f;
}

void UPerf_PhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Optimizer initialized"));
    UpdatePhysicsLODForAllActors();
}

void UPerf_PhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastOptimization += DeltaTime;
    
    if (TimeSinceLastOptimization >= OptimizationUpdateInterval)
    {
        UpdatePhysicsMetrics();
        OptimizePhysicsSettings();
        EnforcePhysicsBudget();
        TimeSinceLastOptimization = 0.0f;
    }
}

void UPerf_PhysicsOptimizer::SetPhysicsLOD(AActor* Actor, EPerf_PhysicsLOD LODLevel)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetPhysicsLOD: Invalid actor"));
        return;
    }

    ActorLODMap.Add(Actor, LODLevel);

    switch (LODLevel)
    {
        case EPerf_PhysicsLOD::High:
            ApplyLODSettings(Actor, HighLODSettings);
            break;
        case EPerf_PhysicsLOD::Medium:
            ApplyLODSettings(Actor, MediumLODSettings);
            break;
        case EPerf_PhysicsLOD::Low:
            ApplyLODSettings(Actor, LowLODSettings);
            break;
        case EPerf_PhysicsLOD::Disabled:
            PutPhysicsBodyToSleep(Actor);
            break;
    }
}

EPerf_PhysicsLOD UPerf_PhysicsOptimizer::GetPhysicsLOD(AActor* Actor) const
{
    if (const EPerf_PhysicsLOD* LOD = ActorLODMap.Find(Actor))
    {
        return *LOD;
    }
    return EPerf_PhysicsLOD::High; // Default
}

void UPerf_PhysicsOptimizer::UpdatePhysicsLODForAllActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent() && Actor->GetRootComponent()->IsSimulatingPhysics())
        {
            OptimizeActorPhysics(Actor);
        }
    }
}

FPerf_PhysicsMetrics UPerf_PhysicsOptimizer::GetPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsOptimizer::OptimizePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 ActiveBodies = 0;
    int32 SleepingBodies = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                if (IsPhysicsBodySleeping(Actor))
                {
                    SleepingBodies++;
                }
                else
                {
                    ActiveBodies++;
                    OptimizeActorPhysics(Actor);
                }
            }
        }
    }

    CurrentMetrics.ActivePhysicsBodies = ActiveBodies;
    CurrentMetrics.SleepingBodies = SleepingBodies;
}

void UPerf_PhysicsOptimizer::SetPhysicsCullingDistance(float Distance)
{
    PhysicsCullingDistance = FMath::Max(Distance, 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Physics culling distance set to: %f"), PhysicsCullingDistance);
}

float UPerf_PhysicsOptimizer::GetPhysicsCullingDistance() const
{
    return PhysicsCullingDistance;
}

void UPerf_PhysicsOptimizer::PutPhysicsBodyToSleep(AActor* Actor)
{
    if (!Actor) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        PrimComp->PutRigidBodyToSleep();
    }
}

void UPerf_PhysicsOptimizer::WakePhysicsBody(AActor* Actor)
{
    if (!Actor) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        PrimComp->WakeRigidBody();
    }
}

bool UPerf_PhysicsOptimizer::IsPhysicsBodySleeping(AActor* Actor) const
{
    if (!Actor) return false;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (PrimComp && PrimComp->IsSimulatingPhysics())
    {
        return PrimComp->RigidBodyIsAwake() == false;
    }
    return false;
}

void UPerf_PhysicsOptimizer::SetMaxPhysicsBodies(int32 MaxBodies)
{
    MaxPhysicsBodies = FMath::Max(MaxBodies, 10);
    UE_LOG(LogTemp, Log, TEXT("Max physics bodies set to: %d"), MaxPhysicsBodies);
}

int32 UPerf_PhysicsOptimizer::GetMaxPhysicsBodies() const
{
    return MaxPhysicsBodies;
}

void UPerf_PhysicsOptimizer::EnforcePhysicsBudget()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> PhysicsActors;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PhysicsActors.Add(Actor);
            }
        }
    }

    if (PhysicsActors.Num() > MaxPhysicsBodies)
    {
        // Sort by distance to player
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn)
        {
            PhysicsActors.Sort([this, PlayerPawn](const AActor& A, const AActor& B) {
                float DistA = FVector::Dist(A.GetActorLocation(), PlayerPawn->GetActorLocation());
                float DistB = FVector::Dist(B.GetActorLocation(), PlayerPawn->GetActorLocation());
                return DistA < DistB;
            });
        }

        // Put distant actors to sleep
        for (int32 i = MaxPhysicsBodies; i < PhysicsActors.Num(); i++)
        {
            PutPhysicsBodyToSleep(PhysicsActors[i]);
        }

        UE_LOG(LogTemp, Log, TEXT("Physics budget enforced: %d bodies active, %d put to sleep"), 
               MaxPhysicsBodies, PhysicsActors.Num() - MaxPhysicsBodies);
    }
}

void UPerf_PhysicsOptimizer::UpdatePhysicsMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return;

    CurrentMetrics.ActivePhysicsBodies = 0;
    CurrentMetrics.SleepingBodies = 0;
    CurrentMetrics.CollisionChecks = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                if (IsPhysicsBodySleeping(Actor))
                {
                    CurrentMetrics.SleepingBodies++;
                }
                else
                {
                    CurrentMetrics.ActivePhysicsBodies++;
                }
            }
        }
    }

    // Estimate physics step time (simplified)
    CurrentMetrics.PhysicsStepTime = World->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    CurrentMetrics.MemoryUsageMB = (CurrentMetrics.ActivePhysicsBodies * 0.5f) + (CurrentMetrics.SleepingBodies * 0.1f);
}

void UPerf_PhysicsOptimizer::ApplyLODSettings(AActor* Actor, const FPerf_PhysicsLODSettings& Settings)
{
    if (!Actor) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
    if (!PrimComp) return;

    // Apply collision complexity
    if (Settings.bEnableComplexCollision)
    {
        PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
    }
    else
    {
        PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
        PrimComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        PrimComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    }

    // Adjust physics update frequency (simplified approach)
    if (Settings.UpdateFrequency < 30.0f)
    {
        PrimComp->SetComponentTickInterval(1.0f / Settings.UpdateFrequency);
    }
    else
    {
        PrimComp->SetComponentTickInterval(0.0f); // Default tick
    }
}

float UPerf_PhysicsOptimizer::GetDistanceToPlayer(AActor* Actor) const
{
    if (!Actor) return 0.0f;

    UWorld* World = GetWorld();
    if (!World) return 0.0f;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return 0.0f;

    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void UPerf_PhysicsOptimizer::OptimizeActorPhysics(AActor* Actor)
{
    if (!Actor) return;

    float Distance = GetDistanceToPlayer(Actor);

    if (Distance > PhysicsCullingDistance)
    {
        SetPhysicsLOD(Actor, EPerf_PhysicsLOD::Disabled);
    }
    else if (Distance > LowLODSettings.MaxDistance)
    {
        SetPhysicsLOD(Actor, EPerf_PhysicsLOD::Low);
    }
    else if (Distance > MediumLODSettings.MaxDistance)
    {
        SetPhysicsLOD(Actor, EPerf_PhysicsLOD::Medium);
    }
    else
    {
        SetPhysicsLOD(Actor, EPerf_PhysicsLOD::High);
    }
}