#include "Combat_DinosaurAI.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACombat_DinosaurAI::ACombat_DinosaurAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Set default dinosaur stats
    DinosaurStats.Health = 100.0f;
    DinosaurStats.AttackDamage = 25.0f;
    DinosaurStats.AttackRange = 200.0f;
    DinosaurStats.SightRange = 1500.0f;
    DinosaurStats.HearingRange = 800.0f;
    DinosaurStats.MovementSpeed = 300.0f;
    DinosaurStats.Aggression = 0.7f;
    DinosaurStats.PackCoordination = 0.5f;
}

void ACombat_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();

    InitializePerception();
    ConfigureSpeciesStats();
    
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
    }

    UpdateBlackboard();
}

void ACombat_DinosaurAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateBlackboard();
    HandlePackBehavior();
}

void ACombat_DinosaurAI::InitializePerception()
{
    if (!PerceptionComponent)
        return;

    // Configure sight
    auto SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = DinosaurStats.SightRange;
        SightConfig->LoseSightRadius = DinosaurStats.SightRange + 200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

        PerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing
    auto HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = DinosaurStats.HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

        PerceptionComponent->ConfigureSense(*HearingConfig);
    }

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurAI::OnPerceptionUpdated);
}

void ACombat_DinosaurAI::ConfigureSpeciesStats()
{
    switch (Species)
    {
        case ECombat_DinosaurSpecies::TRex:
            DinosaurStats.Health = 200.0f;
            DinosaurStats.AttackDamage = 50.0f;
            DinosaurStats.AttackRange = 300.0f;
            DinosaurStats.SightRange = 2000.0f;
            DinosaurStats.MovementSpeed = 400.0f;
            DinosaurStats.Aggression = 0.9f;
            DinosaurStats.PackCoordination = 0.2f; // Solitary hunter
            break;

        case ECombat_DinosaurSpecies::Raptor:
            DinosaurStats.Health = 80.0f;
            DinosaurStats.AttackDamage = 30.0f;
            DinosaurStats.AttackRange = 150.0f;
            DinosaurStats.SightRange = 1800.0f;
            DinosaurStats.MovementSpeed = 600.0f;
            DinosaurStats.Aggression = 0.8f;
            DinosaurStats.PackCoordination = 0.9f; // Pack hunter
            break;

        case ECombat_DinosaurSpecies::Triceratops:
            DinosaurStats.Health = 180.0f;
            DinosaurStats.AttackDamage = 40.0f;
            DinosaurStats.AttackRange = 250.0f;
            DinosaurStats.SightRange = 1200.0f;
            DinosaurStats.MovementSpeed = 250.0f;
            DinosaurStats.Aggression = 0.5f; // Defensive
            DinosaurStats.PackCoordination = 0.6f;
            break;

        case ECombat_DinosaurSpecies::Brachiosaurus:
            DinosaurStats.Health = 300.0f;
            DinosaurStats.AttackDamage = 20.0f;
            DinosaurStats.AttackRange = 400.0f;
            DinosaurStats.SightRange = 1000.0f;
            DinosaurStats.MovementSpeed = 150.0f;
            DinosaurStats.Aggression = 0.2f; // Peaceful
            DinosaurStats.PackCoordination = 0.3f;
            break;

        case ECombat_DinosaurSpecies::Ankylosaurus:
            DinosaurStats.Health = 220.0f;
            DinosaurStats.AttackDamage = 35.0f;
            DinosaurStats.AttackRange = 200.0f;
            DinosaurStats.SightRange = 1000.0f;
            DinosaurStats.MovementSpeed = 180.0f;
            DinosaurStats.Aggression = 0.4f; // Defensive
            DinosaurStats.PackCoordination = 0.4f;
            break;
    }
}

void ACombat_DinosaurAI::UpdateBlackboard()
{
    if (!BlackboardComponent)
        return;

    BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget);
    BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentState));
    BlackboardComponent->SetValueAsFloat(TEXT("Health"), DinosaurStats.Health);
    BlackboardComponent->SetValueAsFloat(TEXT("AttackRange"), DinosaurStats.AttackRange);
    BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), DinosaurStats.Aggression);
    BlackboardComponent->SetValueAsInt(TEXT("PackSize"), PackMembers.Num());
}

void ACombat_DinosaurAI::HandlePackBehavior()
{
    if (PackMembers.Num() == 0 || DinosaurStats.PackCoordination < 0.5f)
        return;

    // Simple pack coordination - share targets
    if (CurrentTarget && CurrentState == ECombat_AIState::Hunt)
    {
        for (auto PackMember : PackMembers)
        {
            if (PackMember && PackMember->CurrentTarget != CurrentTarget)
            {
                PackMember->SetTarget(CurrentTarget);
            }
        }
    }
}

void ACombat_DinosaurAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
            continue;

        // Check if it's a player character
        if (Actor->IsA<ACharacter>())
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
            
            if (Distance <= DinosaurStats.SightRange)
            {
                // Decide whether to attack based on aggression and species
                float AggressionRoll = FMath::RandRange(0.0f, 1.0f);
                
                if (AggressionRoll <= DinosaurStats.Aggression)
                {
                    SetTarget(Actor);
                    SetAIState(ECombat_AIState::Hunt);
                }
            }
        }
    }
}

void ACombat_DinosaurAI::SetDinosaurSpecies(ECombat_DinosaurSpecies NewSpecies)
{
    Species = NewSpecies;
    ConfigureSpeciesStats();
    InitializePerception();
}

void ACombat_DinosaurAI::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    UpdateBlackboard();
}

void ACombat_DinosaurAI::AddPackMember(ACombat_DinosaurAI* PackMember)
{
    if (PackMember && !PackMembers.Contains(PackMember))
    {
        PackMembers.Add(PackMember);
        PackMember->PackMembers.AddUnique(this);
    }
}

void ACombat_DinosaurAI::RemovePackMember(ACombat_DinosaurAI* PackMember)
{
    if (PackMember)
    {
        PackMembers.Remove(PackMember);
        PackMember->PackMembers.Remove(this);
    }
}

void ACombat_DinosaurAI::SetAIState(ECombat_AIState NewState)
{
    CurrentState = NewState;
    UpdateBlackboard();
}

bool ACombat_DinosaurAI::CanAttackTarget() const
{
    if (!CurrentTarget || !GetPawn())
        return false;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    float TimeSinceLastAttack = GetWorld()->GetTimeSeconds() - LastAttackTime;

    return Distance <= DinosaurStats.AttackRange && TimeSinceLastAttack >= AttackCooldown;
}

void ACombat_DinosaurAI::ExecuteAttack()
{
    if (!CanAttackTarget())
        return;

    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // Apply damage to target if it's a character
    if (auto TargetCharacter = Cast<ACharacter>(CurrentTarget))
    {
        // Here you would implement actual damage application
        // For now, just log the attack
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s attacks %s for %f damage"), 
               *GetName(), *CurrentTarget->GetName(), DinosaurStats.AttackDamage);
    }

    SetAIState(ECombat_AIState::Attack);
}

void ACombat_DinosaurAI::CoordinatePackAttack()
{
    if (PackMembers.Num() == 0 || !CurrentTarget)
        return;

    // Coordinate pack attack patterns
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i])
        {
            PackMembers[i]->SetTarget(CurrentTarget);
            PackMembers[i]->SetAIState(ECombat_AIState::Pack);
        }
    }
}