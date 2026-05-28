#include "NPC_PackCoordinationSystem.h"
#include "NPC_TribalNPCBase.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UNPC_PackCoordinationSystem::UNPC_PackCoordinationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxHuntingDistance = 2000.0f;
    AttackCoordinationDelay = 2.0f;
    PackCohesion = 1.0f;
    bIsHunting = false;
    LastSignalTime = 0.0f;
    CommunicationRange = 1500.0f;
    SignalCooldown = 3.0f;
}

void UNPC_PackCoordinationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize pack coordination
    PackCohesion = 1.0f;
    bIsHunting = false;
    
    UE_LOG(LogTemp, Log, TEXT("Pack Coordination System initialized"));
}

void UNPC_PackCoordinationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (PackMembers.Num() > 0)
    {
        UpdatePackCohesion();
        UpdateMemberPositions();
        
        // Update pack behavior every frame
        if (bIsHunting && CurrentTarget.IsValid())
        {
            // Check if target is still valid and within range
            float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistanceToTarget > MaxHuntingDistance)
            {
                // Target too far, abandon hunt
                bIsHunting = false;
                CurrentTarget = nullptr;
                SendPackSignal(ENPC_PackSignal::StandDown);
            }
        }
    }
}

void UNPC_PackCoordinationSystem::InitializeAsPack(TArray<ANPC_TribalNPCBase*> InPackMembers, ANPC_TribalNPCBase* InPackLeader)
{
    PackMembers.Empty();
    PackLeader = InPackLeader;
    
    // Add leader first
    if (InPackLeader)
    {
        FNPC_PackMember LeaderMember;
        LeaderMember.NPCRef = InPackLeader;
        LeaderMember.Role = ENPC_PackRole::Leader;
        LeaderMember.DistanceFromLeader = 0.0f;
        LeaderMember.bIsAlive = true;
        PackMembers.Add(LeaderMember);
    }
    
    // Add other members
    for (ANPC_TribalNPCBase* Member : InPackMembers)
    {
        if (Member && Member != InPackLeader)
        {
            AddPackMember(Member, ENPC_PackRole::Follower);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Pack initialized with %d members"), PackMembers.Num());
}

void UNPC_PackCoordinationSystem::AddPackMember(ANPC_TribalNPCBase* NewMember, ENPC_PackRole Role)
{
    if (!NewMember)
    {
        return;
    }
    
    // Check if member already exists
    for (const FNPC_PackMember& ExistingMember : PackMembers)
    {
        if (ExistingMember.NPCRef == NewMember)
        {
            return; // Already in pack
        }
    }
    
    FNPC_PackMember NewPackMember;
    NewPackMember.NPCRef = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.bIsAlive = true;
    NewPackMember.LastCommunicationTime = GetWorld()->GetTimeSeconds();
    
    if (PackLeader.IsValid())
    {
        NewPackMember.DistanceFromLeader = FVector::Dist(
            NewMember->GetActorLocation(),
            PackLeader->GetActorLocation()
        );
    }
    
    PackMembers.Add(NewPackMember);
    
    UE_LOG(LogTemp, Log, TEXT("Added pack member: %s"), *NewMember->GetName());
}

void UNPC_PackCoordinationSystem::RemovePackMember(ANPC_TribalNPCBase* MemberToRemove)
{
    if (!MemberToRemove)
    {
        return;
    }
    
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (PackMembers[i].NPCRef == MemberToRemove)
        {
            PackMembers.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Removed pack member: %s"), *MemberToRemove->GetName());
            
            // If this was the leader, elect new one
            if (PackLeader == MemberToRemove)
            {
                ElectNewLeader();
            }
            break;
        }
    }
}

bool UNPC_PackCoordinationSystem::IsPackLeader() const
{
    return PackLeader.IsValid() && PackLeader.Get() == GetOwner();
}

ANPC_TribalNPCBase* UNPC_PackCoordinationSystem::GetPackLeader() const
{
    return PackLeader.Get();
}

void UNPC_PackCoordinationSystem::StartPackHunt(AActor* Target)
{
    if (!Target || !IsPackLeader())
    {
        return;
    }
    
    CurrentTarget = Target;
    bIsHunting = true;
    
    // Determine hunting strategy based on target and pack size
    ENPC_HuntingStrategy Strategy = ENPC_HuntingStrategy::Surround;
    if (PackMembers.Num() <= 2)
    {
        Strategy = ENPC_HuntingStrategy::Flank;
    }
    else if (PackMembers.Num() >= 5)
    {
        Strategy = ENPC_HuntingStrategy::Ambush;
    }
    
    UpdateHuntingFormation(Strategy);
    SendPackSignal(ENPC_PackSignal::StartHunt, Target->GetActorLocation());
    
    UE_LOG(LogTemp, Log, TEXT("Pack hunt started on target: %s"), *Target->GetName());
}

void UNPC_PackCoordinationSystem::UpdateHuntingFormation(ENPC_HuntingStrategy Strategy)
{
    CurrentFormation.Strategy = Strategy;
    CurrentFormation.MemberPositions.Empty();
    
    if (!CurrentTarget.IsValid())
    {
        return;
    }
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    
    switch (Strategy)
    {
        case ENPC_HuntingStrategy::Surround:
        {
            // Arrange members in a circle around target
            float AngleStep = 360.0f / FMath::Max(1, PackMembers.Num() - 1); // Exclude leader
            for (int32 i = 1; i < PackMembers.Num(); i++) // Skip leader at index 0
            {
                float Angle = AngleStep * (i - 1);
                FVector Position = TargetLocation + FVector(
                    FMath::Cos(FMath::DegreesToRadians(Angle)) * CurrentFormation.FormationRadius,
                    FMath::Sin(FMath::DegreesToRadians(Angle)) * CurrentFormation.FormationRadius,
                    0.0f
                );
                CurrentFormation.MemberPositions.Add(Position);
            }
            break;
        }
        
        case ENPC_HuntingStrategy::Flank:
        {
            // Position members on sides of target
            if (PackMembers.Num() > 1)
            {
                FVector LeftFlank = TargetLocation + FVector(0, -CurrentFormation.FormationRadius, 0);
                FVector RightFlank = TargetLocation + FVector(0, CurrentFormation.FormationRadius, 0);
                CurrentFormation.MemberPositions.Add(LeftFlank);
                if (PackMembers.Num() > 2)
                {
                    CurrentFormation.MemberPositions.Add(RightFlank);
                }
            }
            break;
        }
        
        case ENPC_HuntingStrategy::Ambush:
        {
            // Hide members in cover positions around target
            for (int32 i = 1; i < PackMembers.Num(); i++)
            {
                // Find cover positions (simplified - in real implementation would use cover system)
                float RandomAngle = FMath::RandRange(0.0f, 360.0f);
                float RandomDistance = FMath::RandRange(CurrentFormation.FormationRadius * 0.8f, CurrentFormation.FormationRadius * 1.2f);
                FVector Position = TargetLocation + FVector(
                    FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
                    FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
                    0.0f
                );
                CurrentFormation.MemberPositions.Add(Position);
            }
            break;
        }
    }
}

FVector UNPC_PackCoordinationSystem::GetOptimalPositionForMember(ANPC_TribalNPCBase* Member) const
{
    if (!Member)
    {
        return FVector::ZeroVector;
    }
    
    // Find member index
    int32 MemberIndex = -1;
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i].NPCRef == Member)
        {
            MemberIndex = i;
            break;
        }
    }
    
    if (MemberIndex <= 0 || MemberIndex - 1 >= CurrentFormation.MemberPositions.Num())
    {
        // Leader or invalid member - return current position
        return Member->GetActorLocation();
    }
    
    return CurrentFormation.MemberPositions[MemberIndex - 1];
}

