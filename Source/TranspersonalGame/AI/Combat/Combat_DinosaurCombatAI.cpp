#include "Combat_DinosaurCombatAI.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

ACombat_DinosaurCombatAI::ACombat_DinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure sight perception
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    // Configure hearing perception
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Default combat settings
    AttackRange = 300.0f;
    FleeHealthThreshold = 0.2f;
    AggressionLevel = 0.7f;
    DinosaurSpecies = ECombat_DinosaurSpecies::TRex;
    CurrentCombatState = ECombat_CombatState::Idle;

    // Initialize state
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    CombatStartTime = 0.0f;
    bIsInCombat = false;
}

void ACombat_DinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatAI::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatAI::OnTargetPerceptionUpdated);
    }

    // Initialize species-specific behavior
    InitializeSpeciesBehavior();

    // Start behavior tree if available
    if (BlackboardComponent && BehaviorTreeComponent)
    {
        UseBlackboard(BlackboardComponent);
        RunBehaviorTree(nullptr); // Will be set via Blueprint
    }
}

void ACombat_DinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCombatState();

    // Update species-specific behavior
    switch (DinosaurSpecies)
    {
        case ECombat_DinosaurSpecies::TRex:
            UpdateTRexBehavior();
            break;
        case ECombat_DinosaurSpecies::Velociraptor:
            UpdateVelociraptorBehavior();
            break;
        case ECombat_DinosaurSpecies::Triceratops:
            UpdateTriceratopsBehavior();
            break;
        case ECombat_DinosaurSpecies::Brachiosaurus:
            UpdateBrachiosaurusBehavior();
            break;
    }
}

void ACombat_DinosaurCombatAI::StartCombat(AActor* Target)
{
    if (!Target) return;

    CurrentTarget = Target;
    bIsInCombat = true;
    CombatStartTime = GetWorld()->GetTimeSeconds();
    CurrentCombatState = ECombat_CombatState::Aggressive;

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("Target"), Target);
        BlackboardComponent->SetValueAsBool(TEXT("InCombat"), true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Combat AI: %s starting combat with %s"), 
           *GetPawn()->GetName(), *Target->GetName());
}

void ACombat_DinosaurCombatAI::EndCombat()
{
    CurrentTarget = nullptr;
    bIsInCombat = false;
    CurrentCombatState = ECombat_CombatState::Idle;

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("Target"), nullptr);
        BlackboardComponent->SetValueAsBool(TEXT("InCombat"), false);
    }

    UE_LOG(LogTemp, Warning, TEXT("Combat AI: %s ending combat"), *GetPawn()->GetName());
}

void ACombat_DinosaurCombatAI::FleeFromThreat(AActor* Threat)
{
    if (!Threat) return;

    CurrentCombatState = ECombat_CombatState::Fleeing;

    // Calculate flee direction (opposite from threat)
    FVector FleeDirection = GetPawn()->GetActorLocation() - Threat->GetActorLocation();
    FleeDirection.Normalize();
    FVector FleeLocation = GetPawn()->GetActorLocation() + (FleeDirection * 2000.0f);

    // Move to flee location
    MoveToLocation(FleeLocation);

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("FleeLocation"), FleeLocation);
        BlackboardComponent->SetValueAsBool(TEXT("IsFleeing"), true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Combat AI: %s fleeing from %s"), 
           *GetPawn()->GetName(), *Threat->GetName());
}

void ACombat_DinosaurCombatAI::AttackTarget(AActor* Target)
{
    if (!Target || !IsInAttackRange(Target)) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    float AttackCooldown = 2.0f; // Base attack cooldown

    // Adjust cooldown based on species
    switch (DinosaurSpecies)
    {
        case ECombat_DinosaurSpecies::TRex:
            AttackCooldown = 3.0f;
            break;
        case ECombat_DinosaurSpecies::Velociraptor:
            AttackCooldown = 1.5f;
            break;
        case ECombat_DinosaurSpecies::Triceratops:
            AttackCooldown = 2.5f;
            break;
        case ECombat_DinosaurSpecies::Brachiosaurus:
            AttackCooldown = 4.0f;
            break;
    }

    if (CurrentTime - LastAttackTime >= AttackCooldown)
    {
        LastAttackTime = CurrentTime;
        CurrentCombatState = ECombat_CombatState::Attacking;

        // Face the target
        FVector LookDirection = Target->GetActorLocation() - GetPawn()->GetActorLocation();
        LookDirection.Z = 0.0f;
        LookDirection.Normalize();
        FRotator LookRotation = FRotationMatrix::MakeFromX(LookDirection).Rotator();
        GetPawn()->SetActorRotation(LookRotation);

        // Update blackboard for attack animation
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsBool(TEXT("ShouldAttack"), true);
        }

        UE_LOG(LogTemp, Warning, TEXT("Combat AI: %s attacking %s"), 
               *GetPawn()->GetName(), *Target->GetName());
    }
}

