#include "Crowd_PrehistoricHerdManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

ACrowd_PrehistoricHerdManager::ACrowd_PrehistoricHerdManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize herd configuration
    HerdSpecies = ECrowd_DinosaurSpecies::Triceratops;
    MaxHerdSize = 25;
    HerdCohesionRadius = 800.0f;
    SeparationRadius = 200.0f;
    AlignmentRadius = 600.0f;

    // Movement parameters
    MaxSpeed = 300.0f;
    MaxForce = 150.0f;
    WanderRadius = 100.0f;
    WanderDistance = 200.0f;
    WanderJitter = 50.0f;

    // Threat detection
    ThreatDetectionRadius = 1500.0f;
    FleeRadius = 2000.0f;

    // Grazing behavior
    GrazingDuration = 30.0f;
    GrazingRadius = 1200.0f;

    // Migration
    CurrentWaypointIndex = 0;
    WaypointReachRadius = 500.0f;

    // Audio setup
    HerdAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("HerdAudioComponent"));
    RootComponent = HerdAudioComponent;

    // Initialize timers
    GrazingTimer = 0.0f;
    WanderTarget = FVector::ZeroVector;
    LastThreatCheckTime = 0.0f;
    ThreatCheckInterval = 2.0f;

    // Initialize behavior state
    BehaviorState.CurrentState = ECrowd_HerdState::Grazing;
    BehaviorState.HerdRadius = HerdCohesionRadius;
    BehaviorState.AlertLevel = 0.0f;
}

void ACrowd_PrehistoricHerdManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize herd center
    BehaviorState.HerdCenter = GetActorLocation();
    
    // Set up initial grazing spots around the spawn location
    for (int32 i = 0; i < 8; i++)
    {
        float Angle = (i * 45.0f) * PI / 180.0f;
        FVector GrazingSpot = GetActorLocation() + FVector(
            FMath::Cos(Angle) * GrazingRadius,
            FMath::Sin(Angle) * GrazingRadius,
            0.0f
        );
        GrazingSpots.Add(GrazingSpot);
    }

    // Set up migration waypoints in a large circuit
    FVector BaseLocation = GetActorLocation();
    MigrationWaypoints.Add(BaseLocation + FVector(3000.0f, 0.0f, 0.0f));
    MigrationWaypoints.Add(BaseLocation + FVector(3000.0f, 3000.0f, 0.0f));
    MigrationWaypoints.Add(BaseLocation + FVector(0.0f, 3000.0f, 0.0f));
    MigrationWaypoints.Add(BaseLocation + FVector(-3000.0f, 3000.0f, 0.0f));
    MigrationWaypoints.Add(BaseLocation + FVector(-3000.0f, 0.0f, 0.0f));
    MigrationWaypoints.Add(BaseLocation + FVector(-3000.0f, -3000.0f, 0.0f));
    MigrationWaypoints.Add(BaseLocation + FVector(0.0f, -3000.0f, 0.0f));
    MigrationWaypoints.Add(BaseLocation + FVector(3000.0f, -3000.0f, 0.0f));

    // Start with grazing behavior
    StartGrazing();
}

void ACrowd_PrehistoricHerdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update herd behavior based on current state
    UpdateHerdBehavior(DeltaTime);

    // Periodic threat detection
    if (GetWorld()->GetTimeSeconds() - LastThreatCheckTime > ThreatCheckInterval)
    {
        DetectThreats();
        LastThreatCheckTime = GetWorld()->GetTimeSeconds();
    }

    // Update behavior state based on current state
    switch (BehaviorState.CurrentState)
    {
        case ECrowd_HerdState::Grazing:
            UpdateGrazing(DeltaTime);
            break;
        case ECrowd_HerdState::Migrating:
            UpdateMigration(DeltaTime);
            break;
        case ECrowd_HerdState::Fleeing:
            // Fleeing behavior is handled in UpdateHerdBehavior
            break;
        case ECrowd_HerdState::Drinking:
            // Similar to grazing but near water sources
            UpdateGrazing(DeltaTime);
            break;
    }
}

