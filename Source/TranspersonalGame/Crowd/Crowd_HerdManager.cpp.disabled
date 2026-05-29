#include "Crowd_HerdManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

UCrowd_HerdManager::UCrowd_HerdManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance

    // Default herd parameters
    MaxHerdSize = 100;
    HerdRadius = 2000.0f;
    MovementSpeed = 300.0f;
    FleeSpeed = 600.0f;
    ThreatDetectionRange = 1500.0f;
    GrazingDuration = 30.0f;
    MigrationDistance = 5000.0f;

    // Flocking parameters (Reynolds' boids)
    SeparationWeight = 2.0f;
    AlignmentWeight = 1.0f;
    CohesionWeight = 1.0f;
    SeparationRadius = 200.0f;
    AlignmentRadius = 500.0f;

    // Performance settings
    UpdateFrequency = 10.0f;
    MaxProcessedPerFrame = 20;
    LODDistance = 3000.0f;

    // Migration setup
    CurrentWaypointIndex = 0;
    SeasonalTimer = 0.0f;
    bIsMigrating = false;

    // Internal state
    LastUpdateTime = 0.0f;
    CurrentProcessIndex = 0;

    // Initialize behavior state
    BehaviorState.CurrentState = ECrowd_HerdState::Grazing;
    BehaviorState.CohesionRadius = HerdRadius;
}

void UCrowd_HerdManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_HerdManager: Initializing herd management system"));
    
    // Set up default migration waypoints if none provided
    if (MigrationWaypoints.Num() == 0)
    {
        SetMigrationWaypoints({
            FVector(0, 0, 0),
            FVector(5000, 0, 0),
            FVector(5000, 5000, 0),
            FVector(0, 5000, 0)
        });
    }
}

void UCrowd_HerdManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (HerdMembers.Num() == 0)
        return;

    // Update seasonal timer for migration triggers
    SeasonalTimer += DeltaTime;
    
    // Performance-optimized update system
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= (1.0f / UpdateFrequency))
    {
        UpdateHerdBehavior(LastUpdateTime);
        DetectThreats();
        LastUpdateTime = 0.0f;
    }
    
    // Process limited number of members per frame for performance
    int32 ProcessedThisFrame = 0;
    for (int32 i = CurrentProcessIndex; i < HerdMembers.Num() && ProcessedThisFrame < MaxProcessedPerFrame; i++)
    {
        if (ShouldProcessMember(HerdMembers[i]))
        {
            UpdateHerdMemberLOD(HerdMembers[i]);
            
            // Apply flocking behavior
            FVector FlockingForce = CalculateFlockingForce(HerdMembers[i]);
            
            if (HerdMembers[i].DinosaurPawn && IsValid(HerdMembers[i].DinosaurPawn))
            {
                // Apply movement based on current state
                FVector MovementVector = FVector::ZeroVector;
                
                switch (BehaviorState.CurrentState)
                {
                    case ECrowd_HerdState::Grazing:
                        MovementVector = FlockingForce * MovementSpeed * 0.3f; // Slow grazing movement
                        break;
                    case ECrowd_HerdState::Migrating:
                        MovementVector = (BehaviorState.TargetLocation - HerdMembers[i].DinosaurPawn->GetActorLocation()).GetSafeNormal() * MovementSpeed + FlockingForce * 0.5f;
                        break;
                    case ECrowd_HerdState::Fleeing:
                        MovementVector = FlockingForce * FleeSpeed;
                        break;
                    case ECrowd_HerdState::Resting:
                        MovementVector = FlockingForce * MovementSpeed * 0.1f; // Minimal movement
                        break;
                }
                
                // Apply movement with physics consideration
                if (MovementVector.Size() > 0.1f)
                {
                    FVector NewLocation = HerdMembers[i].DinosaurPawn->GetActorLocation() + MovementVector * DeltaTime;
                    HerdMembers[i].DinosaurPawn->SetActorLocation(NewLocation);
                    
                    // Update local position for formation tracking
                    HerdMembers[i].LocalPosition = NewLocation - GetHerdCenter();
                }
            }
        }
        
        ProcessedThisFrame++;
        CurrentProcessIndex = (CurrentProcessIndex + 1) % HerdMembers.Num();
    }
}

