#include "NPC_VelociraptorPackBehavior.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Engine/Engine.h"

UNPC_VelociraptorPackBehavior::UNPC_VelociraptorPackBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    CurrentPackState = ENPC_PackState::Hunting;
    CurrentTarget = nullptr;
    PackCenterPoint = FVector::ZeroVector;
    bIsPackLeader = false;
    StateTimer = 0.0f;
    LastCoordinationTime = 0.0f;
    LastTargetPosition = FVector::ZeroVector;
    bTargetLost = false;
}

void UNPC_VelociraptorPackBehavior::BeginPlay()
{
    Super::BeginPlay();
    InitializePackBehavior();
}

void UNPC_VelociraptorPackBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdatePackState(DeltaTime);
}

void UNPC_VelociraptorPackBehavior::InitializePackBehavior()
{
    if (!GetOwner())
        return;

    // Find nearby raptors to form pack
    FindNearbyRaptors();
    
    // Determine if this raptor should be pack leader
    if (PackMembers.Num() == 0 || PackMembers.Num() < 3)
    {
        bIsPackLeader = true;
    }
    
    CurrentPackState = ENPC_PackState::Hunting;
    StateTimer = 0.0f;
}

void UNPC_VelociraptorPackBehavior::UpdatePackState(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    switch (CurrentPackState)
    {
        case ENPC_PackState::Hunting:
            UpdateHuntingBehavior(DeltaTime);
            break;
        case ENPC_PackState::Coordinating:
            UpdateCoordinatingBehavior(DeltaTime);
            break;
        case ENPC_PackState::Attacking:
            UpdateAttackingBehavior(DeltaTime);
            break;
        case ENPC_PackState::Retreating:
            UpdateRetreatingBehavior(DeltaTime);
            break;
    }
}

void UNPC_VelociraptorPackBehavior::FindNearbyRaptors()
{
    if (!GetOwner())
        return;

    PackMembers.Empty();
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
            continue;
            
        APawn* Pawn = Cast<APawn>(Actor);
        if (!Pawn)
            continue;
            
        // Check if it's a raptor (simple name check for now)
        FString ActorName = Actor->GetName();
        if (ActorName.Contains("Raptor") || ActorName.Contains("Veloci"))
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= PackRadius)
            {
                FNPC_PackMember NewMember;
                NewMember.Raptor = Pawn;
                NewMember.LastKnownPosition = Pawn->GetActorLocation();
                NewMember.DistanceToTarget = 0.0f;
                NewMember.bIsAlpha = false;
                PackMembers.Add(NewMember);
                
                if (PackMembers.Num() >= MaxPackSize)
                    break;
            }
        }
    }
    
    // Set alpha raptor (first one found or this one if leader)
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        PackMembers[0].bIsAlpha = true;
    }
}

void UNPC_VelociraptorPackBehavior::CoordinatePackMovement()
{
    if (!bIsPackLeader || PackMembers.Num() == 0)
        return;
        
    // Calculate pack center point
    FVector TotalPosition = GetOwner()->GetActorLocation();
    int32 ValidMembers = 1;
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Raptor && IsValid(Member.Raptor))
        {
            TotalPosition += Member.Raptor->GetActorLocation();
            ValidMembers++;
        }
    }
    
    PackCenterPoint = TotalPosition / ValidMembers;
    LastCoordinationTime = GetWorld()->GetTimeSeconds();
}

void UNPC_VelociraptorPackBehavior::ExecutePackHunt()
{
    if (!CurrentTarget)
    {
        // Search for targets
        TArray<AActor*> PotentialTargets;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), PotentialTargets);
        
        for (AActor* Actor : PotentialTargets)
        {
            APawn* Pawn = Cast<APawn>(Actor);
            if (IsValidTarget(Pawn))
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                if (Distance <= HuntingRadius)
                {
                    CurrentTarget = Pawn;
                    LastTargetPosition = Pawn->GetActorLocation();
                    TransitionToState(ENPC_PackState::Coordinating);
                    break;
                }
            }
        }
    }
}

