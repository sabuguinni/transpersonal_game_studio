#include "NPC_DinosaurPackManager.h"
#include "ANPC_DinosaurAI.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"

UNPC_DinosaurPackManager::UNPC_DinosaurPackManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    MaxPackSize = 8;
    PackCohesionRadius = 2000.0f;
    PackSeparationRadius = 500.0f;
    PackAlignmentRadius = 1500.0f;
    LeaderFollowDistance = 800.0f;
    FormationUpdateInterval = 0.5f;
    ThreatResponseRadius = 3000.0f;

    LastFormationUpdate = 0.0f;
    CurrentThreatLevel = 0.0f;
    LastKnownThreatLocation = FVector::ZeroVector;
}

void UNPC_DinosaurPackManager::BeginPlay()
{
    Super::BeginPlay();
    
    PackState.CurrentActivity = ENPC_PackActivity::Idle;
    PackState.PackRadius = PackCohesionRadius;
    PackState.ThreatLevel = 0.0f;
}

void UNPC_DinosaurPackManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (PackMembers.Num() == 0)
    {
        return;
    }

    // Update pack center location
    CalculatePackCenter();

    // Update formation if enough time has passed
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFormationUpdate >= FormationUpdateInterval)
    {
        UpdatePackFormation();
        LastFormationUpdate = CurrentTime;
    }

    // Apply flocking behaviors
    UpdatePackCohesion();
    UpdatePackSeparation();
    UpdatePackAlignment();

    // Decay threat level over time
    if (CurrentThreatLevel > 0.0f)
    {
        CurrentThreatLevel = FMath::Max(0.0f, CurrentThreatLevel - (DeltaTime * 0.2f));
        PackState.ThreatLevel = CurrentThreatLevel;
    }
}

void UNPC_DinosaurPackManager::AddPackMember(ANPC_DinosaurAI* NewMember, bool bAsAlpha)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize)
    {
        return;
    }

    // Check if already in pack
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef == NewMember)
        {
            return;
        }
    }

    FNPC_PackMember NewPackMember;
    NewPackMember.DinosaurRef = NewMember;
    NewPackMember.bIsAlpha = bAsAlpha;
    NewPackMember.DistanceFromLeader = FMath::RandRange(400.0f, 800.0f);
    NewPackMember.RelativePosition = FVector::ZeroVector;

    PackMembers.Add(NewPackMember);

    // Set as leader if alpha or first member
    if (bAsAlpha || !PackLeader.IsValid())
    {
        SetPackLeader(NewMember);
    }

    // Update formation
    UpdatePackFormation();
}

void UNPC_DinosaurPackManager::RemovePackMember(ANPC_DinosaurAI* MemberToRemove)
{
    if (!MemberToRemove)
    {
        return;
    }

    for (int32 i = PackMembers.Num() - 1; i >= 0; --i)
    {
        if (PackMembers[i].DinosaurRef == MemberToRemove)
        {
            PackMembers.RemoveAt(i);
            break;
        }
    }

    // If removed member was leader, assign new leader
    if (PackLeader == MemberToRemove)
    {
        PackLeader = nullptr;
        if (PackMembers.Num() > 0)
        {
            // Find alpha or assign first member as leader
            for (const FNPC_PackMember& Member : PackMembers)
            {
                if (Member.bIsAlpha && Member.DinosaurRef.IsValid())
                {
                    SetPackLeader(Member.DinosaurRef.Get());
                    break;
                }
            }
            
            if (!PackLeader.IsValid() && PackMembers[0].DinosaurRef.IsValid())
            {
                SetPackLeader(PackMembers[0].DinosaurRef.Get());
            }
        }
    }
}

void UNPC_DinosaurPackManager::SetPackLeader(ANPC_DinosaurAI* NewLeader)
{
    if (!NewLeader)
    {
        return;
    }

    PackLeader = NewLeader;

    // Mark as alpha in pack members
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef == NewLeader)
        {
            Member.bIsAlpha = true;
        }
        else
        {
            Member.bIsAlpha = false;
        }
    }
}

ANPC_DinosaurAI* UNPC_DinosaurPackManager::GetPackLeader() const
{
    return PackLeader.IsValid() ? PackLeader.Get() : nullptr;
}

TArray<ANPC_DinosaurAI*> UNPC_DinosaurPackManager::GetPackMembers() const
{
    TArray<ANPC_DinosaurAI*> Members;
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef.IsValid())
        {
            Members.Add(Member.DinosaurRef.Get());
        }
    }
    return Members;
}

int32 UNPC_DinosaurPackManager::GetPackSize() const
{
    return PackMembers.Num();
}

