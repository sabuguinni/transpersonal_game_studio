#include "NPC_PackBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UNPC_PackBehaviorComponent::UNPC_PackBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for pack behavior

    MaxPackSize = 6.0f;
    FormationRadius = 500.0f;
    CohesionStrength = 1.0f;
    SeparationStrength = 2.0f;
    AlignmentStrength = 1.5f;

    bIsHunting = false;
    bIsDefending = false;
    bIsRetreating = false;
    PackLeader = nullptr;
    CurrentTarget = nullptr;
}

void UNPC_PackBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-initialize as pack leader if no leader is set
    if (!PackLeader.IsValid())
    {
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            InitializePack(OwnerPawn);
        }
    }
}

void UNPC_PackBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ValidatePackMembers();
    
    if (PackLeader.IsValid() && PackMembers.Num() > 0)
    {
        UpdatePackFormation();
    }
}

void UNPC_PackBehaviorComponent::InitializePack(APawn* LeaderPawn)
{
    if (!LeaderPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot initialize pack with null leader"));
        return;
    }

    PackLeader = LeaderPawn;
    PackMembers.Empty();

    // Add leader as first member
    FNPC_PackMember LeaderMember;
    LeaderMember.MemberPawn = LeaderPawn;
    LeaderMember.Role = ENPC_PackRole::Leader;
    LeaderMember.DistanceFromLeader = 0.0f;
    LeaderMember.FormationAngle = 0.0f;
    PackMembers.Add(LeaderMember);

    UE_LOG(LogTemp, Log, TEXT("Pack initialized with leader: %s"), *LeaderPawn->GetName());
}

void UNPC_PackBehaviorComponent::AddPackMember(APawn* NewMember, ENPC_PackRole Role)
{
    if (!NewMember || !PackLeader.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add pack member: Invalid member or no pack leader"));
        return;
    }

    if (PackMembers.Num() >= MaxPackSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pack is at maximum capacity (%d members)"), (int32)MaxPackSize);
        return;
    }

    // Check if already in pack
    for (const FNPC_PackMember& ExistingMember : PackMembers)
    {
        if (ExistingMember.MemberPawn == NewMember)
        {
            UE_LOG(LogTemp, Warning, TEXT("Pawn %s is already in the pack"), *NewMember->GetName());
            return;
        }
    }

    FNPC_PackMember NewPackMember;
    NewPackMember.MemberPawn = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.DistanceFromLeader = FormationRadius;
    NewPackMember.FormationAngle = (PackMembers.Num() * 60.0f) % 360.0f; // Spread around circle
    
    PackMembers.Add(NewPackMember);
    
    UE_LOG(LogTemp, Log, TEXT("Added %s to pack as %s"), 
           *NewMember->GetName(), 
           *UEnum::GetValueAsString(NewPackMember.Role));
}

void UNPC_PackBehaviorComponent::RemovePackMember(APawn* MemberToRemove)
{
    if (!MemberToRemove)
    {
        return;
    }

    for (int32 i = PackMembers.Num() - 1; i >= 0; --i)
    {
        if (PackMembers[i].MemberPawn == MemberToRemove)
        {
            UE_LOG(LogTemp, Log, TEXT("Removed %s from pack"), *MemberToRemove->GetName());
            PackMembers.RemoveAt(i);
            break;
        }
    }

    // Redistribute formation angles
    for (int32 i = 1; i < PackMembers.Num(); ++i) // Skip leader at index 0
    {
        PackMembers[i].FormationAngle = ((i - 1) * 60.0f) % 360.0f;
    }
}

void UNPC_PackBehaviorComponent::SetPackFormation(const TArray<FNPC_PackMember>& NewFormation)
{
    PackMembers = NewFormation;
    UE_LOG(LogTemp, Log, TEXT("Pack formation updated with %d members"), PackMembers.Num());
}

void UNPC_PackBehaviorComponent::UpdatePackFormation()
{
    if (!PackLeader.IsValid())
    {
        return;
    }

    FVector LeaderLocation = PackLeader->GetActorLocation();
    FRotator LeaderRotation = PackLeader->GetActorRotation();

    for (int32 i = 1; i < PackMembers.Num(); ++i) // Skip leader at index 0
    {
        FNPC_PackMember& Member = PackMembers[i];
        if (!Member.MemberPawn.IsValid())
        {
            continue;
        }

        UpdateMemberPosition(Member);
    }
}

void UNPC_PackBehaviorComponent::UpdateMemberPosition(FNPC_PackMember& Member)
{
    if (!Member.MemberPawn.IsValid() || !PackLeader.IsValid())
    {
        return;
    }

    FVector LeaderLocation = PackLeader->GetActorLocation();
    FRotator LeaderRotation = PackLeader->GetActorRotation();

    // Calculate formation position
    float AngleRad = FMath::DegreesToRadians(Member.FormationAngle + LeaderRotation.Yaw);
    FVector FormationOffset = FVector(
        Member.DistanceFromLeader * FMath::Cos(AngleRad),
        Member.DistanceFromLeader * FMath::Sin(AngleRad),
        0.0f
    );

    FVector TargetLocation = LeaderLocation + FormationOffset;

    // Apply flocking forces
    FVector Cohesion, Separation, Alignment;
    CalculateFlockingForces(Member.MemberPawn.Get(), Cohesion, Separation, Alignment);

    FVector FlockingForce = (Cohesion * CohesionStrength) + 
                           (Separation * SeparationStrength) + 
                           (Alignment * AlignmentStrength);

    TargetLocation += FlockingForce * 100.0f; // Scale flocking influence

    // Move member towards target position
    FVector CurrentLocation = Member.MemberPawn->GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    // Simple movement - in real implementation, this would use AI movement component
    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), 2.0f);
    Member.MemberPawn->SetActorLocation(NewLocation);

    // Face movement direction
    if (!Direction.IsZero())
    {
        FRotator NewRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
        Member.MemberPawn->SetActorRotation(FMath::RInterpTo(
            Member.MemberPawn->GetActorRotation(), 
            NewRotation, 
            GetWorld()->GetDeltaSeconds(), 
            3.0f
        ));
    }
}

