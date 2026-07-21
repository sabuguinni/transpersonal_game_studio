#include "Combat_PackCoordinator.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

ACombat_PackCoordinator::ACombat_PackCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize pack settings
    MaxPackSize = 6;
    PackCohesionRadius = 1000.0f;
    CombatEngagementRange = 1500.0f;
    RetreatHealthThreshold = 0.3f;
    MinimumPackSizeForAttack = 2;
    CommunicationRange = 2000.0f;
    FormationUpdateInterval = 2.0f;
    
    // Initialize state
    bInCombat = false;
    CurrentTactic = ECombat_TacticState::Patrol;
    CurrentThreatLevel = ECombat_ThreatLevel::Low;
    LastFormationUpdate = 0.0f;
    LastTacticalEvaluation = 0.0f;
    
    // Initialize formation
    CurrentFormation.FormationType = ECombat_Formation::Circle;
    CurrentFormation.FormationRadius = 500.0f;
    
    // Setup tactical callouts
    TacticalCallouts.Add(TEXT("Alpha formation - surround target"));
    TacticalCallouts.Add(TEXT("Beta maneuver - flank left and right"));
    TacticalCallouts.Add(TEXT("Delta retreat - fall back to safe distance"));
    TacticalCallouts.Add(TEXT("Gamma strike - coordinated attack now"));
}

void ACombat_PackCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize formation positions
    UpdateFormationPositions();
    
    UE_LOG(LogTemp, Warning, TEXT("Pack Coordinator initialized - Max pack size: %d"), MaxPackSize);
}

void ACombat_PackCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    // Validate pack members (remove invalid ones)
    ValidatePackMembers();
    
    // Update pack cohesion
    UpdatePackCohesion(DeltaTime);
    
    // Update tactical state
    UpdateTacticalState(DeltaTime);
    
    // Update formation if needed
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFormationUpdate > FormationUpdateInterval)
    {
        UpdateFormationPositions();
        LastFormationUpdate = CurrentTime;
    }
    
    // Evaluate threat level periodically
    if (CurrentTime - LastTacticalEvaluation > 3.0f)
    {
        EvaluateThreatLevel();
        LastTacticalEvaluation = CurrentTime;
    }
}

void ACombat_PackCoordinator::AddPackMember(APawn* NewMember, ECombat_Role Role)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize)
    {
        return;
    }
    
    // Check if already in pack
    if (IsPackMember(NewMember))
    {
        return;
    }
    
    FCombat_PackMember PackMember;
    PackMember.Member = NewMember;
    PackMember.AssignedRole = Role;
    PackMember.bIsInPosition = false;
    PackMember.DistanceToTarget = 0.0f;
    
    PackMembers.Add(PackMember);
    
    // Set first member as leader if no leader exists
    if (!PackLeader.IsValid() && PackMembers.Num() == 1)
    {
        SetPackLeader(NewMember);
    }
    
    // Update formation with new member
    UpdateFormationPositions();
    
    UE_LOG(LogTemp, Warning, TEXT("Added pack member: %s (Role: %d)"), 
           *NewMember->GetName(), (int32)Role);
}

void ACombat_PackCoordinator::RemovePackMember(APawn* Member)
{
    if (!Member)
    {
        return;
    }
    
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (PackMembers[i].Member == Member)
        {
            PackMembers.RemoveAt(i);
            break;
        }
    }
    
    // If removed member was leader, assign new leader
    if (PackLeader == Member && PackMembers.Num() > 0)
    {
        SetPackLeader(PackMembers[0].Member.Get());
    }
    
    UpdateFormationPositions();
    
    UE_LOG(LogTemp, Warning, TEXT("Removed pack member: %s"), *Member->GetName());
}

void ACombat_PackCoordinator::SetPackLeader(APawn* NewLeader)
{
    if (!NewLeader || !IsPackMember(NewLeader))
    {
        return;
    }
    
    PackLeader = NewLeader;
    
    // Update leader's role
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (Member.Member == NewLeader)
        {
            Member.AssignedRole = ECombat_Role::Leader;
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("New pack leader assigned: %s"), *NewLeader->GetName());
}

void ACombat_PackCoordinator::SetPackTarget(APawn* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (NewTarget)
    {
        // Initiate combat if not already in combat
        if (!bInCombat)
        {
            InitiateCombat(NewTarget);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Pack target set: %s"), *NewTarget->GetName());
    }
}

bool ACombat_PackCoordinator::IsPackMember(APawn* Pawn) const
{
    if (!Pawn)
    {
        return false;
    }
    
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.Member == Pawn)
        {
            return true;
        }
    }
    
    return false;
}

void ACombat_PackCoordinator::SetFormation(ECombat_Formation Formation)
{
    CurrentFormation.FormationType = Formation;
    UpdateFormationPositions();
    
    UE_LOG(LogTemp, Warning, TEXT("Formation changed to: %d"), (int32)Formation);
}