void UCrowd_HerdManager::SpawnHerd(FVector CenterLocation, int32 HerdSize)
{
    if (!DinosaurClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_HerdManager: No dinosaur class specified for herd spawning"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    HerdMembers.Empty();
    
    int32 ActualHerdSize = FMath::Min(HerdSize, MaxHerdSize);
    
    for (int32 i = 0; i < ActualHerdSize; i++)
    {
        // Generate random position within herd radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, HerdRadius);
        
        FVector SpawnLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Spawn dinosaur
        APawn* NewDinosaur = World->SpawnActor<APawn>(DinosaurClass, SpawnLocation, FRotator::ZeroRotator);
        
        if (NewDinosaur)
        {
            FCrowd_HerdMember NewMember;
            NewMember.DinosaurPawn = NewDinosaur;
            NewMember.LocalPosition = SpawnLocation - CenterLocation;
            NewMember.StaminaLevel = FMath::RandRange(80.0f, 100.0f);
            NewMember.bIsAlpha = (i == 0); // First spawned is alpha
            NewMember.LastFeedTime = 0.0f;
            
            HerdMembers.Add(NewMember);
            
            UE_LOG(LogTemp, Log, TEXT("Crowd_HerdManager: Spawned herd member %d at %s"), i, *SpawnLocation.ToString());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_HerdManager: Spawned herd of %d dinosaurs at %s"), HerdMembers.Num(), *CenterLocation.ToString());
}

void UCrowd_HerdManager::AddHerdMember(APawn* NewMember)
{
    if (!NewMember || HerdMembers.Num() >= MaxHerdSize)
        return;
    
    FCrowd_HerdMember Member;
    Member.DinosaurPawn = NewMember;
    Member.LocalPosition = NewMember->GetActorLocation() - GetHerdCenter();
    Member.StaminaLevel = 100.0f;
    Member.bIsAlpha = false;
    Member.LastFeedTime = 0.0f;
    
    HerdMembers.Add(Member);
}

void UCrowd_HerdManager::RemoveHerdMember(APawn* MemberToRemove)
{
    for (int32 i = HerdMembers.Num() - 1; i >= 0; i--)
    {
        if (HerdMembers[i].DinosaurPawn == MemberToRemove)
        {
            HerdMembers.RemoveAt(i);
            break;
        }
    }
}

void UCrowd_HerdManager::SetHerdState(ECrowd_HerdState NewState)
{
    if (BehaviorState.CurrentState != NewState)
    {
        BehaviorState.CurrentState = NewState;
        BehaviorState.StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Crowd_HerdManager: Herd state changed to %d"), (int32)NewState);
        
        // State-specific initialization
        switch (NewState)
        {
            case ECrowd_HerdState::Migrating:
                StartMigration();
                break;
            case ECrowd_HerdState::Fleeing:
                BehaviorState.CohesionRadius = HerdRadius * 0.5f; // Tighter formation when fleeing
                break;
            case ECrowd_HerdState::Grazing:
                BehaviorState.CohesionRadius = HerdRadius; // Normal spread for grazing
                break;
        }
    }
}

void UCrowd_HerdManager::UpdateHerdBehavior(float DeltaTime)
{
    BehaviorState.StateTimer += DeltaTime;
    
    switch (BehaviorState.CurrentState)
    {
        case ECrowd_HerdState::Grazing:
            ProcessGrazingBehavior();
            break;
        case ECrowd_HerdState::Migrating:
            ProcessMigrationBehavior();
            break;
        case ECrowd_HerdState::Fleeing:
            ProcessFleeingBehavior();
            break;
    }
}

void UCrowd_HerdManager::ProcessGrazingBehavior()
{
    // Check if it's time to migrate (seasonal behavior)
    if (SeasonalTimer > 300.0f) // 5 minutes of grazing triggers migration
    {
        SetHerdState(ECrowd_HerdState::Migrating);
        SeasonalTimer = 0.0f;
        return;
    }
    
    // Random movement during grazing
    if (BehaviorState.StateTimer > GrazingDuration)
    {
        FVector HerdCenter = GetHerdCenter();
        BehaviorState.TargetLocation = HerdCenter + FVector(
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(-1000.0f, 1000.0f),
            0.0f
        );
        BehaviorState.StateTimer = 0.0f;
    }
}

void UCrowd_HerdManager::ProcessMigrationBehavior()
{
    FVector HerdCenter = GetHerdCenter();
    float DistanceToTarget = FVector::Dist(HerdCenter, BehaviorState.TargetLocation);
    
    // Check if reached current waypoint
    if (DistanceToTarget < 500.0f)
    {
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % MigrationWaypoints.Num();
        BehaviorState.TargetLocation = GetNextMigrationTarget();
        
        // Chance to stop migrating and start grazing
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            SetHerdState(ECrowd_HerdState::Grazing);
            bIsMigrating = false;
        }
    }
}

void UCrowd_HerdManager::ProcessFleeingBehavior()
{
    // Return to grazing after fleeing for some time
    if (BehaviorState.StateTimer > 10.0f && NearbyThreats.Num() == 0)
    {
        SetHerdState(ECrowd_HerdState::Grazing);
    }
}

void UCrowd_HerdManager::DetectThreats()
{
    NearbyThreats.Empty();
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    FVector HerdCenter = GetHerdCenter();
    
    // Find player and other potential threats
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC && PC->GetPawn())
    {
        float DistanceToPlayer = FVector::Dist(HerdCenter, PC->GetPawn()->GetActorLocation());
        if (DistanceToPlayer < ThreatDetectionRange)
        {
            NearbyThreats.Add(PC->GetPawn());
            
            // Trigger fleeing behavior
            if (BehaviorState.CurrentState != ECrowd_HerdState::Fleeing)
            {
                SetHerdState(ECrowd_HerdState::Fleeing);
                BehaviorState.ThreatTarget = PC->GetPawn();
                
                // Set flee direction away from threat
                FVector FleeDirection = (HerdCenter - PC->GetPawn()->GetActorLocation()).GetSafeNormal();
                BehaviorState.TargetLocation = HerdCenter + FleeDirection * 2000.0f;
            }
        }
    }
}

FVector UCrowd_HerdManager::CalculateFlockingForce(const FCrowd_HerdMember& Member)
{
    if (!Member.DinosaurPawn)
        return FVector::ZeroVector;
    
    FVector Separation = CalculateSeparation(Member);
    FVector Alignment = CalculateAlignment(Member);
    FVector Cohesion = CalculateCohesion(Member);
    
    // Weight and combine forces
    FVector TotalForce = (Separation * SeparationWeight) + 
                        (Alignment * AlignmentWeight) + 
                        (Cohesion * CohesionWeight);
    
    return TotalForce.GetSafeNormal();
}

FVector UCrowd_HerdManager::CalculateSeparation(const FCrowd_HerdMember& Member)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    FVector MemberLocation = Member.DinosaurPawn->GetActorLocation();
    
    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (Other.DinosaurPawn == Member.DinosaurPawn || !Other.DinosaurPawn)
            continue;
        
        float Distance = FVector::Dist(MemberLocation, Other.DinosaurPawn->GetActorLocation());
        if (Distance < SeparationRadius && Distance > 0.0f)
        {
            FVector AwayVector = (MemberLocation - Other.DinosaurPawn->GetActorLocation()).GetSafeNormal();
            SeparationForce += AwayVector / Distance; // Closer neighbors have stronger influence
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
        return SeparationForce.GetSafeNormal();
    }
    
    return FVector::ZeroVector;
}

