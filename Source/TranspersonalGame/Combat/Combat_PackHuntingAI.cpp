#include "Combat_PackHuntingAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCombat_PackHuntingAI::UCombat_PackHuntingAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    CurrentHuntPhase = ECombat_HuntPhase::Patrol;
    HuntRange = 2000.0f;
    AttackRange = 300.0f;
    FlankingDistance = 500.0f;
    CoordinationRadius = 1000.0f;
    PositioningTimeout = 10.0f;
    AttackCooldown = 5.0f;
    LastAttackTime = 0.0f;
    
    AlphaPawn = nullptr;
}

void UCombat_PackHuntingAI::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Component initialized"));
}

void UCombat_PackHuntingAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up invalid pack members
    CleanupInvalidMembers();
    
    // Update pack behavior based on current phase
    switch (CurrentHuntPhase)
    {
        case ECombat_HuntPhase::Patrol:
            // Look for targets in range
            if (CurrentTarget.bIsValidTarget && IsTargetInRange())
            {
                UpdateHuntPhase(ECombat_HuntPhase::Tracking);
            }
            break;
            
        case ECombat_HuntPhase::Tracking:
            // Move pack towards target
            UpdatePackPositions();
            if (IsTargetInRange() && GetPackSize() > 1)
            {
                UpdateHuntPhase(ECombat_HuntPhase::Positioning);
            }
            break;
            
        case ECombat_HuntPhase::Positioning:
            // Assign flanking positions
            AssignPositions();
            CheckPackReadiness();
            if (IsPackReady())
            {
                UpdateHuntPhase(ECombat_HuntPhase::Coordinated_Attack);
            }
            break;
            
        case ECombat_HuntPhase::Coordinated_Attack:
            // Execute synchronized attack
            float CurrentTime = GetWorld()->GetTimeSeconds();
            if (CurrentTime - LastAttackTime > AttackCooldown)
            {
                ExecuteCoordinatedAttack();
                LastAttackTime = CurrentTime;
            }
            break;
            
        case ECombat_HuntPhase::Feeding:
            // Post-hunt behavior
            break;
            
        case ECombat_HuntPhase::Retreat:
            // Retreat and regroup
            UpdateHuntPhase(ECombat_HuntPhase::Patrol);
            break;
    }
}

void UCombat_PackHuntingAI::InitializePack(APawn* AlphaPawn_Input)
{
    if (!AlphaPawn_Input)
    {
        UE_LOG(LogTemp, Error, TEXT("Combat_PackHuntingAI: Cannot initialize pack with null Alpha"));
        return;
    }
    
    AlphaPawn = AlphaPawn_Input;
    PackMembers.Empty();
    
    // Add alpha as first pack member
    FCombat_PackMember AlphaMember;
    AlphaMember.MemberPawn = AlphaPawn_Input;
    AlphaMember.Role = ECombat_PackRole::Alpha;
    AlphaMember.AssignedPosition = AlphaPawn_Input->GetActorLocation();
    AlphaMember.bIsInPosition = true;
    
    PackMembers.Add(AlphaMember);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Pack initialized with Alpha: %s"), *AlphaPawn_Input->GetName());
}

void UCombat_PackHuntingAI::AddPackMember(APawn* MemberPawn, ECombat_PackRole Role)
{
    if (!MemberPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("Combat_PackHuntingAI: Cannot add null pack member"));
        return;
    }
    
    // Check if already in pack
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn == MemberPawn)
        {
            UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Pawn %s already in pack"), *MemberPawn->GetName());
            return;
        }
    }
    
    FCombat_PackMember NewMember;
    NewMember.MemberPawn = MemberPawn;
    NewMember.Role = Role;
    NewMember.AssignedPosition = MemberPawn->GetActorLocation();
    NewMember.bIsInPosition = false;
    NewMember.DistanceToTarget = 0.0f;
    
    PackMembers.Add(NewMember);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Added pack member: %s with role: %d"), 
           *MemberPawn->GetName(), (int32)Role);
}

void UCombat_PackHuntingAI::RemovePackMember(APawn* MemberPawn)
{
    if (!MemberPawn)
    {
        return;
    }
    
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (PackMembers[i].MemberPawn == MemberPawn)
        {
            PackMembers.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Removed pack member: %s"), *MemberPawn->GetName());
            break;
        }
    }
}

