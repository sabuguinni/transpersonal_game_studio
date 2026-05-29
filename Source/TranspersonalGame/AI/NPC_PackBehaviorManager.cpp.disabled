#include "NPC_PackBehaviorManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

UNPC_PackBehaviorManager::UNPC_PackBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 times per second

    // Initialize pack settings
    PackCohesionRadius = 1500.0f;
    PackSeparationRadius = 300.0f;
    MaxPackSize = 8;
    MyRole = ENPC_PackRole::Follower;

    // Initialize hunting settings
    HuntingRange = 3000.0f;
    bIsActivelyHunting = false;
    CurrentStrategy = ENPC_HuntingStrategy::Ambush;

    // Initialize communication settings
    CommunicationRange = 2000.0f;

    // Initialize timing
    LastCommunicationTime = 0.0f;
    LastTargetUpdateTime = 0.0f;
    LastCohesionUpdateTime = 0.0f;
}

void UNPC_PackBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Try to find existing pack or become leader
    if (!PackLeader.IsValid())
    {
        // Look for nearby pack leaders
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
        
        float ClosestDistance = PackCohesionRadius;
        APawn* ClosestLeader = nullptr;
        
        for (AActor* Actor : FoundActors)
        {
            if (APawn* OtherPawn = Cast<APawn>(Actor))
            {
                if (OtherPawn != GetOwner())
                {
                    if (UNPC_PackBehaviorManager* OtherPackManager = OtherPawn->FindComponentByClass<UNPC_PackBehaviorManager>())
                    {
                        if (OtherPackManager->IsPackLeader() && OtherPackManager->PackMembers.Num() < MaxPackSize)
                        {
                            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), OtherPawn->GetActorLocation());
                            if (Distance < ClosestDistance)
                            {
                                ClosestDistance = Distance;
                                ClosestLeader = OtherPawn;
                            }
                        }
                    }
                }
            }
        }
        
        if (ClosestLeader)
        {
            JoinPack(ClosestLeader);
        }
        else
        {
            // Become pack leader if no suitable pack found
            MyRole = ENPC_PackRole::Leader;
            PackLeader = Cast<APawn>(GetOwner());
        }
    }
}

void UNPC_PackBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update pack cohesion every 0.5 seconds
    if (CurrentTime - LastCohesionUpdateTime > 0.5f)
    {
        UpdatePackCohesion(DeltaTime);
        LastCohesionUpdateTime = CurrentTime;
    }

    // Update hunting targets every 1 second
    if (CurrentTime - LastTargetUpdateTime > 1.0f)
    {
        UpdateHuntingTargets(DeltaTime);
        LastTargetUpdateTime = CurrentTime;
    }

    // Process communications every 0.2 seconds
    if (CurrentTime - LastCommunicationTime > 0.2f)
    {
        ProcessCommunications(DeltaTime);
        LastCommunicationTime = CurrentTime;
    }

    // Manage pack leadership
    ManagePackLeadership();
}

void UNPC_PackBehaviorManager::JoinPack(APawn* NewLeader)
{
    if (!NewLeader || NewLeader == GetOwner())
        return;

    if (UNPC_PackBehaviorManager* LeaderPackManager = NewLeader->FindComponentByClass<UNPC_PackBehaviorManager>())
    {
        if (LeaderPackManager->PackMembers.Num() < MaxPackSize)
        {
            PackLeader = NewLeader;
            MyRole = ENPC_PackRole::Follower;
            LeaderPackManager->AddPackMember(Cast<APawn>(GetOwner()), MyRole);
            
            // Clear our own pack members since we're joining another pack
            PackMembers.Empty();
        }
    }
}

void UNPC_PackBehaviorManager::LeavePack()
{
    if (PackLeader.IsValid())
    {
        if (UNPC_PackBehaviorManager* LeaderPackManager = PackLeader->FindComponentByClass<UNPC_PackBehaviorManager>())
        {
            LeaderPackManager->RemovePackMember(Cast<APawn>(GetOwner()));
        }
    }

    PackLeader = nullptr;
    MyRole = ENPC_PackRole::Leader;
    PackMembers.Empty();
}

