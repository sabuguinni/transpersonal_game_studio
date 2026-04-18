#include "CombatAIManager.h"
#include "../Characters/DinosaurPawn.h"
#include "../Characters/TranspersonalCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCombatAIManager::UCombatAIManager()
{
    NextGroupID = 1;
    MaxEngagementRange = 2000.0f;
    MinGroupCoordinationInterval = 2.0f;
    FlankingDistance = 800.0f;
    AmbushPositionRadius = 1200.0f;
}

void UCombatAIManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager initialized"));
    
    // Set up periodic group updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]()
            {
                UpdateAllGroups(GetWorld()->GetDeltaSeconds());
            },
            0.1f, // Update every 100ms
            true
        );
    }
}

void UCombatAIManager::Deinitialize()
{
    TacticalGroups.Empty();
    Super::Deinitialize();
}

int32 UCombatAIManager::CreateTacticalGroup(const TArray<ADinosaurPawn*>& Members, ADinosaurPawn* Alpha)
{
    if (Members.Num() == 0 || !Alpha)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create tactical group: invalid members or alpha"));
        return -1;
    }

    int32 GroupID = NextGroupID++;
    FCombat_TacticalGroup NewGroup;
    
    // Add members
    for (ADinosaurPawn* Member : Members)
    {
        if (Member)
        {
            NewGroup.Members.Add(Member);
        }
    }
    
    NewGroup.Alpha = Alpha;
    NewGroup.CurrentState = ECombat_TacticalState::Idle;
    NewGroup.FormationRadius = 500.0f;
    
    TacticalGroups.Add(GroupID, NewGroup);
    
    UE_LOG(LogTemp, Log, TEXT("Created tactical group %d with %d members"), GroupID, NewGroup.Members.Num());
    
    return GroupID;
}

void UCombatAIManager::DisbandTacticalGroup(int32 GroupID)
{
    if (TacticalGroups.Contains(GroupID))
    {
        TacticalGroups.Remove(GroupID);
        UE_LOG(LogTemp, Log, TEXT("Disbanded tactical group %d"), GroupID);
    }
}

void UCombatAIManager::AddMemberToGroup(int32 GroupID, ADinosaurPawn* NewMember)
{
    if (FCombat_TacticalGroup* Group = TacticalGroups.Find(GroupID))
    {
        if (NewMember && !Group->Members.Contains(NewMember))
        {
            Group->Members.Add(NewMember);
            UE_LOG(LogTemp, Log, TEXT("Added member to tactical group %d"), GroupID);
        }
    }
}

void UCombatAIManager::RemoveMemberFromGroup(int32 GroupID, ADinosaurPawn* Member)
{
    if (FCombat_TacticalGroup* Group = TacticalGroups.Find(GroupID))
    {
        Group->Members.RemoveAll([Member](const TWeakObjectPtr<ADinosaurPawn>& WeakPtr)
        {
            return WeakPtr.Get() == Member;
        });
        
        // If alpha was removed, assign new alpha
        if (Group->Alpha.Get() == Member && Group->Members.Num() > 0)
        {
            Group->Alpha = Group->Members[0];
        }
        
        // Disband if no members left
        if (Group->Members.Num() == 0)
        {
            DisbandTacticalGroup(GroupID);
        }
    }
}

void UCombatAIManager::SetGroupTarget(int32 GroupID, ATranspersonalCharacter* Target)
{
    if (FCombat_TacticalGroup* Group = TacticalGroups.Find(GroupID))
    {
        Group->CurrentTarget = Target;
        if (Target)
        {
            Group->TargetLocation = Target->GetActorLocation();
            UE_LOG(LogTemp, Log, TEXT("Set target for tactical group %d"), GroupID);
        }
    }
}

void UCombatAIManager::SetGroupState(int32 GroupID, ECombat_TacticalState NewState)
{
    if (FCombat_TacticalGroup* Group = TacticalGroups.Find(GroupID))
    {
        Group->CurrentState = NewState;
        Group->LastCoordinationTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("Set tactical group %d state to %d"), GroupID, (int32)NewState);
    }
}

