#include "CrowdHerdSimulator.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UCrowdHerdSimulator::UCrowdHerdSimulator()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ECrowd_HerdState::Grazing;
    HerdCentroid = FVector::ZeroVector;
    ActiveMemberCount = 0;
    TickAccumulator = 0.0f;
}

void UCrowdHerdSimulator::BeginPlay()
{
    Super::BeginPlay();
    UpdateCentroid();
}

void UCrowdHerdSimulator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickAccumulator += DeltaTime;
    if (TickAccumulator < TickInterval)
    {
        return;
    }
    TickAccumulator = 0.0f;

    // Remove invalid members
    Members.RemoveAll([](const FCrowd_HerdMember& M) { return !IsValid(M.Actor); });
    ActiveMemberCount = Members.Num();

    if (ActiveMemberCount == 0)
    {
        return;
    }

    UpdateCentroid();

    if (CurrentState == ECrowd_HerdState::Wandering || CurrentState == ECrowd_HerdState::Grazing)
    {
        ApplyBoids(TickInterval);
    }
}

void UCrowdHerdSimulator::RegisterMember(AActor* NewMember, ECrowd_HerdRole Role)
{
    if (!IsValid(NewMember))
    {
        return;
    }

    // Avoid duplicates
    for (const FCrowd_HerdMember& M : Members)
    {
        if (M.Actor == NewMember)
        {
            return;
        }
    }

    FCrowd_HerdMember Entry;
    Entry.Actor = NewMember;
    Entry.Role = Role;
    Entry.SeparationRadius = 200.0f + FMath::RandRange(-50.0f, 50.0f);
    Entry.TargetOffset = FVector(
        FMath::RandRange(-300.0f, 300.0f),
        FMath::RandRange(-300.0f, 300.0f),
        0.0f
    );

    Members.Add(Entry);
    ActiveMemberCount = Members.Num();

    // First member becomes leader
    if (Members.Num() == 1)
    {
        Members[0].Role = ECrowd_HerdRole::Leader;
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdHerd: Registered %s as %s (total: %d)"),
        *NewMember->GetName(),
        Role == ECrowd_HerdRole::Leader ? TEXT("Leader") : TEXT("Follower"),
        ActiveMemberCount);
}

void UCrowdHerdSimulator::UnregisterMember(AActor* Member)
{
    if (!IsValid(Member))
    {
        return;
    }

    Members.RemoveAll([Member](const FCrowd_HerdMember& M) { return M.Actor == Member; });
    ActiveMemberCount = Members.Num();

    // Re-elect leader if needed
    bool bHasLeader = false;
    for (const FCrowd_HerdMember& M : Members)
    {
        if (M.Role == ECrowd_HerdRole::Leader)
        {
            bHasLeader = true;
            break;
        }
    }
    if (!bHasLeader && Members.Num() > 0)
    {
        ElectNewLeader();
    }
}

void UCrowdHerdSimulator::TriggerFlee(FVector ThreatLocation)
{
    CurrentState = ECrowd_HerdState::Fleeing;

    for (FCrowd_HerdMember& M : Members)
    {
        if (!IsValid(M.Actor))
        {
            continue;
        }

        FVector MemberLoc = M.Actor->GetActorLocation();
        FVector FleeDir = (MemberLoc - ThreatLocation).GetSafeNormal();
        FVector FleeTarget = MemberLoc + FleeDir * HerdConfig.FleeRadius;

        // Apply flee velocity via offset — actual movement handled by NavMesh/AI in full impl
        M.TargetOffset = FleeTarget - HerdCentroid;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdHerd: FLEE triggered from threat at %s — %d members fleeing"),
        *ThreatLocation.ToString(), ActiveMemberCount);
}