void ACombat_PackCoordinator::UpdateFormationPositions()
{
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    FVector CenterPoint = CalculateFormationCenter();
    CurrentFormation.CenterPoint = CenterPoint;
    CurrentFormation.Positions.Empty();
    
    float AngleStep = 360.0f / FMath::Max(PackMembers.Num(), 1);
    
    switch (CurrentFormation.FormationType)
    {
        case ECombat_Formation::Circle:
        {
            for (int32 i = 0; i < PackMembers.Num(); i++)
            {
                float Angle = FMath::DegreesToRadians(AngleStep * i);
                FVector Position = CenterPoint + FVector(
                    FMath::Cos(Angle) * CurrentFormation.FormationRadius,
                    FMath::Sin(Angle) * CurrentFormation.FormationRadius,
                    0.0f
                );
                CurrentFormation.Positions.Add(Position);
                PackMembers[i].TargetPosition = Position;
            }
            break;
        }
        
        case ECombat_Formation::Line:
        {
            FVector LineDirection = FVector::ForwardVector;
            if (CurrentTarget.IsValid())
            {
                LineDirection = (CurrentTarget->GetActorLocation() - CenterPoint).GetSafeNormal();
            }
            
            FVector RightVector = FVector::CrossProduct(LineDirection, FVector::UpVector);
            float Spacing = 200.0f;
            
            for (int32 i = 0; i < PackMembers.Num(); i++)
            {
                float Offset = (i - PackMembers.Num() / 2.0f) * Spacing;
                FVector Position = CenterPoint + RightVector * Offset;
                CurrentFormation.Positions.Add(Position);
                PackMembers[i].TargetPosition = Position;
            }
            break;
        }
        
        case ECombat_Formation::Wedge:
        {
            // V-formation with leader at point
            for (int32 i = 0; i < PackMembers.Num(); i++)
            {
                FVector Position;
                if (i == 0) // Leader at front
                {
                    Position = CenterPoint;
                }
                else
                {
                    float Side = (i % 2 == 1) ? 1.0f : -1.0f;
                    float Distance = (i / 2 + 1) * 150.0f;
                    Position = CenterPoint + FVector(-Distance, Side * Distance, 0.0f);
                }
                CurrentFormation.Positions.Add(Position);
                PackMembers[i].TargetPosition = Position;
            }
            break;
        }
    }
}

FVector ACombat_PackCoordinator::GetAssignedPosition(APawn* Member) const
{
    for (const FCombat_PackMember& PackMember : PackMembers)
    {
        if (PackMember.Member == Member)
        {
            return PackMember.TargetPosition;
        }
    }
    
    return FVector::ZeroVector;
}

bool ACombat_PackCoordinator::IsPackInFormation() const
{
    int32 MembersInPosition = 0;
    
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.bIsInPosition)
        {
            MembersInPosition++;
        }
    }
    
    return MembersInPosition >= (PackMembers.Num() * 0.8f); // 80% in position
}

void ACombat_PackCoordinator::InitiateCombat(APawn* Target)
{
    if (!Target)
    {
        return;
    }
    
    bInCombat = true;
    CurrentTarget = Target;
    CurrentTactic = ECombat_TacticState::Engage;
    
    // Assign combat roles
    AssignCombatRoles();
    
    // Set aggressive formation
    SetFormation(ECombat_Formation::Circle);
    
    // Broadcast combat initiation
    BroadcastTacticalCommand(TEXT("Engage target! Execute combat formation!"));
    
    UE_LOG(LogTemp, Warning, TEXT("Combat initiated against: %s"), *Target->GetName());
}

void ACombat_PackCoordinator::ExecuteTactic(ECombat_TacticState Tactic)
{
    CurrentTactic = Tactic;
    
    switch (Tactic)
    {
        case ECombat_TacticState::Flank:
            SetFormation(ECombat_Formation::Line);
            BroadcastTacticalCommand(TacticalCallouts[1]); // Beta maneuver
            break;
            
        case ECombat_TacticState::Surround:
            SetFormation(ECombat_Formation::Circle);
            BroadcastTacticalCommand(TacticalCallouts[0]); // Alpha formation
            break;
            
        case ECombat_TacticState::Retreat:
            OrderRetreat();
            break;
            
        case ECombat_TacticState::Engage:
            SetFormation(ECombat_Formation::Wedge);
            BroadcastTacticalCommand(TacticalCallouts[3]); // Gamma strike
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Executing tactic: %d"), (int32)Tactic);
}

void ACombat_PackCoordinator::OrderRetreat()
{
    bInCombat = false;
    CurrentTactic = ECombat_TacticState::Retreat;
    CurrentTarget = nullptr;
    
    // Set defensive formation
    SetFormation(ECombat_Formation::Circle);
    
    // Broadcast retreat order
    BroadcastTacticalCommand(TacticalCallouts[2]); // Delta retreat
    
    UE_LOG(LogTemp, Warning, TEXT("Pack retreat ordered"));
}

void ACombat_PackCoordinator::AssignCombatRoles()
{
    if (PackMembers.Num() == 0)
    {
        return;
    }
    
    // Assign roles based on pack size and member capabilities
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i].Member == PackLeader)
        {
            PackMembers[i].AssignedRole = ECombat_Role::Leader;
        }
        else if (i < 2)
        {
            PackMembers[i].AssignedRole = ECombat_Role::Flanker;
        }
        else if (i < 4)
        {
            PackMembers[i].AssignedRole = ECombat_Role::Attacker;
        }
        else
        {
            PackMembers[i].AssignedRole = ECombat_Role::Support;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Combat roles assigned to %d pack members"), PackMembers.Num());
}

