#include "NPC_PackHuntingSystem.h"
#include "DinosaurBase.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_PackHuntingSystem::UNPC_PackHuntingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for AI updates
    
    // Initialize pack settings
    bIsPackLeader = false;
    PackLeaderRef = nullptr;
    CurrentPackState = ENPC_PackState::Idle;
    MaxPackSize = 6;
    
    // Initialize hunting settings
    HuntingRange = 3000.0f;
    CoordinationRadius = 1500.0f;
    
    // Initialize behavior parameters
    PackCohesionStrength = 0.7f;
    HuntingAggression = 0.8f;
    RetreatThreshold = 0.3f;
    CommunicationRange = 2000.0f;
    
    // Initialize timers
    LastTargetScanTime = 0.0f;
    LastCohesionUpdateTime = 0.0f;
    LastCommunicationTime = 0.0f;
}

void UNPC_PackHuntingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize pack member array
    PackMembers.Reserve(MaxPackSize);
    PotentialTargets.Reserve(10);
    
    // Set initial state
    SetPackState(ENPC_PackState::Idle);
}

void UNPC_PackHuntingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetWorld())
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update pack cohesion
    if (CurrentTime - LastCohesionUpdateTime >= COHESION_UPDATE_INTERVAL)
    {
        UpdatePackCohesion();
        LastCohesionUpdateTime = CurrentTime;
    }
    
    // Scan for targets
    if (CurrentTime - LastTargetScanTime >= TARGET_SCAN_INTERVAL)
    {
        ScanForTargets();
        LastTargetScanTime = CurrentTime;
    }
    
    // Update pack member status
    CheckPackMemberStatus();
    
    // Execute behavior based on current state
    switch (CurrentPackState)
    {
        case ENPC_PackState::Hunting:
            if (CurrentTarget.TargetActor.IsValid())
            {
                AssignHuntingPositions();
            }
            else
            {
                SetPackState(ENPC_PackState::Idle);
            }
            break;
            
        case ENPC_PackState::Pursuing:
            if (CurrentTarget.TargetActor.IsValid())
            {
                UpdatePackMemberPositions();
                
                // Check if close enough to attack
                if (PackLeaderRef.IsValid())
                {
                    float DistanceToTarget = FVector::Dist(
                        PackLeaderRef->GetActorLocation(),
                        CurrentTarget.TargetActor->GetActorLocation()
                    );
                    
                    if (DistanceToTarget < 500.0f) // Attack range
                    {
                        SetPackState(ENPC_PackState::Attacking);
                    }
                }
            }
            else
            {
                SetPackState(ENPC_PackState::Idle);
            }
            break;
            
        case ENPC_PackState::Attacking:
            ExecuteCoordinatedAttack();
            break;
            
        case ENPC_PackState::Retreating:
            RetreatFromDanger();
            break;
            
        case ENPC_PackState::Regrouping:
            UpdatePackCohesion();
            
            // Check if regrouped
            bool bAllMembersClose = true;
            if (PackLeaderRef.IsValid())
            {
                for (const FNPC_PackMember& Member : PackMembers)
                {
                    if (Member.DinosaurRef.IsValid())
                    {
                        float Distance = FVector::Dist(
                            PackLeaderRef->GetActorLocation(),
                            Member.DinosaurRef->GetActorLocation()
                        );
                        
                        if (Distance > CoordinationRadius * 0.5f)
                        {
                            bAllMembersClose = false;
                            break;
                        }
                    }
                }
            }
            
            if (bAllMembersClose)
            {
                SetPackState(ENPC_PackState::Idle);
            }
            break;
            
        default:
            // Idle state - look for targets
            if (PotentialTargets.Num() > 0)
            {
                StartHunt(PotentialTargets[0].TargetActor.Get());
            }
            break;
    }
    
    // Clean up invalid targets
    CleanupInvalidTargets();
}

void UNPC_PackHuntingSystem::InitializePack(ADinosaurBase* PackLeader)
{
    if (!PackLeader)
        return;
    
    bIsPackLeader = (GetOwner() == PackLeader);
    PackLeaderRef = PackLeader;
    
    // Clear existing pack members
    PackMembers.Empty();
    
    // Add self to pack if not the leader
    if (!bIsPackLeader)
    {
        if (ADinosaurBase* SelfDinosaur = Cast<ADinosaurBase>(GetOwner()))
        {
            AddPackMember(SelfDinosaur, ENPC_PackRole::Hunter);
        }
    }
    
    SetPackState(ENPC_PackState::Idle);
}