bool ACombat_DinosaurCombatAI::IsInAttackRange(AActor* Target) const
{
    if (!Target || !GetPawn()) return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

void ACombat_DinosaurCombatAI::UpdateCombatState()
{
    if (!bIsInCombat) return;

    // Check if target is still valid
    if (!CurrentTarget || !IsValid(CurrentTarget))
    {
        EndCombat();
        return;
    }

    // Check if target is too far away
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistanceToTarget > 3000.0f) // Lost target
    {
        EndCombat();
        return;
    }

    // Check health for fleeing (if pawn has health component)
    if (GetPawn())
    {
        // This would need to be connected to actual health system
        // For now, use random chance to simulate low health fleeing
        if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per tick to flee
        {
            FleeFromThreat(CurrentTarget);
        }
    }
}

void ACombat_DinosaurCombatAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        // Check if this is a player or other dinosaur
        if (Actor->IsA<ACharacter>() && !bIsInCombat)
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            
            // Start combat if close enough and aggressive enough
            if (Distance < AttackRange * 2.0f && FMath::RandRange(0.0f, 1.0f) < AggressionLevel)
            {
                StartCombat(Actor);
            }
        }
    }
}

void ACombat_DinosaurCombatAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // Target acquired
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("SensedTarget"), Actor);
        }
    }
    else
    {
        // Target lost
        if (BlackboardComponent && BlackboardComponent->GetValueAsObject(TEXT("SensedTarget")) == Actor)
        {
            BlackboardComponent->SetValueAsObject(TEXT("SensedTarget"), nullptr);
        }
    }
}

void ACombat_DinosaurCombatAI::InitializeSpeciesBehavior()
{
    switch (DinosaurSpecies)
    {
        case ECombat_DinosaurSpecies::TRex:
            AttackRange = 500.0f;
            AggressionLevel = 0.9f;
            FleeHealthThreshold = 0.1f;
            break;
        case ECombat_DinosaurSpecies::Velociraptor:
            AttackRange = 250.0f;
            AggressionLevel = 0.8f;
            FleeHealthThreshold = 0.3f;
            break;
        case ECombat_DinosaurSpecies::Triceratops:
            AttackRange = 400.0f;
            AggressionLevel = 0.5f;
            FleeHealthThreshold = 0.2f;
            break;
        case ECombat_DinosaurSpecies::Brachiosaurus:
            AttackRange = 600.0f;
            AggressionLevel = 0.2f;
            FleeHealthThreshold = 0.4f;
            break;
    }
}

void ACombat_DinosaurCombatAI::UpdateTRexBehavior()
{
    // T-Rex: Aggressive, powerful attacks, territorial
    if (bIsInCombat && CurrentTarget)
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (Distance <= AttackRange)
        {
            AttackTarget(CurrentTarget);
        }
        else if (Distance < 1000.0f)
        {
            // Chase the target
            MoveToActor(CurrentTarget);
        }
    }
}

void ACombat_DinosaurCombatAI::UpdateVelociraptorBehavior()
{
    // Velociraptor: Pack hunter, quick attacks, flanking
    if (bIsInCombat && CurrentTarget)
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (Distance <= AttackRange)
        {
            AttackTarget(CurrentTarget);
        }
        else
        {
            // Try to flank the target
            FVector TargetLocation = CurrentTarget->GetActorLocation();
            FVector FlankDirection = FVector::CrossProduct(
                CurrentTarget->GetActorForwardVector(), 
                FVector::UpVector
            );
            FVector FlankPosition = TargetLocation + (FlankDirection * 300.0f);
            MoveToLocation(FlankPosition);
        }
    }
}

void ACombat_DinosaurCombatAI::UpdateTriceratopsBehavior()
{
    // Triceratops: Defensive, charges when threatened
    if (bIsInCombat && CurrentTarget)
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (Distance <= AttackRange)
        {
            AttackTarget(CurrentTarget);
        }
        else if (Distance < 800.0f)
        {
            // Charge attack
            MoveToActor(CurrentTarget);
        }
    }
}

void ACombat_DinosaurCombatAI::UpdateBrachiosaurusBehavior()
{
    // Brachiosaurus: Mostly peaceful, defensive when attacked
    if (bIsInCombat && CurrentTarget)
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (Distance <= AttackRange)
        {
            AttackTarget(CurrentTarget);
        }
        else
        {
            // Try to move away (defensive behavior)
            FVector AwayDirection = GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation();
            AwayDirection.Normalize();
            FVector MoveLocation = GetPawn()->GetActorLocation() + (AwayDirection * 500.0f);
            MoveToLocation(MoveLocation);
        }
    }
}