#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/HealthComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance

    // Initialize tactical info with default values
    TacticalInfo.CurrentState = ECombat_TacticalState::Idle;
    TacticalInfo.Role = ECombat_DinosaurRole::Hunter;
    TacticalInfo.AggressionLevel = 0.5f;
    TacticalInfo.FearLevel = 0.0f;
    TacticalInfo.PackCoordination = 0.7f;

    // Initialize pack info
    PackInfo.FormationRadius = 500.0f;
    PackInfo.bIsCoordinatedAttack = false;

    // Combat settings
    AttackRange = 200.0f;
    DetectionRange = 1000.0f;
    FlankingDistance = 300.0f;
    RetreatHealthThreshold = 0.3f;

    // Internal timers
    StateTimer = 0.0f;
    LastDecisionTime = 0.0f;
    DecisionCooldown = 1.0f;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAIController();
    
    // Set initial state based on role
    switch (TacticalInfo.Role)
    {
        case ECombat_DinosaurRole::Alpha:
            TacticalInfo.AggressionLevel = 0.8f;
            TacticalInfo.PackCoordination = 1.0f;
            break;
        case ECombat_DinosaurRole::Scout:
            TacticalInfo.AggressionLevel = 0.3f;
            DetectionRange = 1500.0f;
            break;
        case ECombat_DinosaurRole::Ambusher:
            TacticalInfo.AggressionLevel = 0.6f;
            AttackRange = 150.0f;
            break;
        default:
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Combat_TacticalAI initialized for %s with role %s"), 
           *GetOwner()->GetName(), 
           *UEnum::GetValueAsString(TacticalInfo.Role));
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner() || !AIController)
    {
        return;
    }

    // Update tactical state and decisions
    UpdateTacticalState(DeltaTime);
    
    // Update pack coordination if we're part of a pack
    if (PackInfo.PackLeader || IsPackLeader())
    {
        UpdatePackFormation();
    }

    // Update blackboard values for behavior tree
    UpdateBlackboardValues();

    StateTimer += DeltaTime;
}

void UCombat_TacticalAI::UpdateTacticalState(float DeltaTime)
{
    if (GetWorld()->GetTimeSeconds() - LastDecisionTime < DecisionCooldown)
    {
        return;
    }

    // Update target information if we have one
    if (CurrentTarget)
    {
        UpdateTargetInfo(CurrentTarget);
    }

    // Determine the best tactical state based on current situation
    ECombat_TacticalState NewState = DetermineBestTacticalState();

    if (NewState != TacticalInfo.CurrentState)
    {
        HandleStateTransition(NewState);
    }

    ProcessTacticalDecision(DeltaTime);
    LastDecisionTime = GetWorld()->GetTimeSeconds();
}

void UCombat_TacticalAI::SetTarget(AActor* NewTarget)
{
    if (NewTarget != CurrentTarget)
    {
        CurrentTarget = NewTarget;
        
        if (CurrentTarget)
        {
            TacticalInfo.LastKnownTargetLocation = CurrentTarget->GetActorLocation();
            TacticalInfo.TimeSinceLastSighting = 0.0f;
            TacticalInfo.bIsInCombat = true;
            
            UE_LOG(LogTemp, Log, TEXT("%s acquired new target: %s"), 
                   *GetOwner()->GetName(), *CurrentTarget->GetName());
        }
        else
        {
            TacticalInfo.bIsInCombat = false;
            TacticalInfo.bCanSeeTarget = false;
        }
    }
}

void UCombat_TacticalAI::UpdateTargetInfo(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    // Update distance to target
    TacticalInfo.DistanceToTarget = GetDistanceToTarget(Target);
    
    // Update line of sight
    TacticalInfo.bCanSeeTarget = HasLineOfSight(Target);
    
    if (TacticalInfo.bCanSeeTarget)
    {
        TacticalInfo.LastKnownTargetLocation = Target->GetActorLocation();
        TacticalInfo.TimeSinceLastSighting = 0.0f;
    }
    else
    {
        TacticalInfo.TimeSinceLastSighting += GetWorld()->GetDeltaSeconds();
    }
}

