#include "Combat_PackCoordination.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UCombat_PackCoordination::UCombat_PackCoordination()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    PackLeader = nullptr;
    CurrentTarget = nullptr;
    CurrentTacticalState = ECombat_TacticalState::Idle;
    MaxPackSize = 6;
    
    FormationRadius = 800.0f;
    PositionTolerance = 150.0f;
    StallingDistance = 1500.0f;
    AttackRange = 400.0f;
    FlankingAngle = 120.0f;
    CoordinationDelay = 2.0f;
    bUseAdvancedTactics = true;
    
    CoordinationTimer = 0.0f;
    LastCommandTime = 0.0f;
    FormationUpdateTimer = 0.0f;

    // Setup default formations
    FCombat_AttackFormation PincerFormation;
    PincerFormation.FormationName = TEXT("Pincer");
    PincerFormation.RelativePositions = {
        FVector(0, 0, 0),      // Alpha center
        FVector(-300, 400, 0), // Left flanker
        FVector(300, 400, 0),  // Right flanker
        FVector(0, -600, 0)    // Rear ambusher
    };
    PincerFormation.MinDistanceToTarget = 300.0f;
    PincerFormation.MaxDistanceToTarget = 1000.0f;
    AvailableFormations.Add(PincerFormation);

    FCombat_AttackFormation CircleFormation;
    CircleFormation.FormationName = TEXT("Circle");
    CircleFormation.RelativePositions = {
        FVector(400, 0, 0),    // Front
        FVector(200, 346, 0),  // Front-right
        FVector(-200, 346, 0), // Front-left
        FVector(-400, 0, 0),   // Rear
        FVector(-200, -346, 0), // Rear-left
        FVector(200, -346, 0)  // Rear-right
    };
    CircleFormation.MinDistanceToTarget = 500.0f;
    CircleFormation.MaxDistanceToTarget = 1200.0f;
    AvailableFormations.Add(CircleFormation);

    if (AvailableFormations.Num() > 0)
    {
        CurrentFormation = AvailableFormations[0];
    }
}

void UCombat_PackCoordination::BeginPlay()
{
    Super::BeginPlay();
    
    // Try to find pack communication component
    if (AActor* Owner = GetOwner())
    {
        PackCommunicationComponent = Owner->FindComponentByClass<UNPC_PackCommunication>();
    }
    
    // Set this pawn as pack leader if none exists
    if (!PackLeader && GetOwner())
    {
        PackLeader = Cast<APawn>(GetOwner());
    }
}

void UCombat_PackCoordination::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePackCoordination(DeltaTime);
}

bool UCombat_PackCoordination::AddPackMember(APawn* NewMember, ECombat_PackRole Role)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize)
    {
        return false;
    }
    
    // Check if member already exists
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn == NewMember)
        {
            return false;
        }
    }
    
    FCombat_PackMember NewPackMember;
    NewPackMember.MemberPawn = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.bInPosition = false;
    NewPackMember.DistanceToTarget = 0.0f;
    
    PackMembers.Add(NewPackMember);
    
    // Broadcast pack join message
    if (PackCommunicationComponent)
    {
        BroadcastTacticalCommand(TEXT("NewMemberJoined"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Pack member added: %s with role %d"), 
           NewMember ? *NewMember->GetName() : TEXT("NULL"), (int32)Role);
    
    return true;
}

bool UCombat_PackCoordination::RemovePackMember(APawn* Member)
{
    if (!Member)
    {
        return false;
    }
    
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (PackMembers[i].MemberPawn == Member)
        {
            PackMembers.RemoveAt(i);
            
            // Broadcast pack leave message
            if (PackCommunicationComponent)
            {
                BroadcastTacticalCommand(TEXT("MemberLeft"));
            }
            
            UE_LOG(LogTemp, Log, TEXT("Pack member removed: %s"), *Member->GetName());
            return true;
        }
    }
    
    return false;
}

void UCombat_PackCoordination::SetPackTarget(APawn* Target)
{
    if (CurrentTarget != Target)
    {
        CurrentTarget = Target;
        
        if (Target)
        {
            CurrentTacticalState = ECombat_TacticalState::Stalking;
            CalculateFormationPositions();
            
            // Broadcast target acquired
            if (PackCommunicationComponent)
            {
                BroadcastTacticalCommand(TEXT("TargetAcquired"));
            }
            
            UE_LOG(LogTemp, Log, TEXT("Pack target set: %s"), *Target->GetName());
        }
        else
        {
            CurrentTacticalState = ECombat_TacticalState::Idle;
        }
    }
}

