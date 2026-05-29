#include "Combat_PackHuntingAI.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UCombat_PackHuntingAI::UCombat_PackHuntingAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize pack settings
    bIsPackLeader = false;
    MyPackRole = ECombat_PackRole::Beta;
    CurrentHuntPhase = ECombat_HuntPhase::Idle;
    
    // Hunt parameters
    FlankingDistance = 800.0f;
    CoordinationRadius = 1500.0f;
    AttackSignalDelay = 2.0f;
    MaxHuntDuration = 60.0f;
    
    // Timers
    HuntStartTime = 0.0f;
    LastCoordinationUpdate = 0.0f;
    PositionUpdateInterval = 1.0f;
    
    AssignedHuntPosition = FVector::ZeroVector;
}

void UCombat_PackHuntingAI::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_PackHuntingAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
        return;

    // Update hunt phases and coordination
    UpdateHuntPhase(DeltaTime);
    
    if (bIsPackLeader)
    {
        UpdatePackCoordination(DeltaTime);
        CleanupInvalidMembers();
    }
    
    UpdateTargetTracking(DeltaTime);
    ExecuteRoleBasedBehavior(DeltaTime);
}

void UCombat_PackHuntingAI::InitializeAsPackLeader()
{
    bIsPackLeader = true;
    MyPackRole = ECombat_PackRole::Alpha;
    PackLeaderRef = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: %s initialized as pack leader"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_PackHuntingAI::JoinPack(UCombat_PackHuntingAI* PackLeader, ECombat_PackRole AssignedRole)
{
    if (!PackLeader || !PackLeader->IsValidLowLevel())
        return;

    bIsPackLeader = false;
    MyPackRole = AssignedRole;
    PackLeaderRef = PackLeader;
    
    // Clear any existing pack members if we were a leader
    PackMembers.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: %s joined pack with role %d"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), (int32)AssignedRole);
}

void UCombat_PackHuntingAI::AddPackMember(APawn* NewMember, ECombat_PackRole Role)
{
    if (!bIsPackLeader || !NewMember || !NewMember->IsValidLowLevel())
        return;

    // Check if member already exists
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn.Get() == NewMember)
            return;
    }

    FCombat_PackMember NewPackMember;
    NewPackMember.MemberPawn = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.AssignedPosition = FVector::ZeroVector;
    NewPackMember.DistanceFromTarget = 0.0f;
    NewPackMember.bInPosition = false;

    PackMembers.Add(NewPackMember);
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Added pack member %s with role %d"), 
           *NewMember->GetName(), (int32)Role);
}

void UCombat_PackHuntingAI::RemovePackMember(APawn* Member)
{
    if (!bIsPackLeader || !Member)
        return;

    PackMembers.RemoveAll([Member](const FCombat_PackMember& PackMember)
    {
        return PackMember.MemberPawn.Get() == Member;
    });
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Removed pack member %s"), *Member->GetName());
}

void UCombat_PackHuntingAI::SetHuntTarget(APawn* Target)
{
    if (!Target || !Target->IsValidLowLevel())
        return;

    CurrentTarget.TargetPawn = Target;
    CurrentTarget.LastKnownPosition = Target->GetActorLocation();
    CurrentTarget.ThreatLevel = 1.0f;
    CurrentTarget.TimeSinceLastSeen = 0.0f;
    CurrentTarget.bIsValidTarget = true;
    
    if (CurrentHuntPhase == ECombat_HuntPhase::Idle)
    {
        HandlePhaseTransition(ECombat_HuntPhase::Stalking);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Hunt target set to %s"), *Target->GetName());
}

void UCombat_PackHuntingAI::ClearHuntTarget()
{
    CurrentTarget.TargetPawn = nullptr;
    CurrentTarget.bIsValidTarget = false;
    HandlePhaseTransition(ECombat_HuntPhase::Idle);
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Hunt target cleared"));
}

APawn* UCombat_PackHuntingAI::GetCurrentTarget() const
{
    return CurrentTarget.TargetPawn.Get();
}

void UCombat_PackHuntingAI::StartCoordinatedHunt()
{
    if (!bIsPackLeader || !CurrentTarget.bIsValidTarget)
        return;

    HandlePhaseTransition(ECombat_HuntPhase::Positioning);
    CalculatePackPositions();
    BroadcastToPackMembers(TEXT("COORDINATE_HUNT"));
    
    HuntStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Coordinated hunt started"));
}

void UCombat_PackHuntingAI::ExecutePackAttack()
{
    if (!bIsPackLeader || CurrentHuntPhase != ECombat_HuntPhase::Positioning)
        return;

    if (!IsPackInPosition())
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Pack not in position for attack"));
        return;
    }

    HandlePhaseTransition(ECombat_HuntPhase::Coordinated);
    BroadcastToPackMembers(TEXT("EXECUTE_ATTACK"));
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Pack attack executed"));
}

