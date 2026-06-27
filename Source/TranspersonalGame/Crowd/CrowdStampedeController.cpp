// CrowdStampedeController.cpp
// Agent #13 — Crowd & Traffic Simulation
// Stampede and mass-panic crowd behavior for prehistoric herds

#include "CrowdStampedeController.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

UCrowdStampedeController::UCrowdStampedeController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for crowd agents

    StampedeState = ECrowd_StampedeState::Grazing;
    StampedeDirection = FVector(1.0f, 0.0f, 0.0f);
    StampedeSpeed = 0.0f;
    MaxStampedeSpeed = 1200.0f;
    PanicRadius = 800.0f;
    HerdCohesionRadius = 400.0f;
    bIsLeader = false;
    LeaderActor = nullptr;
    PanicTriggerActor = nullptr;
    TimeSincePanic = 0.0f;
    PanicDecayTime = 30.0f;
}

void UCrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with herd group
    FindHerdLeader();
}

void UCrowdStampedeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    switch (StampedeState)
    {
        case ECrowd_StampedeState::Grazing:
            UpdateGrazing(DeltaTime);
            break;
        case ECrowd_StampedeState::Alert:
            UpdateAlert(DeltaTime);
            break;
        case ECrowd_StampedeState::Stampeding:
            UpdateStampeding(DeltaTime);
            break;
        case ECrowd_StampedeState::Dispersing:
            UpdateDispersing(DeltaTime);
            break;
    }
}

void UCrowdStampedeController::TriggerPanic(AActor* ThreatActor, float ThreatRadius)
{
    if (!ThreatActor) return;

    PanicTriggerActor = ThreatActor;
    TimeSincePanic = 0.0f;

    // Calculate flee direction — away from threat
    FVector ThreatLocation = ThreatActor->GetActorLocation();
    FVector MyLocation = GetOwner()->GetActorLocation();
    FVector FleeDir = (MyLocation - ThreatLocation).GetSafeNormal();

    // Add lateral scatter to prevent all agents going same direction
    float ScatterAngle = FMath::RandRange(-35.0f, 35.0f);
    FQuat ScatterRot = FQuat(FVector::UpVector, FMath::DegreesToRadians(ScatterAngle));
    StampedeDirection = ScatterRot.RotateVector(FleeDir);

    StampedeSpeed = MaxStampedeSpeed * 0.7f; // Initial burst
    StampedeState = ECrowd_StampedeState::Stampeding;

    // Propagate panic to nearby herd members
    PropagatePanicToNeighbors(ThreatRadius);
}

void UCrowdStampedeController::PropagatePanicToNeighbors(float PropagationRadius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    FVector MyLocation = GetOwner()->GetActorLocation();
    int32 PropagatedCount = 0;

    for (AActor* NearbyActor : NearbyActors)
    {
        if (!NearbyActor || NearbyActor == GetOwner()) continue;

        float Distance = FVector::Dist(MyLocation, NearbyActor->GetActorLocation());
        if (Distance > PropagationRadius) continue;

        UCrowdStampedeController* NeighborController = 
            NearbyActor->FindComponentByClass<UCrowdStampedeController>();
        
        if (NeighborController && 
            NeighborController->StampedeState == ECrowd_StampedeState::Grazing)
        {
            // Propagation delay based on distance
            float PropagationDelay = Distance / 500.0f;
            FTimerHandle TimerHandle;
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUFunction(NeighborController, FName("TriggerPanic"), 
                PanicTriggerActor, PropagationRadius * 0.8f);
            World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 
                PropagationDelay, false);
            PropagatedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdStampede: Panic propagated to %d neighbors"), PropagatedCount);
}

void UCrowdStampedeController::FindHerdLeader()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, GetOwner()->GetClass(), AllActors);

    // Nearest herd member with bIsLeader flag
    float NearestDist = MAX_FLT;
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == GetOwner()) continue;
        UCrowdStampedeController* OtherController = 
            Actor->FindComponentByClass<UCrowdStampedeController>();
        if (OtherController && OtherController->bIsLeader)
        {
            float Dist = FVector::Dist(GetOwner()->GetActorLocation(), 
                Actor->GetActorLocation());
            if (Dist < NearestDist)
            {
                NearestDist = Dist;
                LeaderActor = Actor;
            }
        }
    }
}

