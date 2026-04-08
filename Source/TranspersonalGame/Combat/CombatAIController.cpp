#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Inicializar componentes
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Estados iniciais
    CurrentCombatState = ECombatState::Idle;
    CurrentThreatLevel = EThreatLevel::None;
    TimeSinceLastTargetSeen = 0.0f;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerception();
    
    // Inicializar Behavior Tree se definido
    if (BehaviorTree && BlackboardAsset)
    {
        RunBehaviorTree(BehaviorTree);
        GetBlackboardComponent()->InitializeBlackboard(*BlackboardAsset);
        
        // Configurar valores iniciais no Blackboard
        GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        GetBlackboardComponent()->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CurrentThreatLevel));
        GetBlackboardComponent()->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
        GetBlackboardComponent()->SetValueAsFloat(TEXT("TerritorialRadius"), TerritorialRadius);
        GetBlackboardComponent()->SetValueAsFloat(TEXT("OptimalAttackRange"), OptimalAttackRange);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCombatMemory(DeltaTime);
    AnalyzeTacticalSituation();
    UpdateThreatAssessment();
}

void ACombatAIController::InitializePerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configurar Visão
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // Configurar Audição
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // Configurar Detecção de Dano
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);

    // Adicionar configurações à percepção
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind eventos
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState == NewState)
        return;

    ECombatState OldState = CurrentCombatState;
    CurrentCombatState = NewState;

    // Atualizar Blackboard
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    }

    // Trigger Blueprint event
    OnCombatStateChanged(OldState, NewState);

    // Log para debug
    UE_LOG(LogTemp, Log, TEXT("Combat State changed from %d to %d"), static_cast<int32>(OldState), static_cast<int32>(NewState));
}

void ACombatAIController::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CurrentThreatLevel == NewThreatLevel)
        return;

    EThreatLevel OldLevel = CurrentThreatLevel;
    CurrentThreatLevel = NewThreatLevel;

    // Atualizar Blackboard
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CurrentThreatLevel));
    }

    // Trigger Blueprint event
    OnThreatLevelChanged(OldLevel, NewThreatLevel);
}

bool ACombatAIController::IsInCombat() const
{
    return CurrentCombatState != ECombatState::Idle;
}

float ACombatAIController::GetDistanceToTarget() const
{
    if (!LastKnownTarget || !GetPawn())
        return -1.0f;

    return FVector::Dist(GetPawn()->GetActorLocation(), LastKnownTarget->GetActorLocation());
}

bool ACombatAIController::HasLineOfSightToTarget() const
{
    if (!LastKnownTarget || !GetPawn())
        return false;

    FHitResult HitResult;
    FVector Start = GetPawn()->GetActorLocation();
    FVector End = LastKnownTarget->GetActorLocation();

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility
    );

    return !bHit || HitResult.GetActor() == LastKnownTarget;
}

FVector ACombatAIController::GetOptimalAttackPosition() const
{
    if (!LastKnownTarget || !GetPawn())
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = LastKnownTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();

    // Posição ideal: na distância ótima de ataque
    return TargetLocation - (DirectionToTarget * OptimalAttackRange);
}

void ACombatAIController::UpdateThreatAssessment()
{
    if (!LastKnownTarget)
    {
        SetThreatLevel(EThreatLevel::None);
        return;
    }

    float DistanceToTarget = GetDistanceToTarget();
    bool bHasLineOfSight = HasLineOfSightToTarget();
    
    // Calcular nível de ameaça baseado em múltiplos fatores
    EThreatLevel NewThreatLevel = EThreatLevel::Low;

    if (DistanceToTarget < OptimalAttackRange * 0.5f && bHasLineOfSight)
    {
        NewThreatLevel = EThreatLevel::Critical;
    }
    else if (DistanceToTarget < OptimalAttackRange && bHasLineOfSight)
    {
        NewThreatLevel = EThreatLevel::High;
    }
    else if (DistanceToTarget < TerritorialRadius)
    {
        NewThreatLevel = EThreatLevel::Medium;
    }

    SetThreatLevel(NewThreatLevel);
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Verificar se é o player ou outro alvo válido
            FAIStimulus Stimulus;
            if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    LastKnownTarget = Actor;
                    LastKnownTargetLocation = Actor->GetActorLocation();
                    TimeSinceLastTargetSeen = 0.0f;

                    // Atualizar Blackboard
                    if (GetBlackboardComponent())
                    {
                        GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Actor);
                        GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
                    }

                    // Transição para estado de caça se estava idle
                    if (CurrentCombatState == ECombatState::Idle)
                    {
                        SetCombatState(ECombatState::Hunting);
                    }
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == LastKnownTarget)
    {
        if (!Stimulus.WasSuccessfullySensed())
        {
            // Perdeu o alvo
            TimeSinceLastTargetSeen = 0.0f;
            
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), nullptr);
            }
        }
    }
}