void UNPC_DinosaurPackManager::SetPackActivity(ENPC_PackActivity NewActivity)
{
    PackState.CurrentActivity = NewActivity;
    
    // Broadcast activity change to all pack members
    BroadcastPackSignal(ENPC_PackSignal::ActivityChange);
}

void UNPC_DinosaurPackManager::UpdatePackFormation()
{
    if (!PackLeader.IsValid() || PackMembers.Num() <= 1)
    {
        return;
    }

    AssignFormationPositions();
}

void UNPC_DinosaurPackManager::MovePackToLocation(const FVector& TargetLocation)
{
    PackState.TargetLocation = TargetLocation;
    SetPackActivity(ENPC_PackActivity::Moving);

    // Leader moves to target, others follow in formation
    if (PackLeader.IsValid())
    {
        // Set leader destination
        // Note: This would call into the AI controller's movement system
    }
}

void UNPC_DinosaurPackManager::InitiatePackHunt(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    PackState.TargetLocation = Target->GetActorLocation();
    SetPackActivity(ENPC_PackActivity::Hunting);
    
    BroadcastPackSignal(ENPC_PackSignal::HuntTarget);
}

void UNPC_DinosaurPackManager::AlertPackToThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
    {
        return;
    }

    LastKnownThreatLocation = ThreatActor->GetActorLocation();
    CurrentThreatLevel = FMath::Max(CurrentThreatLevel, ThreatLevel);
    PackState.ThreatLevel = CurrentThreatLevel;

    if (ThreatLevel > 0.7f)
    {
        SetPackActivity(ENPC_PackActivity::Fleeing);
        BroadcastPackSignal(ENPC_PackSignal::Danger);
    }
    else if (ThreatLevel > 0.4f)
    {
        SetPackActivity(ENPC_PackActivity::Alert);
        BroadcastPackSignal(ENPC_PackSignal::Alert);
    }
}

void UNPC_DinosaurPackManager::BroadcastPackSignal(ENPC_PackSignal Signal)
{
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef.IsValid())
        {
            // Note: This would call into the AI's signal handling system
            // Member.DinosaurRef->HandlePackSignal(Signal);
        }
    }
}

void UNPC_DinosaurPackManager::HandlePackMemberSignal(ANPC_DinosaurAI* Sender, ENPC_PackSignal Signal)
{
    if (!Sender)
    {
        return;
    }

    switch (Signal)
    {
        case ENPC_PackSignal::Danger:
            AlertPackToThreat(nullptr, 0.8f);
            break;
        case ENPC_PackSignal::FoodFound:
            if (PackState.CurrentActivity == ENPC_PackActivity::Foraging)
            {
                MovePackToLocation(Sender->GetActorLocation());
            }
            break;
        case ENPC_PackSignal::Alert:
            if (PackState.CurrentActivity == ENPC_PackActivity::Idle)
            {
                SetPackActivity(ENPC_PackActivity::Alert);
            }
            break;
        default:
            break;
    }
}

FVector UNPC_DinosaurPackManager::GetPackCenterLocation() const
{
    return PackState.PackCenterLocation;
}

float UNPC_DinosaurPackManager::GetPackRadius() const
{
    return PackState.PackRadius;
}

bool UNPC_DinosaurPackManager::IsPackInCombat() const
{
    return PackState.CurrentActivity == ENPC_PackActivity::Hunting || 
           PackState.CurrentActivity == ENPC_PackActivity::Fighting;
}

bool UNPC_DinosaurPackManager::IsPackHunting() const
{
    return PackState.CurrentActivity == ENPC_PackActivity::Hunting;
}

void UNPC_DinosaurPackManager::UpdatePackCohesion()
{
    FVector PackCenter = GetPackCenterLocation();
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (!Member.DinosaurRef.IsValid())
        {
            continue;
        }

        FVector MemberLocation = Member.DinosaurRef->GetActorLocation();
        float DistanceToCenter = FVector::Dist(MemberLocation, PackCenter);
        
        if (DistanceToCenter > PackCohesionRadius)
        {
            // Apply cohesion force toward pack center
            FVector CohesionForce = (PackCenter - MemberLocation).GetSafeNormal();
            // Note: This would be applied to the AI's movement system
        }
    }
}

void UNPC_DinosaurPackManager::UpdatePackSeparation()
{
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (!PackMembers[i].DinosaurRef.IsValid())
        {
            continue;
        }

        FVector SeparationForce = FVector::ZeroVector;
        FVector MemberLocation = PackMembers[i].DinosaurRef->GetActorLocation();
        
        for (int32 j = 0; j < PackMembers.Num(); ++j)
        {
            if (i == j || !PackMembers[j].DinosaurRef.IsValid())
            {
                continue;
            }

            FVector OtherLocation = PackMembers[j].DinosaurRef->GetActorLocation();
            float Distance = FVector::Dist(MemberLocation, OtherLocation);
            
            if (Distance < PackSeparationRadius && Distance > 0.0f)
            {
                FVector AwayVector = (MemberLocation - OtherLocation).GetSafeNormal();
                SeparationForce += AwayVector / Distance;
            }
        }
        
        // Apply separation force
        // Note: This would be applied to the AI's movement system
    }
}