void UCombat_PackHuntingAI::SetHuntTarget(APawn* TargetPawn)
{
    if (!TargetPawn)
    {
        CurrentTarget.bIsValidTarget = false;
        CurrentTarget.TargetPawn = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Hunt target cleared"));
        return;
    }
    
    CurrentTarget.TargetPawn = TargetPawn;
    CurrentTarget.LastKnownPosition = TargetPawn->GetActorLocation();
    CurrentTarget.ThreatLevel = 1.0f; // Default threat level
    CurrentTarget.EstimatedHealth = 100.0f;
    CurrentTarget.bIsValidTarget = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Hunt target set to: %s"), *TargetPawn->GetName());
}

void UCombat_PackHuntingAI::StartHunt()
{
    if (!CurrentTarget.bIsValidTarget)
    {
        UE_LOG(LogTemp, Error, TEXT("Combat_PackHuntingAI: Cannot start hunt without valid target"));
        return;
    }
    
    if (GetPackSize() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Pack too small for coordinated hunt"));
        return;
    }
    
    UpdateHuntPhase(ECombat_HuntPhase::Tracking);
    BroadcastPackSignal(TEXT("Hunt Initiated"));
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Hunt started with %d pack members"), GetPackSize());
}

void UCombat_PackHuntingAI::UpdateHuntPhase(ECombat_HuntPhase NewPhase)
{
    if (CurrentHuntPhase == NewPhase)
    {
        return;
    }
    
    ECombat_HuntPhase OldPhase = CurrentHuntPhase;
    CurrentHuntPhase = NewPhase;
    
    FString PhaseNames[] = {
        TEXT("Patrol"), TEXT("Tracking"), TEXT("Positioning"), 
        TEXT("Coordinated_Attack"), TEXT("Feeding"), TEXT("Retreat")
    };
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Hunt phase changed from %s to %s"), 
           *PhaseNames[(int32)OldPhase], *PhaseNames[(int32)NewPhase]);
    
    BroadcastPackSignal(FString::Printf(TEXT("Phase: %s"), *PhaseNames[(int32)NewPhase]));
}

void UCombat_PackHuntingAI::AssignPositions()
{
    if (!CurrentTarget.bIsValidTarget || !CurrentTarget.TargetPawn.IsValid())
    {
        return;
    }
    
    FVector TargetLocation = CurrentTarget.TargetPawn->GetActorLocation();
    
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (!Member.MemberPawn.IsValid())
        {
            continue;
        }
        
        FVector AssignedPos = GetOptimalAttackPosition(Member.Role);
        Member.AssignedPosition = TargetLocation + AssignedPos;
        Member.bIsInPosition = false;
        
        // Calculate distance to assigned position
        FVector CurrentPos = Member.MemberPawn->GetActorLocation();
        float DistanceToAssigned = FVector::Dist(CurrentPos, Member.AssignedPosition);
        Member.bIsInPosition = (DistanceToAssigned < 100.0f); // Within 1 meter
        
        UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Assigned position to %s at %s"), 
               *Member.MemberPawn->GetName(), *Member.AssignedPosition.ToString());
    }
}

void UCombat_PackHuntingAI::ExecuteCoordinatedAttack()
{
    if (!CurrentTarget.bIsValidTarget || !CurrentTarget.TargetPawn.IsValid())
    {
        return;
    }
    
    BroadcastPackSignal(TEXT("Attack Now!"));
    
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (!Member.MemberPawn.IsValid())
        {
            continue;
        }
        
        // Simulate attack command to AI controller
        if (AAIController* AIController = Cast<AAIController>(Member.MemberPawn->GetController()))
        {
            // In a real implementation, this would trigger attack behavior trees
            UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: %s executing attack on target"), 
                   *Member.MemberPawn->GetName());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Coordinated attack executed by %d pack members"), 
           GetPackSize());
}

void UCombat_PackHuntingAI::BroadcastPackSignal(const FString& Signal)
{
    UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Broadcasting signal to pack: %s"), *Signal);
    
    // In a real implementation, this would send signals to individual AI controllers
    // or trigger audio/visual cues for pack communication
}

void UCombat_PackHuntingAI::ReportMemberStatus(APawn* MemberPawn, bool bInPosition)
{
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn == MemberPawn)
        {
            Member.bIsInPosition = bInPosition;
            UE_LOG(LogTemp, Log, TEXT("Combat_PackHuntingAI: Member %s reported position status: %s"), 
                   *MemberPawn->GetName(), bInPosition ? TEXT("In Position") : TEXT("Moving"));
            break;
        }
    }
}