FVector UCrowd_HerdManager::CalculateAlignment(const FCrowd_HerdMember& Member)
{
    FVector AverageVelocity = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    FVector MemberLocation = Member.DinosaurPawn->GetActorLocation();
    
    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (Other.DinosaurPawn == Member.DinosaurPawn || !Other.DinosaurPawn)
            continue;
        
        float Distance = FVector::Dist(MemberLocation, Other.DinosaurPawn->GetActorLocation());
        if (Distance < AlignmentRadius)
        {
            AverageVelocity += Other.DinosaurPawn->GetVelocity();
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        AverageVelocity /= NeighborCount;
        return AverageVelocity.GetSafeNormal();
    }
    
    return FVector::ZeroVector;
}

FVector UCrowd_HerdManager::CalculateCohesion(const FCrowd_HerdMember& Member)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    FVector MemberLocation = Member.DinosaurPawn->GetActorLocation();
    
    for (const FCrowd_HerdMember& Other : HerdMembers)
    {
        if (Other.DinosaurPawn == Member.DinosaurPawn || !Other.DinosaurPawn)
            continue;
        
        float Distance = FVector::Dist(MemberLocation, Other.DinosaurPawn->GetActorLocation());
        if (Distance < BehaviorState.CohesionRadius)
        {
            CenterOfMass += Other.DinosaurPawn->GetActorLocation();
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        CenterOfMass /= NeighborCount;
        return (CenterOfMass - MemberLocation).GetSafeNormal();
    }
    
    return FVector::ZeroVector;
}