void UCombat_PackCoordination::SetFormation(const FCombat_AttackFormation& NewFormation)
{
    CurrentFormation = NewFormation;
    CalculateFormationPositions();
    
    UE_LOG(LogTemp, Log, TEXT("Formation changed to: %s"), *NewFormation.FormationName);
}

void UCombat_PackCoordination::CalculateFormationPositions()
{
    if (!CurrentTarget || PackMembers.Num() == 0)
    {
        return;
    }
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector LeaderLocation = PackLeader ? PackLeader->GetActorLocation() : TargetLocation;
    
    // Calculate formation center (between leader and target)
    FVector FormationCenter = FMath::Lerp(LeaderLocation, TargetLocation, 0.7f);
    
    // Assign positions based on formation
    for (int32 i = 0; i < PackMembers.Num() && i < CurrentFormation.RelativePositions.Num(); i++)
    {
        FVector RelativePos = CurrentFormation.RelativePositions[i];
        
        // Rotate relative position to face target
        FVector DirectionToTarget = (TargetLocation - FormationCenter).GetSafeNormal();
        FRotator FormationRotation = DirectionToTarget.Rotation();
        FVector RotatedPosition = FormationRotation.RotateVector(RelativePos);
        
        PackMembers[i].AssignedPosition = FormationCenter + RotatedPosition;
        PackMembers[i].bInPosition = false;
        
        // Calculate distance to target for this position
        PackMembers[i].DistanceToTarget = FVector::Dist(PackMembers[i].AssignedPosition, TargetLocation);
    }
}

