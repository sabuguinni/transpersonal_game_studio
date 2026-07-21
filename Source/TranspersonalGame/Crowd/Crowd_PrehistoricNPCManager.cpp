#include "Crowd_PrehistoricNPCManager.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

ACrowd_PrehistoricNPCManager::ACrowd_PrehistoricNPCManager()
{
    PrimaryActorTick.bCanEverTick = true;

    MaxNPCCount = 50;
    SpawnRadius = 2000.0f;
    UpdateInterval = 0.5f;
    ActivityChangeChance = 0.1f;
    GroupFormationRadius = 300.0f;
    bEnableTribalBehavior = true;

    HumanMesh = nullptr;
    HumanMaterial = nullptr;
}

void ACrowd_PrehistoricNPCManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeNPCMeshes();

    // Spawn initial tribal groups
    SpawnTribalGroup(GetActorLocation() + FVector(500, 0, 0), 8);
    SpawnTribalGroup(GetActorLocation() + FVector(-500, 500, 0), 6);
    SpawnTribalGroup(GetActorLocation() + FVector(0, -500, 0), 10);

    // Start update timer
    GetWorldTimerManager().SetTimer(UpdateTimer, this, &ACrowd_PrehistoricNPCManager::UpdateNPCBehavior, UpdateInterval, true);

    UE_LOG(LogTemp, Warning, TEXT("Prehistoric NPC Manager initialized with %d tribal groups"), TribalGroups.Num());
}

void ACrowd_PrehistoricNPCManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update all tribal groups
    for (int32 i = 0; i < TribalGroups.Num(); i++)
    {
        UpdateTribalGroupBehavior(i, DeltaTime);
    }

    // Update visual representation
    UpdateNPCVisuals();
}

void ACrowd_PrehistoricNPCManager::SpawnTribalGroup(FVector CenterLocation, int32 GroupSize)
{
    FCrowd_TribalGroup NewGroup;
    NewGroup.GroupCenter = CenterLocation;
    NewGroup.GroupRadius = GroupFormationRadius;
    NewGroup.GroupBehavior = ECrowd_GroupBehavior::Gathering;

    for (int32 i = 0; i < GroupSize; i++)
    {
        FCrowd_PrehistoricHuman NewHuman;
        NewHuman.Location = GetRandomLocationInRadius(CenterLocation, GroupFormationRadius);
        NewHuman.Location.Z = CenterLocation.Z; // Keep on ground level
        NewHuman.Rotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        NewHuman.MovementSpeed = FMath::RandRange(100.0f, 200.0f);
        NewHuman.CurrentActivity = ECrowd_NPCActivity::Idle;

        NewGroup.Members.Add(NewHuman);

        // Create mesh component for this NPC
        UStaticMeshComponent* NPCMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(*FString::Printf(TEXT("NPCMesh_%d_%d"), TribalGroups.Num(), i)));
        if (NPCMesh)
        {
            NPCMesh->SetupAttachment(RootComponent);
            NPCMesh->SetWorldLocation(NewHuman.Location);
            NPCMesh->SetWorldRotation(NewHuman.Rotation);
            
            if (HumanMesh)
            {
                NPCMesh->SetStaticMesh(HumanMesh);
            }
            if (HumanMaterial)
            {
                NPCMesh->SetMaterial(0, HumanMaterial);
            }

            NPCMeshComponents.Add(NPCMesh);
        }
    }

    TribalGroups.Add(NewGroup);
    UE_LOG(LogTemp, Warning, TEXT("Spawned tribal group with %d members at location %s"), GroupSize, *CenterLocation.ToString());
}

