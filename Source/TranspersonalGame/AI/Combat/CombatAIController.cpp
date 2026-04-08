#include "CombatAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Inicializar componentes
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Configurar percepção visual
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    // Configurar percepção auditiva
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    // Configurar percepção de dano
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(10.0f);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);

    // Definir sentido dominante
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Estados iniciais
    CurrentCombatState = ECombatState::Passive;
    CurrentTarget = nullptr;
    CurrentThreat = nullptr;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Conectar callbacks de percepção
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
    }

    // Iniciar behavior tree se definido
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
    }

    // Configurar timer para atualização de lógica de combate
    GetWorld()->GetTimerManager().SetTimer(StateUpdateTimer, this, &ACombatAIController::UpdateCombatLogic, 0.5f, true);

    // Configurar stats baseado no tipo de dinossauro
    switch (DinosaurType)
    {
        case EDinosaurType::SmallPredator:
            CombatStats.AttackDamage = 25.0f;
            CombatStats.AttackRange = 150.0f;
            CombatStats.MovementSpeed = 600.0f;
            CombatStats.DetectionRadius = 1200.0f;
            break;
        case EDinosaurType::MediumPredator:
            CombatStats.AttackDamage = 50.0f;
            CombatStats.AttackRange = 200.0f;
            CombatStats.MovementSpeed = 450.0f;
            CombatStats.DetectionRadius = 1500.0f;
            break;
        case EDinosaurType::LargePredator:
            CombatStats.AttackDamage = 100.0f;
            CombatStats.AttackRange = 300.0f;
            CombatStats.MovementSpeed = 350.0f;
            CombatStats.DetectionRadius = 2000.0f;
            break;
        case EDinosaurType::SmallHerbivore:
            CombatStats.AttackDamage = 10.0f;
            CombatStats.AttackRange = 100.0f;
            CombatStats.MovementSpeed = 700.0f;
            CombatStats.bCanBeDomesticated = true;
            break;
        case EDinosaurType::MediumHerbivore:
            CombatStats.AttackDamage = 30.0f;
            CombatStats.AttackRange = 200.0f;
            CombatStats.MovementSpeed = 400.0f;
            CombatStats.bCanBeDomesticated = true;
            break;
        case EDinosaurType::LargeHerbivore:
            CombatStats.AttackDamage = 80.0f;
            CombatStats.AttackRange = 400.0f;
            CombatStats.MovementSpeed = 250.0f;
            break;
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Atualizar valores do blackboard
    SetBlackboardValues();
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (IsValidTarget(Actor))
        {
            // Novo alvo detectado
            if (!CurrentTarget || GetDistanceToActor(Actor) < GetDistanceToActor(CurrentTarget))
            {
                CurrentTarget = Actor;
                SetCombatState(ECombatState::Hunting);
            }
        }
        else if (IsValidThreat(Actor))
        {
            // Nova ameaça detectada
            CurrentThreat = Actor;
            if (ShouldFlee(Actor))
            {
                SetCombatState(ECombatState::Fleeing);
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Stimulus.WasSuccessfullySensed())
    {
        // Perdeu de vista o alvo/ameaça
        if (Actor == CurrentTarget)
        {
            CurrentTarget = nullptr;
            SetCombatState(ECombatState::Passive);
        }
        else if (Actor == CurrentThreat)
        {
            CurrentThreat = nullptr;
            if (CurrentCombatState == ECombatState::Fleeing)
            {
                SetCombatState(ECombatState::Passive);
            }
        }
    }
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;

        // Lógica específica para mudança de estado
        switch (NewState)
        {
            case ECombatState::Attacking:
                if (CurrentTarget)
                {
                    SetFocus(CurrentTarget);
                }
                break;
            case ECombatState::Fleeing:
                if (CurrentThreat)
                {
                    SetFocus(nullptr);
                }
                break;
            case ECombatState::Passive:
                SetFocus(nullptr);
                CurrentTarget = nullptr;
                break;
        }

        // Atualizar blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
        }
    }
}

void ACombatAIController::AttackTarget(AActor* Target)
{
    if (!CanAttackTarget(Target))
        return;

    // Verificar cooldown
    if (GetWorld()->GetTimerManager().IsTimerActive(AttackCooldownTimer))
        return;

    // Executar ataque
    if (APawn* ControlledPawn = GetPawn())
    {
        // Aqui seria implementado o sistema de ataque específico
        // Por agora, apenas aplicar dano direto
        if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
        {
            // Aplicar dano (implementação básica)
            UE_LOG(LogTemp, Warning, TEXT("Dinosaur attacking target for %f damage"), CombatStats.AttackDamage);
        }
    }

    // Iniciar cooldown
    GetWorld()->GetTimerManager().SetTimer(AttackCooldownTimer, CombatStats.AttackCooldown, false);
}