void UNPC_DinosaurPackManager::UpdatePackAlignment()
{
    if (!PackLeader.IsValid())
    {
        return;
    }

    FVector LeaderForward = PackLeader->GetActorForwardVector();
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (!Member.DinosaurRef.IsValid() || Member.DinosaurRef == PackLeader)
        {
            continue;
        }

        FVector MemberLocation = Member.DinosaurRef->GetActorLocation();
        FVector LeaderLocation = PackLeader->GetActorLocation();
        float DistanceToLeader = FVector::Dist(MemberLocation, LeaderLocation);
        
        if (DistanceToLeader < PackAlignmentRadius)
        {
            // Align with leader's direction
            // Note: This would be applied to the AI's rotation system
        }
    }
}

void UNPC_DinosaurPackManager::CalculatePackCenter()
{
    if (PackMembers.Num() == 0)
    {
        PackState.PackCenterLocation = FVector::ZeroVector;
        return;
    }

    FVector CenterSum = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.DinosaurRef.IsValid())
        {
            CenterSum += Member.DinosaurRef->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers > 0)
    {
        PackState.PackCenterLocation = CenterSum / ValidMembers;
    }
}

void UNPC_DinosaurPackManager::AssignFormationPositions()
{
    if (!PackLeader.IsValid() || PackMembers.Num() <= 1)
    {
        return;
    }

    FVector LeaderLocation = PackLeader->GetActorLocation();
    FVector LeaderForward = PackLeader->GetActorForwardVector();
    FVector LeaderRight = PackLeader->GetActorRightVector();
    
    int32 MemberIndex = 0;
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (!Member.DinosaurRef.IsValid() || Member.DinosaurRef == PackLeader)
        {
            continue;
        }

        // Calculate formation position
        float Angle = (MemberIndex * 60.0f) * PI / 180.0f; // 60 degree spacing
        float Distance = Member.DistanceFromLeader;
        
        FVector Offset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Transform to world space
        FVector WorldOffset = LeaderForward * Offset.X + LeaderRight * Offset.Y;
        Member.RelativePosition = LeaderLocation + WorldOffset;
        
        MemberIndex++;
    }
}

FVector UNPC_DinosaurPackManager::CalculateFlockingForce(ANPC_DinosaurAI* Member)
{
    if (!Member)
    {
        return FVector::ZeroVector;
    }

    FVector CohesionForce = FVector::ZeroVector;
    FVector SeparationForce = FVector::ZeroVector;
    FVector AlignmentForce = FVector::ZeroVector;
    
    FVector MemberLocation = Member->GetActorLocation();
    FVector PackCenter = GetPackCenterLocation();
    
    // Cohesion - move toward pack center
    float DistanceToCenter = FVector::Dist(MemberLocation, PackCenter);
    if (DistanceToCenter > PackCohesionRadius * 0.5f)
    {
        CohesionForce = (PackCenter - MemberLocation).GetSafeNormal() * 0.3f;
    }
    
    // Separation - avoid other pack members
    for (const FNPC_PackMember& OtherMember : PackMembers)
    {
        if (!OtherMember.DinosaurRef.IsValid() || OtherMember.DinosaurRef == Member)
        {
            continue;
        }

        FVector OtherLocation = OtherMember.DinosaurRef->GetActorLocation();
        float Distance = FVector::Dist(MemberLocation, OtherLocation);
        
        if (Distance < PackSeparationRadius && Distance > 0.0f)
        {
            FVector AwayVector = (MemberLocation - OtherLocation).GetSafeNormal();
            SeparationForce += AwayVector * (PackSeparationRadius - Distance) / PackSeparationRadius;
        }
    }
    
    // Alignment - match leader's direction
    if (PackLeader.IsValid() && PackLeader.Get() != Member)
    {
        AlignmentForce = PackLeader->GetActorForwardVector() * 0.2f;
    }
    
    return CohesionForce + SeparationForce + AlignmentForce;
}

bool UNPC_DinosaurPackManager::ValidatePackMember(ANPC_DinosaurAI* Member) const
{
    if (!Member || !Member->IsValidLowLevel())
    {
        return false;
    }

    // Check if member is still alive and active
    if (Member->IsPendingKill())
    {
        return false;
    }

    return true;
}