void UCombatAIManager::CoordinateGroupAttack(int32 GroupID)
{
    FCombat_TacticalGroup* Group = TacticalGroups.Find(GroupID);
    if (!Group || !Group->CurrentTarget.IsValid())
    {
        return;
    }

    ATranspersonalCharacter* Target = Group->CurrentTarget.Get();
    if (!Target)
    {
        return;
    }

    // Assign roles and positions to each member
    for (int32 i = 0; i < Group->Members.Num(); ++i)
    {
        ADinosaurPawn* Member = Group->Members[i].Get();
        if (!Member)
        {
            continue;
        }

        ECombat_PackRole Role = AssignOptimalRole(Member, *Group);
        FVector TargetPosition = CalculateFlankingPosition(*Group, Target, Role);
        
        // Create tactical command
        FCombat_TacticalCommand Command;
        Command.CommandType = ECombat_TacticalState::Attacking;
        Command.TargetPosition = TargetPosition;
        Command.AssignedRole = Role;
        Command.Priority = (Role == ECombat_PackRole::Alpha) ? 10.0f : 5.0f;
        Command.ExecutionTime = GetWorld()->GetTimeSeconds() + (i * 0.5f); // Stagger attacks
        
        ExecuteGroupCommand(*Group, Command);
    }
    
    SetGroupState(GroupID, ECombat_TacticalState::Attacking);
}

void UCombatAIManager::ExecuteTacticalRetreat(int32 GroupID)
{
    FCombat_TacticalGroup* Group = TacticalGroups.Find(GroupID);
    if (!Group)
    {
        return;
    }

    // Calculate retreat positions away from target
    FVector RetreatCenter = FVector::ZeroVector;
    if (Group->CurrentTarget.IsValid())
    {
        FVector TargetLocation = Group->CurrentTarget->GetActorLocation();
        FVector GroupCenter = FVector::ZeroVector;
        
        // Calculate group center
        int32 ValidMembers = 0;
        for (const TWeakObjectPtr<ADinosaurPawn>& MemberPtr : Group->Members)
        {
            if (ADinosaurPawn* Member = MemberPtr.Get())
            {
                GroupCenter += Member->GetActorLocation();
                ValidMembers++;
            }
        }
        
        if (ValidMembers > 0)
        {
            GroupCenter /= ValidMembers;
            FVector RetreatDirection = (GroupCenter - TargetLocation).GetSafeNormal();
            RetreatCenter = GroupCenter + (RetreatDirection * 1500.0f);
        }
    }

    // Assign retreat positions
    for (int32 i = 0; i < Group->Members.Num(); ++i)
    {
        ADinosaurPawn* Member = Group->Members[i].Get();
        if (!Member)
        {
            continue;
        }

        FVector RetreatPosition = RetreatCenter + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        
        FCombat_TacticalCommand Command;
        Command.CommandType = ECombat_TacticalState::Retreating;
        Command.TargetPosition = RetreatPosition;
        Command.AssignedRole = ECombat_PackRole::Support;
        Command.Priority = 8.0f;
        Command.ExecutionTime = GetWorld()->GetTimeSeconds();
        
        ExecuteGroupCommand(*Group, Command);
    }
    
    SetGroupState(GroupID, ECombat_TacticalState::Retreating);
}

