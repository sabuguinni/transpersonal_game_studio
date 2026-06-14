#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    MaxCrowdEntities = 1000;
    LODUpdateInterval = 0.1f;
    HighLODDistance = 1000.0f;
    MediumLODDistance = 2500.0f;
    CullDistance = 5000.0f;
    LODUpdateTimer = 0.0f;
    
    CrowdEntities.Reserve(MaxCrowdEntities);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Mass Entity Subsystem Initialized"));
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    CrowdEntities.Empty();
    Waypoints.Empty();
    
    Super::Deinitialize();
}

void UCrowd_MassEntitySubsystem::Tick(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }
    
    ProcessCrowdMovement(DeltaTime);
    
    LODUpdateTimer += DeltaTime;
    if (LODUpdateTimer >= LODUpdateInterval)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            UpdateCrowdLOD(PlayerPawn->GetActorLocation());
        }
        LODUpdateTimer = 0.0f;
    }
}

bool UCrowd_MassEntitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntities(int32 Count, const FVector& Center, float Radius)
{
    if (!GetWorld())
    {
        return;
    }
    
    int32 ActualCount = FMath::Min(Count, MaxCrowdEntities - CrowdEntities.Num());
    
    for (int32 i = 0; i < ActualCount; i++)
    {
        FCrowd_EntityData NewEntity;
        
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Radius);
        
        NewEntity.Location = Center + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        NewEntity.Speed = FMath::RandRange(80.0f, 150.0f);
        NewEntity.Velocity = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal() * NewEntity.Speed;
        
        NewEntity.LODLevel = 0;
        NewEntity.bIsActive = true;
        
        CrowdEntities.Add(NewEntity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d crowd entities at %s"), ActualCount, *Center.ToString());
}

void UCrowd_MassEntitySubsystem::UpdateCrowdLOD(const FVector& PlayerLocation)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (Entity.bIsActive)
        {
            UpdateEntityLOD(Entity, PlayerLocation);
        }
    }
}

void UCrowd_MassEntitySubsystem::UpdateEntityLOD(FCrowd_EntityData& Entity, const FVector& PlayerLocation)
{
    float Distance = FVector::Dist(Entity.Location, PlayerLocation);
    
    if (Distance > CullDistance)
    {
        Entity.LODLevel = static_cast<int32>(ECrowd_LODLevel::Culled);
        Entity.bIsActive = false;
    }
    else if (Distance > MediumLODDistance)
    {
        Entity.LODLevel = static_cast<int32>(ECrowd_LODLevel::Low);
        Entity.bIsActive = true;
    }
    else if (Distance > HighLODDistance)
    {
        Entity.LODLevel = static_cast<int32>(ECrowd_LODLevel::Medium);
        Entity.bIsActive = true;
    }
    else
    {
        Entity.LODLevel = static_cast<int32>(ECrowd_LODLevel::High);
        Entity.bIsActive = true;
    }
}

void UCrowd_MassEntitySubsystem::ProcessCrowdMovement(float DeltaTime)
{
    for (int32 i = 0; i < CrowdEntities.Num(); i++)
    {
        FCrowd_EntityData& Entity = CrowdEntities[i];
        
        if (!Entity.bIsActive || Entity.LODLevel >= static_cast<int32>(ECrowd_LODLevel::Culled))
        {
            continue;
        }
        
        FVector FlockingForce = CalculateFlockingBehavior(Entity, i);
        Entity.Velocity += FlockingForce * DeltaTime;
        Entity.Velocity = Entity.Velocity.GetClampedToMaxSize(Entity.Speed);
        
        Entity.Location += Entity.Velocity * DeltaTime;
        
        if (GetWorld() && Entity.LODLevel == static_cast<int32>(ECrowd_LODLevel::High))
        {
            DrawDebugSphere(GetWorld(), Entity.Location, 25.0f, 8, FColor::Green, false, 0.1f);
        }
    }
}

FVector UCrowd_MassEntitySubsystem::CalculateFlockingBehavior(const FCrowd_EntityData& Entity, int32 EntityIndex)
{
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    
    int32 NeighborCount = 0;
    float NeighborRadius = 200.0f;
    
    for (int32 i = 0; i < CrowdEntities.Num(); i++)
    {
        if (i == EntityIndex || !CrowdEntities[i].bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Entity.Location, CrowdEntities[i].Location);
        
        if (Distance < NeighborRadius)
        {
            FVector Diff = Entity.Location - CrowdEntities[i].Location;
            if (Distance > 0.0f)
            {
                Separation += Diff.GetSafeNormal() / Distance;
            }
            
            Alignment += CrowdEntities[i].Velocity;
            Cohesion += CrowdEntities[i].Location;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        Alignment /= NeighborCount;
        Alignment = Alignment.GetSafeNormal() * Entity.Speed;
        Alignment = (Alignment - Entity.Velocity).GetClampedToMaxSize(50.0f);
        
        Cohesion /= NeighborCount;
        Cohesion = (Cohesion - Entity.Location).GetSafeNormal() * Entity.Speed;
        Cohesion = (Cohesion - Entity.Velocity).GetClampedToMaxSize(30.0f);
    }
    
    Separation = Separation.GetClampedToMaxSize(100.0f);
    
    return (Separation * 1.5f + Alignment * 1.0f + Cohesion * 1.0f);
}

void UCrowd_MassEntitySubsystem::SetCrowdBehavior(int32 EntityIndex, const FVector& TargetLocation)
{
    if (CrowdEntities.IsValidIndex(EntityIndex))
    {
        FCrowd_EntityData& Entity = CrowdEntities[EntityIndex];
        FVector Direction = (TargetLocation - Entity.Location).GetSafeNormal();
        Entity.Velocity = Direction * Entity.Speed;
    }
}

int32 UCrowd_MassEntitySubsystem::GetActiveCrowdCount() const
{
    int32 ActiveCount = 0;
    for (const FCrowd_EntityData& Entity : CrowdEntities)
    {
        if (Entity.bIsActive)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

void UCrowd_MassEntitySubsystem::CullDistantEntities(const FVector& PlayerLocation, float CullDistance)
{
    for (FCrowd_EntityData& Entity : CrowdEntities)
    {
        float Distance = FVector::Dist(Entity.Location, PlayerLocation);
        if (Distance > CullDistance)
        {
            Entity.bIsActive = false;
        }
    }
}