void UCrowdHerdSimulator::ReturnToGrazing()
{
    CurrentState = ECrowd_HerdState::Grazing;

    for (FCrowd_HerdMember& M : Members)
    {
        M.TargetOffset = FVector(
            FMath::RandRange(-300.0f, 300.0f),
            FMath::RandRange(-300.0f, 300.0f),
            0.0f
        );
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdHerd: Returned to Grazing state"));
}

AActor* UCrowdHerdSimulator::GetLeader() const
{
    for (const FCrowd_HerdMember& M : Members)
    {
        if (M.Role == ECrowd_HerdRole::Leader && IsValid(M.Actor))
        {
            return M.Actor;
        }
    }
    return nullptr;
}

FVector UCrowdHerdSimulator::ComputeCentroid() const
{
    if (Members.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector Sum = FVector::ZeroVector;
    int32 ValidCount = 0;

    for (const FCrowd_HerdMember& M : Members)
    {
        if (IsValid(M.Actor))
        {
            Sum += M.Actor->GetActorLocation();
            ++ValidCount;
        }
    }

    return ValidCount > 0 ? Sum / ValidCount : FVector::ZeroVector;
}

void UCrowdHerdSimulator::LogHerdState() const
{
    UE_LOG(LogTemp, Log, TEXT("=== CrowdHerd State ==="));
    UE_LOG(LogTemp, Log, TEXT("  State: %d | Members: %d | Centroid: %s"),
        (int32)CurrentState, ActiveMemberCount, *HerdCentroid.ToString());

    for (int32 i = 0; i < Members.Num(); ++i)
    {
        const FCrowd_HerdMember& M = Members[i];
        if (IsValid(M.Actor))
        {
            UE_LOG(LogTemp, Log, TEXT("  [%d] %s | Role:%d | Loc:%s"),
                i, *M.Actor->GetName(), (int32)M.Role, *M.Actor->GetActorLocation().ToString());
        }
    }
}

void UCrowdHerdSimulator::UpdateCentroid()
{
    HerdCentroid = ComputeCentroid();
}

void UCrowdHerdSimulator::ApplyBoids(float DeltaTime)
{
    for (FCrowd_HerdMember& M : Members)
    {
        if (!IsValid(M.Actor))
        {
            continue;
        }

        FVector Cohesion   = ComputeCohesion(M)   * HerdConfig.CohesionStrength;
        FVector Separation = ComputeSeparation(M) * HerdConfig.SeparationStrength;
        FVector Alignment  = ComputeAlignment(M)  * HerdConfig.AlignmentStrength;

        FVector SteeringForce = Cohesion + Separation + Alignment;
        SteeringForce.Z = 0.0f; // Keep on ground plane

        FVector CurrentLoc = M.Actor->GetActorLocation();
        FVector NewLoc = CurrentLoc + SteeringForce * HerdConfig.MoveSpeed * DeltaTime;

        // Clamp within wander radius from centroid
        FVector OffsetFromCentroid = NewLoc - HerdCentroid;
        if (OffsetFromCentroid.Size() > HerdConfig.WanderRadius)
        {
            NewLoc = HerdCentroid + OffsetFromCentroid.GetSafeNormal() * HerdConfig.WanderRadius;
        }

        M.Actor->SetActorLocation(NewLoc, true);
    }
}

FVector UCrowdHerdSimulator::ComputeCohesion(const FCrowd_HerdMember& Member) const
{
    if (!IsValid(Member.Actor))
    {
        return FVector::ZeroVector;
    }
    FVector ToCenter = HerdCentroid - Member.Actor->GetActorLocation();
    return ToCenter.GetSafeNormal();
}

FVector UCrowdHerdSimulator::ComputeSeparation(const FCrowd_HerdMember& Member) const
{
    if (!IsValid(Member.Actor))
    {
        return FVector::ZeroVector;
    }

    FVector SeparationForce = FVector::ZeroVector;
    FVector MyLoc = Member.Actor->GetActorLocation();

    for (const FCrowd_HerdMember& Other : Members)
    {
        if (!IsValid(Other.Actor) || Other.Actor == Member.Actor)
        {
            continue;
        }

        FVector Diff = MyLoc - Other.Actor->GetActorLocation();
        float Dist = Diff.Size();

        if (Dist < Member.SeparationRadius && Dist > KINDA_SMALL_NUMBER)
        {
            SeparationForce += Diff.GetSafeNormal() * (Member.SeparationRadius - Dist) / Member.SeparationRadius;
        }
    }

    return SeparationForce;
}

FVector UCrowdHerdSimulator::ComputeAlignment(const FCrowd_HerdMember& Member) const
{
    if (!IsValid(Member.Actor))
    {
        return FVector::ZeroVector;
    }

    // Align toward leader's forward direction
    AActor* Leader = GetLeader();
    if (!IsValid(Leader) || Leader == Member.Actor)
    {
        return FVector::ZeroVector;
    }

    return Leader->GetActorForwardVector();
}

void UCrowdHerdSimulator::ElectNewLeader()
{
    if (Members.Num() == 0)
    {
        return;
    }

    Members[0].Role = ECrowd_HerdRole::Leader;
    UE_LOG(LogTemp, Log, TEXT("CrowdHerd: New leader elected: %s"),
        IsValid(Members[0].Actor) ? *Members[0].Actor->GetName() : TEXT("INVALID"));
}