void UNPC_PackBehaviorManager::AddPackMember(APawn* NewMember, ENPC_PackRole Role)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize)
        return;

    // Check if member is already in pack
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Member.Get() == NewMember)
            return;
    }

    FNPC_PackMember NewPackMember;
    NewPackMember.Member = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.bIsAlive = true;
    NewPackMember.DistanceFromLeader = FVector::Dist(GetOwner()->GetActorLocation(), NewMember->GetActorLocation());

    PackMembers.Add(NewPackMember);
}

void UNPC_PackBehaviorManager::RemovePackMember(APawn* Member)
{
    PackMembers.RemoveAll([Member](const FNPC_PackMember& PackMember)
    {
        return PackMember.Member.Get() == Member;
    });
}

bool UNPC_PackBehaviorManager::IsPackLeader() const
{
    return MyRole == ENPC_PackRole::Leader;
}

FVector UNPC_PackBehaviorManager::GetPackCenterLocation() const
{
    if (PackMembers.Num() == 0)
        return GetOwner()->GetActorLocation();

    FVector CenterLocation = GetOwner()->GetActorLocation();
    int32 ValidMembers = 1;

    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Member.IsValid() && Member.bIsAlive)
        {
            CenterLocation += Member.Member->GetActorLocation();
            ValidMembers++;
        }
    }

    return CenterLocation / ValidMembers;
}

void UNPC_PackBehaviorManager::ShareTargetWithPack(APawn* Target, float ThreatLevel)
{
    if (!Target)
        return;

    // Add or update target in shared targets
    bool bTargetFound = false;
    for (FNPC_HuntingTarget& HuntTarget : SharedTargets)
    {
        if (HuntTarget.Target.Get() == Target)
        {
            HuntTarget.LastKnownLocation = Target->GetActorLocation();
            HuntTarget.ThreatLevel = ThreatLevel;
            HuntTarget.TimeSinceLastSeen = 0.0f;
            bTargetFound = true;
            break;
        }
    }

    if (!bTargetFound)
    {
        FNPC_HuntingTarget NewTarget;
        NewTarget.Target = Target;
        NewTarget.LastKnownLocation = Target->GetActorLocation();
        NewTarget.ThreatLevel = ThreatLevel;
        NewTarget.TimeSinceLastSeen = 0.0f;
        SharedTargets.Add(NewTarget);
    }

    // Communicate target to pack members
    SendCommunication(ENPC_CommunicationType::TargetSpotted, Target->GetActorLocation());
}

void UNPC_PackBehaviorManager::SetHuntingStrategy(ENPC_HuntingStrategy Strategy)
{
    CurrentStrategy = Strategy;
    
    // Communicate strategy change to pack
    SendCommunication(ENPC_CommunicationType::StrategyChange, GetOwner()->GetActorLocation());
}

APawn* UNPC_PackBehaviorManager::GetBestHuntingTarget() const
{
    APawn* BestTarget = nullptr;
    float BestScore = 0.0f;

    for (const FNPC_HuntingTarget& Target : SharedTargets)
    {
        if (Target.Target.IsValid() && Target.TimeSinceLastSeen < 10.0f)
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target.LastKnownLocation);
            float Score = Target.ThreatLevel / (Distance + 1.0f); // Higher threat, closer distance = higher score

            if (Score > BestScore)
            {
                BestScore = Score;
                BestTarget = Target.Target.Get();
            }
        }
    }

    return BestTarget;
}

void UNPC_PackBehaviorManager::CoordinateAttack(APawn* Target)
{
    if (!Target)
        return;

    bIsActivelyHunting = true;
    ShareTargetWithPack(Target, 10.0f); // High threat level for coordinated attack
    
    // Send attack coordination signal
    SendCommunication(ENPC_CommunicationType::AttackCoordination, Target->GetActorLocation());
}

void UNPC_PackBehaviorManager::SendCommunication(ENPC_CommunicationType Type, const FVector& Location)
{
    // Add to recent communications
    RecentCommunications.Add(Type);
    if (RecentCommunications.Num() > 10)
    {
        RecentCommunications.RemoveAt(0);
    }

    // Send to pack members within communication range
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Member.IsValid())
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Member.Member->GetActorLocation());
            if (Distance <= CommunicationRange)
            {
                if (UNPC_PackBehaviorManager* MemberPackManager = Member.Member->FindComponentByClass<UNPC_PackBehaviorManager>())
                {
                    MemberPackManager->ReceiveCommunication(Type, Location, Cast<APawn>(GetOwner()));
                }
            }
        }
    }
}

