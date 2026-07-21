#include "Crowd_Entity.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACrowd_Entity::ACrowd_Entity()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(50.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

    // Create mesh component
    EntityMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EntityMesh"));
    EntityMesh->SetupAttachment(RootComponent);

    // Load default mesh (cube for now)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        EntityMesh->SetStaticMesh(CubeMeshAsset.Object);
        EntityMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 1.0f));
    }

    // Initialize default stats
    EntityStats.Health = 100.0f;
    EntityStats.Stamina = 100.0f;
    EntityStats.Speed = 300.0f;
    EntityStats.DetectionRadius = 500.0f;
    EntityStats.GroupCohesion = 0.7f;

    BehaviorType = ECrowd_BehaviorType::Wanderer;
    WanderRadius = 1000.0f;
    GroupDistance = 200.0f;
}

void ACrowd_Entity::BeginPlay()
{
    Super::BeginPlay();
    
    HomeLocation = GetActorLocation();
    TargetLocation = HomeLocation;
    
    // Randomize behavior change timer
    NextBehaviorChange = FMath::RandRange(3.0f, 8.0f);
}

void ACrowd_Entity::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateBehavior(DeltaTime);
    
    // Apply flocking forces
    FVector FlockingForce = CalculateFlockingForce();
    if (!FlockingForce.IsZero())
    {
        FVector NewLocation = GetActorLocation() + FlockingForce * DeltaTime;
        SetActorLocation(NewLocation);
    }
}

void ACrowd_Entity::SetBehaviorType(ECrowd_BehaviorType NewBehaviorType)
{
    BehaviorType = NewBehaviorType;
    BehaviorTimer = 0.0f;
    NextBehaviorChange = FMath::RandRange(3.0f, 8.0f);
}

void ACrowd_Entity::MoveToLocation(const FVector& Location)
{
    TargetLocation = Location;
    bIsMoving = true;
}

void ACrowd_Entity::UpdateBehavior(float DeltaTime)
{
    BehaviorTimer += DeltaTime;
    
    switch (BehaviorType)
    {
        case ECrowd_BehaviorType::Gatherer:
            ExecuteGathererBehavior(DeltaTime);
            break;
        case ECrowd_BehaviorType::Hunter:
            ExecuteHunterBehavior(DeltaTime);
            break;
        case ECrowd_BehaviorType::Wanderer:
            ExecuteWandererBehavior(DeltaTime);
            break;
        case ECrowd_BehaviorType::Guard:
            ExecuteGuardBehavior(DeltaTime);
            break;
        case ECrowd_BehaviorType::Scout:
            ExecuteScoutBehavior(DeltaTime);
            break;
    }
}

TArray<ACrowd_Entity*> ACrowd_Entity::FindNearbyEntities(float Radius)
{
    TArray<ACrowd_Entity*> NearbyEntities;
    TArray<AActor*> FoundActors;
    
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrowd_Entity::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != this)
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyEntities.Add(Cast<ACrowd_Entity>(Actor));
            }
        }
    }
    
    return NearbyEntities;
}

FVector ACrowd_Entity::CalculateFlockingForce()
{
    FVector SeparationForce = CalculateSeparationForce();
    FVector AlignmentForce = CalculateAlignmentForce();
    FVector CohesionForce = CalculateCohesionForce();
    
    // Weight the forces
    FVector TotalForce = SeparationForce * 2.0f + AlignmentForce * 1.0f + CohesionForce * 1.0f;
    
    // Normalize and apply speed
    if (!TotalForce.IsZero())
    {
        TotalForce.Normalize();
        TotalForce *= EntityStats.Speed;
    }
    
    return TotalForce;
}

FVector ACrowd_Entity::CalculateSeparationForce()
{
    FVector SeparationForce = FVector::ZeroVector;
    TArray<ACrowd_Entity*> NearbyEntities = FindNearbyEntities(GroupDistance);
    
    for (ACrowd_Entity* Entity : NearbyEntities)
    {
        FVector Direction = GetActorLocation() - Entity->GetActorLocation();
        float Distance = Direction.Size();
        
        if (Distance > 0.0f && Distance < GroupDistance)
        {
            Direction.Normalize();
            SeparationForce += Direction / Distance; // Closer entities have stronger repulsion
        }
    }
    
    return SeparationForce;
}

FVector ACrowd_Entity::CalculateAlignmentForce()
{
    FVector AverageVelocity = FVector::ZeroVector;
    TArray<ACrowd_Entity*> NearbyEntities = FindNearbyEntities(EntityStats.DetectionRadius);
    
    if (NearbyEntities.Num() > 0)
    {
        for (ACrowd_Entity* Entity : NearbyEntities)
        {
            FVector EntityVelocity = Entity->TargetLocation - Entity->GetActorLocation();
            AverageVelocity += EntityVelocity;
        }
        
        AverageVelocity /= NearbyEntities.Num();
        AverageVelocity.Normalize();
    }
    
    return AverageVelocity;
}

