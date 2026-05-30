#include "Combat_PredatorHuntingAI.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UCombat_PredatorHuntingAI::UCombat_PredatorHuntingAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize hunting parameters
    CurrentHuntingState = ECombat_HuntingState::Idle;
    HuntingRange = 8000.0f;
    StalkingRange = 3000.0f;
    AttackRange = 500.0f;
    
    // Target selection
    CurrentTarget = nullptr;
    PreferredPreySize = 1.0f;
    bPreferIsolatedTargets = true;
    
    // Hunting behavior
    StalkingSpeed = 200.0f;
    ChaseSpeed = 600.0f;
    AttackCooldown = 3.0f;
    LastAttackTime = 0.0f;
    
    // Pack hunting
    bIsPackLeader = false;
    PackLeader = nullptr;
}

void UCombat_PredatorHuntingAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize hunting system
    CurrentHuntingState = ECombat_HuntingState::Idle;
    PotentialTargets.Empty();
}

void UCombat_PredatorHuntingAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
        
    UpdateHuntingState(DeltaTime);
    
    // Scan for targets periodically
    static float LastScanTime = 0.0f;
    LastScanTime += DeltaTime;
    if (LastScanTime >= 2.0f)
    {
        ScanForTargets();
        LastScanTime = 0.0f;
    }
}

void UCombat_PredatorHuntingAI::UpdateHuntingState(float DeltaTime)
{
    switch (CurrentHuntingState)
    {
        case ECombat_HuntingState::Idle:
            HandleIdleState();
            break;
        case ECombat_HuntingState::Stalking:
            HandleStalkingState();
            break;
        case ECombat_HuntingState::Chasing:
            HandleChasingState();
            break;
        case ECombat_HuntingState::Attacking:
            HandleAttackingState();
            break;
        case ECombat_HuntingState::Feeding:
            HandleFeedingState();
            break;
        case ECombat_HuntingState::Retreating:
            HandleRetreatingState();
            break;
    }
}

void UCombat_PredatorHuntingAI::HandleIdleState()
{
    // Look for targets
    if (PotentialTargets.Num() > 0)
    {
        SelectBestTarget();
        if (CurrentTarget)
        {
            StartStalking(CurrentTarget);
        }
    }
}

void UCombat_PredatorHuntingAI::HandleStalkingState()
{
    if (!CurrentTarget)
    {
        CurrentHuntingState = ECombat_HuntingState::Idle;
        return;
    }
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget <= AttackRange)
    {
        StartChasing(CurrentTarget);
    }
    else if (DistanceToTarget > StalkingRange)
    {
        // Move closer while staying hidden
        FVector StalkingPosition = CalculateStalkingPosition(CurrentTarget);
        
        // Move character towards stalking position
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = StalkingSpeed;
                FVector Direction = (StalkingPosition - GetOwner()->GetActorLocation()).GetSafeNormal();
                OwnerCharacter->AddMovementInput(Direction, 1.0f);
            }
        }
    }
}

void UCombat_PredatorHuntingAI::HandleChasingState()
{
    if (!CurrentTarget)
    {
        CurrentHuntingState = ECombat_HuntingState::Idle;
        return;
    }
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget <= AttackRange)
    {
        ExecuteAttack();
    }
    else if (DistanceToTarget > HuntingRange)
    {
        // Target escaped
        CurrentTarget = nullptr;
        CurrentHuntingState = ECombat_HuntingState::Idle;
    }
    else
    {
        // Chase the target
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = ChaseSpeed;
                FVector Direction = (CurrentTarget->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
                OwnerCharacter->AddMovementInput(Direction, 1.0f);
            }
        }
        
        // Trigger herd flee response if target is in a herd
        if (IsTargetInHerd(CurrentTarget))
        {
            TriggerHerdFleeResponse(CurrentTarget);
        }
    }
}

void UCombat_PredatorHuntingAI::HandleAttackingState()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime >= AttackCooldown)
    {
        CurrentHuntingState = ECombat_HuntingState::Feeding;
    }
}

void UCombat_PredatorHuntingAI::HandleFeedingState()
{
    // Simple feeding behavior - stay in place for a while
    static float FeedingStartTime = GetWorld()->GetTimeSeconds();
    float FeedingDuration = 10.0f;
    
    if (GetWorld()->GetTimeSeconds() - FeedingStartTime >= FeedingDuration)
    {
        CurrentTarget = nullptr;
        CurrentHuntingState = ECombat_HuntingState::Idle;
    }
}

void UCombat_PredatorHuntingAI::HandleRetreatingState()
{
    // Move away from threats
    CurrentHuntingState = ECombat_HuntingState::Idle;
}

void UCombat_PredatorHuntingAI::ScanForTargets()
{
    if (!GetOwner())
        return;
        
    PotentialTargets.Empty();
    
    // Get all actors in hunting range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == GetOwner())
            continue;
            
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= HuntingRange)
        {
            // Check if it's a valid target (herbivore dinosaur or player)
            if (Actor->GetName().Contains("Triceratops") || 
                Actor->GetName().Contains("Brachiosaurus") || 
                Actor->GetName().Contains("Parasaurolophus") ||
                Actor->IsA<ACharacter>())
            {
                FCombat_HuntTarget NewTarget;
                NewTarget.TargetActor = Actor;
                NewTarget.DistanceToTarget = Distance;
                NewTarget.ThreatLevel = CalculateThreatLevel(Actor);
                NewTarget.bIsHerdMember = IsTargetInHerd(Actor);
                
                PotentialTargets.Add(NewTarget);
            }
        }
    }
}

