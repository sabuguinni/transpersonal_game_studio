#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "CombatBehaviorComponent.h"
#include "DinosaurPersonalityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Inicializar componentes
    CombatBehaviorComponent = CreateDefaultSubobject<UCombatBehaviorComponent>(TEXT("CombatBehaviorComponent"));
    PersonalityComponent = CreateDefaultSubobject<UDinosaurPersonalityComponent>(TEXT("PersonalityComponent"));

    // Configurar AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configurar visão
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = DetectionRadius;
    SightConfig->LoseSightRadius = DetectionRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configurar audição
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = DetectionRadius * 0.8f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configurar dano
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);

    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Estado inicial
    CurrentCombatState = ECombatState::Idle;
    ThreatLevel = EDinosaurThreatLevel::Cautious;
    LastStateChangeTime = 0.0f;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    }

    // Inicializar personalidade procedural se não definida
    if (PersonalityComponent)
    {
        PersonalityComponent->GeneratePersonality(DinosaurSpeciesTag);
        Personality = PersonalityComponent->GetPersonality();
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCombatLogic(DeltaTime);
    UpdateBlackboardValues();
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (CombatBehaviorTree && Blackboard)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();

        // Atualizar gameplay tag
        switch (NewState)
        {
            case ECombatState::Hunting:
                CurrentBehaviorTag = FGameplayTag::RequestGameplayTag(FName("Dinosaur.Behavior.Hunting"));
                break;
            case ECombatState::Stalking:
                CurrentBehaviorTag = FGameplayTag::RequestGameplayTag(FName("Dinosaur.Behavior.Stalking"));
                break;
            case ECombatState::Attacking:
                CurrentBehaviorTag = FGameplayTag::RequestGameplayTag(FName("Dinosaur.Behavior.Attacking"));
                break;
            case ECombatState::Fleeing:
                CurrentBehaviorTag = FGameplayTag::RequestGameplayTag(FName("Dinosaur.Behavior.Fleeing"));
                break;
            case ECombatState::Territorial:
                CurrentBehaviorTag = FGameplayTag::RequestGameplayTag(FName("Dinosaur.Behavior.Territorial"));
                break;
            default:
                CurrentBehaviorTag = FGameplayTag::RequestGameplayTag(FName("Dinosaur.Behavior.Idle"));
                break;
        }

        // Notificar componente de comportamento
        if (CombatBehaviorComponent)
        {
            CombatBehaviorComponent->OnStateChanged(PreviousState, NewState);
        }
    }
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (NewTarget)
    {
        // Se é o jogador, atualizar última localização conhecida
        if (NewTarget->IsA<APawn>() && Cast<APawn>(NewTarget)->IsPlayerControlled())
        {
            LastKnownPlayerLocation = NewTarget;
        }
    }
}

bool ACombatAIController::CanAttackTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return DistanceToTarget <= AttackRange && CurrentCombatState == ECombatState::Attacking;
}

bool ACombatAIController::ShouldFleeFromTarget() const
{
    if (!CurrentTarget || ThreatLevel == EDinosaurThreatLevel::ApexPredator)
    {
        return false;
    }

    // Herbívoros fogem mais facilmente
    if (ThreatLevel == EDinosaurThreatLevel::Passive || ThreatLevel == EDinosaurThreatLevel::Cautious)
    {
        return true;
    }

    // Predadores fogem se feridos ou superados
    if (CombatBehaviorComponent)
    {
        float HealthPercentage = CombatBehaviorComponent->GetHealthPercentage();
        return HealthPercentage < (0.3f + Personality.Fearfulness * 0.4f);
    }

    return false;
}

FVector ACombatAIController::GetFleeDirection() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return FVector::ZeroVector;
    }

    FVector FleeDirection = GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation();
    FleeDirection.Normalize();
    
    // Adicionar variação baseada na personalidade
    float RandomAngle = FMath::RandRange(-45.0f, 45.0f) * (1.0f - Personality.Patience);
    FleeDirection = FleeDirection.RotateAngleAxis(RandomAngle, FVector::UpVector);
    
    return FleeDirection * FleeDistance;
}