ECombat_TacticalState UCombat_TacticalAI::DetermineBestTacticalState()
{
    // Check for retreat conditions first
    if (ShouldRetreat())
    {
        return ECombat_TacticalState::Retreat;
    }

    // If we don't have a target, patrol or idle
    if (!CurrentTarget || !IsValidTarget(CurrentTarget))
    {
        return (TacticalInfo.Role == ECombat_DinosaurRole::Scout) ? 
               ECombat_TacticalState::Patrol : ECombat_TacticalState::Idle;
    }

    // Target-based decision making
    float DistanceToTarget = TacticalInfo.DistanceToTarget;
    bool CanSeeTarget = TacticalInfo.bCanSeeTarget;

    // If target is very close and we can see them, attack
    if (DistanceToTarget <= AttackRange && CanSeeTarget && ShouldAttack(CurrentTarget))
    {
        return ECombat_TacticalState::Attack;
    }

    // If we should flank, do so
    if (ShouldFlank(CurrentTarget))
    {
        return ECombat_TacticalState::Flank;
    }

    // If we can see the target but they're not in attack range
    if (CanSeeTarget && DistanceToTarget > AttackRange)
    {
        // Ambushers prefer to wait for the right moment
        if (TacticalInfo.Role == ECombat_DinosaurRole::Ambusher)
        {
            return ECombat_TacticalState::Ambush;
        }
        else
        {
            return ECombat_TacticalState::Hunt;
        }
    }

    // If we lost sight of the target recently, stalk them
    if (TacticalInfo.TimeSinceLastSighting < 10.0f)
    {
        return ECombat_TacticalState::Stalk;
    }

    // Default to patrol for scouts, idle for others
    return (TacticalInfo.Role == ECombat_DinosaurRole::Scout) ? 
           ECombat_TacticalState::Patrol : ECombat_TacticalState::Idle;
}

FVector UCombat_TacticalAI::CalculateFlankingPosition(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return GetOwner()->GetActorLocation();
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate direction from target to owner
    FVector DirectionToOwner = (OwnerLocation - TargetLocation).GetSafeNormal();
    
    // Create a perpendicular vector for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToOwner, FVector::UpVector).GetSafeNormal();
    
    // Randomly choose left or right flank
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    // Calculate flanking position
    FVector FlankPosition = TargetLocation + (FlankDirection * FlankingDistance);
    
    // Ensure the position is on the ground
    FHitResult HitResult;
    FVector TraceStart = FlankPosition + FVector(0, 0, 1000);
    FVector TraceEnd = FlankPosition - FVector(0, 0, 1000);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        FlankPosition = HitResult.Location;
    }
    
    return FlankPosition;
}

FVector UCombat_TacticalAI::CalculateAmbushPosition(AActor* Target)
{
    if (!Target || !GetOwner())
    {
        return GetOwner()->GetActorLocation();
    }

    // For ambush, find a position that's hidden but close to the target's path
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Try to find cover near the target
    TArray<FHitResult> HitResults;
    FVector SearchCenter = TargetLocation;
    
    // Search for cover in a radius around the target
    for (int32 i = 0; i < 8; i++)
    {
        float Angle = (i / 8.0f) * 2.0f * PI;
        FVector SearchDirection = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0);
        FVector SearchPosition = SearchCenter + (SearchDirection * AttackRange * 0.8f);
        
        // Check if this position provides cover
        FHitResult CoverCheck;
        if (GetWorld()->LineTraceSingleByChannel(CoverCheck, SearchPosition, TargetLocation, ECC_WorldStatic))
        {
            return SearchPosition; // Found cover
        }
    }
    
    // If no cover found, return a position slightly behind the target
    FVector TargetForward = Target->GetActorForwardVector();
    return TargetLocation - (TargetForward * AttackRange * 0.5f);
}

void UCombat_TacticalAI::JoinPack(UCombat_TacticalAI* PackLeaderAI)
{
    if (!PackLeaderAI || PackLeaderAI == this)
    {
        return;
    }

    // Leave current pack if we're in one
    LeavePack();

    // Join new pack
    PackInfo.PackLeader = PackLeaderAI->GetOwner();
    PackLeaderAI->PackInfo.PackMembers.AddUnique(GetOwner());

    UE_LOG(LogTemp, Log, TEXT("%s joined pack led by %s"), 
           *GetOwner()->GetName(), *PackInfo.PackLeader->GetName());
}