void UNPC_VelociraptorPackBehavior::PerformCoordinatedAttack()
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        CurrentTarget = nullptr;
        TransitionToState(ENPC_PackState::Hunting);
        return;
    }
    
    // Calculate flanking positions for pack members
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i].Raptor && IsValid(PackMembers[i].Raptor))
        {
            FVector FlankPosition = CalculateFlankingPosition(CurrentTarget, i);
            
            // Move raptor to flanking position (simplified - would use AI controller in practice)
            AAIController* AIController = Cast<AAIController>(PackMembers[i].Raptor->GetController());
            if (AIController)
            {
                // In a real implementation, this would use proper AI movement
                // For now, just store the target position
                PackMembers[i].LastKnownPosition = FlankPosition;
            }
        }
    }
}

FVector UNPC_VelociraptorPackBehavior::CalculateFlankingPosition(APawn* Target, int32 RaptorIndex)
{
    if (!Target)
        return FVector::ZeroVector;
        
    FVector TargetLocation = Target->GetActorLocation();
    float AngleOffset = (360.0f / FMath::Max(PackMembers.Num(), 1)) * RaptorIndex;
    float RadianOffset = FMath::DegreesToRadians(AngleOffset);
    
    FVector FlankOffset = FVector(
        FMath::Cos(RadianOffset) * AttackDistance,
        FMath::Sin(RadianOffset) * AttackDistance,
        0.0f
    );
    
    return TargetLocation + FlankOffset;
}

bool UNPC_VelociraptorPackBehavior::IsValidTarget(APawn* PotentialTarget)
{
    if (!PotentialTarget || !IsValid(PotentialTarget))
        return false;
        
    // Don't target other raptors
    FString TargetName = PotentialTarget->GetName();
    if (TargetName.Contains("Raptor") || TargetName.Contains("Veloci"))
        return false;
        
    // Target players and other dinosaurs
    if (TargetName.Contains("Character") || TargetName.Contains("Player") || 
        TargetName.Contains("Trex") || TargetName.Contains("Tricer"))
    {
        return true;
    }
    
    return false;
}

void UNPC_VelociraptorPackBehavior::TransitionToState(ENPC_PackState NewState)
{
    if (CurrentPackState == NewState)
        return;
        
    CurrentPackState = NewState;
    StateTimer = 0.0f;
    
    switch (NewState)
    {
        case ENPC_PackState::Hunting:
            CurrentTarget = nullptr;
            bTargetLost = false;
            break;
        case ENPC_PackState::Coordinating:
            CoordinatePackMovement();
            break;
        case ENPC_PackState::Attacking:
            break;
        case ENPC_PackState::Retreating:
            CurrentTarget = nullptr;
            break;
    }
}

void UNPC_VelociraptorPackBehavior::UpdateHuntingBehavior(float DeltaTime)
{
    ExecutePackHunt();
    
    // Periodically update pack composition
    if (StateTimer > 5.0f)
    {
        FindNearbyRaptors();
        StateTimer = 0.0f;
    }
}

void UNPC_VelociraptorPackBehavior::UpdateCoordinatingBehavior(float DeltaTime)
{
    if (StateTimer > CoordinationDelay)
    {
        if (CurrentTarget && IsValid(CurrentTarget))
        {
            float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistanceToTarget <= AttackDistance * 1.5f)
            {
                TransitionToState(ENPC_PackState::Attacking);
            }
        }
        else
        {
            TransitionToState(ENPC_PackState::Hunting);
        }
    }
    
    CoordinatePackMovement();
}

void UNPC_VelociraptorPackBehavior::UpdateAttackingBehavior(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        TransitionToState(ENPC_PackState::Hunting);
        return;
    }
    
    PerformCoordinatedAttack();
    
    // Check if target is too far away
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistanceToTarget > HuntingRadius * 1.5f)
    {
        TransitionToState(ENPC_PackState::Retreating);
    }
}

void UNPC_VelociraptorPackBehavior::UpdateRetreatingBehavior(float DeltaTime)
{
    if (StateTimer > 3.0f)
    {
        TransitionToState(ENPC_PackState::Hunting);
    }
}