void ACombatAIController::OnPlayerDetected(AActor* Player)
{
    SetTarget(Player);
    
    // Determinar reação baseada no tipo de dinossauro e personalidade
    switch (ThreatLevel)
    {
        case EDinosaurThreatLevel::Passive:
            if (FMath::RandRange(0.0f, 1.0f) < Personality.Fearfulness)
            {
                SetCombatState(ECombatState::Fleeing);
            }
            break;
            
        case EDinosaurThreatLevel::Cautious:
            if (FMath::RandRange(0.0f, 1.0f) < Personality.Curiosity)
            {
                SetCombatState(ECombatState::Stalking);
            }
            else
            {
                SetCombatState(ECombatState::Fleeing);
            }
            break;
            
        case EDinosaurThreatLevel::Territorial:
            SetCombatState(ECombatState::Territorial);
            break;
            
        case EDinosaurThreatLevel::Predator:
        case EDinosaurThreatLevel::ApexPredator:
            if (FMath::RandRange(0.0f, 1.0f) < Personality.Aggression)
            {
                SetCombatState(ECombatState::Hunting);
            }
            else
            {
                SetCombatState(ECombatState::Stalking);
            }
            break;
    }
}

void ACombatAIController::OnPlayerLost()
{
    if (CurrentCombatState != ECombatState::Idle)
    {
        // Transição gradual de volta ao idle
        if (CurrentCombatState == ECombatState::Hunting || CurrentCombatState == ECombatState::Stalking)
        {
            SetCombatState(ECombatState::Idle);
        }
    }
    
    // Manter referência à última localização por um tempo
    if (LastKnownPlayerLocation)
    {
        GetWorldTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { LastKnownPlayerLocation = nullptr; },
            10.0f,
            false
        );
    }
}

void ACombatAIController::OnDamageReceived(float Damage, AActor* DamageSource)
{
    if (DamageSource)
    {
        SetTarget(DamageSource);
        
        // Reação ao dano baseada na personalidade
        if (ShouldFleeFromTarget())
        {
            SetCombatState(ECombatState::Fleeing);
        }
        else if (ThreatLevel >= EDinosaurThreatLevel::Predator)
        {
            SetCombatState(ECombatState::Attacking);
        }
    }
}

void ACombatAIController::UpdateCombatLogic(float DeltaTime)
{
    HandleStateTransitions();
    
    // Atualizar componente de comportamento
    if (CombatBehaviorComponent)
    {
        CombatBehaviorComponent->UpdateBehavior(DeltaTime, CurrentCombatState, CurrentTarget);
    }
}

void ACombatAIController::UpdateBlackboardValues()
{
    if (!Blackboard)
    {
        return;
    }

    Blackboard->SetValueAsObject(TargetActorKey, CurrentTarget);
    Blackboard->SetValueAsEnum(CombatStateKey, static_cast<uint8>(CurrentCombatState));
    Blackboard->SetValueAsBool(CanAttackKey, CanAttackTarget());
    Blackboard->SetValueAsBool(ShouldFleeKey, ShouldFleeFromTarget());
    
    if (LastKnownPlayerLocation)
    {
        Blackboard->SetValueAsVector(PlayerLocationKey, LastKnownPlayerLocation->GetActorLocation());
    }
    
    if (CurrentCombatState == ECombatState::Fleeing)
    {
        Blackboard->SetValueAsVector(FleeDirectionKey, GetFleeDirection());
    }
}

void ACombatAIController::HandleStateTransitions()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Evitar mudanças de estado muito frequentes
    if (CurrentTime - LastStateChangeTime < StateChangeDelay)
    {
        return;
    }

    // Lógica de transição baseada no contexto atual
    if (CurrentTarget)
    {
        float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        switch (CurrentCombatState)
        {
            case ECombatState::Stalking:
                if (DistanceToTarget <= AttackRange && ThreatLevel >= EDinosaurThreatLevel::Predator)
                {
                    if (FMath::RandRange(0.0f, 1.0f) < Personality.Aggression)
                    {
                        SetCombatState(ECombatState::Attacking);
                    }
                }
                else if (DistanceToTarget > DetectionRadius)
                {
                    SetCombatState(ECombatState::Hunting);
                }
                break;
                
            case ECombatState::Hunting:
                if (DistanceToTarget <= AttackRange * 2.0f)
                {
                    SetCombatState(ECombatState::Stalking);
                }
                break;
                
            case ECombatState::Attacking:
                if (DistanceToTarget > AttackRange * 1.5f)
                {
                    SetCombatState(ECombatState::Hunting);
                }
                break;
        }
    }
    else
    {
        // Sem target, voltar ao idle
        if (CurrentCombatState != ECombatState::Idle)
        {
            SetCombatState(ECombatState::Idle);
        }
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            APawn* DetectedPawn = Cast<APawn>(Actor);
            if (DetectedPawn && DetectedPawn->IsPlayerControlled())
            {
                // Player detectado
                if (AIPerceptionComponent->GetActorsPerception(Actor).Num() > 0)
                {
                    OnPlayerDetected(Actor);
                }
                else
                {
                    OnPlayerLost();
                }
            }
        }
    }
}