void UCrowdStampedeController::UpdateGrazing(float DeltaTime)
{
    // Slow random wandering — grazing behavior
    // Occasionally change direction
    if (FMath::RandRange(0.0f, 1.0f) < 0.01f)
    {
        float RandomYaw = FMath::RandRange(-15.0f, 15.0f);
        FQuat RandomRot = FQuat(FVector::UpVector, FMath::DegreesToRadians(RandomYaw));
        StampedeDirection = RandomRot.RotateVector(StampedeDirection);
    }

    // Cohesion — drift toward herd center
    if (LeaderActor)
    {
        FVector ToLeader = (LeaderActor->GetActorLocation() - 
            GetOwner()->GetActorLocation()).GetSafeNormal();
        float LeaderDist = FVector::Dist(GetOwner()->GetActorLocation(), 
            LeaderActor->GetActorLocation());
        
        if (LeaderDist > HerdCohesionRadius)
        {
            StampedeDirection = FMath::Lerp(StampedeDirection, ToLeader, 0.1f);
            StampedeDirection.Normalize();
        }
    }

    StampedeSpeed = 80.0f; // Slow grazing pace
}

void UCrowdStampedeController::UpdateAlert(float DeltaTime)
{
    // Stopped, head up, scanning for threat
    StampedeSpeed = 0.0f;
    TimeSincePanic += DeltaTime;

    if (TimeSincePanic > 3.0f)
    {
        // Threat passed — return to grazing
        StampedeState = ECrowd_StampedeState::Grazing;
        TimeSincePanic = 0.0f;
    }
}

void UCrowdStampedeController::UpdateStampeding(float DeltaTime)
{
    TimeSincePanic += DeltaTime;

    // Accelerate to full speed
    StampedeSpeed = FMath::FInterpTo(StampedeSpeed, MaxStampedeSpeed, DeltaTime, 2.0f);

    // Move actor in stampede direction
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector NewLocation = Owner->GetActorLocation() + 
            StampedeDirection * StampedeSpeed * DeltaTime;
        Owner->SetActorLocation(NewLocation, true);

        // Face stampede direction
        FRotator NewRotation = StampedeDirection.Rotation();
        Owner->SetActorRotation(NewRotation);
    }

    // Separation — avoid other stampeding agents
    ApplySeparationForce(DeltaTime);

    // Panic decays over time
    if (TimeSincePanic > PanicDecayTime)
    {
        StampedeState = ECrowd_StampedeState::Dispersing;
    }
}

void UCrowdStampedeController::UpdateDispersing(float DeltaTime)
{
    // Slow down and scatter
    StampedeSpeed = FMath::FInterpTo(StampedeSpeed, 150.0f, DeltaTime, 1.0f);

    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector NewLocation = Owner->GetActorLocation() + 
            StampedeDirection * StampedeSpeed * DeltaTime;
        Owner->SetActorLocation(NewLocation, true);
    }

    TimeSincePanic += DeltaTime;
    if (TimeSincePanic > PanicDecayTime + 15.0f)
    {
        StampedeState = ECrowd_StampedeState::Grazing;
        TimeSincePanic = 0.0f;
        PanicTriggerActor = nullptr;
    }
}

void UCrowdStampedeController::ApplySeparationForce(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    FVector MyLocation = GetOwner()->GetActorLocation();
    FVector SeparationForce = FVector::ZeroVector;
    float SeparationRadius = 150.0f;
    int32 NeighborCount = 0;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, GetOwner()->GetClass(), NearbyActors);

    for (AActor* NearbyActor : NearbyActors)
    {
        if (!NearbyActor || NearbyActor == GetOwner()) continue;

        FVector ToNeighbor = NearbyActor->GetActorLocation() - MyLocation;
        float Distance = ToNeighbor.Size();

        if (Distance < SeparationRadius && Distance > 0.0f)
        {
            // Push away from neighbor, stronger when closer
            float Force = (SeparationRadius - Distance) / SeparationRadius;
            SeparationForce -= ToNeighbor.GetSafeNormal() * Force;
            NeighborCount++;
        }
    }

    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
        StampedeDirection = (StampedeDirection + SeparationForce * 0.3f).GetSafeNormal();
    }
}

ECrowd_StampedeState UCrowdStampedeController::GetStampedeState() const
{
    return StampedeState;
}

float UCrowdStampedeController::GetStampedeSpeed() const
{
    return StampedeSpeed;
}