FVector ACrowd_Entity::CalculateCohesionForce()
{
    FVector CenterOfMass = FVector::ZeroVector;
    TArray<ACrowd_Entity*> NearbyEntities = FindNearbyEntities(EntityStats.DetectionRadius);
    
    if (NearbyEntities.Num() > 0)
    {
        for (ACrowd_Entity* Entity : NearbyEntities)
        {
            CenterOfMass += Entity->GetActorLocation();
        }
        
        CenterOfMass /= NearbyEntities.Num();
        FVector CohesionForce = CenterOfMass - GetActorLocation();
        CohesionForce.Normalize();
        
        return CohesionForce * EntityStats.GroupCohesion;
    }
    
    return FVector::ZeroVector;
}

void ACrowd_Entity::ExecuteGathererBehavior(float DeltaTime)
{
    if (BehaviorTimer >= NextBehaviorChange)
    {
        // Find a new gathering spot within wander radius
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f; // Keep on ground level
        RandomDirection.Normalize();
        
        TargetLocation = HomeLocation + RandomDirection * FMath::RandRange(100.0f, WanderRadius * 0.5f);
        BehaviorTimer = 0.0f;
        NextBehaviorChange = FMath::RandRange(5.0f, 10.0f);
    }
    
    // Move towards target slowly (gathering behavior)
    FVector Direction = TargetLocation - GetActorLocation();
    if (Direction.Size() > 50.0f)
    {
        Direction.Normalize();
        FVector NewLocation = GetActorLocation() + Direction * EntityStats.Speed * 0.3f * DeltaTime;
        SetActorLocation(NewLocation);
    }
}

void ACrowd_Entity::ExecuteHunterBehavior(float DeltaTime)
{
    if (BehaviorTimer >= NextBehaviorChange)
    {
        // Find a new hunting target location
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f;
        RandomDirection.Normalize();
        
        TargetLocation = HomeLocation + RandomDirection * FMath::RandRange(200.0f, WanderRadius);
        BehaviorTimer = 0.0f;
        NextBehaviorChange = FMath::RandRange(3.0f, 6.0f);
    }
    
    // Move towards target quickly (hunting behavior)
    FVector Direction = TargetLocation - GetActorLocation();
    if (Direction.Size() > 50.0f)
    {
        Direction.Normalize();
        FVector NewLocation = GetActorLocation() + Direction * EntityStats.Speed * 0.8f * DeltaTime;
        SetActorLocation(NewLocation);
    }
}

void ACrowd_Entity::ExecuteWandererBehavior(float DeltaTime)
{
    if (BehaviorTimer >= NextBehaviorChange)
    {
        // Random wandering
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f;
        RandomDirection.Normalize();
        
        TargetLocation = GetActorLocation() + RandomDirection * FMath::RandRange(200.0f, 600.0f);
        BehaviorTimer = 0.0f;
        NextBehaviorChange = FMath::RandRange(4.0f, 8.0f);
    }
    
    // Move towards target at normal speed
    FVector Direction = TargetLocation - GetActorLocation();
    if (Direction.Size() > 50.0f)
    {
        Direction.Normalize();
        FVector NewLocation = GetActorLocation() + Direction * EntityStats.Speed * 0.5f * DeltaTime;
        SetActorLocation(NewLocation);
    }
}

void ACrowd_Entity::ExecuteGuardBehavior(float DeltaTime)
{
    // Guards patrol around their home location
    if (BehaviorTimer >= NextBehaviorChange)
    {
        // Create patrol points around home
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float PatrolRadius = FMath::RandRange(100.0f, 300.0f);
        
        FVector PatrolOffset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * PatrolRadius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * PatrolRadius,
            0.0f
        );
        
        TargetLocation = HomeLocation + PatrolOffset;
        BehaviorTimer = 0.0f;
        NextBehaviorChange = FMath::RandRange(6.0f, 12.0f);
    }
    
    // Move towards patrol point
    FVector Direction = TargetLocation - GetActorLocation();
    if (Direction.Size() > 50.0f)
    {
        Direction.Normalize();
        FVector NewLocation = GetActorLocation() + Direction * EntityStats.Speed * 0.4f * DeltaTime;
        SetActorLocation(NewLocation);
    }
}

void ACrowd_Entity::ExecuteScoutBehavior(float DeltaTime)
{
    if (BehaviorTimer >= NextBehaviorChange)
    {
        // Scouts move in wide patterns, exploring far from home
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f;
        RandomDirection.Normalize();
        
        TargetLocation = HomeLocation + RandomDirection * FMath::RandRange(WanderRadius * 0.7f, WanderRadius * 1.5f);
        BehaviorTimer = 0.0f;
        NextBehaviorChange = FMath::RandRange(8.0f, 15.0f);
    }
    
    // Move towards target at high speed (scouting behavior)
    FVector Direction = TargetLocation - GetActorLocation();
    if (Direction.Size() > 50.0f)
    {
        Direction.Normalize();
        FVector NewLocation = GetActorLocation() + Direction * EntityStats.Speed * 0.9f * DeltaTime;
        SetActorLocation(NewLocation);
    }
}