bool ACombat_PackCoordinator::ShouldEngageTarget(APawn* PotentialTarget) const
{
    if (!PotentialTarget || PackMembers.Num() < MinimumPackSizeForAttack)
    {
        return false;
    }
    
    // Check if target is within engagement range
    float DistanceToTarget = FVector::Dist(GetActorLocation(), PotentialTarget->GetActorLocation());
    if (DistanceToTarget > CombatEngagementRange)
    {
        return false;
    }
    
    // Additional logic for threat assessment could be added here
    
    return true;
}

void ACombat_PackCoordinator::BroadcastTacticalCommand(const FString& Command)
{
    UE_LOG(LogTemp, Warning, TEXT("Tactical Command: %s"), *Command);
    
    // In a full implementation, this would send commands to AI controllers
    // For now, we log the command for debugging
}

void ACombat_PackCoordinator::SendPackSignal(ECombat_PackSignal Signal)
{
    // Implementation for pack communication signals
    UE_LOG(LogTemp, Warning, TEXT("Pack signal sent: %d"), (int32)Signal);
}

void ACombat_PackCoordinator::UpdatePackCohesion(float DeltaTime)
{
    if (PackMembers.Num() <= 1)
    {
        return;
    }
    
    // Update member positions and distances
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (Member.Member.IsValid())
        {
            FVector MemberLocation = Member.Member->GetActorLocation();
            float DistanceToPosition = FVector::Dist(MemberLocation, Member.TargetPosition);
            Member.DistanceToTarget = DistanceToPosition;
            Member.bIsInPosition = (DistanceToPosition < 100.0f); // 1 meter tolerance
        }
    }
}

void ACombat_PackCoordinator::EvaluateThreatLevel()
{
    // Simple threat evaluation based on target proximity and pack health
    if (CurrentTarget.IsValid())
    {
        float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (DistanceToTarget < 500.0f)
        {
            CurrentThreatLevel = ECombat_ThreatLevel::High;
        }
        else if (DistanceToTarget < 1000.0f)
        {
            CurrentThreatLevel = ECombat_ThreatLevel::Medium;
        }
        else
        {
            CurrentThreatLevel = ECombat_ThreatLevel::Low;
        }
    }
    else
    {
        CurrentThreatLevel = ECombat_ThreatLevel::Low;
    }
}

void ACombat_PackCoordinator::UpdateTacticalState(float DeltaTime)
{
    if (!bInCombat)
    {
        return;
    }
    
    // Tactical decision making based on current situation
    switch (CurrentThreatLevel)
    {
        case ECombat_ThreatLevel::High:
            if (CurrentTactic != ECombat_TacticState::Engage && PackMembers.Num() >= MinimumPackSizeForAttack)
            {
                ExecuteTactic(ECombat_TacticState::Engage);
            }
            break;
            
        case ECombat_ThreatLevel::Medium:
            if (CurrentTactic == ECombat_TacticState::Patrol)
            {
                ExecuteTactic(ECombat_TacticState::Flank);
            }
            break;
            
        case ECombat_ThreatLevel::Low:
            if (bInCombat && CurrentTactic != ECombat_TacticState::Patrol)
            {
                CurrentTactic = ECombat_TacticState::Patrol;
            }
            break;
    }
}

FVector ACombat_PackCoordinator::CalculateFormationCenter() const
{
    if (PackLeader.IsValid())
    {
        return PackLeader->GetActorLocation();
    }
    
    if (PackMembers.Num() > 0 && PackMembers[0].Member.IsValid())
    {
        return PackMembers[0].Member->GetActorLocation();
    }
    
    return GetActorLocation();
}

void ACombat_PackCoordinator::ValidatePackMembers()
{
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (!PackMembers[i].Member.IsValid())
        {
            PackMembers.RemoveAt(i);
        }
    }
    
    // Validate leader
    if (!PackLeader.IsValid() && PackMembers.Num() > 0)
    {
        SetPackLeader(PackMembers[0].Member.Get());
    }
}