void UCombat_PredatorHuntingAI::SelectBestTarget()
{
    if (PotentialTargets.Num() == 0)
    {
        CurrentTarget = nullptr;
        return;
    }
    
    FCombat_HuntTarget* BestTarget = nullptr;
    float BestScore = -1.0f;
    
    for (FCombat_HuntTarget& Target : PotentialTargets)
    {
        if (!Target.TargetActor)
            continue;
            
        float Score = 0.0f;
        
        // Prefer closer targets
        Score += (HuntingRange - Target.DistanceToTarget) / HuntingRange * 50.0f;
        
        // Prefer isolated targets if configured
        if (bPreferIsolatedTargets && !Target.bIsHerdMember)
        {
            Score += 30.0f;
        }
        
        // Avoid high threat targets
        Score -= Target.ThreatLevel * 20.0f;
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = &Target;
        }
    }
    
    CurrentTarget = BestTarget ? BestTarget->TargetActor : nullptr;
}

void UCombat_PredatorHuntingAI::StartStalking(AActor* Target)
{
    if (!Target)
        return;
        
    CurrentTarget = Target;
    CurrentHuntingState = ECombat_HuntingState::Stalking;
    
    UE_LOG(LogTemp, Warning, TEXT("Predator %s started stalking %s"), 
           *GetOwner()->GetName(), *Target->GetName());
}

void UCombat_PredatorHuntingAI::StartChasing(AActor* Target)
{
    if (!Target)
        return;
        
    CurrentTarget = Target;
    CurrentHuntingState = ECombat_HuntingState::Chasing;
    
    // Coordinate pack attack if this is a pack leader
    if (bIsPackLeader)
    {
        CoordinatePackAttack();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Predator %s started chasing %s"), 
           *GetOwner()->GetName(), *Target->GetName());
}

void UCombat_PredatorHuntingAI::ExecuteAttack()
{
    if (!CurrentTarget)
        return;
        
    CurrentHuntingState = ECombat_HuntingState::Attacking;
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Predator %s attacking %s"), 
           *GetOwner()->GetName(), *CurrentTarget->GetName());
}

void UCombat_PredatorHuntingAI::TriggerHerdFleeResponse(AActor* HerdTarget)
{
    if (!HerdTarget)
        return;
        
    // Find herd behavior component on the target
    if (UNPC_DinosaurHerdBehavior* HerdComponent = HerdTarget->FindComponentByClass<UNPC_DinosaurHerdBehavior>())
    {
        // Trigger flee state in the herd
        HerdComponent->SetHerdState(ENPC_HerdState::Fleeing);
        
        UE_LOG(LogTemp, Warning, TEXT("Triggered herd flee response for %s"), *HerdTarget->GetName());
    }
}

void UCombat_PredatorHuntingAI::CoordinatePackAttack()
{
    if (!bIsPackLeader)
        return;
        
    for (UCombat_PredatorHuntingAI* PackMember : PackMembers)
    {
        if (PackMember && PackMember->GetOwner())
        {
            PackMember->StartChasing(CurrentTarget);
        }
    }
}

void UCombat_PredatorHuntingAI::JoinPack(UCombat_PredatorHuntingAI* Leader)
{
    if (!Leader)
        return;
        
    PackLeader = Leader;
    bIsPackLeader = false;
    Leader->PackMembers.AddUnique(this);
}

void UCombat_PredatorHuntingAI::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }
    
    PackMembers.Empty();
    bIsPackLeader = false;
}

float UCombat_PredatorHuntingAI::CalculateThreatLevel(AActor* Target)
{
    if (!Target)
        return 0.0f;
        
    float ThreatLevel = 0.0f;
    
    // Player characters are high threat
    if (Target->IsA<ACharacter>())
    {
        ThreatLevel += 0.8f;
    }
    
    // Large herbivores are medium threat
    if (Target->GetName().Contains("Triceratops") || Target->GetName().Contains("Brachiosaurus"))
    {
        ThreatLevel += 0.4f;
    }
    
    // Herd members have reduced individual threat
    if (IsTargetInHerd(Target))
    {
        ThreatLevel *= 0.7f;
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

bool UCombat_PredatorHuntingAI::IsTargetInHerd(AActor* Target)
{
    if (!Target)
        return false;
        
    // Check if target has herd behavior component
    return Target->FindComponentByClass<UNPC_DinosaurHerdBehavior>() != nullptr;
}

FVector UCombat_PredatorHuntingAI::CalculateStalkingPosition(AActor* Target)
{
    if (!Target)
        return GetOwner()->GetActorLocation();
        
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate position behind cover or at optimal stalking distance
    FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
    FVector StalkingPosition = TargetLocation - Direction * (StalkingRange * 0.8f);
    
    return StalkingPosition;
}