void UCrowd_HerdManager::StartMigration()
{
    bIsMigrating = true;
    BehaviorState.TargetLocation = GetNextMigrationTarget();
    UE_LOG(LogTemp, Log, TEXT("Crowd_HerdManager: Starting migration to %s"), *BehaviorState.TargetLocation.ToString());
}

void UCrowd_HerdManager::SetMigrationWaypoints(const TArray<FVector>& Waypoints)
{
    MigrationWaypoints = Waypoints;
    CurrentWaypointIndex = 0;
}

FVector UCrowd_HerdManager::GetNextMigrationTarget()
{
    if (MigrationWaypoints.Num() == 0)
        return GetHerdCenter();
    
    return MigrationWaypoints[CurrentWaypointIndex];
}

float UCrowd_HerdManager::GetHerdCohesion()
{
    if (HerdMembers.Num() < 2)
        return 1.0f;
    
    FVector HerdCenter = GetHerdCenter();
    float TotalDistance = 0.0f;
    int32 ValidMembers = 0;
    
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.DinosaurPawn)
        {
            TotalDistance += FVector::Dist(Member.DinosaurPawn->GetActorLocation(), HerdCenter);
            ValidMembers++;
        }
    }
    
    if (ValidMembers == 0)
        return 0.0f;
    
    float AverageDistance = TotalDistance / ValidMembers;
    return FMath::Clamp(1.0f - (AverageDistance / HerdRadius), 0.0f, 1.0f);
}

FVector UCrowd_HerdManager::GetHerdCenter()
{
    FVector Center = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.DinosaurPawn)
        {
            Center += Member.DinosaurPawn->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers > 0)
    {
        Center /= ValidMembers;
    }
    
    return Center;
}

APawn* UCrowd_HerdManager::GetAlphaMember()
{
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.bIsAlpha && Member.DinosaurPawn)
        {
            return Member.DinosaurPawn;
        }
    }
    return nullptr;
}

int32 UCrowd_HerdManager::GetActiveHerdSize()
{
    int32 ActiveCount = 0;
    for (const FCrowd_HerdMember& Member : HerdMembers)
    {
        if (Member.DinosaurPawn && IsValid(Member.DinosaurPawn))
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

void UCrowd_HerdManager::UpdateHerdMemberLOD(FCrowd_HerdMember& Member)
{
    if (!Member.DinosaurPawn)
        return;
    
    // Simple LOD based on distance to player
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC && PC->GetPawn())
    {
        float DistanceToPlayer = FVector::Dist(Member.DinosaurPawn->GetActorLocation(), PC->GetPawn()->GetActorLocation());
        
        // Adjust tick frequency based on distance
        if (DistanceToPlayer > LODDistance)
        {
            // Far away - reduce processing
            Member.DinosaurPawn->SetActorTickEnabled(false);
        }
        else
        {
            Member.DinosaurPawn->SetActorTickEnabled(true);
        }
    }
}

bool UCrowd_HerdManager::ShouldProcessMember(const FCrowd_HerdMember& Member)
{
    return Member.DinosaurPawn && IsValid(Member.DinosaurPawn);
}

void UCrowd_HerdManager::OptimizeHerdPerformance()
{
    // Remove invalid members
    for (int32 i = HerdMembers.Num() - 1; i >= 0; i--)
    {
        if (!HerdMembers[i].DinosaurPawn || !IsValid(HerdMembers[i].DinosaurPawn))
        {
            HerdMembers.RemoveAt(i);
        }
    }
}