void ACrowd_PrehistoricHerdManager::AddHerdMember(AActor* NewMember, bool bIsAlpha)
{
    if (!NewMember || HerdMembers.Num() >= MaxHerdSize)
    {
        return;
    }

    FCrowd_HerdMember NewHerdMember;
    NewHerdMember.Actor = NewMember;
    NewHerdMember.Position = NewMember->GetActorLocation();
    NewHerdMember.Velocity = FVector::ZeroVector;
    NewHerdMember.Health = 100.0f;
    NewHerdMember.Fear = 0.0f;
    NewHerdMember.bIsAlpha = bIsAlpha;
    NewHerdMember.Species = HerdSpecies;

    HerdMembers.Add(NewHerdMember);

    UE_LOG(LogTemp, Log, TEXT("Added herd member. Total herd size: %d"), HerdMembers.Num());
}

void ACrowd_PrehistoricHerdManager::RemoveHerdMember(AActor* Member)
{
    for (int32 i = HerdMembers.Num() - 1; i >= 0; i--)
    {
        if (HerdMembers[i].Actor == Member)
        {
            HerdMembers.RemoveAt(i);
            break;
        }
    }
}

void ACrowd_PrehistoricHerdManager::SetHerdState(ECrowd_HerdState NewState)
{
    if (BehaviorState.CurrentState != NewState)
    {
        BehaviorState.CurrentState = NewState;
        PlayHerdSound(NewState);

        switch (NewState)
        {
            case ECrowd_HerdState::Grazing:
                StartGrazing();
                break;
            case ECrowd_HerdState::Migrating:
                StartMigration();
                break;
            case ECrowd_HerdState::Fleeing:
                BehaviorState.AlertLevel = 1.0f;
                break;
            case ECrowd_HerdState::Drinking:
                // Find nearest water source
                break;
        }
    }
}

FVector ACrowd_PrehistoricHerdManager::CalculateHerdCenter()
{
    if (HerdMembers.Num() == 0)
    {
        return GetActorLocation();
    }

    FVector Center = FVector::ZeroVector;
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.Actor)
        {
            Center += Member.Actor->GetActorLocation();
        }
    }

    Center /= HerdMembers.Num();
    BehaviorState.HerdCenter = Center;
    return Center;
}

void ACrowd_PrehistoricHerdManager::UpdateHerdBehavior(float DeltaTime)
{
    if (HerdMembers.Num() == 0)
    {
        return;
    }

    // Update herd center
    CalculateHerdCenter();

    // Update each herd member
    for (FCrowd_HerdMember& Member : HerdMembers)
    {
        if (!Member.Actor)
        {
            continue;
        }

        // Update member position
        Member.Position = Member.Actor->GetActorLocation();

        // Calculate steering forces
        FVector CohesionForce = CalculateCohesionForce(Member);
        FVector SeparationForce = CalculateSeparationForce(Member);
        FVector AlignmentForce = CalculateAlignmentForce(Member);
        FVector WanderForce = CalculateWanderForce(Member);

        // Combine forces based on current state
        FVector TotalForce = FVector::ZeroVector;
        
        switch (BehaviorState.CurrentState)
        {
            case ECrowd_HerdState::Grazing:
                TotalForce = CohesionForce * 0.3f + SeparationForce * 0.5f + WanderForce * 0.2f;
                break;
            case ECrowd_HerdState::Migrating:
                if (MigrationWaypoints.Num() > 0)
                {
                    FVector SeekForce = CalculateSeekForce(Member, MigrationWaypoints[CurrentWaypointIndex]);
                    TotalForce = SeekForce * 0.4f + CohesionForce * 0.3f + SeparationForce * 0.3f;
                }
                break;
            case ECrowd_HerdState::Fleeing:
                if (BehaviorState.ThreatTarget)
                {
                    FVector FleeForce = CalculateFleeForce(Member, BehaviorState.ThreatTarget->GetActorLocation());
                    TotalForce = FleeForce * 0.6f + SeparationForce * 0.4f;
                }
                break;
            case ECrowd_HerdState::Drinking:
                TotalForce = CohesionForce * 0.4f + SeparationForce * 0.6f;
                break;
        }

        // Apply force limits
        if (TotalForce.Size() > MaxForce)
        {
            TotalForce = TotalForce.GetSafeNormal() * MaxForce;
        }

        // Update velocity
        Member.Velocity += TotalForce * DeltaTime;
        if (Member.Velocity.Size() > MaxSpeed)
        {
            Member.Velocity = Member.Velocity.GetSafeNormal() * MaxSpeed;
        }

        // Update position
        FVector NewLocation = Member.Position + Member.Velocity * DeltaTime;
        Member.Actor->SetActorLocation(NewLocation);

        // Update rotation to face movement direction
        if (Member.Velocity.Size() > 10.0f)
        {
            FRotator NewRotation = FRotationMatrix::MakeFromX(Member.Velocity.GetSafeNormal()).Rotator();
            Member.Actor->SetActorRotation(NewRotation);
        }
    }
}