void UNPC_PackBehaviorComponent::CalculateFlockingForces(APawn* Member, FVector& Cohesion, FVector& Separation, FVector& Alignment)
{
    if (!Member)
    {
        Cohesion = Separation = Alignment = FVector::ZeroVector;
        return;
    }

    FVector MemberLocation = Member->GetActorLocation();
    FVector CenterOfMass = FVector::ZeroVector;
    FVector AvgVelocity = FVector::ZeroVector;
    FVector SeparationForce = FVector::ZeroVector;
    int32 NeighborCount = 0;

    for (const FNPC_PackMember& OtherMember : PackMembers)
    {
        if (!OtherMember.MemberPawn.IsValid() || OtherMember.MemberPawn == Member)
        {
            continue;
        }

        FVector OtherLocation = OtherMember.MemberPawn->GetActorLocation();
        float Distance = FVector::Dist(MemberLocation, OtherLocation);

        if (Distance < FormationRadius * 2.0f) // Influence radius
        {
            // Cohesion: move towards center of mass
            CenterOfMass += OtherLocation;

            // Alignment: match average velocity
            AvgVelocity += OtherMember.MemberPawn->GetVelocity();

            // Separation: avoid crowding
            if (Distance < FormationRadius * 0.5f && Distance > 0.0f)
            {
                FVector AwayVector = (MemberLocation - OtherLocation).GetSafeNormal();
                SeparationForce += AwayVector / Distance; // Stronger when closer
            }

            NeighborCount++;
        }
    }

    if (NeighborCount > 0)
    {
        // Cohesion
        CenterOfMass /= NeighborCount;
        Cohesion = (CenterOfMass - MemberLocation).GetSafeNormal();

        // Alignment
        AvgVelocity /= NeighborCount;
        Alignment = AvgVelocity.GetSafeNormal();

        // Separation
        Separation = SeparationForce.GetSafeNormal();
    }
    else
    {
        Cohesion = Separation = Alignment = FVector::ZeroVector;
    }
}

void UNPC_PackBehaviorComponent::ValidatePackMembers()
{
    for (int32 i = PackMembers.Num() - 1; i >= 0; --i)
    {
        if (!PackMembers[i].MemberPawn.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("Removing invalid pack member at index %d"), i);
            PackMembers.RemoveAt(i);
        }
    }
}

void UNPC_PackBehaviorComponent::ExecutePackHunt(APawn* Target)
{
    if (!Target || !PackLeader.IsValid())
    {
        return;
    }

    bIsHunting = true;
    bIsDefending = false;
    bIsRetreating = false;
    CurrentTarget = Target;

    BroadcastPackSignal(TEXT("HUNT_TARGET"));
    
    UE_LOG(LogTemp, Log, TEXT("Pack executing hunt on target: %s"), *Target->GetName());
}

void UNPC_PackBehaviorComponent::ExecutePackDefense(FVector ThreatLocation)
{
    bIsDefending = true;
    bIsHunting = false;
    bIsRetreating = false;

    BroadcastPackSignal(TEXT("DEFEND_POSITION"));
    
    UE_LOG(LogTemp, Log, TEXT("Pack executing defense against threat at: %s"), *ThreatLocation.ToString());
}

void UNPC_PackBehaviorComponent::ExecutePackRetreat(FVector SafeLocation)
{
    bIsRetreating = true;
    bIsHunting = false;
    bIsDefending = false;

    BroadcastPackSignal(TEXT("RETREAT_TO_SAFETY"));
    
    UE_LOG(LogTemp, Log, TEXT("Pack retreating to safe location: %s"), *SafeLocation.ToString());
}

void UNPC_PackBehaviorComponent::BroadcastPackSignal(const FString& SignalType)
{
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn.IsValid())
        {
            // In real implementation, this would trigger behavior tree events
            UE_LOG(LogTemp, Log, TEXT("Broadcasting signal '%s' to %s"), *SignalType, *Member.MemberPawn->GetName());
        }
    }
}

void UNPC_PackBehaviorComponent::ReceivePackSignal(const FString& SignalType, APawn* Sender)
{
    if (!Sender)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Received pack signal '%s' from %s"), *SignalType, *Sender->GetName());

    // Process signal based on type
    if (SignalType == TEXT("HUNT_TARGET"))
    {
        // Join hunt behavior
    }
    else if (SignalType == TEXT("DEFEND_POSITION"))
    {
        // Join defense behavior
    }
    else if (SignalType == TEXT("RETREAT_TO_SAFETY"))
    {
        // Join retreat behavior
    }
}

bool UNPC_PackBehaviorComponent::IsPackLeader() const
{
    return PackLeader.IsValid() && PackLeader.Get() == GetOwner();
}