void UCombat_PackHuntingAI::OrderRetreat()
{
    HandlePhaseTransition(ECombat_HuntPhase::Retreat);
    BroadcastToPackMembers(TEXT("RETREAT"));
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Retreat ordered"));
}

FVector UCombat_PackHuntingAI::CalculateFlankingPosition(ECombat_PackRole Role, const FVector& TargetLocation)
{
    if (!GetOwner())
        return FVector::ZeroVector;

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    float AngleOffset = 0.0f;
    float DistanceMultiplier = 1.0f;
    
    switch (Role)
    {
        case ECombat_PackRole::Alpha:
            AngleOffset = 0.0f;
            DistanceMultiplier = 0.8f;
            break;
        case ECombat_PackRole::Beta:
            AngleOffset = 120.0f;
            DistanceMultiplier = 1.0f;
            break;
        case ECombat_PackRole::Gamma:
            AngleOffset = -120.0f;
            DistanceMultiplier = 1.0f;
            break;
        case ECombat_PackRole::Scout:
            AngleOffset = 180.0f;
            DistanceMultiplier = 1.5f;
            break;
        case ECombat_PackRole::Ambusher:
            AngleOffset = 60.0f;
            DistanceMultiplier = 1.2f;
            break;
    }
    
    // Calculate flanking position
    FVector FlankDirection = DirectionToTarget.RotateAngleAxis(AngleOffset, FVector::UpVector);
    FVector FlankPosition = TargetLocation + (FlankDirection * FlankingDistance * DistanceMultiplier);
    
    return FlankPosition;
}

bool UCombat_PackHuntingAI::IsPackInPosition() const
{
    if (!bIsPackLeader)
        return true;

    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (!Member.bInPosition)
            return false;
    }
    
    return true;
}

void UCombat_PackHuntingAI::BroadcastToPackMembers(const FString& Command)
{
    if (!bIsPackLeader)
        return;

    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (APawn* MemberPawn = Member.MemberPawn.Get())
        {
            if (UCombat_PackHuntingAI* MemberAI = MemberPawn->FindComponentByClass<UCombat_PackHuntingAI>())
            {
                MemberAI->ReceivePackCommand(Command, this);
            }
        }
    }
}

void UCombat_PackHuntingAI::ReceivePackCommand(const FString& Command, UCombat_PackHuntingAI* Sender)
{
    if (bIsPackLeader || !Sender)
        return;

    if (Command == TEXT("COORDINATE_HUNT"))
    {
        HandlePhaseTransition(ECombat_HuntPhase::Positioning);
        if (CurrentTarget.bIsValidTarget)
        {
            AssignedHuntPosition = CalculateFlankingPosition(MyPackRole, CurrentTarget.LastKnownPosition);
        }
    }
    else if (Command == TEXT("EXECUTE_ATTACK"))
    {
        HandlePhaseTransition(ECombat_HuntPhase::Coordinated);
    }
    else if (Command == TEXT("RETREAT"))
    {
        HandlePhaseTransition(ECombat_HuntPhase::Retreat);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: %s received command: %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), *Command);
}

void UCombat_PackHuntingAI::UpdateHuntPhase(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    switch (CurrentHuntPhase)
    {
        case ECombat_HuntPhase::Stalking:
            if (bIsPackLeader && PackMembers.Num() > 0)
            {
                StartCoordinatedHunt();
            }
            break;
            
        case ECombat_HuntPhase::Positioning:
            if (bIsPackLeader && IsPackInPosition())
            {
                if (CurrentTime - HuntStartTime > AttackSignalDelay)
                {
                    ExecutePackAttack();
                }
            }
            break;
            
        case ECombat_HuntPhase::Coordinated:
            if (CurrentTime - HuntStartTime > MaxHuntDuration)
            {
                OrderRetreat();
            }
            break;
            
        case ECombat_HuntPhase::Retreat:
            if (CurrentTime - HuntStartTime > 10.0f)
            {
                HandlePhaseTransition(ECombat_HuntPhase::Idle);
            }
            break;
    }
}

