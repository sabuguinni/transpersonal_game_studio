#include "Combat_TacticalAI.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "Components/StaticMeshComponent.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentState = ECombat_TacticalState::Idle;
    DinosaurType = ECombat_DinosaurType::Predator;
    CurrentTarget = nullptr;
    AIController = nullptr;
    BlackboardComponent = nullptr;
    StateChangeTimer = 0.0f;
    LastAttackTime = 0.0f;
    LastKnownTargetLocation = FVector::ZeroVector;
    bIsInPack = false;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Get AI Controller reference
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        AIController = Cast<AAIController>(OwnerPawn->GetController());
        if (AIController)
        {
            BlackboardComponent = AIController->GetBlackboardComponent();
        }
    }
    
    // Initialize with default tactical data
    FCombat_TacticalData DefaultData;
    InitializeTacticalAI(DinosaurType, DefaultData);
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateChangeTimer += DeltaTime;
    
    // Update tactical state and execute behavior
    UpdateTacticalState();
    ExecuteTacticalBehavior(DeltaTime);
    
    // Update blackboard values for behavior tree
    UpdateBlackboardValues();
}

void UCombat_TacticalAI::InitializeTacticalAI(ECombat_DinosaurType InDinosaurType, const FCombat_TacticalData& InTacticalData)
{
    DinosaurType = InDinosaurType;
    TacticalData = InTacticalData;
    CurrentState = ECombat_TacticalState::Idle;
    
    // Adjust tactical data based on dinosaur type
    switch (DinosaurType)
    {
        case ECombat_DinosaurType::Predator:
            TacticalData.AttackRange = 400.0f;
            TacticalData.RetreatHealthThreshold = 0.2f;
            break;
        case ECombat_DinosaurType::PackHunter:
            TacticalData.AttackRange = 250.0f;
            TacticalData.FlankingDistance = 600.0f;
            bIsInPack = true;
            break;
        case ECombat_DinosaurType::Herbivore:
            TacticalData.AttackRange = 200.0f;
            TacticalData.RetreatHealthThreshold = 0.5f;
            break;
        case ECombat_DinosaurType::Scavenger:
            TacticalData.AttackRange = 150.0f;
            TacticalData.AmbushWaitTime = 8.0f;
            break;
    }
}

void UCombat_TacticalAI::SetTarget(APawn* NewTarget)
{
    CurrentTarget = NewTarget;
    if (CurrentTarget)
    {
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        if (CurrentState == ECombat_TacticalState::Idle)
        {
            HandleStateTransition(ECombat_TacticalState::Hunting);
        }
    }
    else
    {
        HandleStateTransition(ECombat_TacticalState::Idle);
    }
}