void ACombatAIController::UpdateCombatMemory(float DeltaTime)
{
    if (LastKnownTarget)
    {
        if (!HasLineOfSightToTarget())
        {
            TimeSinceLastTargetSeen += DeltaTime;
            
            // Se perdeu o alvo por muito tempo, voltar ao idle
            if (TimeSinceLastTargetSeen > 10.0f)
            {
                LastKnownTarget = nullptr;
                SetCombatState(ECombatState::Idle);
                SetThreatLevel(EThreatLevel::None);
            }
        }
        else
        {
            TimeSinceLastTargetSeen = 0.0f;
            LastKnownTargetLocation = LastKnownTarget->GetActorLocation();
        }
    }

    // Atualizar lista de aliados próximos
    AlliesInRange.Empty();
    if (GetPawn())
    {
        TArray<AActor*> FoundActors;
        UWorld* World = GetWorld();
        if (World)
        {
            for (TActorIterator<APawn> ActorIterator(World); ActorIterator; ++ActorIterator)
            {
                APawn* OtherPawn = *ActorIterator;
                if (OtherPawn && OtherPawn != GetPawn() && OtherPawn->GetController())
                {
                    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), OtherPawn->GetActorLocation());
                    if (Distance < TerritorialRadius)
                    {
                        // Verificar se é aliado (mesma espécie, etc.)
                        if (OtherPawn->GetController()->IsA<ACombatAIController>())
                        {
                            AlliesInRange.Add(OtherPawn);
                        }
                    }
                }
            }
        }
    }

    // Atualizar Blackboard com informações de memória
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsFloat(TEXT("TimeSinceLastTargetSeen"), TimeSinceLastTargetSeen);
        GetBlackboardComponent()->SetValueAsInt(TEXT("AlliesCount"), AlliesInRange.Num());
    }
}

void ACombatAIController::AnalyzeTacticalSituation()
{
    if (!LastKnownTarget || CurrentCombatState == ECombatState::Idle)
        return;

    float DistanceToTarget = GetDistanceToTarget();
    bool bHasLineOfSight = HasLineOfSightToTarget();
    
    // Decidir estratégia baseada na situação atual
    switch (CurrentCombatState)
    {
        case ECombatState::Hunting:
            if (bHasLineOfSight && DistanceToTarget < OptimalAttackRange * 1.5f)
            {
                if (bCanAmbush && AggressionLevel > 0.7f)
                {
                    SetCombatState(ECombatState::Ambushing);
                }
                else
                {
                    SetCombatState(ECombatState::Engaging);
                }
            }
            else if (bHasLineOfSight && DistanceToTarget > OptimalAttackRange * 2.0f)
            {
                SetCombatState(ECombatState::Stalking);
            }
            break;

        case ECombatState::Engaging:
            if (ShouldRetreat())
            {
                SetCombatState(ECombatState::Retreating);
            }
            else if (bCanCircleTarget && DistanceToTarget > OptimalAttackRange * 0.8f)
            {
                SetCombatState(ECombatState::Circling);
            }
            break;

        case ECombatState::Circling:
            if (DistanceToTarget < OptimalAttackRange * 0.6f)
            {
                SetCombatState(ECombatState::Engaging);
            }
            else if (ShouldRetreat())
            {
                SetCombatState(ECombatState::Retreating);
            }
            break;

        case ECombatState::Retreating:
            if (DistanceToTarget > TerritorialRadius && CurrentThreatLevel <= EThreatLevel::Low)
            {
                SetCombatState(ECombatState::Idle);
            }
            break;
    }
}

bool ACombatAIController::ShouldRetreat() const
{
    // Verificar saúde
    if (GetPawn())
    {
        // Assumindo que o Pawn tem um componente de saúde
        // Esta lógica seria expandida com o sistema de saúde real
        return CurrentThreatLevel >= EThreatLevel::Critical && AggressionLevel < 0.3f;
    }
    return false;
}

bool ACombatAIController::ShouldCallForHelp() const
{
    return bCanCallForHelp && 
           CurrentThreatLevel >= EThreatLevel::High && 
           AlliesInRange.Num() > 0;
}

FVector ACombatAIController::CalculateCirclingPosition() const
{
    if (!LastKnownTarget || !GetPawn())
        return GetPawn()->GetActorLocation();

    FVector TargetLocation = LastKnownTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calcular posição perpendicular para circling
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    FVector CirclingDirection = (FMath::RandBool() ? RightVector : -RightVector);
    
    return TargetLocation + (CirclingDirection * CirclingRadius);
}

FVector ACombatAIController::CalculateAmbushPosition() const
{
    if (!LastKnownTarget || !GetPawn())
        return GetPawn()->GetActorLocation();

    // Encontrar posição de emboscada baseada no movimento previsto do alvo
    FVector TargetLocation = LastKnownTarget->GetActorLocation();
    FVector PredictedLocation = TargetLocation; // Seria expandido com predição de movimento
    
    // Encontrar cobertura próxima à posição prevista
    return PredictedLocation + FVector(FMath::RandRange(-300.0f, 300.0f), FMath::RandRange(-300.0f, 300.0f), 0);
}