void UCombat_PackHuntingAI::UpdatePackCoordination(float DeltaTime)
{
    if (!bIsPackLeader)
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCoordinationUpdate < PositionUpdateInterval)
        return;

    LastCoordinationUpdate = CurrentTime;
    
    // Update pack member positions and status
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (APawn* MemberPawn = Member.MemberPawn.Get())
        {
            FVector MemberLocation = MemberPawn->GetActorLocation();
            Member.DistanceFromTarget = FVector::Dist(MemberLocation, Member.AssignedPosition);
            Member.bInPosition = Member.DistanceFromTarget < 200.0f;
        }
    }
}

void UCombat_PackHuntingAI::UpdateTargetTracking(float DeltaTime)
{
    if (!CurrentTarget.bIsValidTarget)
        return;

    if (APawn* Target = CurrentTarget.TargetPawn.Get())
    {
        CurrentTarget.LastKnownPosition = Target->GetActorLocation();
        CurrentTarget.TimeSinceLastSeen = 0.0f;
    }
    else
    {
        CurrentTarget.TimeSinceLastSeen += DeltaTime;
        if (CurrentTarget.TimeSinceLastSeen > 10.0f)
        {
            ClearHuntTarget();
        }
    }
}

void UCombat_PackHuntingAI::CalculatePackPositions()
{
    if (!bIsPackLeader || !CurrentTarget.bIsValidTarget)
        return;

    FVector TargetLocation = CurrentTarget.LastKnownPosition;
    
    for (FCombat_PackMember& Member : PackMembers)
    {
        Member.AssignedPosition = CalculateFlankingPosition(Member.Role, TargetLocation);
        Member.bInPosition = false;
    }
}

bool UCombat_PackHuntingAI::ValidateTarget()
{
    if (!CurrentTarget.bIsValidTarget)
        return false;

    APawn* Target = CurrentTarget.TargetPawn.Get();
    if (!Target || !Target->IsValidLowLevel())
    {
        CurrentTarget.bIsValidTarget = false;
        return false;
    }

    return true;
}

void UCombat_PackHuntingAI::HandlePhaseTransition(ECombat_HuntPhase NewPhase)
{
    if (CurrentHuntPhase == NewPhase)
        return;

    ECombat_HuntPhase OldPhase = CurrentHuntPhase;
    CurrentHuntPhase = NewPhase;
    
    UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Phase transition %d -> %d"), 
           (int32)OldPhase, (int32)NewPhase);
}

void UCombat_PackHuntingAI::ExecuteRoleBasedBehavior(float DeltaTime)
{
    if (!GetOwner())
        return;

    switch (MyPackRole)
    {
        case ECombat_PackRole::Alpha:
            // Alpha leads and makes decisions
            if (bIsPackLeader && CurrentHuntPhase == ECombat_HuntPhase::Stalking)
            {
                // Look for opportunities to start coordinated hunt
            }
            break;
            
        case ECombat_PackRole::Beta:
        case ECombat_PackRole::Gamma:
            // Flankers move to assigned positions
            if (CurrentHuntPhase == ECombat_HuntPhase::Positioning && AssignedHuntPosition != FVector::ZeroVector)
            {
                // Move towards assigned position (would integrate with movement system)
            }
            break;
            
        case ECombat_PackRole::Scout:
            // Scout maintains distance and watches for threats
            break;
            
        case ECombat_PackRole::Ambusher:
            // Ambusher waits for optimal strike moment
            break;
    }
}

FVector UCombat_PackHuntingAI::GetOptimalAttackVector() const
{
    if (!CurrentTarget.bIsValidTarget || !GetOwner())
        return FVector::ZeroVector;

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = CurrentTarget.LastKnownPosition;
    
    return (TargetLocation - OwnerLocation).GetSafeNormal();
}

bool UCombat_PackHuntingAI::CanExecuteCoordinatedAttack() const
{
    return bIsPackLeader && 
           CurrentHuntPhase == ECombat_HuntPhase::Positioning && 
           IsPackInPosition() && 
           CurrentTarget.bIsValidTarget;
}

void UCombat_PackHuntingAI::CleanupInvalidMembers()
{
    if (!bIsPackLeader)
        return;

    PackMembers.RemoveAll([](const FCombat_PackMember& Member)
    {
        return !Member.MemberPawn.IsValid() || !Member.MemberPawn.Get();
    });
}