void ACrowd_PrehistoricNPCManager::UpdateNPCBehavior()
{
    for (int32 GroupIndex = 0; GroupIndex < TribalGroups.Num(); GroupIndex++)
    {
        FCrowd_TribalGroup& Group = TribalGroups[GroupIndex];
        
        // Random chance to change group behavior
        if (FMath::RandRange(0.0f, 1.0f) < ActivityChangeChance)
        {
            int32 BehaviorChoice = FMath::RandRange(0, 2);
            switch (BehaviorChoice)
            {
                case 0:
                    Group.GroupBehavior = ECrowd_GroupBehavior::Gathering;
                    StartGatheringBehavior(GroupIndex, Group.GroupCenter + GetRandomLocationInRadius(FVector::ZeroVector, 200.0f));
                    break;
                case 1:
                    Group.GroupBehavior = ECrowd_GroupBehavior::Hunting;
                    StartHuntingBehavior(GroupIndex, Group.GroupCenter + GetRandomLocationInRadius(FVector::ZeroVector, 500.0f));
                    break;
                case 2:
                    Group.GroupBehavior = ECrowd_GroupBehavior::Migrating;
                    Group.GroupCenter = GetRandomLocationInRadius(GetActorLocation(), SpawnRadius);
                    break;
            }
        }

        // Update individual NPC activities
        for (int32 i = 0; i < Group.Members.Num(); i++)
        {
            FCrowd_PrehistoricHuman& Human = Group.Members[i];
            
            if (FMath::RandRange(0.0f, 1.0f) < ActivityChangeChance * 0.5f)
            {
                int32 ActivityChoice = FMath::RandRange(0, 4);
                Human.CurrentActivity = static_cast<ECrowd_NPCActivity>(ActivityChoice);
                
                if (Human.CurrentActivity == ECrowd_NPCActivity::Walking)
                {
                    Human.TargetLocation = GetRandomLocationInRadius(Human.Location, 300.0f);
                    Human.bIsMoving = true;
                }
            }
        }
    }
}

void ACrowd_PrehistoricNPCManager::UpdateTribalGroupBehavior(int32 GroupIndex, float DeltaTime)
{
    if (GroupIndex >= TribalGroups.Num()) return;

    FCrowd_TribalGroup& Group = TribalGroups[GroupIndex];
    
    for (int32 i = 0; i < Group.Members.Num(); i++)
    {
        FCrowd_PrehistoricHuman& Human = Group.Members[i];
        UpdateNPCMovement(Human, DeltaTime);
        
        // Apply group cohesion if enabled
        if (bEnableTribalBehavior)
        {
            FVector FlockingForce = CalculateFlockingForce(Human, Group);
            Human.TargetLocation += FlockingForce * DeltaTime;
        }
    }

    // Update group center based on member positions
    if (Group.Members.Num() > 0)
    {
        FVector NewCenter = FVector::ZeroVector;
        for (const FCrowd_PrehistoricHuman& Human : Group.Members)
        {
            NewCenter += Human.Location;
        }
        Group.GroupCenter = NewCenter / Group.Members.Num();
    }
}

void ACrowd_PrehistoricNPCManager::UpdateNPCMovement(FCrowd_PrehistoricHuman& NPC, float DeltaTime)
{
    if (NPC.bIsMoving)
    {
        FVector Direction = (NPC.TargetLocation - NPC.Location).GetSafeNormal();
        FVector Movement = Direction * NPC.MovementSpeed * DeltaTime;
        
        NPC.Location += Movement;
        
        // Check if reached target
        if (FVector::Dist(NPC.Location, NPC.TargetLocation) < 50.0f)
        {
            NPC.bIsMoving = false;
            NPC.CurrentActivity = ECrowd_NPCActivity::Idle;
        }
        
        // Update rotation to face movement direction
        if (!Direction.IsZero())
        {
            NPC.Rotation = Direction.Rotation();
        }
    }
    
    // Update activity timer
    NPC.ActivityTimer += DeltaTime;
}