void UCombat_TacticalAI::UpdateTacticalState()
{
    if (!ValidateTarget())
    {
        if (CurrentState != ECombat_TacticalState::Idle)
        {
            HandleStateTransition(ECombat_TacticalState::Idle);
        }
        return;
    }
    
    float DistanceToTarget = GetDistanceToTarget();
    bool bInAttackRange = IsTargetInAttackRange();
    bool bShouldRetreat = ShouldRetreat();
    bool bHasLineOfSight = HasClearLineOfSight();
    
    switch (CurrentState)
    {
        case ECombat_TacticalState::Idle:
            if (CurrentTarget)
            {
                HandleStateTransition(ECombat_TacticalState::Hunting);
            }
            break;
            
        case ECombat_TacticalState::Hunting:
            if (bShouldRetreat)
            {
                HandleStateTransition(ECombat_TacticalState::Retreating);
            }
            else if (bInAttackRange && bHasLineOfSight)
            {
                HandleStateTransition(ECombat_TacticalState::Attacking);
            }
            else if (DistanceToTarget < TacticalData.FlankingDistance && !bHasLineOfSight)
            {
                HandleStateTransition(ECombat_TacticalState::Stalking);
            }
            break;
            
        case ECombat_TacticalState::Stalking:
            if (bShouldRetreat)
            {
                HandleStateTransition(ECombat_TacticalState::Retreating);
            }
            else if (bInAttackRange)
            {
                HandleStateTransition(ECombat_TacticalState::Attacking);
            }
            else if (StateChangeTimer > 3.0f && DinosaurType == ECombat_DinosaurType::PackHunter)
            {
                HandleStateTransition(ECombat_TacticalState::Flanking);
            }
            break;
            
        case ECombat_TacticalState::Attacking:
            if (bShouldRetreat)
            {
                HandleStateTransition(ECombat_TacticalState::Retreating);
            }
            else if (!bInAttackRange)
            {
                HandleStateTransition(ECombat_TacticalState::Hunting);
            }
            break;
            
        case ECombat_TacticalState::Flanking:
            if (bShouldRetreat)
            {
                HandleStateTransition(ECombat_TacticalState::Retreating);
            }
            else if (bInAttackRange)
            {
                HandleStateTransition(ECombat_TacticalState::Attacking);
            }
            else if (StateChangeTimer > 5.0f)
            {
                HandleStateTransition(ECombat_TacticalState::Hunting);
            }
            break;
            
        case ECombat_TacticalState::Retreating:
            if (DistanceToTarget > TacticalData.FlankingDistance * 2.0f)
            {
                HandleStateTransition(ECombat_TacticalState::Idle);
            }
            break;
            
        case ECombat_TacticalState::Ambushing:
            if (StateChangeTimer > TacticalData.AmbushWaitTime)
            {
                HandleStateTransition(ECombat_TacticalState::Attacking);
            }
            break;
    }
}

void UCombat_TacticalAI::ExecuteTacticalBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
        case ECombat_TacticalState::Hunting:
            ExecuteHuntingBehavior();
            break;
        case ECombat_TacticalState::Stalking:
            ExecuteStalkingBehavior();
            break;
        case ECombat_TacticalState::Attacking:
            ExecuteAttackBehavior();
            break;
        case ECombat_TacticalState::Flanking:
            ExecuteFlankingBehavior();
            break;
        case ECombat_TacticalState::Retreating:
            ExecuteRetreatBehavior();
            break;
        case ECombat_TacticalState::Ambushing:
            ExecuteAmbushBehavior();
            break;
    }
}

void UCombat_TacticalAI::ExecuteHuntingBehavior()
{
    if (!AIController || !CurrentTarget) return;
    
    // Move towards target with direct path
    AIController->MoveToLocation(CurrentTarget->GetActorLocation(), TacticalData.AttackRange * 0.8f);
    
    // Update last known position
    LastKnownTargetLocation = CurrentTarget->GetActorLocation();
}

void UCombat_TacticalAI::ExecuteStalkingBehavior()
{
    if (!AIController || !CurrentTarget) return;
    
    // Move more cautiously, staying in cover
    FVector StalkPosition = LastKnownTargetLocation + FVector(FMath::RandRange(-200.0f, 200.0f), FMath::RandRange(-200.0f, 200.0f), 0.0f);
    AIController->MoveToLocation(StalkPosition, 100.0f);
}

void UCombat_TacticalAI::ExecuteAttackBehavior()
{
    if (!AIController || !CurrentTarget) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime > 2.0f) // Attack cooldown
    {
        // Execute attack
        AIController->MoveToLocation(CurrentTarget->GetActorLocation(), 50.0f);
        LastAttackTime = CurrentTime;
        
        // Trigger attack animation/damage here
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur attacking target!"));
    }
}

void UCombat_TacticalAI::ExecuteFlankingBehavior()
{
    if (!AIController || !CurrentTarget) return;
    
    FVector FlankPosition = CalculateFlankingPosition();
    AIController->MoveToLocation(FlankPosition, 100.0f);
    
    // Coordinate with pack if applicable
    if (bIsInPack)
    {
        CoordinatePackAttack();
    }
}