void UNPC_PackHuntingSystem::AddPackMember(ADinosaurBase* NewMember, ENPC_PackRole Role)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize)
        return;
    
    // Check if already in pack
    for (const FNPC_PackMember& ExistingMember : PackMembers)
    {
        if (ExistingMember.DinosaurRef == NewMember)
            return;
    }
    
    // Create new pack member
    FNPC_PackMember NewPackMember;
    NewPackMember.DinosaurRef = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.bIsAlive = true;
    NewPackMember.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    PackMembers.Add(NewPackMember);
    
    // Notify the new member
    if (UNPC_PackHuntingSystem* MemberPackSystem = NewMember->FindComponentByClass<UNPC_PackHuntingSystem>())
    {
        if (PackLeaderRef.IsValid())
        {
            MemberPackSystem->PackLeaderRef = PackLeaderRef;
            MemberPackSystem->bIsPackLeader = false;
        }
    }
}

void UNPC_PackHuntingSystem::RemovePackMember(ADinosaurBase* Member)
{
    if (!Member)
        return;
    
    for (int32 i = PackMembers.Num() - 1; i >= 0; --i)
    {
        if (PackMembers[i].DinosaurRef == Member)
        {
            PackMembers.RemoveAt(i);
            break;
        }
    }
}

void UNPC_PackHuntingSystem::SetPackState(ENPC_PackState NewState)
{
    if (CurrentPackState == NewState)
        return;
    
    CurrentPackState = NewState;
    
    // Broadcast state change to pack members
    if (bIsPackLeader)
    {
        BroadcastPackSignal(FString::Printf(TEXT("StateChange_%d"), static_cast<int32>(NewState)));
    }
}

void UNPC_PackHuntingSystem::StartHunt(AActor* Target)
{
    if (!Target)
        return;
    
    // Set current target
    CurrentTarget.TargetActor = Target;
    CurrentTarget.LastKnownLocation = Target->GetActorLocation();
    CurrentTarget.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    CurrentTarget.bIsPlayer = Target->IsA<ACharacter>();
    
    // Evaluate threat level
    EvaluateTargetThreat(Target, CurrentTarget);
    
    // Change to hunting state
    SetPackState(ENPC_PackState::Hunting);
    
    // Assign hunting positions
    AssignHuntingPositions();
}

void UNPC_PackHuntingSystem::AssignHuntingPositions()
{
    if (!CurrentTarget.TargetActor.IsValid() || !PackLeaderRef.IsValid())
        return;
    
    FVector TargetLocation = CurrentTarget.TargetActor->GetActorLocation();
    
    // Assign positions based on pack roles
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (!PackMembers[i].DinosaurRef.IsValid())
            continue;
        
        FVector FlankingPosition = CalculateFlankingPosition(TargetLocation, i);
        
        // Send movement command to pack member
        // This would integrate with the AI behavior tree system
        if (UNPC_PackHuntingSystem* MemberPackSystem = PackMembers[i].DinosaurRef->FindComponentByClass<UNPC_PackHuntingSystem>())
        {
            // Store the assigned position for the member to use
            // In a full implementation, this would interface with the movement system
        }
    }
    
    // Transition to pursuing state
    SetPackState(ENPC_PackState::Pursuing);
}

void UNPC_PackHuntingSystem::ExecuteCoordinatedAttack()
{
    if (!CurrentTarget.TargetActor.IsValid())
        return;
    
    // Calculate attack coordination
    int32 AttackingMembers = 0;
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef.IsValid() && Member.bIsAlive)
        {
            float DistanceToTarget = FVector::Dist(
                Member.DinosaurRef->GetActorLocation(),
                CurrentTarget.TargetActor->GetActorLocation()
            );
            
            if (DistanceToTarget < 300.0f) // Attack range
            {
                AttackingMembers++;
                // Execute attack behavior
                // This would integrate with the combat system
            }
        }
    }
    
    // Check if target is defeated or if pack should retreat
    if (CurrentTarget.ThreatLevel > HuntingAggression * PackMembers.Num())
    {
        SetPackState(ENPC_PackState::Retreating);
    }
}