bool UCombat_PackHuntingAI::IsPackReady() const
{
    if (PackMembers.Num() < 2)
    {
        return false;
    }
    
    int32 ReadyMembers = 0;
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.bIsInPosition)
        {
            ReadyMembers++;
        }
    }
    
    // Pack is ready if at least 75% of members are in position
    float ReadyPercentage = (float)ReadyMembers / (float)PackMembers.Num();
    return ReadyPercentage >= 0.75f;
}

void UCombat_PackHuntingAI::UpdatePackPositions()
{
    if (!CurrentTarget.bIsValidTarget || !CurrentTarget.TargetPawn.IsValid())
    {
        return;
    }
    
    // Update target's last known position
    CurrentTarget.LastKnownPosition = CurrentTarget.TargetPawn->GetActorLocation();
    
    // Update distance calculations for each pack member
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn.IsValid())
        {
            FVector MemberLocation = Member.MemberPawn->GetActorLocation();
            Member.DistanceToTarget = FVector::Dist(MemberLocation, CurrentTarget.LastKnownPosition);
        }
    }
}

void UCombat_PackHuntingAI::CheckPackReadiness()
{
    // Update position status for all members
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (!Member.MemberPawn.IsValid())
        {
            continue;
        }
        
        FVector CurrentPos = Member.MemberPawn->GetActorLocation();
        float DistanceToAssigned = FVector::Dist(CurrentPos, Member.AssignedPosition);
        Member.bIsInPosition = (DistanceToAssigned < 150.0f); // Within 1.5 meters
    }
}

void UCombat_PackHuntingAI::CalculateFlankingPositions()
{
    // This method calculates optimal flanking positions based on target location and terrain
    // Implementation would involve pathfinding and tactical positioning algorithms
}

FVector UCombat_PackHuntingAI::GetOptimalAttackPosition(ECombat_PackRole Role)
{
    FVector BaseOffset = FVector::ZeroVector;
    
    switch (Role)
    {
        case ECombat_PackRole::Alpha:
            BaseOffset = FVector(0.0f, 0.0f, 0.0f); // Direct approach
            break;
        case ECombat_PackRole::Beta:
            BaseOffset = FVector(-200.0f, 0.0f, 0.0f); // Behind alpha
            break;
        case ECombat_PackRole::Hunter:
            BaseOffset = FVector(0.0f, FlankingDistance, 0.0f); // Right flank
            break;
        case ECombat_PackRole::Flanker:
            BaseOffset = FVector(0.0f, -FlankingDistance, 0.0f); // Left flank
            break;
        case ECombat_PackRole::Scout:
            BaseOffset = FVector(FlankingDistance, 0.0f, 0.0f); // Forward position
            break;
    }
    
    return BaseOffset;
}

bool UCombat_PackHuntingAI::IsTargetInRange() const
{
    if (!CurrentTarget.bIsValidTarget || !CurrentTarget.TargetPawn.IsValid() || !AlphaPawn.IsValid())
    {
        return false;
    }
    
    FVector AlphaLocation = AlphaPawn->GetActorLocation();
    FVector TargetLocation = CurrentTarget.TargetPawn->GetActorLocation();
    float DistanceToTarget = FVector::Dist(AlphaLocation, TargetLocation);
    
    return DistanceToTarget <= HuntRange;
}

void UCombat_PackHuntingAI::CleanupInvalidMembers()
{
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (!PackMembers[i].MemberPawn.IsValid())
        {
            PackMembers.RemoveAt(i);
        }
    }
    
    // Update alpha reference if invalid
    if (!AlphaPawn.IsValid() && PackMembers.Num() > 0)
    {
        // Promote first valid member to alpha
        for (FCombat_PackMember& Member : PackMembers)
        {
            if (Member.MemberPawn.IsValid())
            {
                AlphaPawn = Member.MemberPawn;
                Member.Role = ECombat_PackRole::Alpha;
                UE_LOG(LogTemp, Warning, TEXT("Combat_PackHuntingAI: Promoted %s to new Alpha"), 
                       *Member.MemberPawn->GetName());
                break;
            }
        }
    }
}