bool UCombatAIManager::ShouldEngageTarget(const FCombat_TacticalGroup& Group, ATranspersonalCharacter* PotentialTarget) const
{
    if (!PotentialTarget || Group.Members.Num() == 0)
    {
        return false;
    }

    // Calculate distance to target
    FVector GroupCenter = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (const TWeakObjectPtr<ADinosaurPawn>& MemberPtr : Group.Members)
    {
        if (ADinosaurPawn* Member = MemberPtr.Get())
        {
            GroupCenter += Member->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers == 0)
    {
        return false;
    }
    
    GroupCenter /= ValidMembers;
    float DistanceToTarget = FVector::Dist(GroupCenter, PotentialTarget->GetActorLocation());
    
    // Check if target is within engagement range
    if (DistanceToTarget > MaxEngagementRange)
    {
        return false;
    }
    
    // Calculate threat levels
    float GroupThreat = CalculateGroupThreatLevel(Group);
    float TargetThreat = CalculateTargetThreatLevel(PotentialTarget);
    
    // Engage if group has advantage
    return GroupThreat > TargetThreat * 0.7f; // 70% confidence threshold
}

FVector UCombatAIManager::CalculateFlankingPosition(const FCombat_TacticalGroup& Group, ATranspersonalCharacter* Target, ECombat_PackRole Role) const
{
    if (!Target)
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    
    switch (Role)
    {
        case ECombat_PackRole::Alpha:
            // Alpha attacks from front
            return TargetLocation + (TargetForward * FlankingDistance);
            
        case ECombat_PackRole::Flanker:
            // Flanker attacks from side
            {
                FVector RightVector = Target->GetActorRightVector();
                float SideMultiplier = FMath::RandBool() ? 1.0f : -1.0f;
                return TargetLocation + (RightVector * FlankingDistance * SideMultiplier);
            }
            
        case ECombat_PackRole::Ambusher:
            // Ambusher attacks from behind
            return TargetLocation - (TargetForward * FlankingDistance);
            
        case ECombat_PackRole::Distractor:
            // Distractor stays at medium range
            {
                FVector RandomDirection = FMath::VRand();
                RandomDirection.Z = 0.0f;
                RandomDirection.Normalize();
                return TargetLocation + (RandomDirection * FlankingDistance * 0.7f);
            }
            
        default:
            // Support role - maintain distance
            {
                FVector RandomDirection = FMath::VRand();
                RandomDirection.Z = 0.0f;
                RandomDirection.Normalize();
                return TargetLocation + (RandomDirection * FlankingDistance * 1.2f);
            }
    }
}

ECombat_PackRole UCombatAIManager::AssignOptimalRole(ADinosaurPawn* Member, const FCombat_TacticalGroup& Group) const
{
    if (!Member)
    {
        return ECombat_PackRole::Support;
    }

    // Alpha is always the alpha
    if (Group.Alpha.Get() == Member)
    {
        return ECombat_PackRole::Alpha;
    }

    // Assign roles based on member capabilities and group needs
    // For now, use simple rotation
    int32 MemberIndex = Group.Members.IndexOfByPredicate([Member](const TWeakObjectPtr<ADinosaurPawn>& Ptr)
    {
        return Ptr.Get() == Member;
    });

    switch (MemberIndex % 4)
    {
        case 0: return ECombat_PackRole::Flanker;
        case 1: return ECombat_PackRole::Ambusher;
        case 2: return ECombat_PackRole::Distractor;
        default: return ECombat_PackRole::Support;
    }
}

FCombat_TacticalGroup* UCombatAIManager::GetTacticalGroup(int32 GroupID)
{
    return TacticalGroups.Find(GroupID);
}

TArray<int32> UCombatAIManager::GetActiveGroupIDs() const
{
    TArray<int32> GroupIDs;
    TacticalGroups.GetKeys(GroupIDs);
    return GroupIDs;
}

void UCombatAIManager::UpdateAllGroups(float DeltaTime)
{
    // Clean up invalid groups first
    CleanupInvalidGroups();
    
    // Update each group
    for (auto& GroupPair : TacticalGroups)
    {
        UpdateGroupCoordination(GroupPair.Value, DeltaTime);
    }
}

void UCombatAIManager::UpdateGroupCoordination(FCombat_TacticalGroup& Group, float DeltaTime)
{
    // Validate group integrity
    if (!ValidateGroupIntegrity(Group))
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if it's time for coordination update
    if (CurrentTime - Group.LastCoordinationTime < MinGroupCoordinationInterval)
    {
        return;
    }

    // Update based on current state
    switch (Group.CurrentState)
    {
        case ECombat_TacticalState::Hunting:
            // Look for targets
            if (!Group.CurrentTarget.IsValid())
            {
                // Find nearest player
                if (UWorld* World = GetWorld())
                {
                    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(
                        UGameplayStatics::GetPlayerCharacter(World, 0)
                    );
                    
                    if (Player && ShouldEngageTarget(Group, Player))
                    {
                        Group.CurrentTarget = Player;
                        Group.CurrentState = ECombat_TacticalState::Stalking;
                    }
                }
            }
            break;
            
        case ECombat_TacticalState::Stalking:
            // Check if target is still valid and in range
            if (Group.CurrentTarget.IsValid())
            {
                if (ShouldEngageTarget(Group, Group.CurrentTarget.Get()))
                {
                    Group.CurrentState = ECombat_TacticalState::Coordinating;
                }
            }
            else
            {
                Group.CurrentState = ECombat_TacticalState::Hunting;
            }
            break;
            
        case ECombat_TacticalState::Coordinating:
            // Prepare for attack
            Group.CurrentState = ECombat_TacticalState::Attacking;
            break;
    }
    
    Group.LastCoordinationTime = CurrentTime;
}

void UCombatAIManager::ExecuteGroupCommand(FCombat_TacticalGroup& Group, const FCombat_TacticalCommand& Command)
{
    // This would integrate with the AI controller to execute the command
    // For now, just log the command
    UE_LOG(LogTemp, Log, TEXT("Executing tactical command: State=%d, Position=(%f,%f,%f), Role=%d"),
        (int32)Command.CommandType,
        Command.TargetPosition.X, Command.TargetPosition.Y, Command.TargetPosition.Z,
        (int32)Command.AssignedRole
    );
}

bool UCombatAIManager::ValidateGroupIntegrity(FCombat_TacticalGroup& Group)
{
    // Remove invalid members
    Group.Members.RemoveAll([](const TWeakObjectPtr<ADinosaurPawn>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });
    
    // Check if alpha is still valid
    if (!Group.Alpha.IsValid() && Group.Members.Num() > 0)
    {
        Group.Alpha = Group.Members[0];
    }
    
    return Group.Members.Num() > 0;
}

void UCombatAIManager::CleanupInvalidGroups()
{
    TArray<int32> GroupsToRemove;
    
    for (auto& GroupPair : TacticalGroups)
    {
        if (!ValidateGroupIntegrity(GroupPair.Value))
        {
            GroupsToRemove.Add(GroupPair.Key);
        }
    }
    
    for (int32 GroupID : GroupsToRemove)
    {
        TacticalGroups.Remove(GroupID);
        UE_LOG(LogTemp, Log, TEXT("Removed invalid tactical group %d"), GroupID);
    }
}

float UCombatAIManager::CalculateGroupThreatLevel(const FCombat_TacticalGroup& Group) const
{
    float ThreatLevel = 0.0f;
    
    for (const TWeakObjectPtr<ADinosaurPawn>& MemberPtr : Group.Members)
    {
        if (ADinosaurPawn* Member = MemberPtr.Get())
        {
            // Base threat per member
            ThreatLevel += 10.0f;
            
            // Bonus for pack coordination
            ThreatLevel += Group.Members.Num() * 2.0f;
        }
    }
    
    return ThreatLevel;
}

float UCombatAIManager::CalculateTargetThreatLevel(ATranspersonalCharacter* Target) const
{
    if (!Target)
    {
        return 0.0f;
    }
    
    // Base player threat
    float ThreatLevel = 15.0f;
    
    // Add weapon bonuses, health, etc. when those systems exist
    
    return ThreatLevel;
}

bool UCombatAIManager::IsPositionStrategic(const FVector& Position, const FCombat_TacticalGroup& Group, ATranspersonalCharacter* Target) const
{
    if (!Target)
    {
        return false;
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    float DistanceToTarget = FVector::Dist(Position, TargetLocation);
    
    // Position is strategic if it's within optimal range
    return DistanceToTarget >= FlankingDistance * 0.5f && DistanceToTarget <= FlankingDistance * 1.5f;
}