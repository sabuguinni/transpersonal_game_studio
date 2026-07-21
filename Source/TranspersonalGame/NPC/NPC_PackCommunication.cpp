#include "NPC_PackCommunication.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UNPC_PackCommunication::UNPC_PackCommunication()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Pack Settings
    CommunicationRange = 2000.0f;
    CohesionRange = 1500.0f;
    FormationDistance = 300.0f;
    MaxPackSize = 8;
    bIsPackLeader = false;

    // Pack State
    PackLeader = nullptr;
    LastCommunicationTime = 0.0f;

    // Internal Settings
    MessageRetentionTime = 30.0f;
    LastPackUpdate = 0.0f;
    PackUpdateInterval = 1.0f;
}

void UNPC_PackCommunication::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize pack system
    UpdatePackMembersList();
}

void UNPC_PackCommunication::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Update pack members periodically
    if (CurrentTime - LastPackUpdate > PackUpdateInterval)
    {
        UpdatePackMembersList();
        LastPackUpdate = CurrentTime;
    }

    // Process incoming messages
    ProcessMessages();

    // Clean up old messages
    CleanupOldMessages();

    // Update pack cohesion if we're in a pack
    if (IsInPack())
    {
        UpdatePackCohesion();
    }
}

void UNPC_PackCommunication::SendPackSignal(ENPC_PackSignal Signal, FVector TargetLocation, AActor* TargetActor, float Priority)
{
    if (!IsInPack())
    {
        return;
    }

    FNPC_PackMessage Message;
    Message.Signal = Signal;
    Message.TargetLocation = TargetLocation;
    Message.TargetActor = TargetActor;
    Message.Priority = Priority;
    Message.Timestamp = GetWorld()->GetTimeSeconds();

    // Broadcast to all pack members
    BroadcastToPackMembers(Message);

    LastCommunicationTime = Message.Timestamp;

    UE_LOG(LogTemp, Log, TEXT("Pack signal sent: %d from %s"), (int32)Signal, *GetOwner()->GetName());
}

void UNPC_PackCommunication::ReceivePackMessage(const FNPC_PackMessage& Message)
{
    // Add message to recent messages
    RecentMessages.Add(Message);

    // Sort by priority (highest first)
    RecentMessages.Sort([](const FNPC_PackMessage& A, const FNPC_PackMessage& B) {
        return A.Priority > B.Priority;
    });

    UE_LOG(LogTemp, Log, TEXT("Pack message received: %d by %s"), (int32)Message.Signal, *GetOwner()->GetName());
}