bool ACombatAIController::CanAttackTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
        return false;

    float Distance = GetDistanceToActor(Target);
    return Distance <= CombatStats.AttackRange && HasLineOfSight(Target);
}

void ACombatAIController::FleeFromThreat(AActor* Threat)
{
    if (!Threat)
        return;

    SetCombatState(ECombatState::Fleeing);
    CurrentThreat = Threat;
    
    // Calcular direção de fuga
    if (APawn* ControlledPawn = GetPawn())
    {
        FVector FleeDirection = (ControlledPawn->GetActorLocation() - Threat->GetActorLocation()).GetSafeNormal();
        FVector FleeLocation = ControlledPawn->GetActorLocation() + (FleeDirection * 2000.0f);
        
        // Mover para posição de fuga
        MoveToLocation(FleeLocation, 50.0f);
    }
}

void ACombatAIController::StartHunting()
{
    if (DinosaurType == EDinosaurType::SmallHerbivore || 
        DinosaurType == EDinosaurType::MediumHerbivore || 
        DinosaurType == EDinosaurType::LargeHerbivore)
    {
        // Herbívoros não caçam
        return;
    }

    SetCombatState(ECombatState::Hunting);
}

void ACombatAIController::StopCombat()
{
    SetCombatState(ECombatState::Passive);
    CurrentTarget = nullptr;
    CurrentThreat = nullptr;
    StopMovement();
}

void ACombatAIController::IncreaseDomesticationProgress(float Amount)
{
    if (!CombatStats.bCanBeDomesticated)
        return;

    CombatStats.DomesticationProgress = FMath::Clamp(CombatStats.DomesticationProgress + Amount, 0.0f, 100.0f);
    
    if (CombatStats.DomesticationProgress >= 100.0f)
    {
        SetCombatState(ECombatState::Domesticated);
    }
}

bool ACombatAIController::IsDomesticated() const
{
    return CurrentCombatState == ECombatState::Domesticated;
}

void ACombatAIController::UpdateCombatLogic()
{
    if (!GetPawn())
        return;

    EvaluateThreats();
    
    switch (DinosaurType)
    {
        case EDinosaurType::SmallPredator:
        case EDinosaurType::MediumPredator:
        case EDinosaurType::LargePredator:
            UpdatePredatorBehavior();
            break;
        case EDinosaurType::SmallHerbivore:
        case EDinosaurType::MediumHerbivore:
        case EDinosaurType::LargeHerbivore:
            UpdateHerbivoreBehavior();
            break;
    }
}

void ACombatAIController::EvaluateThreats()
{
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

    AActor* ClosestThreat = nullptr;
    float ClosestDistance = FLT_MAX;

    for (AActor* Actor : PerceivedActors)
    {
        if (IsValidThreat(Actor))
        {
            float Distance = GetDistanceToActor(Actor);
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestThreat = Actor;
            }
        }
    }

    if (ClosestThreat && ShouldFlee(ClosestThreat))
    {
        FleeFromThreat(ClosestThreat);
    }
}

void ACombatAIController::SelectTarget()
{
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

    AActor* BestTarget = nullptr;
    float ClosestDistance = FLT_MAX;

    for (AActor* Actor : PerceivedActors)
    {
        if (IsValidTarget(Actor))
        {
            float Distance = GetDistanceToActor(Actor);
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                BestTarget = Actor;
            }
        }
    }

    if (BestTarget)
    {
        CurrentTarget = BestTarget;
        SetCombatState(ECombatState::Hunting);
    }
}

bool ACombatAIController::ShouldFlee(AActor* Threat) const
{
    if (!Threat)
        return false;

    // Herbívoros sempre fogem de predadores
    if (DinosaurType == EDinosaurType::SmallHerbivore || 
        DinosaurType == EDinosaurType::MediumHerbivore)
    {
        return true;
    }

    // Predadores pequenos fogem de predadores maiores
    if (DinosaurType == EDinosaurType::SmallPredator)
    {
        // Verificar se a ameaça é um predador maior
        if (ACombatAIController* ThreatController = Cast<ACombatAIController>(Threat->GetInstigatorController()))
        {
            return ThreatController->GetDinosaurType() == EDinosaurType::MediumPredator ||
                   ThreatController->GetDinosaurType() == EDinosaurType::LargePredator;
        }
    }

    return false;
}