void UCombat_TacticalAI::LeavePack()
{
    if (PackInfo.PackLeader)
    {
        // Find the pack leader's AI component and remove ourselves
        if (UCombat_TacticalAI* LeaderAI = PackInfo.PackLeader->FindComponentByClass<UCombat_TacticalAI>())
        {
            LeaderAI->PackInfo.PackMembers.Remove(GetOwner());
        }
        
        PackInfo.PackLeader = nullptr;
        UE_LOG(LogTemp, Log, TEXT("%s left pack"), *GetOwner()->GetName());
    }

    // If we were leading a pack, disband it
    if (IsPackLeader())
    {
        for (AActor* Member : PackInfo.PackMembers)
        {
            if (UCombat_TacticalAI* MemberAI = Member->FindComponentByClass<UCombat_TacticalAI>())
            {
                MemberAI->PackInfo.PackLeader = nullptr;
            }
        }
        PackInfo.PackMembers.Empty();
    }
}

void UCombat_TacticalAI::CoordinatePackAttack(AActor* Target)
{
    if (!IsPackLeader() || !Target)
    {
        return;
    }

    PackInfo.bIsCoordinatedAttack = true;
    PackInfo.CoordinationTimer = 5.0f; // Coordinate for 5 seconds

    // Assign roles to pack members
    for (int32 i = 0; i < PackInfo.PackMembers.Num(); i++)
    {
        if (UCombat_TacticalAI* MemberAI = PackInfo.PackMembers[i]->FindComponentByClass<UCombat_TacticalAI>())
        {
            MemberAI->SetTarget(Target);
            
            // Assign different tactical states based on position
            if (i % 3 == 0)
            {
                MemberAI->TacticalInfo.CurrentState = ECombat_TacticalState::Flank;
            }
            else if (i % 3 == 1)
            {
                MemberAI->TacticalInfo.CurrentState = ECombat_TacticalState::Attack;
            }
            else
            {
                MemberAI->TacticalInfo.CurrentState = ECombat_TacticalState::Ambush;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Pack leader %s coordinating attack on %s with %d members"), 
           *GetOwner()->GetName(), *Target->GetName(), PackInfo.PackMembers.Num());
}

FVector UCombat_TacticalAI::GetPackFormationPosition()
{
    if (!PackInfo.PackLeader)
    {
        return GetOwner()->GetActorLocation();
    }

    FVector LeaderLocation = PackInfo.PackLeader->GetActorLocation();
    
    // Find our index in the pack
    int32 MemberIndex = PackInfo.PackMembers.Find(GetOwner());
    if (MemberIndex == INDEX_NONE)
    {
        return LeaderLocation;
    }

    // Calculate formation position based on index
    float Angle = (MemberIndex / float(PackInfo.PackMembers.Num())) * 2.0f * PI;
    FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0) * PackInfo.FormationRadius;
    
    return LeaderLocation + Offset;
}

bool UCombat_TacticalAI::IsPackLeader() const
{
    return PackInfo.PackMembers.Num() > 0 && PackInfo.PackLeader == nullptr;
}

bool UCombat_TacticalAI::ShouldAttack(AActor* Target)
{
    if (!Target || !IsValidTarget(Target))
    {
        return false;
    }

    // Check distance
    if (TacticalInfo.DistanceToTarget > AttackRange)
    {
        return false;
    }

    // Check line of sight
    if (!TacticalInfo.bCanSeeTarget)
    {
        return false;
    }

    // Check aggression level and fear
    float AttackChance = TacticalInfo.AggressionLevel - TacticalInfo.FearLevel;
    
    // Pack coordination increases attack chance
    if (PackInfo.bIsCoordinatedAttack)
    {
        AttackChance += 0.3f;
    }

    return AttackChance > 0.5f;
}

bool UCombat_TacticalAI::ShouldRetreat()
{
    float HealthPercentage = GetHealthPercentage();
    
    // Retreat if health is low
    if (HealthPercentage < RetreatHealthThreshold)
    {
        return true;
    }

    // Retreat if fear level is too high
    if (TacticalInfo.FearLevel > 0.8f)
    {
        return true;
    }

    // Retreat if outnumbered significantly
    TArray<AActor*> NearbyEnemies = FindNearbyEnemies(DetectionRange);
    TArray<AActor*> NearbyAllies = FindNearbyAllies(DetectionRange);
    
    if (NearbyEnemies.Num() > NearbyAllies.Num() * 2 && NearbyEnemies.Num() > 2)
    {
        return true;
    }

    return false;
}

bool UCombat_TacticalAI::ShouldFlank(AActor* Target)
{
    if (!Target || TacticalInfo.Role == ECombat_DinosaurRole::Defender)
    {
        return false;
    }

    // Flank if we're in a good position and have pack support
    if (PackInfo.PackMembers.Num() > 1 && TacticalInfo.DistanceToTarget > AttackRange)
    {
        return true;
    }

    // Ambushers and scouts prefer flanking
    if (TacticalInfo.Role == ECombat_DinosaurRole::Ambusher || TacticalInfo.Role == ECombat_DinosaurRole::Scout)
    {
        return TacticalInfo.DistanceToTarget > AttackRange * 0.5f;
    }

    return false;
}

void UCombat_TacticalAI::ExecuteAttack(AActor* Target)
{
    if (!Target || !AIController)
    {
        return;
    }

    // Move to attack range if not already there
    if (TacticalInfo.DistanceToTarget > AttackRange)
    {
        AIController->MoveToActor(Target, AttackRange * 0.8f);
    }

    UE_LOG(LogTemp, Log, TEXT("%s executing attack on %s"), 
           *GetOwner()->GetName(), *Target->GetName());
}

void UCombat_TacticalAI::ExecuteRetreat()
{
    if (!AIController)
    {
        return;
    }

    FVector RetreatPosition = FindSafeRetreatPosition();
    AIController->MoveToLocation(RetreatPosition);

    UE_LOG(LogTemp, Log, TEXT("%s retreating to safety"), *GetOwner()->GetName());
}

void UCombat_TacticalAI::ExecuteFlank(AActor* Target)
{
    if (!Target || !AIController)
    {
        return;
    }

    FVector FlankPosition = CalculateFlankingPosition(Target);
    AIController->MoveToLocation(FlankPosition);

    UE_LOG(LogTemp, Log, TEXT("%s flanking %s"), 
           *GetOwner()->GetName(), *Target->GetName());
}

float UCombat_TacticalAI::GetHealthPercentage() const
{
    // Try to find a health component
    if (UActorComponent* HealthComp = GetOwner()->GetComponentByClass(UActorComponent::StaticClass()))
    {
        // This is a simplified version - in a real implementation, you'd have a proper health component
        return 1.0f; // Placeholder
    }
    
    return 1.0f; // Default to full health
}

float UCombat_TacticalAI::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return 0.0f;
    }

    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
}