void UNPC_PackCommunication::JoinPack(AActor* NewPackLeader)
{
    if (!NewPackLeader)
    {
        return;
    }

    // Leave current pack if in one
    if (IsInPack())
    {
        LeavePack();
    }

    PackLeader = NewPackLeader;

    // If we're joining as the leader, initialize pack
    if (NewPackLeader == GetOwner())
    {
        bIsPackLeader = true;
        PackMembers.Empty();
        PackMembers.Add(GetOwner());
    }
    else
    {
        bIsPackLeader = false;
        
        // Notify pack leader of new member
        UNPC_PackCommunication* LeaderComm = NewPackLeader->FindComponentByClass<UNPC_PackCommunication>();
        if (LeaderComm && LeaderComm->bIsPackLeader)
        {
            LeaderComm->PackMembers.AddUnique(GetOwner());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("%s joined pack led by %s"), *GetOwner()->GetName(), *NewPackLeader->GetName());
}

void UNPC_PackCommunication::LeavePack()
{
    if (!IsInPack())
    {
        return;
    }

    // If we're the leader, disband the pack
    if (bIsPackLeader)
    {
        for (AActor* Member : PackMembers)
        {
            if (Member && Member != GetOwner())
            {
                UNPC_PackCommunication* MemberComm = Member->FindComponentByClass<UNPC_PackCommunication>();
                if (MemberComm)
                {
                    MemberComm->PackLeader = nullptr;
                }
            }
        }
        PackMembers.Empty();
        bIsPackLeader = false;
    }
    else
    {
        // Remove ourselves from leader's pack
        if (PackLeader)
        {
            UNPC_PackCommunication* LeaderComm = PackLeader->FindComponentByClass<UNPC_PackCommunication>();
            if (LeaderComm)
            {
                LeaderComm->PackMembers.Remove(GetOwner());
            }
        }
    }

    PackLeader = nullptr;
    RecentMessages.Empty();

    UE_LOG(LogTemp, Log, TEXT("%s left pack"), *GetOwner()->GetName());
}

void UNPC_PackCommunication::UpdatePackCohesion()
{
    if (!IsInPack() || !PackLeader)
    {
        return;
    }

    FVector PackCenter = GetPackCenterLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    float DistanceToCenter = FVector::Dist(OwnerLocation, PackCenter);

    // If we're too far from the pack, send a regroup signal
    if (DistanceToCenter > CohesionRange)
    {
        if (bIsPackLeader)
        {
            SendPackSignal(ENPC_PackSignal::Regroup, PackCenter, nullptr, 0.8f);
        }
    }
}

FVector UNPC_PackCommunication::GetPackCenterLocation() const
{
    if (!IsInPack())
    {
        return GetOwner()->GetActorLocation();
    }

    FVector CenterLocation = FVector::ZeroVector;
    int32 ValidMembers = 0;

    if (bIsPackLeader)
    {
        for (AActor* Member : PackMembers)
        {
            if (Member)
            {
                CenterLocation += Member->GetActorLocation();
                ValidMembers++;
            }
        }
    }
    else if (PackLeader)
    {
        CenterLocation = PackLeader->GetActorLocation();
        ValidMembers = 1;

        UNPC_PackCommunication* LeaderComm = PackLeader->FindComponentByClass<UNPC_PackCommunication>();
        if (LeaderComm)
        {
            for (AActor* Member : LeaderComm->PackMembers)
            {
                if (Member && Member != PackLeader)
                {
                    CenterLocation += Member->GetActorLocation();
                    ValidMembers++;
                }
            }
        }
    }

    if (ValidMembers > 0)
    {
        CenterLocation /= ValidMembers;
    }
    else
    {
        CenterLocation = GetOwner()->GetActorLocation();
    }

    return CenterLocation;
}

float UNPC_PackCommunication::GetDistanceToPackCenter() const
{
    FVector PackCenter = GetPackCenterLocation();
    return FVector::Dist(GetOwner()->GetActorLocation(), PackCenter);
}

bool UNPC_PackCommunication::ShouldMaintainFormation() const
{
    if (!IsInPack())
    {
        return false;
    }

    float DistanceToCenter = GetDistanceToPackCenter();
    return DistanceToCenter <= FormationDistance;
}

void UNPC_PackCommunication::BroadcastToPackMembers(const FNPC_PackMessage& Message)
{
    if (!bIsPackLeader)
    {
        return;
    }

    for (AActor* Member : PackMembers)
    {
        if (Member && Member != GetOwner())
        {
            UNPC_PackCommunication* MemberComm = Member->FindComponentByClass<UNPC_PackCommunication>();
            if (MemberComm && IsActorInRange(Member, CommunicationRange))
            {
                MemberComm->ReceivePackMessage(Message);
            }
        }
    }
}

void UNPC_PackCommunication::UpdatePackMembersList()
{
    if (!bIsPackLeader)
    {
        return;
    }

    // Remove invalid or out-of-range members
    PackMembers.RemoveAll([this](AActor* Member) {
        if (!Member || !IsValid(Member))
        {
            return true;
        }

        if (!IsActorInRange(Member, CommunicationRange * 1.5f)) // Allow some buffer
        {
            UNPC_PackCommunication* MemberComm = Member->FindComponentByClass<UNPC_PackCommunication>();
            if (MemberComm)
            {
                MemberComm->PackLeader = nullptr;
            }
            return true;
        }

        return false;
    });

    // Limit pack size
    if (PackMembers.Num() > MaxPackSize)
    {
        // Remove excess members (keep the closest ones)
        FVector LeaderLocation = GetOwner()->GetActorLocation();
        PackMembers.Sort([LeaderLocation](const AActor& A, const AActor& B) {
            float DistA = FVector::Dist(A.GetActorLocation(), LeaderLocation);
            float DistB = FVector::Dist(B.GetActorLocation(), LeaderLocation);
            return DistA < DistB;
        });

        while (PackMembers.Num() > MaxPackSize)
        {
            AActor* RemovedMember = PackMembers.Last();
            PackMembers.RemoveLast();

            if (RemovedMember)
            {
                UNPC_PackCommunication* MemberComm = RemovedMember->FindComponentByClass<UNPC_PackCommunication>();
                if (MemberComm)
                {
                    MemberComm->PackLeader = nullptr;
                }
            }
        }
    }
}

void UNPC_PackCommunication::ProcessMessages()
{
    if (RecentMessages.Num() == 0)
    {
        return;
    }

    // Process the highest priority message
    FNPC_PackMessage& TopMessage = RecentMessages[0];
    
    // Handle different signal types
    switch (TopMessage.Signal)
    {
        case ENPC_PackSignal::TargetSpotted:
            // React to spotted target
            break;
        case ENPC_PackSignal::Danger:
            // React to danger
            break;
        case ENPC_PackSignal::FoodFound:
            // React to food discovery
            break;
        case ENPC_PackSignal::Regroup:
            // Move towards pack center
            break;
        case ENPC_PackSignal::Attack:
            // Join attack on target
            break;
        case ENPC_PackSignal::Retreat:
            // Retreat from danger
            break;
        default:
            break;
    }
}

bool UNPC_PackCommunication::IsActorInRange(AActor* Actor, float Range) const
{
    if (!Actor)
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
    return Distance <= Range;
}

void UNPC_PackCommunication::CleanupOldMessages()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    RecentMessages.RemoveAll([CurrentTime, this](const FNPC_PackMessage& Message) {
        return (CurrentTime - Message.Timestamp) > MessageRetentionTime;
    });
}