bool ACombatAIController::IsValidTarget(AActor* Actor) const
{
    if (!Actor || Actor == GetPawn())
        return false;

    // Predadores caçam jogador e herbívoros menores
    if (DinosaurType == EDinosaurType::SmallPredator || 
        DinosaurType == EDinosaurType::MediumPredator || 
        DinosaurType == EDinosaurType::LargePredator)
    {
        // Verificar se é o jogador
        if (Actor->IsA<ACharacter>() && Actor->Tags.Contains("Player"))
        {
            return true;
        }

        // Verificar se é herbívoro menor
        if (ACombatAIController* OtherController = Cast<ACombatAIController>(Actor->GetInstigatorController()))
        {
            EDinosaurType OtherType = OtherController->GetDinosaurType();
            return OtherType == EDinosaurType::SmallHerbivore || 
                   (DinosaurType != EDinosaurType::SmallPredator && OtherType == EDinosaurType::MediumHerbivore);
        }
    }

    return false;
}

bool ACombatAIController::IsValidThreat(AActor* Actor) const
{
    if (!Actor || Actor == GetPawn())
        return false;

    // Verificar se é um predador maior
    if (ACombatAIController* OtherController = Cast<ACombatAIController>(Actor->GetInstigatorController()))
    {
        EDinosaurType OtherType = OtherController->GetDinosaurType();
        
        // Herbívoros vêem todos os predadores como ameaça
        if (DinosaurType == EDinosaurType::SmallHerbivore || 
            DinosaurType == EDinosaurType::MediumHerbivore || 
            DinosaurType == EDinosaurType::LargeHerbivore)
        {
            return OtherType == EDinosaurType::SmallPredator ||
                   OtherType == EDinosaurType::MediumPredator ||
                   OtherType == EDinosaurType::LargePredator;
        }

        // Predadores pequenos vêem predadores maiores como ameaça
        if (DinosaurType == EDinosaurType::SmallPredator)
        {
            return OtherType == EDinosaurType::MediumPredator ||
                   OtherType == EDinosaurType::LargePredator;
        }

        // Predadores médios vêem predadores grandes como ameaça
        if (DinosaurType == EDinosaurType::MediumPredator)
        {
            return OtherType == EDinosaurType::LargePredator;
        }
    }

    return false;
}

void ACombatAIController::UpdatePredatorBehavior()
{
    switch (CurrentCombatState)
    {
        case ECombatState::Passive:
            // Procurar por alvos
            SelectTarget();
            break;
        case ECombatState::Hunting:
            if (CurrentTarget)
            {
                float Distance = GetDistanceToActor(CurrentTarget);
                if (Distance <= CombatStats.AttackRange)
                {
                    SetCombatState(ECombatState::Attacking);
                }
                else
                {
                    // Mover em direção ao alvo
                    MoveToActor(CurrentTarget, CombatStats.AttackRange * 0.8f);
                }
            }
            else
            {
                SetCombatState(ECombatState::Passive);
            }
            break;
        case ECombatState::Attacking:
            if (CurrentTarget)
            {
                AttackTarget(CurrentTarget);
                
                // Verificar se ainda está em range
                if (GetDistanceToActor(CurrentTarget) > CombatStats.AttackRange)
                {
                    SetCombatState(ECombatState::Hunting);
                }
            }
            else
            {
                SetCombatState(ECombatState::Passive);
            }
            break;
    }
}

void ACombatAIController::UpdateHerbivoreBehavior()
{
    switch (CurrentCombatState)
    {
        case ECombatState::Passive:
            // Comportamento de pastoreio/exploração
            break;
        case ECombatState::Fleeing:
            // Continuar fugindo se a ameaça ainda estiver presente
            if (!CurrentThreat || GetDistanceToActor(CurrentThreat) > CombatStats.DetectionRadius * 2.0f)
            {
                SetCombatState(ECombatState::Passive);
            }
            break;
    }
}

void ACombatAIController::UpdatePackBehavior()
{
    // Implementação futura para comportamento de matilha
    // Velociraptors, por exemplo, caçariam em grupo
}

float ACombatAIController::GetDistanceToActor(AActor* Actor) const
{
    if (!Actor || !GetPawn())
        return FLT_MAX;

    return FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
}

bool ACombatAIController::HasLineOfSight(AActor* Actor) const
{
    if (!Actor || !GetPawn())
        return false;

    FHitResult HitResult;
    FVector Start = GetPawn()->GetActorLocation();
    FVector End = Actor->GetActorLocation();
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    QueryParams.AddIgnoredActor(Actor);

    return !GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
}

void ACombatAIController::SetBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
    BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
    BlackboardComponent->SetValueAsObject(TEXT("CurrentThreat"), CurrentThreat);
    BlackboardComponent->SetValueAsFloat(TEXT("AttackRange"), CombatStats.AttackRange);
    BlackboardComponent->SetValueAsFloat(TEXT("DetectionRadius"), CombatStats.DetectionRadius);
    BlackboardComponent->SetValueAsBool(TEXT("IsDomesticated"), IsDomesticated());
    BlackboardComponent->SetValueAsFloat(TEXT("DomesticationProgress"), CombatStats.DomesticationProgress);
}