void UNPC_PackHuntingSystem::RetreatFromDanger()
{
    // Calculate retreat direction
    FVector RetreatDirection = FVector::ZeroVector;
    
    if (CurrentTarget.TargetActor.IsValid() && PackLeaderRef.IsValid())
    {
        RetreatDirection = PackLeaderRef->GetActorLocation() - CurrentTarget.TargetActor->GetActorLocation();
        RetreatDirection.Normalize();
    }
    
    // Move pack members to safety
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef.IsValid())
        {
            FVector SafePosition = Member.DinosaurRef->GetActorLocation() + (RetreatDirection * 1000.0f);
            // Send retreat command to member
        }
    }
    
    // Clear current target
    CurrentTarget = FNPC_HuntTarget();
    
    // Transition to regrouping
    SetPackState(ENPC_PackState::Regrouping);
}

void UNPC_PackHuntingSystem::BroadcastPackSignal(const FString& SignalType)
{
    if (!bIsPackLeader)
        return;
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef.IsValid())
        {
            if (UNPC_PackHuntingSystem* MemberPackSystem = Member.DinosaurRef->FindComponentByClass<UNPC_PackHuntingSystem>())
            {
                MemberPackSystem->RespondToPackSignal(SignalType, PackLeaderRef.Get());
            }
        }
    }
    
    LastCommunicationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UNPC_PackHuntingSystem::RespondToPackSignal(const FString& SignalType, ADinosaurBase* Sender)
{
    if (!Sender || bIsPackLeader)
        return;
    
    // Parse signal type and respond accordingly
    if (SignalType.StartsWith(TEXT("StateChange_")))
    {
        FString StateString = SignalType.RightChop(12); // Remove "StateChange_"
        int32 StateValue = FCString::Atoi(*StateString);
        CurrentPackState = static_cast<ENPC_PackState>(StateValue);
    }
}

void UNPC_PackHuntingSystem::UpdatePackCohesion()
{
    if (!PackLeaderRef.IsValid() || PackMembers.Num() == 0)
        return;
    
    FVector LeaderLocation = PackLeaderRef->GetActorLocation();
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef.IsValid())
        {
            Member.DistanceFromLeader = FVector::Dist(
                LeaderLocation,
                Member.DinosaurRef->GetActorLocation()
            );
            
            // Apply cohesion force if too far
            if (Member.DistanceFromLeader > CoordinationRadius)
            {
                // Calculate desired position closer to leader
                FVector ToLeader = LeaderLocation - Member.DinosaurRef->GetActorLocation();
                ToLeader.Normalize();
                
                FVector DesiredPosition = LeaderLocation - (ToLeader * CoordinationRadius * 0.7f);
                // Apply movement towards desired position
            }
        }
    }
}

void UNPC_PackHuntingSystem::ScanForTargets()
{
    if (!GetWorld())
        return;
    
    // Clear old potential targets
    PotentialTargets.Empty();
    
    // Get all actors in range
    TArray<AActor*> OverlappingActors;
    FVector ScanCenter = GetOwner()->GetActorLocation();
    
    // Simple sphere scan for potential targets
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == GetOwner())
            continue;
        
        float Distance = FVector::Dist(ScanCenter, Actor->GetActorLocation());
        if (Distance <= HuntingRange)
        {
            // Check if it's a valid target (player, other dinosaurs, etc.)
            if (Actor->IsA<ACharacter>() || Actor->IsA<ADinosaurBase>())
            {
                FNPC_HuntTarget NewTarget;
                NewTarget.TargetActor = Actor;
                NewTarget.LastKnownLocation = Actor->GetActorLocation();
                NewTarget.LastSeenTime = GetWorld()->GetTimeSeconds();
                NewTarget.bIsPlayer = Actor->IsA<ACharacter>();
                
                EvaluateTargetThreat(Actor, NewTarget);
                PotentialTargets.Add(NewTarget);
            }
        }
    }
    
    // Sort targets by threat level and distance
    PotentialTargets.Sort([this](const FNPC_HuntTarget& A, const FNPC_HuntTarget& B)
    {
        float DistanceA = FVector::Dist(GetOwner()->GetActorLocation(), A.LastKnownLocation);
        float DistanceB = FVector::Dist(GetOwner()->GetActorLocation(), B.LastKnownLocation);
        
        // Prioritize closer targets with higher threat
        float ScoreA = A.ThreatLevel / (DistanceA + 1.0f);
        float ScoreB = B.ThreatLevel / (DistanceB + 1.0f);
        
        return ScoreA > ScoreB;
    });
}