void UCombat_TacticalAI::ExecuteRetreatBehavior()
{
    if (!AIController || !CurrentTarget) return;
    
    // Move away from target
    FVector RetreatDirection = (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
    FVector RetreatPosition = GetOwner()->GetActorLocation() + RetreatDirection * 1000.0f;
    
    AIController->MoveToLocation(RetreatPosition, 50.0f);
}

void UCombat_TacticalAI::ExecuteAmbushBehavior()
{
    // Stay still and wait for target to come closer
    if (AIController)
    {
        AIController->StopMovement();
    }
}

void UCombat_TacticalAI::CoordinatePackAttack()
{
    TArray<UCombat_TacticalAI*> NearbyPack = GetNearbyPackMembers();
    
    for (UCombat_TacticalAI* PackMember : NearbyPack)
    {
        if (PackMember && PackMember != this)
        {
            // Signal pack member to flank from opposite side
            if (PackMember->CurrentState == ECombat_TacticalState::Hunting)
            {
                PackMember->HandleStateTransition(ECombat_TacticalState::Flanking);
            }
        }
    }
}

void UCombat_TacticalAI::JoinPackFormation()
{
    // Implementation for pack formation behavior
    bIsInPack = true;
}

TArray<UCombat_TacticalAI*> UCombat_TacticalAI::GetNearbyPackMembers(float SearchRadius)
{
    TArray<UCombat_TacticalAI*> NearbyMembers;
    
    if (!GetOwner()) return NearbyMembers;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetOwner())
        {
            UCombat_TacticalAI* OtherAI = Actor->FindComponentByClass<UCombat_TacticalAI>();
            if (OtherAI && OtherAI->DinosaurType == DinosaurType)
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                if (Distance <= SearchRadius)
                {
                    NearbyMembers.Add(OtherAI);
                }
            }
        }
    }
    
    return NearbyMembers;
}

float UCombat_TacticalAI::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetOwner()) return -1.0f;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool UCombat_TacticalAI::IsTargetInAttackRange() const
{
    float Distance = GetDistanceToTarget();
    return Distance > 0.0f && Distance <= TacticalData.AttackRange;
}

bool UCombat_TacticalAI::ShouldRetreat() const
{
    // Check health threshold (placeholder - would need actual health component)
    return false; // For now, never retreat
}

FVector UCombat_TacticalAI::CalculateFlankingPosition() const
{
    if (!CurrentTarget || !GetOwner()) return FVector::ZeroVector;
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate perpendicular flanking position
    FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // Choose left or right flank randomly
    float FlankDirection = FMath::RandBool() ? 1.0f : -1.0f;
    FVector FlankPosition = TargetLocation + RightVector * TacticalData.FlankingDistance * FlankDirection;
    
    return FlankPosition;
}

bool UCombat_TacticalAI::HasClearLineOfSight() const
{
    if (!CurrentTarget || !GetOwner()) return false;
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = CurrentTarget->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(CurrentTarget);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    
    return !bHit; // Clear line of sight if no hit
}

void UCombat_TacticalAI::UpdateBlackboardValues()
{
    if (!BlackboardComponent) return;
    
    // Update blackboard with current tactical state
    BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(CurrentState));
    
    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
    }
    
    BlackboardComponent->SetValueAsFloat(TEXT("DistanceToTarget"), GetDistanceToTarget());
    BlackboardComponent->SetValueAsBool(TEXT("InAttackRange"), IsTargetInAttackRange());
}

void UCombat_TacticalAI::HandleStateTransition(ECombat_TacticalState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Tactical AI state changed to: %d"), static_cast<int32>(NewState));
    }
}

bool UCombat_TacticalAI::ValidateTarget() const
{
    return CurrentTarget && IsValid(CurrentTarget) && !CurrentTarget->IsPendingKill();
}

FVector UCombat_TacticalAI::GetRandomFlankingPosition() const
{
    if (!CurrentTarget) return FVector::ZeroVector;
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Distance = TacticalData.FlankingDistance;
    
    FVector FlankPosition = TargetLocation + FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
        0.0f
    );
    
    return FlankPosition;
}