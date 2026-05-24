#include "DinosaurCombatAI.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure sight perception
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 2000.0f;
        SightConfig->LoseSightRadius = 2500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing perception
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1500.0f;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize combat state
    CurrentCombatState = ECombat_DinosaurCombatState::Idle;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    AttackCooldown = 2.0f;

    // Initialize combat stats
    CombatStats.AttackDamage = 50.0f;
    CombatStats.AttackRange = 300.0f;
    CombatStats.SightRange = 2000.0f;
    CombatStats.HearingRange = 1500.0f;
    CombatStats.AggressionLevel = 0.7f;
    CombatStats.FearThreshold = 0.3f;
    CombatStats.bIsPackHunter = false;
    CombatStats.MaxPackSize = 3;
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAI::OnPerceptionUpdated);
    }

    // Set up blackboard
    if (BlackboardComponent && Blackboard)
    {
        BlackboardComponent->InitializeBlackboard(*Blackboard);
    }

    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI initialized for %s"), GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCombatBehavior(DeltaTime);
}

void ADinosaurCombatAI::SetCombatState(ECombat_DinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombat_DinosaurCombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }

        UE_LOG(LogTemp, Log, TEXT("Combat state changed from %d to %d"), 
               static_cast<int32>(PreviousState), static_cast<int32>(NewState));
    }
}

void ADinosaurCombatAI::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
    }

    if (NewTarget)
    {
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
        UE_LOG(LogTemp, Log, TEXT("New target acquired: %s"), *NewTarget->GetName());
    }
    else
    {
        SetCombatState(ECombat_DinosaurCombatState::Idle);
    }
}

bool ADinosaurCombatAI::CanAttack() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return false;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return DistanceToTarget <= CombatStats.AttackRange;
}

void ADinosaurCombatAI::PerformAttack()
{
    if (!CanAttack())
    {
        return;
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();

    // Apply damage to target
    if (CurrentTarget)
    {
        // Simple damage application - in a full system this would use UE5's damage system
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI attacking %s for %.1f damage"), 
               *CurrentTarget->GetName(), CombatStats.AttackDamage);

        // Trigger attack animation and effects here
        ExecuteAttackPattern();
    }

    // Coordinate pack attack if in pack
    if (IsInPack())
    {
        CoordinatePackAttack();
    }
}

void ADinosaurCombatAI::JoinPack(ADinosaurCombatAI* PackLeader)
{
    if (!PackLeader || PackLeader == this)
    {
        return;
    }

    // Leave current pack first
    LeavePack();

    // Join new pack
    PackMembers.Empty();
    PackMembers.Add(PackLeader);
    
    // Add this to leader's pack
    PackLeader->PackMembers.AddUnique(this);

    CombatStats.bIsPackHunter = true;
    
    UE_LOG(LogTemp, Log, TEXT("Joined pack led by %s"), 
           PackLeader->GetPawn() ? *PackLeader->GetPawn()->GetName() : TEXT("Unknown"));
}

void ADinosaurCombatAI::LeavePack()
{
    if (PackMembers.Num() > 0)
    {
        // Remove from leader's pack
        if (PackMembers[0])
        {
            PackMembers[0]->PackMembers.Remove(this);
        }
        PackMembers.Empty();
    }

    // Remove others from this pack if this was the leader
    for (ADinosaurCombatAI* Member : PackMembers)
    {
        if (Member)
        {
            Member->PackMembers.Empty();
            Member->CombatStats.bIsPackHunter = false;
        }
    }

    CombatStats.bIsPackHunter = false;
}

bool ADinosaurCombatAI::IsInPack() const
{
    return CombatStats.bIsPackHunter && PackMembers.Num() > 0;
}

void ADinosaurCombatAI::CoordinatePackAttack()
{
    if (!IsInPack() || !CurrentTarget)
    {
        return;
    }

    // Signal pack members to attack
    for (ADinosaurCombatAI* Member : PackMembers)
    {
        if (Member && Member != this)
        {
            Member->SetTarget(CurrentTarget);
            Member->SetCombatState(ECombat_DinosaurCombatState::PackCoordination);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Pack attack coordinated with %d members"), PackMembers.Num());
}

void ADinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
        {
            continue;
        }

        // Check if this is a potential target (player character)
        if (Actor->IsA<ACharacter>() && Actor != GetPawn())
        {
            float ThreatLevel = CalculateThreatLevel(Actor);
            
            if (ThreatLevel > CombatStats.FearThreshold)
            {
                if (!CurrentTarget || ThreatLevel > CalculateThreatLevel(CurrentTarget))
                {
                    SetTarget(Actor);
                }
            }
        }
    }
}

void ADinosaurCombatAI::UpdateCombatBehavior(float DeltaTime)
{
    if (!GetPawn())
    {
        return;
    }

    ProcessTargetSelection();

    switch (CurrentCombatState)
    {
        case ECombat_DinosaurCombatState::Idle:
            // Look for targets
            break;

        case ECombat_DinosaurCombatState::Hunting:
            if (CurrentTarget)
            {
                float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
                
                if (DistanceToTarget <= CombatStats.AttackRange)
                {
                    SetCombatState(ECombat_DinosaurCombatState::Attacking);
                }
                else if (DistanceToTarget > CombatStats.SightRange * 1.5f)
                {
                    SetTarget(nullptr); // Lost target
                }
            }
            break;

        case ECombat_DinosaurCombatState::Attacking:
            if (CanAttack())
            {
                PerformAttack();
            }
            break;

        case ECombat_DinosaurCombatState::Retreating:
            if (ShouldRetreat())
            {
                // Move away from threat
                SetCombatState(ECombat_DinosaurCombatState::Idle);
            }
            break;

        case ECombat_DinosaurCombatState::PackCoordination:
            HandlePackCoordination();
            break;
    }
}

void ADinosaurCombatAI::ProcessTargetSelection()
{
    // Simple target validation
    if (CurrentTarget && !IsValid(CurrentTarget))
    {
        SetTarget(nullptr);
    }
}

void ADinosaurCombatAI::HandlePackCoordination()
{
    if (!IsInPack())
    {
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
        return;
    }

    // Implement pack coordination logic
    // This could include flanking maneuvers, coordinated attacks, etc.
}

float ADinosaurCombatAI::CalculateThreatLevel(AActor* Actor) const
{
    if (!Actor || !GetPawn())
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
    float ThreatLevel = CombatStats.AggressionLevel;

    // Closer targets are more threatening
    if (Distance < CombatStats.AttackRange)
    {
        ThreatLevel += 0.3f;
    }

    // Player characters are high priority
    if (Actor->IsA<ACharacter>())
    {
        ThreatLevel += 0.4f;
    }

    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

bool ADinosaurCombatAI::ShouldRetreat() const
{
    // Simple retreat logic - could be expanded with health, pack status, etc.
    return false;
}

void ADinosaurCombatAI::ExecuteAttackPattern()
{
    // Placeholder for attack animation and effects
    UE_LOG(LogTemp, Warning, TEXT("Executing attack pattern"));
    
    // In a full implementation, this would:
    // - Play attack animation
    // - Spawn particle effects
    // - Play sound effects
    // - Apply actual damage through UE5's damage system
}