FVector ACrowd_PrehistoricHerdManager::CalculateCohesionForce(const FCrowd_HerdMember& Member)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (Other.Actor && Other.Actor != Member.Actor)
        {
            float Distance = FVector::Dist(Member.Position, Other.Position);
            if (Distance < HerdCohesionRadius)
            {
                CenterOfMass += Other.Position;
                Count++;
            }
        }
    }

    if (Count > 0)
    {
        CenterOfMass /= Count;
        return CalculateSeekForce(Member, CenterOfMass);
    }

    return FVector::ZeroVector;
}

FVector ACrowd_PrehistoricHerdManager::CalculateSeparationForce(const FCrowd_HerdMember& Member)
{
    FVector SeparationForce = FVector::ZeroVector;

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (Other.Actor && Other.Actor != Member.Actor)
        {
            float Distance = FVector::Dist(Member.Position, Other.Position);
            if (Distance < SeparationRadius && Distance > 0.0f)
            {
                FVector Diff = Member.Position - Other.Position;
                Diff.Normalize();
                Diff /= Distance; // Weight by distance
                SeparationForce += Diff;
            }
        }
    }

    return SeparationForce * MaxForce;
}

FVector ACrowd_PrehistoricHerdManager::CalculateAlignmentForce(const FCrowd_HerdMember& Member)
{
    FVector AverageVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (Other.Actor && Other.Actor != Member.Actor)
        {
            float Distance = FVector::Dist(Member.Position, Other.Position);
            if (Distance < AlignmentRadius)
            {
                AverageVelocity += Other.Velocity;
                Count++;
            }
        }
    }

    if (Count > 0)
    {
        AverageVelocity /= Count;
        AverageVelocity.Normalize();
        return AverageVelocity * MaxForce;
    }

    return FVector::ZeroVector;
}

FVector ACrowd_PrehistoricHerdManager::CalculateWanderForce(FCrowd_HerdMember& Member)
{
    // Wander behavior using Craig Reynolds' algorithm
    if (WanderTarget.IsZero())
    {
        WanderTarget = FVector(WanderRadius, 0.0f, 0.0f);
    }

    // Add random jitter to the wander target
    WanderTarget += FVector(
        FMath::RandRange(-WanderJitter, WanderJitter),
        FMath::RandRange(-WanderJitter, WanderJitter),
        0.0f
    );

    // Normalize and scale to wander radius
    WanderTarget.Normalize();
    WanderTarget *= WanderRadius;

    // Project the target in front of the agent
    FVector WanderCenter = Member.Position + Member.Velocity.GetSafeNormal() * WanderDistance;
    FVector Target = WanderCenter + WanderTarget;

    return CalculateSeekForce(Member, Target);
}

FVector ACrowd_PrehistoricHerdManager::CalculateSeekForce(const FCrowd_HerdMember& Member, FVector Target)
{
    FVector Desired = Target - Member.Position;
    Desired.Normalize();
    Desired *= MaxSpeed;

    FVector Steering = Desired - Member.Velocity;
    return Steering;
}