bool UCombat_TacticalAI::HasLineOfSight(AActor* Target) const
{
    if (!Target || !GetOwner() || !GetWorld())
    {
        return false;
    }

    FVector StartLocation = GetOwner()->GetActorLocation() + FVector(0, 0, 100); // Eye level
    FVector EndLocation = Target->GetActorLocation() + FVector(0, 0, 100);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );

    return !bHit; // If no hit, we have line of sight
}

TArray<AActor*> UCombat_TacticalAI::FindNearbyAllies(float SearchRadius) const
{
    TArray<AActor*> Allies;
    
    if (!GetWorld() || !GetOwner())
    {
        return Allies;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
        {
            continue;
        }

        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= SearchRadius)
        {
            // Check if this actor has the same AI component (indicating it's an ally)
            if (Actor->FindComponentByClass<UCombat_TacticalAI>())
            {
                Allies.Add(Actor);
            }
        }
    }

    return Allies;
}

TArray<AActor*> UCombat_TacticalAI::FindNearbyEnemies(float SearchRadius) const
{
    TArray<AActor*> Enemies;
    
    if (!GetWorld() || !GetOwner())
    {
        return Enemies;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
        {
            continue;
        }

        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= SearchRadius)
        {
            // Check if this actor doesn't have our AI component (indicating it's an enemy)
            if (!Actor->FindComponentByClass<UCombat_TacticalAI>())
            {
                Enemies.Add(Actor);
            }
        }
    }

    return Enemies;
}

void UCombat_TacticalAI::InitializeAIController()
{
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        AIController = Cast<AAIController>(OwnerPawn->GetController());
        
        if (!AIController)
        {
            UE_LOG(LogTemp, Warning, TEXT("No AI Controller found for %s"), *GetOwner()->GetName());
        }
    }
}

void UCombat_TacticalAI::UpdateBlackboardValues()
{
    if (!AIController || !AIController->GetBlackboardComponent())
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();

    // Update target actor
    BlackboardComp->SetValueAsObject(TargetActorKey, CurrentTarget);

    // Update tactical state
    BlackboardComp->SetValueAsEnum(TacticalStateKey, static_cast<uint8>(TacticalInfo.CurrentState));

    // Update pack leader
    BlackboardComp->SetValueAsObject(PackLeaderKey, PackInfo.PackLeader);

    // Update flank position if flanking
    if (TacticalInfo.CurrentState == ECombat_TacticalState::Flank && CurrentTarget)
    {
        FVector FlankPos = CalculateFlankingPosition(CurrentTarget);
        BlackboardComp->SetValueAsVector(FlankPositionKey, FlankPos);
    }
}