void UCombat_PackCoordination::InitiateCoordinatedAttack()
{
    if (!CurrentTarget || !IsPackInPosition())
    {
        return;
    }
    
    CurrentTacticalState = ECombat_TacticalState::Coordinated;
    
    // Broadcast attack command
    if (PackCommunicationComponent)
    {
        BroadcastTacticalCommand(TEXT("CoordinatedAttack"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Coordinated attack initiated!"));
}

void UCombat_PackCoordination::ExecuteFlankingManeuver()
{
    if (!CurrentTarget)
    {
        return;
    }
    
    CurrentTacticalState = ECombat_TacticalState::Flanking;
    
    // Assign flanking positions to pack members
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (Member.Role == ECombat_PackRole::Flanker || Member.Role == ECombat_PackRole::Ambusher)
        {
            Member.AssignedPosition = CalculateFlankingPosition(Member.MemberPawn);
            Member.bInPosition = false;
        }
    }
    
    // Broadcast flanking command
    if (PackCommunicationComponent)
    {
        BroadcastTacticalCommand(TEXT("FlankingManeuver"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Flanking maneuver executed"));
}

void UCombat_PackCoordination::OrderRetreat()
{
    CurrentTacticalState = ECombat_TacticalState::Retreating;
    
    // Calculate retreat positions (away from target)
    if (CurrentTarget && PackLeader)
    {
        FVector RetreatDirection = (PackLeader->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector RetreatCenter = PackLeader->GetActorLocation() + RetreatDirection * 1000.0f;
        
        for (int32 i = 0; i < PackMembers.Num(); i++)
        {
            FVector RandomOffset = FMath::VRand() * 300.0f;
            RandomOffset.Z = 0.0f;
            PackMembers[i].AssignedPosition = RetreatCenter + RandomOffset;
            PackMembers[i].bInPosition = false;
        }
    }
    
    // Broadcast retreat command
    if (PackCommunicationComponent)
    {
        BroadcastTacticalCommand(TEXT("Retreat"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Pack retreat ordered"));
}

void UCombat_PackCoordination::RegroupPack()
{
    CurrentTacticalState = ECombat_TacticalState::Regrouping;
    
    if (PackLeader)
    {
        FVector RegroupCenter = PackLeader->GetActorLocation();
        
        for (int32 i = 0; i < PackMembers.Num(); i++)
        {
            float Angle = (360.0f / PackMembers.Num()) * i;
            FVector Offset = FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * FormationRadius * 0.5f,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * FormationRadius * 0.5f,
                0.0f
            );
            
            PackMembers[i].AssignedPosition = RegroupCenter + Offset;
            PackMembers[i].bInPosition = false;
        }
    }
    
    // Broadcast regroup command
    if (PackCommunicationComponent)
    {
        BroadcastTacticalCommand(TEXT("Regroup"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Pack regroup initiated"));
}

void UCombat_PackCoordination::BroadcastTacticalCommand(const FString& Command)
{
    if (PackCommunicationComponent)
    {
        // Integration with pack communication system
        // This would send tactical commands through the communication system
        UE_LOG(LogTemp, Log, TEXT("Broadcasting tactical command: %s"), *Command);
    }
}

void UCombat_PackCoordination::UpdatePackCoordination(float DeltaTime)
{
    CoordinationTimer += DeltaTime;
    FormationUpdateTimer += DeltaTime;
    
    // Update formation positions periodically
    if (FormationUpdateTimer >= 1.0f)
    {
        CheckFormationPositions();
        FormationUpdateTimer = 0.0f;
    }
    
    // Execute tactical AI
    if (CoordinationTimer >= 0.5f)
    {
        ExecuteTacticalAI();
        CoordinationTimer = 0.0f;
    }
    
    UpdateTacticalState();
}

void UCombat_PackCoordination::CheckFormationPositions()
{
    for (FCombat_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn && !Member.AssignedPosition.IsZero())
        {
            float DistanceToPosition = FVector::Dist(Member.MemberPawn->GetActorLocation(), Member.AssignedPosition);
            Member.bInPosition = (DistanceToPosition <= PositionTolerance);
        }
    }
}

void UCombat_PackCoordination::UpdateTacticalState()
{
    if (!CurrentTarget)
    {
        if (CurrentTacticalState != ECombat_TacticalState::Idle)
        {
            CurrentTacticalState = ECombat_TacticalState::Idle;
        }
        return;
    }
    
    // State transitions based on pack status and target distance
    float DistanceToTarget = PackLeader ? FVector::Dist(PackLeader->GetActorLocation(), CurrentTarget->GetActorLocation()) : 0.0f;
    
    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Stalking:
            if (DistanceToTarget <= StallingDistance)
            {
                CurrentTacticalState = ECombat_TacticalState::Positioning;
                CalculateFormationPositions();
            }
            break;
            
        case ECombat_TacticalState::Positioning:
            if (IsPackInPosition() && DistanceToTarget <= AttackRange)
            {
                InitiateCoordinatedAttack();
            }
            break;
            
        case ECombat_TacticalState::Coordinated:
            if (DistanceToTarget > AttackRange * 2.0f)
            {
                CurrentTacticalState = ECombat_TacticalState::Stalking;
            }
            break;
    }
}

bool UCombat_PackCoordination::IsPackInPosition()
{
    int32 MembersInPosition = 0;
    for (const FCombat_PackMember& Member : PackMembers)
    {
        if (Member.bInPosition)
        {
            MembersInPosition++;
        }
    }
    
    return (PackMembers.Num() > 0) && (MembersInPosition >= FMath::Max(1, PackMembers.Num() * 0.7f));
}

bool UCombat_PackCoordination::HasLineOfSightToTarget(APawn* Member)
{
    if (!Member || !CurrentTarget)
    {
        return false;
    }
    
    FVector Start = Member->GetActorLocation();
    FVector End = CurrentTarget->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Member);
    QueryParams.AddIgnoredActor(CurrentTarget);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    
    return !bHit;
}

FVector UCombat_PackCoordination::CalculateFlankingPosition(APawn* Member)
{
    if (!Member || !CurrentTarget || !PackLeader)
    {
        return FVector::ZeroVector;
    }
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector LeaderLocation = PackLeader->GetActorLocation();
    
    // Calculate flanking angle
    FVector DirectionToTarget = (TargetLocation - LeaderLocation).GetSafeNormal();
    FVector FlankDirection = DirectionToTarget.RotateAngleAxis(FlankingAngle, FVector::UpVector);
    
    // Position flanker at flanking distance
    return TargetLocation + FlankDirection * (AttackRange * 1.5f);
}

void UCombat_PackCoordination::ExecuteTacticalAI()
{
    if (!bUseAdvancedTactics || !CurrentTarget)
    {
        return;
    }
    
    // Advanced tactical decision making
    float DistanceToTarget = PackLeader ? FVector::Dist(PackLeader->GetActorLocation(), CurrentTarget->GetActorLocation()) : 0.0f;
    
    // Decide on tactical maneuvers based on situation
    if (CurrentTacticalState == ECombat_TacticalState::Positioning)
    {
        // Check if flanking would be beneficial
        bool bShouldFlank = (PackMembers.Num() >= 3) && (DistanceToTarget > AttackRange * 0.8f);
        
        if (bShouldFlank && GetWorld()->GetTimeSeconds() - LastCommandTime > CoordinationDelay)
        {
            ExecuteFlankingManeuver();
            LastCommandTime = GetWorld()->GetTimeSeconds();
        }
    }
}