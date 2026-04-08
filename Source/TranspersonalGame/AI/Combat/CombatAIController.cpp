#include "CombatAIController.h"
#include "CombatAIComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Definição das Blackboard Keys
const FName ACombatAIController::BB_CurrentTarget(TEXT("CurrentTarget"));
const FName ACombatAIController::BB_CombatState(TEXT("CombatState"));
const FName ACombatAIController::BB_LastKnownTargetLocation(TEXT("LastKnownTargetLocation"));
const FName ACombatAIController::BB_CurrentAggression(TEXT("CurrentAggression"));
const FName ACombatAIController::BB_CurrentFear(TEXT("CurrentFear"));
const FName ACombatAIController::BB_InCombat(TEXT("InCombat"));
const FName ACombatAIController::BB_ShouldRetreat(TEXT("ShouldRetreat"));
const FName ACombatAIController::BB_ShouldFlee(TEXT("ShouldFlee"));
const FName ACombatAIController::BB_OptimalCombatPosition(TEXT("OptimalCombatPosition"));
const FName ACombatAIController::BB_AlliesInRange(TEXT("AlliesInRange"));
const FName ACombatAIController::BB_EnemiesInRange(TEXT("EnemiesInRange"));

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Criar componentes
    CombatAIComponent = CreateDefaultSubobject<UCombatAIComponent>(TEXT("CombatAIComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configurar Blackboard e Behavior Tree components
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerception();
    InitializeBehaviorTree();
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
    
    if (CombatBehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
}

void ACombatAIController::InitializePerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configurar Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = CombatConfig.PerceptionConfig.SightRadius;
    SightConfig->LoseSightRadius = CombatConfig.PerceptionConfig.SightRadius * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = CombatConfig.PerceptionConfig.SightAngle;
    SightConfig->SetMaxAge(CombatConfig.PerceptionConfig.LoseTargetTime);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configurar Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = CombatConfig.PerceptionConfig.HearingRadius;
    HearingConfig->SetMaxAge(CombatConfig.PerceptionConfig.LoseTargetTime);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configurar Damage
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    DamageConfig->SetMaxAge(5.0f);

    // Adicionar configurações ao componente
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->ConfigureSense(*DamageConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind eventos
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
}

void ACombatAIController::InitializeBehaviorTree()
{
    if (CombatBlackboard && BlackboardComponent)
    {
        UseBlackboard(CombatBlackboard);
    }
}

void ACombatAIController::UpdateCombatLogic(float DeltaTime)
{
    UpdateAggression(DeltaTime);
    UpdateFear(DeltaTime);
    ProcessNearbyActors();
    
    // Atualizar tempo desde última vez que viu o target
    if (RuntimeData.CurrentTarget)
    {
        RuntimeData.TimeSinceLastTargetSeen += DeltaTime;
        
        // Perder target se não visto há muito tempo
        if (RuntimeData.TimeSinceLastTargetSeen > CombatConfig.PerceptionConfig.LoseTargetTime)
        {
            SetTarget(nullptr);
        }
    }
    
    // Atualizar cooldown de ataques
    RuntimeData.LastAttackTime += DeltaTime;
}

void ACombatAIController::UpdateAggression(float DeltaTime)
{
    float TargetAggression = CombatConfig.AggressionLevel;
    
    // Aumentar agressão se em combate
    if (RuntimeData.bInCombat)
    {
        TargetAggression += 0.3f;
    }
    
    // Aumentar agressão se há inimigos próximos
    if (RuntimeData.EnemiesInRange.Num() > 0)
    {
        TargetAggression += 0.2f * RuntimeData.EnemiesInRange.Num();
    }
    
    // Diminuir agressão se com medo
    TargetAggression -= RuntimeData.CurrentFear * 0.5f;
    
    // Interpolar suavemente
    RuntimeData.CurrentAggression = FMath::FInterpTo(RuntimeData.CurrentAggression, 
        FMath::Clamp(TargetAggression, 0.0f, 1.0f), DeltaTime, 2.0f);
}

void ACombatAIController::UpdateFear(float DeltaTime)
{
    float TargetFear = 0.0f;
    
    // Aumentar medo se há muitos inimigos
    if (RuntimeData.EnemiesInRange.Num() > RuntimeData.AlliesInRange.Num() + 1)
    {
        TargetFear += 0.4f;
    }
    
    // Aumentar medo se com pouca vida
    if (APawn* ControlledPawn = GetPawn())
    {
        // Assumindo que o Pawn tem um sistema de saúde
        // TargetFear += (1.0f - HealthPercentage) * 0.3f;
    }
    
    // Diminuir medo se há aliados próximos
    if (RuntimeData.AlliesInRange.Num() > 0)
    {
        TargetFear -= 0.2f * RuntimeData.AlliesInRange.Num();
    }
    
    // Interpolar suavemente
    RuntimeData.CurrentFear = FMath::FInterpTo(RuntimeData.CurrentFear, 
        FMath::Clamp(TargetFear, 0.0f, 1.0f), DeltaTime, 3.0f);
}

void ACombatAIController::ProcessNearbyActors()
{
    RuntimeData.AlliesInRange = FindAlliesInRange();
    RuntimeData.EnemiesInRange = FindEnemiesInRange();
}

void ACombatAIController::SetCombatState(ECombatState NewState)
{
    if (RuntimeData.CurrentState != NewState)
    {
        ECombatState OldState = RuntimeData.CurrentState;
        RuntimeData.CurrentState = NewState;
        
        // Atualizar flag de combate
        RuntimeData.bInCombat = (NewState == ECombatState::Engaging || 
                                NewState == ECombatState::Attacking ||
                                NewState == ECombatState::Stalking);
        
        OnCombatStateChanged.Broadcast(OldState, NewState);
    }
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    if (RuntimeData.CurrentTarget != NewTarget)
    {
        AActor* OldTarget = RuntimeData.CurrentTarget;
        RuntimeData.CurrentTarget = NewTarget;
        
        if (NewTarget)
        {
            RuntimeData.LastKnownTargetLocation = NewTarget->GetActorLocation();
            RuntimeData.TimeSinceLastTargetSeen = 0.0f;
            OnTargetAcquired.Broadcast(NewTarget);
        }
        else if (OldTarget)
        {
            OnTargetLost.Broadcast(OldTarget);
        }
    }
}

bool ACombatAIController::CanAttackTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
        return false;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    
    // Verificar se algum ataque está no range
    for (const FCombatAttackConfig& Attack : CombatConfig.AvailableAttacks)
    {
        if (Distance <= Attack.Range)
        {
            // Verificar cooldown
            if (RuntimeData.LastAttackTime >= Attack.Cooldown)
            {
                // Verificar line of sight se necessário
                if (!Attack.bRequiresLineOfSight || 
                    GetWorld()->LineTraceSingleByChannel(FHitResult(), 
                        GetPawn()->GetActorLocation(), Target->GetActorLocation(), 
                        ECC_Visibility))
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void ACombatAIController::ExecuteAttack(EAttackType AttackType)
{
    // Encontrar configuração do ataque
    const FCombatAttackConfig* AttackConfig = nullptr;
    for (const FCombatAttackConfig& Attack : CombatConfig.AvailableAttacks)
    {
        if (Attack.AttackType == AttackType)
        {
            AttackConfig = &Attack;
            break;
        }
    }
    
    if (!AttackConfig || !RuntimeData.CurrentTarget)
        return;
    
    // Verificar se pode atacar
    if (!CanAttackTarget(RuntimeData.CurrentTarget))
        return;
    
    // Executar ataque (implementação específica do jogo)
    RuntimeData.LastAttackTime = 0.0f;
    
    // Aqui seria chamado o sistema de animação e dano
    // Por exemplo: PlayAttackAnimation(AttackType);
    // ApplyDamageToTarget(RuntimeData.CurrentTarget, AttackConfig->Damage);
}

void ACombatAIController::CallForHelp()
{
    if (!CombatConfig.bCanCallForHelp || !GetPawn())
        return;
    
    // Encontrar aliados próximos e notificá-los
    TArray<AActor*> Allies = FindAlliesInRange();
    for (AActor* Ally : Allies)
    {
        if (ACombatAIController* AllyController = Cast<ACombatAIController>(Ally->GetInstigatorController()))
        {
            AllyController->RespondToHelpCall(GetPawn());
        }
    }
}

void ACombatAIController::RespondToHelpCall(AActor* Caller)
{
    // Lógica para responder a chamado de ajuda
    if (RuntimeData.CurrentState == ECombatState::Idle || 
        RuntimeData.CurrentState == ECombatState::Patrolling)
    {
        SetCombatState(ECombatState::Investigating);
        // Mover em direção ao caller
    }
}

float ACombatAIController::CalculateThreatLevel(AActor* Target) const
{
    if (!Target || !GetPawn())
        return 0.0f;
    
    float ThreatLevel = 0.5f; // Base threat
    
    // Fator distância (mais próximo = mais ameaçador)
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    ThreatLevel += FMath::Clamp(1.0f - (Distance / 1000.0f), 0.0f, 0.3f);
    
    // Fator de movimento (se movendo em direção = mais ameaçador)
    FVector ToTarget = (Target->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
    FVector TargetVelocity = Target->GetVelocity().GetSafeNormal();
    float MovementThreat = FVector::DotProduct(TargetVelocity, ToTarget);
    ThreatLevel += FMath::Clamp(MovementThreat, 0.0f, 0.2f);
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

FVector ACombatAIController::FindOptimalCombatPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
        return GetPawn()->GetActorLocation();
    
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    // Posição ideal: na distância preferida de combate
    FVector OptimalPosition = TargetLocation - (DirectionToTarget * CombatConfig.PreferredCombatDistance);
    
    // Aqui poderia usar EQS para encontrar a melhor posição considerando:
    // - Cobertura disponível
    // - Terreno favorável
    // - Posição de aliados
    // - Rotas de escape
    
    return OptimalPosition;
}

bool ACombatAIController::ShouldRetreat() const
{
    // Recuar se com medo alto ou muitos inimigos
    return RuntimeData.CurrentFear > CombatConfig.FearThreshold ||
           RuntimeData.EnemiesInRange.Num() > RuntimeData.AlliesInRange.Num() + 2;
}

bool ACombatAIController::ShouldFlee() const
{
    // Fugir se com muito medo e poucos aliados
    return RuntimeData.CurrentFear > 0.7f && RuntimeData.AlliesInRange.Num() == 0;
}

TArray<AActor*> ACombatAIController::FindAlliesInRange() const
{
    TArray<AActor*> Allies;
    
    if (!GetPawn())
        return Allies;
    
    // Implementar lógica para encontrar aliados
    // Por exemplo, usando overlap sphere ou perception
    
    return Allies;
}

TArray<AActor*> ACombatAIController::FindEnemiesInRange() const
{
    TArray<AActor*> Enemies;
    
    if (!GetPawn())
        return Enemies;
    
    // Implementar lógica para encontrar inimigos
    // Por exemplo, usando overlap sphere ou perception
    
    return Enemies;
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            // Processar ator percebido
            float ThreatLevel = CalculateThreatLevel(Actor);
            
            // Se é uma ameaça significativa e não temos target, definir como target
            if (ThreatLevel > 0.6f && !RuntimeData.CurrentTarget)
            {
                SetTarget(Actor);
                SetCombatState(ECombatState::Investigating);
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == RuntimeData.CurrentTarget)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // Target visto novamente
            RuntimeData.LastKnownTargetLocation = Stimulus.StimulusLocation;
            RuntimeData.TimeSinceLastTargetSeen = 0.0f;
        }
    }
}

void ACombatAIController::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
        return;
    
    BlackboardComponent->SetValueAsObject(BB_CurrentTarget, RuntimeData.CurrentTarget);
    BlackboardComponent->SetValueAsEnum(BB_CombatState, static_cast<uint8>(RuntimeData.CurrentState));
    BlackboardComponent->SetValueAsVector(BB_LastKnownTargetLocation, RuntimeData.LastKnownTargetLocation);
    BlackboardComponent->SetValueAsFloat(BB_CurrentAggression, RuntimeData.CurrentAggression);
    BlackboardComponent->SetValueAsFloat(BB_CurrentFear, RuntimeData.CurrentFear);
    BlackboardComponent->SetValueAsBool(BB_InCombat, RuntimeData.bInCombat);
    BlackboardComponent->SetValueAsBool(BB_ShouldRetreat, ShouldRetreat());
    BlackboardComponent->SetValueAsBool(BB_ShouldFlee, ShouldFlee());
    
    if (RuntimeData.CurrentTarget)
    {
        FVector OptimalPos = FindOptimalCombatPosition(RuntimeData.CurrentTarget);
        BlackboardComponent->SetValueAsVector(BB_OptimalCombatPosition, OptimalPos);
    }
}