void UNPC_PackCoordinationSystem::CoordinateAttack()
{
    if (!IsPackLeader() || !bIsHunting || !CurrentTarget.IsValid())
    {
        return;
    }
    
    // Signal coordinated attack
    SendPackSignal(ENPC_PackSignal::Attack, CurrentTarget->GetActorLocation());
    
    UE_LOG(LogTemp, Log, TEXT("Coordinated attack initiated"));
}

void UNPC_PackCoordinationSystem::SendPackSignal(ENPC_PackSignal Signal, const FVector& TargetLocation)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastSignalTime < SignalCooldown)
    {
        return; // Still in cooldown
    }
    
    LastSignalTime = CurrentTime;
    BroadcastToPackMembers(Signal, TargetLocation);
    
    UE_LOG(LogTemp, Log, TEXT("Pack signal sent: %d"), (int32)Signal);
}

void UNPC_PackCoordinationSystem::ReceivePackSignal(ENPC_PackSignal Signal, const FVector& SignalLocation, ANPC_TribalNPCBase* Sender)
{
    if (!Sender || !IsWithinCommunicationRange(Sender))
    {
        return;
    }
    
    // Process the signal based on type
    switch (Signal)
    {
        case ENPC_PackSignal::StartHunt:
            if (!IsPackLeader())
            {
                bIsHunting = true;
                // Move to formation position
            }
            break;
            
        case ENPC_PackSignal::Attack:
            if (!IsPackLeader() && bIsHunting)
            {
                // Execute attack behavior
            }
            break;
            
        case ENPC_PackSignal::Retreat:
            bIsHunting = false;
            CurrentTarget = nullptr;
            break;
            
        case ENPC_PackSignal::StandDown:
            bIsHunting = false;
            CurrentTarget = nullptr;
            break;
    }
}