void UCombat_TacticalAI::ProcessTacticalDecision(float DeltaTime)
{
    switch (TacticalInfo.CurrentState)
    {
        case ECombat_TacticalState::Attack:
            if (CurrentTarget)
            {
                ExecuteAttack(CurrentTarget);
            }
            break;

        case ECombat_TacticalState::Retreat:
            ExecuteRetreat();
            break;

        case ECombat_TacticalState::Flank:
            if (CurrentTarget)
            {
                ExecuteFlank(CurrentTarget);
            }
            break;

        case ECombat_TacticalState::Coordinate:
            if (IsPackLeader() && CurrentTarget)
            {
                CoordinatePackAttack(CurrentTarget);
            }
            break;

        default:
            break;
    }

    // Update pack coordination timer
    if (PackInfo.bIsCoordinatedAttack)
    {
        PackInfo.CoordinationTimer -= DeltaTime;
        if (PackInfo.CoordinationTimer <= 0.0f)
        {
            PackInfo.bIsCoordinatedAttack = false;
        }
    }
}

bool UCombat_TacticalAI::IsValidTarget(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }

    // Check if target is still alive and valid
    if (Target->IsPendingKill())
    {
        return false;
    }

    // Check if target is too far away
    if (TacticalInfo.DistanceToTarget > DetectionRange * 2.0f)
    {
        return false;
    }

    return true;
}

void UCombat_TacticalAI::HandleStateTransition(ECombat_TacticalState NewState)
{
    ECombat_TacticalState OldState = TacticalInfo.CurrentState;
    TacticalInfo.CurrentState = NewState;
    StateTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("%s transitioning from %s to %s"), 
           *GetOwner()->GetName(),
           *UEnum::GetValueAsString(OldState),
           *UEnum::GetValueAsString(NewState));
}

void UCombat_TacticalAI::UpdatePackFormation()
{
    if (!IsPackLeader())
    {
        return;
    }

    // Update formation center to our location
    PackInfo.FormationCenter = GetOwner()->GetActorLocation();

    // Adjust formation radius based on pack size
    float BaseRadius = 300.0f;
    float RadiusMultiplier = FMath::Sqrt(PackInfo.PackMembers.Num());
    PackInfo.FormationRadius = BaseRadius * RadiusMultiplier;
}

FVector UCombat_TacticalAI::FindSafeRetreatPosition() const
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector BestRetreatPos = OwnerLocation;
    float BestSafetyScore = 0.0f;

    // Try multiple directions to find the safest retreat position
    for (int32 i = 0; i < 8; i++)
    {
        float Angle = (i / 8.0f) * 2.0f * PI;
        FVector Direction = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0);
        FVector TestPosition = OwnerLocation + (Direction * 1000.0f);

        // Check if this position is safe
        if (IsPositionSafe(TestPosition))
        {
            // Calculate safety score based on distance from enemies
            float SafetyScore = 1000.0f; // Base score
            
            TArray<AActor*> NearbyEnemies = FindNearbyEnemies(2000.0f);
            for (AActor* Enemy : NearbyEnemies)
            {
                float DistanceToEnemy = FVector::Dist(TestPosition, Enemy->GetActorLocation());
                SafetyScore += DistanceToEnemy; // Farther from enemies = safer
            }

            if (SafetyScore > BestSafetyScore)
            {
                BestSafetyScore = SafetyScore;
                BestRetreatPos = TestPosition;
            }
        }
    }

    return BestRetreatPos;
}

bool UCombat_TacticalAI::IsPositionSafe(const FVector& Position) const
{
    if (!GetWorld())
    {
        return false;
    }

    // Check if the position is on solid ground
    FHitResult HitResult;
    FVector TraceStart = Position + FVector(0, 0, 500);
    FVector TraceEnd = Position - FVector(0, 0, 500);

    if (!GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        return false; // No ground found
    }

    // Check if there are too many enemies nearby
    TArray<AActor*> NearbyEnemies = FindNearbyEnemies(500.0f);
    if (NearbyEnemies.Num() > 2)
    {
        return false; // Too dangerous
    }

    return true;
}