void UNPC_PackBehaviorManager::ReceiveCommunication(ENPC_CommunicationType Type, const FVector& Location, APawn* Sender)
{
    if (!Sender)
        return;

    switch (Type)
    {
        case ENPC_CommunicationType::TargetSpotted:
            // React to target spotted communication
            if (bIsActivelyHunting)
            {
                // Move towards the spotted location
                // This would typically trigger behavior tree changes
            }
            break;

        case ENPC_CommunicationType::AttackCoordination:
            // Coordinate attack response
            bIsActivelyHunting = true;
            break;

        case ENPC_CommunicationType::DangerWarning:
            // React to danger warning
            bIsActivelyHunting = false;
            break;

        case ENPC_CommunicationType::StrategyChange:
            // Acknowledge strategy change
            break;
    }
}

bool UNPC_PackBehaviorManager::ShouldFollowLeader() const
{
    return PackLeader.IsValid() && MyRole != ENPC_PackRole::Leader;
}

bool UNPC_PackBehaviorManager::ShouldMaintainFormation() const
{
    if (!PackLeader.IsValid())
        return false;

    float DistanceToLeader = FVector::Dist(GetOwner()->GetActorLocation(), PackLeader->GetActorLocation());
    return DistanceToLeader > PackSeparationRadius && DistanceToLeader < PackCohesionRadius;
}

FVector UNPC_PackBehaviorManager::GetFormationPosition() const
{
    if (!PackLeader.IsValid())
        return GetOwner()->GetActorLocation();

    // Calculate formation position based on role and pack strategy
    FVector LeaderLocation = PackLeader->GetActorLocation();
    FVector LeaderForward = PackLeader->GetActorForwardVector();
    
    switch (MyRole)
    {
        case ENPC_PackRole::Scout:
            return LeaderLocation + LeaderForward * 500.0f; // Ahead of leader

        case ENPC_PackRole::Flanker:
            return LeaderLocation + LeaderForward.RotateAngleAxis(45.0f, FVector::UpVector) * 300.0f;

        case ENPC_PackRole::Follower:
        default:
            return LeaderLocation - LeaderForward * 200.0f; // Behind leader
    }
}

void UNPC_PackBehaviorManager::UpdatePackCohesion(float DeltaTime)
{
    // Remove invalid or dead pack members
    PackMembers.RemoveAll([](const FNPC_PackMember& Member)
    {
        return !Member.Member.IsValid() || !Member.bIsAlive;
    });

    // Update distances from leader
    if (PackLeader.IsValid())
    {
        for (FNPC_PackMember& Member : PackMembers)
        {
            if (Member.Member.IsValid())
            {
                Member.DistanceFromLeader = FVector::Dist(PackLeader->GetActorLocation(), Member.Member->GetActorLocation());
            }
        }
    }
}

void UNPC_PackBehaviorManager::UpdateHuntingTargets(float DeltaTime)
{
    // Update time since last seen for all targets
    for (FNPC_HuntingTarget& Target : SharedTargets)
    {
        Target.TimeSinceLastSeen += DeltaTime;
    }

    // Remove targets that haven't been seen for too long
    SharedTargets.RemoveAll([](const FNPC_HuntingTarget& Target)
    {
        return Target.TimeSinceLastSeen > 30.0f || !Target.Target.IsValid();
    });
}

void UNPC_PackBehaviorManager::ProcessCommunications(float DeltaTime)
{
    // Clear old communications
    if (RecentCommunications.Num() > 5)
    {
        RecentCommunications.RemoveAt(0);
    }
}

void UNPC_PackBehaviorManager::ManagePackLeadership()
{
    if (IsPackLeader())
    {
        // Check if we should remain leader or pass leadership
        // For now, maintain leadership unless we die
        return;
    }

    // Check if our leader is still valid
    if (!PackLeader.IsValid())
    {
        // Try to find new leader or become leader
        bool bFoundNewLeader = false;
        
        for (const FNPC_PackMember& Member : PackMembers)
        {
            if (Member.Member.IsValid() && Member.Role == ENPC_PackRole::Leader)
            {
                PackLeader = Member.Member.Get();
                bFoundNewLeader = true;
                break;
            }
        }

        if (!bFoundNewLeader)
        {
            // Become the new leader
            MyRole = ENPC_PackRole::Leader;
            PackLeader = Cast<APawn>(GetOwner());
        }
    }
}