void UNPC_PackCoordinationSystem::UpdatePackCohesion()
{
    if (PackMembers.Num() <= 1)
    {
        PackCohesion = 1.0f;
        return;
    }
    
    CalculatePackCohesion();
}

void UNPC_PackCoordinationSystem::HandleMemberDeath(ANPC_TribalNPCBase* DeadMember)
{
    if (!DeadMember)
    {
        return;
    }
    
    // Mark member as dead
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.NPCRef == DeadMember)
        {
            Member.bIsAlive = false;
            break;
        }
    }
    
    // If leader died, elect new leader
    if (PackLeader == DeadMember)
    {
        ElectNewLeader();
    }
    
    // Adjust pack behavior based on losses
    if (GetPackSize() <= 2 && bIsHunting)
    {
        // Too few members, retreat
        SendPackSignal(ENPC_PackSignal::Retreat);
    }
}

void UNPC_PackCoordinationSystem::ElectNewLeader()
{
    ANPC_TribalNPCBase* NewLeader = nullptr;
    
    // Find the strongest living member to be new leader
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.NPCRef.IsValid())
        {
            if (!NewLeader || Member.Role == ENPC_PackRole::Alpha)
            {
                NewLeader = Member.NPCRef.Get();
            }
        }
    }
    
    if (NewLeader)
    {
        PackLeader = NewLeader;
        
        // Update the new leader's role
        for (FNPC_PackMember& Member : PackMembers)
        {
            if (Member.NPCRef == NewLeader)
            {
                Member.Role = ENPC_PackRole::Leader;
                break;
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("New pack leader elected: %s"), *NewLeader->GetName());
    }
}

void UNPC_PackCoordinationSystem::CalculatePackCohesion()
{
    if (!PackLeader.IsValid() || PackMembers.Num() <= 1)
    {
        PackCohesion = 1.0f;
        return;
    }
    
    FVector LeaderLocation = PackLeader->GetActorLocation();
    float TotalDistance = 0.0f;
    int32 LivingMembers = 0;
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.NPCRef.IsValid())
        {
            float Distance = FVector::Dist(LeaderLocation, Member.NPCRef->GetActorLocation());
            TotalDistance += Distance;
            LivingMembers++;
        }
    }
    
    if (LivingMembers > 0)
    {
        float AverageDistance = TotalDistance / LivingMembers;
        // Cohesion decreases as average distance increases
        PackCohesion = FMath::Clamp(1.0f - (AverageDistance / 1000.0f), 0.1f, 1.0f);
    }
}

void UNPC_PackCoordinationSystem::UpdateMemberPositions()
{
    if (!PackLeader.IsValid())
    {
        return;
    }
    
    FVector LeaderLocation = PackLeader->GetActorLocation();
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.NPCRef.IsValid())
        {
            Member.DistanceFromLeader = FVector::Dist(LeaderLocation, Member.NPCRef->GetActorLocation());
        }
    }
}

FVector UNPC_PackCoordinationSystem::CalculateFormationPosition(const FNPC_PackMember& Member) const
{
    // This would calculate the ideal position for a member based on formation
    // For now, return a simple offset from leader
    if (PackLeader.IsValid())
    {
        FVector LeaderLocation = PackLeader->GetActorLocation();
        return LeaderLocation + FVector(FMath::RandRange(-200.0f, 200.0f), FMath::RandRange(-200.0f, 200.0f), 0.0f);
    }
    
    return FVector::ZeroVector;
}

bool UNPC_PackCoordinationSystem::IsWithinCommunicationRange(const ANPC_TribalNPCBase* Member) const
{
    if (!Member || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Member->GetActorLocation());
    return Distance <= CommunicationRange;
}

void UNPC_PackCoordinationSystem::BroadcastToPackMembers(ENPC_PackSignal Signal, const FVector& Location)
{
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.bIsAlive && Member.NPCRef.IsValid() && IsWithinCommunicationRange(Member.NPCRef.Get()))
        {
            // In a real implementation, this would call the member's ReceivePackSignal method
            // For now, just log the broadcast
            UE_LOG(LogTemp, Log, TEXT("Broadcasting signal %d to member %s"), (int32)Signal, *Member.NPCRef->GetName());
        }
    }
}