void UNPC_PackHuntingSystem::EvaluateTargetThreat(AActor* Target, FNPC_HuntTarget& HuntTarget)
{
    if (!Target)
        return;
    
    float ThreatLevel = 0.5f; // Base threat
    
    // Player characters are high priority
    if (Target->IsA<ACharacter>())
    {
        ThreatLevel += 0.4f;
    }
    
    // Large dinosaurs are dangerous
    if (ADinosaurBase* TargetDinosaur = Cast<ADinosaurBase>(Target))
    {
        // This would check dinosaur size/type in a full implementation
        ThreatLevel += 0.3f;
    }
    
    // Distance factor
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / HuntingRange), 0.0f, 1.0f);
    ThreatLevel *= DistanceFactor;
    
    HuntTarget.ThreatLevel = ThreatLevel;
}

void UNPC_PackHuntingSystem::UpdatePackMemberPositions()
{
    // Update positions during pursuit/attack phases
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef.IsValid())
        {
            // Calculate optimal position based on role and current situation
            FVector OptimalPosition = CalculateFlankingPosition(
                CurrentTarget.LastKnownLocation,
                &Member - &PackMembers[0]
            );
            
            // Apply movement towards optimal position
            // This would integrate with the movement/AI system
        }
    }
}

void UNPC_PackHuntingSystem::CheckPackMemberStatus()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef.IsValid())
        {
            // Update last seen time
            Member.LastSeenTime = CurrentTime;
            
            // Check if member is still alive
            // This would check health/status in a full implementation
            Member.bIsAlive = true;
        }
        else
        {
            // Mark as not alive if reference is invalid
            Member.bIsAlive = false;
        }
    }
    
    // Remove dead members
    PackMembers.RemoveAll([](const FNPC_PackMember& Member)
    {
        return !Member.bIsAlive || !Member.DinosaurRef.IsValid();
    });
}

FVector UNPC_PackHuntingSystem::CalculateFlankingPosition(const FVector& TargetLocation, int32 MemberIndex)
{
    if (!PackLeaderRef.IsValid())
        return TargetLocation;
    
    FVector LeaderLocation = PackLeaderRef->GetActorLocation();
    FVector ToTarget = TargetLocation - LeaderLocation;
    ToTarget.Normalize();
    
    // Calculate flanking angle based on member index
    float FlankAngle = (MemberIndex % 2 == 0) ? 45.0f : -45.0f;
    FlankAngle += (MemberIndex / 2) * 30.0f; // Spread out multiple flankers
    
    // Rotate the direction vector
    FVector FlankDirection = ToTarget.RotateAngleAxis(FlankAngle, FVector::UpVector);
    
    // Calculate flanking position
    float FlankDistance = 800.0f + (MemberIndex * 200.0f);
    FVector FlankPosition = TargetLocation + (FlankDirection * FlankDistance);
    
    return FlankPosition;
}

bool UNPC_PackHuntingSystem::IsTargetWithinRange(AActor* Target) const
{
    if (!Target)
        return false;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= HuntingRange;
}

void UNPC_PackHuntingSystem::CleanupInvalidTargets()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Remove invalid or stale targets
    PotentialTargets.RemoveAll([CurrentTime](const FNPC_HuntTarget& Target)
    {
        return !Target.TargetActor.IsValid() || 
               (CurrentTime - Target.LastSeenTime) > 10.0f; // 10 second timeout
    });
    
    // Clear current target if invalid
    if (!CurrentTarget.TargetActor.IsValid() || 
        (CurrentTime - CurrentTarget.LastSeenTime) > 15.0f)
    {
        CurrentTarget = FNPC_HuntTarget();
    }
}