FVector ACrowd_PrehistoricHerdManager::CalculateFleeForce(const FCrowd_HerdMember& Member, FVector Threat)
{
    FVector Desired = Member.Position - Threat;
    Desired.Normalize();
    Desired *= MaxSpeed;

    FVector Steering = Desired - Member.Velocity;
    return Steering;
}

void ACrowd_PrehistoricHerdManager::DetectThreats()
{
    if (!GetWorld())
    {
        return;
    }

    // Find all actors within threat detection radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    AActor* NearestThreat = nullptr;
    float NearestDistance = ThreatDetectionRadius;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor)
        {
            continue;
        }

        // Check if this actor is a threat
        bool bIsThreat = false;
        for (TSubclassOf<AActor> ThreatClass : ThreatClasses)
        {
            if (Actor->IsA(ThreatClass))
            {
                bIsThreat = true;
                break;
            }
        }

        if (bIsThreat)
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestThreat = Actor;
                NearestDistance = Distance;
            }
        }
    }

    if (NearestThreat)
    {
        HandleThreatResponse(NearestThreat);
    }
    else if (BehaviorState.CurrentState == ECrowd_HerdState::Fleeing)
    {
        // No more threats, return to grazing
        SetHerdState(ECrowd_HerdState::Grazing);
    }
}

void ACrowd_PrehistoricHerdManager::HandleThreatResponse(AActor* Threat)
{
    BehaviorState.ThreatTarget = Threat;
    
    float Distance = FVector::Dist(GetActorLocation(), Threat->GetActorLocation());
    if (Distance < FleeRadius)
    {
        SetHerdState(ECrowd_HerdState::Fleeing);
        BehaviorState.AlertLevel = FMath::Clamp(1.0f - (Distance / FleeRadius), 0.0f, 1.0f);
    }
}

void ACrowd_PrehistoricHerdManager::StartMigration()
{
    if (MigrationWaypoints.Num() > 0)
    {
        CurrentWaypointIndex = 0;
        BehaviorState.MovementDirection = (MigrationWaypoints[0] - GetActorLocation()).GetSafeNormal();
    }
}

void ACrowd_PrehistoricHerdManager::UpdateMigration(float DeltaTime)
{
    if (MigrationWaypoints.Num() == 0)
    {
        return;
    }

    FVector CurrentWaypoint = MigrationWaypoints[CurrentWaypointIndex];
    float DistanceToWaypoint = FVector::Dist(BehaviorState.HerdCenter, CurrentWaypoint);

    if (DistanceToWaypoint < WaypointReachRadius)
    {
        // Move to next waypoint
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % MigrationWaypoints.Num();
        if (CurrentWaypointIndex == 0)
        {
            // Completed full migration circuit, switch to grazing
            SetHerdState(ECrowd_HerdState::Grazing);
        }
    }
}

void ACrowd_PrehistoricHerdManager::StartGrazing()
{
    GrazingTimer = 0.0f;
    BehaviorState.AlertLevel = 0.0f;
}

void ACrowd_PrehistoricHerdManager::UpdateGrazing(float DeltaTime)
{
    GrazingTimer += DeltaTime;
    
    // After grazing for a while, consider migration
    if (GrazingTimer > GrazingDuration)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance to start migration
        {
            SetHerdState(ECrowd_HerdState::Migrating);
        }
        else
        {
            GrazingTimer = 0.0f; // Continue grazing
        }
    }
}

void ACrowd_PrehistoricHerdManager::PlayHerdSound(ECrowd_HerdState State)
{
    if (!HerdAudioComponent)
    {
        return;
    }

    // Play different sounds based on herd state
    switch (State)
    {
        case ECrowd_HerdState::Grazing:
            UE_LOG(LogTemp, Log, TEXT("Herd grazing sounds"));
            break;
        case ECrowd_HerdState::Migrating:
            UE_LOG(LogTemp, Log, TEXT("Herd migration sounds"));
            break;
        case ECrowd_HerdState::Fleeing:
            UE_LOG(LogTemp, Warning, TEXT("Herd alarm sounds"));
            break;
        case ECrowd_HerdState::Drinking:
            UE_LOG(LogTemp, Log, TEXT("Herd drinking sounds"));
            break;
    }
}