FVector ACrowd_PrehistoricNPCManager::CalculateFlockingForce(const FCrowd_PrehistoricHuman& NPC, const FCrowd_TribalGroup& Group)
{
    FVector CohesionForce = FVector::ZeroVector;
    FVector SeparationForce = FVector::ZeroVector;
    FVector AlignmentForce = FVector::ZeroVector;
    
    int32 NeighborCount = 0;
    
    for (const FCrowd_PrehistoricHuman& Other : Group.Members)
    {
        if (&Other == &NPC) continue;
        
        float Distance = FVector::Dist(NPC.Location, Other.Location);
        
        if (Distance < Group.GroupRadius)
        {
            // Cohesion - move towards group center
            CohesionForce += Other.Location;
            
            // Separation - avoid crowding
            if (Distance < Group.SeparationDistance && Distance > 0.1f)
            {
                FVector AwayVector = (NPC.Location - Other.Location).GetSafeNormal();
                SeparationForce += AwayVector / Distance; // Stronger when closer
            }
            
            // Alignment - match neighbor velocities
            if (Other.bIsMoving)
            {
                FVector OtherVelocity = (Other.TargetLocation - Other.Location).GetSafeNormal();
                AlignmentForce += OtherVelocity;
            }
            
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        CohesionForce = (CohesionForce / NeighborCount - NPC.Location).GetSafeNormal() * Group.CohesionStrength;
        AlignmentForce = (AlignmentForce / NeighborCount).GetSafeNormal();
    }
    
    return (CohesionForce + SeparationForce * 2.0f + AlignmentForce) * 100.0f;
}

void ACrowd_PrehistoricNPCManager::UpdateNPCVisuals()
{
    int32 MeshIndex = 0;
    
    for (const FCrowd_TribalGroup& Group : TribalGroups)
    {
        for (const FCrowd_PrehistoricHuman& Human : Group.Members)
        {
            if (MeshIndex < NPCMeshComponents.Num() && NPCMeshComponents[MeshIndex])
            {
                NPCMeshComponents[MeshIndex]->SetWorldLocation(Human.Location);
                NPCMeshComponents[MeshIndex]->SetWorldRotation(Human.Rotation);
            }
            MeshIndex++;
        }
    }
}

void ACrowd_PrehistoricNPCManager::InitializeNPCMeshes()
{
    // Try to load default primitive mesh for NPCs
    HumanMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
    HumanMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    
    if (!HumanMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not load human mesh, NPCs will be invisible"));
    }
}

FVector ACrowd_PrehistoricNPCManager::GetRandomLocationInRadius(FVector Center, float Radius)
{
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(0.0f, Radius);
    
    return Center + FVector(
        FMath::Cos(Angle) * Distance,
        FMath::Sin(Angle) * Distance,
        0.0f
    );
}

void ACrowd_PrehistoricNPCManager::MoveNPCToLocation(int32 NPCIndex, FVector TargetLocation)
{
    int32 CurrentIndex = 0;
    for (FCrowd_TribalGroup& Group : TribalGroups)
    {
        for (FCrowd_PrehistoricHuman& Human : Group.Members)
        {
            if (CurrentIndex == NPCIndex)
            {
                Human.TargetLocation = TargetLocation;
                Human.bIsMoving = true;
                Human.CurrentActivity = ECrowd_NPCActivity::Walking;
                return;
            }
            CurrentIndex++;
        }
    }
}

void ACrowd_PrehistoricNPCManager::SetNPCActivity(int32 NPCIndex, ECrowd_NPCActivity NewActivity)
{
    int32 CurrentIndex = 0;
    for (FCrowd_TribalGroup& Group : TribalGroups)
    {
        for (FCrowd_PrehistoricHuman& Human : Group.Members)
        {
            if (CurrentIndex == NPCIndex)
            {
                Human.CurrentActivity = NewActivity;
                return;
            }
            CurrentIndex++;
        }
    }
}

void ACrowd_PrehistoricNPCManager::UpdateGroupCohesion(int32 GroupIndex)
{
    if (GroupIndex >= TribalGroups.Num()) return;
    
    FCrowd_TribalGroup& Group = TribalGroups[GroupIndex];
    ApplyGroupBehavior(Group, GetWorld()->GetDeltaSeconds());
}

void ACrowd_PrehistoricNPCManager::ApplyGroupBehavior(FCrowd_TribalGroup& Group, float DeltaTime)
{
    switch (Group.GroupBehavior)
    {
        case ECrowd_GroupBehavior::Gathering:
            // Move all members towards group center
            for (FCrowd_PrehistoricHuman& Human : Group.Members)
            {
                if (!Human.bIsMoving)
                {
                    Human.TargetLocation = Group.GroupCenter + GetRandomLocationInRadius(FVector::ZeroVector, 100.0f);
                    Human.bIsMoving = true;
                }
            }
            break;
            
        case ECrowd_GroupBehavior::Hunting:
            // Spread out in formation
            for (int32 i = 0; i < Group.Members.Num(); i++)
            {
                FCrowd_PrehistoricHuman& Human = Group.Members[i];
                if (!Human.bIsMoving)
                {
                    float Angle = (2.0f * PI * i) / Group.Members.Num();
                    FVector FormationPos = Group.GroupCenter + FVector(
                        FMath::Cos(Angle) * Group.GroupRadius * 0.7f,
                        FMath::Sin(Angle) * Group.GroupRadius * 0.7f,
                        0.0f
                    );
                    Human.TargetLocation = FormationPos;
                    Human.bIsMoving = true;
                }
            }
            break;
            
        case ECrowd_GroupBehavior::Migrating:
            // All move in same general direction
            for (FCrowd_PrehistoricHuman& Human : Group.Members)
            {
                if (!Human.bIsMoving)
                {
                    Human.TargetLocation = Group.GroupCenter + GetRandomLocationInRadius(FVector::ZeroVector, 200.0f);
                    Human.bIsMoving = true;
                }
            }
            break;
    }
}

void ACrowd_PrehistoricNPCManager::HandlePlayerProximity(FVector PlayerLocation, float ProximityRadius)
{
    for (FCrowd_TribalGroup& Group : TribalGroups)
    {
        float DistanceToGroup = FVector::Dist(PlayerLocation, Group.GroupCenter);
        
        if (DistanceToGroup < ProximityRadius)
        {
            // React to player presence
            for (FCrowd_PrehistoricHuman& Human : Group.Members)
            {
                float DistanceToPlayer = FVector::Dist(PlayerLocation, Human.Location);
                
                if (DistanceToPlayer < ProximityRadius * 0.5f)
                {
                    // Move away from player
                    FVector AwayDirection = (Human.Location - PlayerLocation).GetSafeNormal();
                    Human.TargetLocation = Human.Location + AwayDirection * 300.0f;
                    Human.bIsMoving = true;
                    Human.CurrentActivity = ECrowd_NPCActivity::Fleeing;
                }
            }
        }
    }
}

void ACrowd_PrehistoricNPCManager::StartGatheringBehavior(int32 GroupIndex, FVector GatherLocation)
{
    if (GroupIndex >= TribalGroups.Num()) return;
    
    FCrowd_TribalGroup& Group = TribalGroups[GroupIndex];
    Group.GroupBehavior = ECrowd_GroupBehavior::Gathering;
    Group.GroupCenter = GatherLocation;
    
    UE_LOG(LogTemp, Warning, TEXT("Group %d started gathering at %s"), GroupIndex, *GatherLocation.ToString());
}

void ACrowd_PrehistoricNPCManager::StartHuntingBehavior(int32 GroupIndex, FVector HuntTarget)
{
    if (GroupIndex >= TribalGroups.Num()) return;
    
    FCrowd_TribalGroup& Group = TribalGroups[GroupIndex];
    Group.GroupBehavior = ECrowd_GroupBehavior::Hunting;
    Group.GroupCenter = HuntTarget;
    
    UE_LOG(LogTemp, Warning, TEXT("Group %d started hunting towards %s"), GroupIndex, *HuntTarget.ToString());
}