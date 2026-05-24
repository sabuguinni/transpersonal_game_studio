#include "Crowd_FlockingComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UCrowd_FlockingComponent::UCrowd_FlockingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    CurrentVelocity = FVector::ZeroVector;
    LastBehaviorChange = 0.0f;
    TargetDirection = FVector::ForwardVector;
}

void UCrowd_FlockingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize random behavior
    CurrentBehavior = ECrowd_FlockBehavior::Wandering;
    LastBehaviorChange = GetWorld()->GetTimeSeconds();
    
    // Set random initial direction
    TargetDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();
}

void UCrowd_FlockingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }

    // Update nearby flockmates
    UpdateNearbyFlockmates();
    
    // Check for threats
    bool bThreatDetected = DetectThreats();
    
    // Change behavior based on time and threats
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (bThreatDetected)
    {
        SetBehavior(ECrowd_FlockBehavior::Fleeing);
    }
    else if (CurrentTime - LastBehaviorChange > BehaviorChangeInterval)
    {
        // Random behavior change
        int32 RandomBehavior = FMath::RandRange(0, 3);
        ECrowd_FlockBehavior NewBehavior = static_cast<ECrowd_FlockBehavior>(RandomBehavior);
        SetBehavior(NewBehavior);
        LastBehaviorChange = CurrentTime;
    }

    // Calculate movement forces based on current behavior
    FVector TotalForce = FVector::ZeroVector;
    
    switch (CurrentBehavior)
    {
        case ECrowd_FlockBehavior::Flocking:
            TotalForce += CalculateSeparation() * FlockingParams.SeparationWeight;
            TotalForce += CalculateAlignment() * FlockingParams.AlignmentWeight;
            TotalForce += CalculateCohesion() * FlockingParams.CohesionWeight;
            break;
            
        case ECrowd_FlockBehavior::Fleeing:
            TotalForce += CalculateFleeFromThreat() * 3.0f;
            TotalForce += CalculateSeparation() * FlockingParams.SeparationWeight;
            break;
            
        case ECrowd_FlockBehavior::Wandering:
            TotalForce += Wander() * 1.5f;
            TotalForce += CalculateSeparation() * FlockingParams.SeparationWeight * 0.5f;
            break;
            
        case ECrowd_FlockBehavior::Feeding:
            // Slow movement, stay close to ground
            TotalForce += Wander() * 0.3f;
            break;
            
        case ECrowd_FlockBehavior::Resting:
            // Minimal movement
            TotalForce += CalculateSeparation() * FlockingParams.SeparationWeight * 0.2f;
            break;
    }

    // Apply forces and update position
    ApplyForce(TotalForce, DeltaTime);
}

void UCrowd_FlockingComponent::UpdateNearbyFlockmates()
{
    NearbyFlockmates.Empty();
    
    if (!GetOwner())
    {
        return;
    }

    FVector MyLocation = GetOwner()->GetActorLocation();
    float SearchRadius = FMath::Max(FlockingParams.CohesionRadius, FlockingParams.AlignmentRadius);
    
    // Find all pawns within range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetOwner() && Actor)
        {
            float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                // Check if this actor also has flocking component (same species)
                if (Actor->FindComponentByClass<UCrowd_FlockingComponent>())
                {
                    NearbyFlockmates.Add(Actor);
                }
            }
        }
    }
}

FVector UCrowd_FlockingComponent::CalculateSeparation()
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    if (!GetOwner())
    {
        return SeparationForce;
    }

    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Flockmate : NearbyFlockmates)
    {
        if (!Flockmate) continue;
        
        float Distance = FVector::Dist(MyLocation, Flockmate->GetActorLocation());
        if (Distance > 0 && Distance < FlockingParams.SeparationRadius)
        {
            FVector Diff = MyLocation - Flockmate->GetActorLocation();
            Diff.Normalize();
            Diff /= Distance; // Weight by distance
            SeparationForce += Diff;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce.Normalize();
        SeparationForce *= FlockingParams.MaxSpeed;
        SeparationForce -= CurrentVelocity;
        SeparationForce = LimitVector(SeparationForce, FlockingParams.MaxForce);
    }
    
    return SeparationForce;
}

FVector UCrowd_FlockingComponent::CalculateAlignment()
{
    FVector AlignmentForce = FVector::ZeroVector;
    int32 Count = 0;
    
    if (!GetOwner())
    {
        return AlignmentForce;
    }

    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Flockmate : NearbyFlockmates)
    {
        if (!Flockmate) continue;
        
        float Distance = FVector::Dist(MyLocation, Flockmate->GetActorLocation());
        if (Distance > 0 && Distance < FlockingParams.AlignmentRadius)
        {
            UCrowd_FlockingComponent* FlockComp = Flockmate->FindComponentByClass<UCrowd_FlockingComponent>();
            if (FlockComp)
            {
                AlignmentForce += FlockComp->CurrentVelocity;
                Count++;
            }
        }
    }
    
    if (Count > 0)
    {
        AlignmentForce /= Count;
        AlignmentForce.Normalize();
        AlignmentForce *= FlockingParams.MaxSpeed;
        AlignmentForce -= CurrentVelocity;
        AlignmentForce = LimitVector(AlignmentForce, FlockingParams.MaxForce);
    }
    
    return AlignmentForce;
}

FVector UCrowd_FlockingComponent::CalculateCohesion()
{
    FVector CohesionForce = FVector::ZeroVector;
    int32 Count = 0;
    
    if (!GetOwner())
    {
        return CohesionForce;
    }

    FVector MyLocation = GetOwner()->GetActorLocation();
    FVector CenterOfMass = FVector::ZeroVector;
    
    for (AActor* Flockmate : NearbyFlockmates)
    {
        if (!Flockmate) continue;
        
        float Distance = FVector::Dist(MyLocation, Flockmate->GetActorLocation());
        if (Distance > 0 && Distance < FlockingParams.CohesionRadius)
        {
            CenterOfMass += Flockmate->GetActorLocation();
            Count++;
        }
    }
    
    if (Count > 0)
    {
        CenterOfMass /= Count;
        CohesionForce = Seek(CenterOfMass);
    }
    
    return CohesionForce;
}

FVector UCrowd_FlockingComponent::CalculateFleeFromThreat()
{
    FVector FleeForce = FVector::ZeroVector;
    
    if (DetectedThreat && GetOwner())
    {
        FVector MyLocation = GetOwner()->GetActorLocation();
        FVector ThreatLocation = DetectedThreat->GetActorLocation();
        FVector FleeDirection = MyLocation - ThreatLocation;
        FleeDirection.Normalize();
        
        FleeForce = FleeDirection * FlockingParams.MaxSpeed;
        FleeForce -= CurrentVelocity;
        FleeForce = LimitVector(FleeForce, FlockingParams.MaxForce * 2.0f); // Stronger flee force
    }
    
    return FleeForce;
}

bool UCrowd_FlockingComponent::DetectThreats()
{
    DetectedThreat = nullptr;
    
    if (!GetOwner())
    {
        return false;
    }

    FVector MyLocation = GetOwner()->GetActorLocation();
    
    // Check for player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float PlayerDistance = FVector::Dist(MyLocation, PlayerPawn->GetActorLocation());
        if (PlayerDistance <= PlayerDetectionRadius)
        {
            DetectedThreat = PlayerPawn;
            return true;
        }
    }
    
    // Check for predators (actors with "Predator" in name)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != GetOwner())
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains("Predator") || ActorName.Contains("TRex") || ActorName.Contains("Raptor"))
            {
                float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
                if (Distance <= PredatorDetectionRadius)
                {
                    DetectedThreat = Actor;
                    return true;
                }
            }
        }
    }
    
    return false;
}

void UCrowd_FlockingComponent::SetBehavior(ECrowd_FlockBehavior NewBehavior)
{
    if (CurrentBehavior != NewBehavior)
    {
        CurrentBehavior = NewBehavior;
        LastBehaviorChange = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    }
}

FVector UCrowd_FlockingComponent::Seek(FVector Target)
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector Desired = Target - GetOwner()->GetActorLocation();
    Desired.Normalize();
    Desired *= FlockingParams.MaxSpeed;
    
    FVector Steer = Desired - CurrentVelocity;
    Steer = LimitVector(Steer, FlockingParams.MaxForce);
    
    return Steer;
}

FVector UCrowd_FlockingComponent::Wander()
{
    // Simple wandering behavior
    float WanderStrength = 50.0f;
    FVector WanderForce = FVector(
        FMath::RandRange(-WanderStrength, WanderStrength),
        FMath::RandRange(-WanderStrength, WanderStrength),
        0.0f
    );
    
    // Add some persistence to direction
    TargetDirection += WanderForce * 0.1f;
    TargetDirection.Normalize();
    
    return TargetDirection * FlockingParams.MaxSpeed * 0.5f;
}

void UCrowd_FlockingComponent::ApplyForce(FVector Force, float DeltaTime)
{
    if (!GetOwner())
    {
        return;
    }

    // Update velocity
    CurrentVelocity += Force * DeltaTime;
    CurrentVelocity = LimitVector(CurrentVelocity, FlockingParams.MaxSpeed);
    
    // Update position
    FVector NewLocation = GetOwner()->GetActorLocation() + (CurrentVelocity * DeltaTime);
    
    // Keep actors above ground (simple ground clamping)
    NewLocation.Z = FMath::Max(NewLocation.Z, 100.0f);
    
    GetOwner()->SetActorLocation(NewLocation);
    
    // Update rotation to face movement direction
    if (!CurrentVelocity.IsNearlyZero())
    {
        FRotator NewRotation = CurrentVelocity.Rotation();
        NewRotation.Pitch = 0.0f; // Keep level
        GetOwner()->SetActorRotation(NewRotation);
    }
}

FVector UCrowd_FlockingComponent::LimitVector(FVector Vector, float MaxLength)
{
    if (Vector.SizeSquared() > MaxLength * MaxLength)
    {
        Vector.Normalize();
        Vector *= MaxLength;
    }
    return Vector;
}