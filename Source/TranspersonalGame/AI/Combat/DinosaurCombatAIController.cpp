#include "DinosaurCombatAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurCombatAIController::ADinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Inicializar componentes
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configurar percepção
    InitializePerception();
}

void ADinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    if (BlackboardAsset)
    {
        UseBlackboard(BlackboardAsset);
    }
    
    if (BehaviorTree)
    {
        RunBehaviorTree(BehaviorTree);
    }
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAIController::OnTargetPerceptionUpdated);
    }
    
    bIsInitialized = true;
}

void ADinosaurCombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsInitialized) return;
    
    UpdateCombatLogic(DeltaTime);
}

void ADinosaurCombatAIController::InitializePerception()
{
    if (!AIPerceptionComponent) return;
    
    // Configurar visão
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 2000.0f;
        SightConfig->LoseSightRadius = 2200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }
    
    // Configurar audição
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1500.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }
    
    // Configurar dano
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    if (DamageConfig)
    {
        DamageConfig->SetMaxAge(10.0f);
        AIPerceptionComponent->ConfigureSense(*DamageConfig);
    }
    
    // Definir sentido dominante
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ADinosaurCombatAIController::UpdateCombatLogic(float DeltaTime)
{
    // Atualizar memória de combate
    CombatMemory.TimeSinceLastSeen += DeltaTime;
    
    // Processar avaliação de ameaças
    ProcessThreatAssessment();
    
    // Executar comportamento de combate
    ExecuteCombatBehavior();
    
    // Atualizar blackboard
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CurrentCombatState));
        GetBlackboardComponent()->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(CombatMemory.ThreatLevel));
        
        if (CombatMemory.LastKnownTarget)
        {
            GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), CombatMemory.LastKnownTarget);
            GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownTargetLocation"), CombatMemory.LastKnownTargetLocation);
        }
    }
}

void ADinosaurCombatAIController::ProcessThreatAssessment()
{
    if (!GetPawn()) return;
    
    // Limpar ameaças antigas
    KnownThreats.RemoveAll([this](AActor* Actor) {
        return !IsValid(Actor) || 
               FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation()) > 3000.0f;
    });
    
    // Avaliar ameaça atual
    if (CombatMemory.LastKnownTarget && IsValid(CombatMemory.LastKnownTarget))
    {
        CombatMemory.ThreatLevel = EvaluateThreatLevel(CombatMemory.LastKnownTarget);
    }
    else
    {
        CombatMemory.ThreatLevel = EDinosaurThreatLevel::None;
    }
}

void ADinosaurCombatAIController::ExecuteCombatBehavior()
{
    switch (CurrentCombatState)
    {
        case EDinosaurCombatState::Hunting:
            ExecuteHuntingBehavior();
            break;
        case EDinosaurCombatState::Stalking:
            ExecuteStalkingBehavior();
            break;
        case EDinosaurCombatState::Attacking:
            ExecuteAttackingBehavior();
            break;
        case EDinosaurCombatState::Fleeing:
            ExecuteFleeingBehavior();
            break;
        case EDinosaurCombatState::Territorial:
            ExecuteTerritorialBehavior();
            break;
        case EDinosaurCombatState::Investigating:
            ExecuteInvestigatingBehavior();
            break;
        default:
            // Estado Idle - comportamento padrão
            break;
    }
}

void ADinosaurCombatAIController::ExecuteHuntingBehavior()
{
    if (!CombatMemory.LastKnownTarget) return;
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CombatMemory.LastKnownTargetLocation);
    
    if (CanSeeTarget(CombatMemory.LastKnownTarget))
    {
        if (bIsAmbushPredator && DistanceToTarget > StalkingDistance)
        {
            SetCombatState(EDinosaurCombatState::Stalking);
        }
        else if (DistanceToTarget <= AttackRange)
        {
            SetCombatState(EDinosaurCombatState::Attacking);
        }
    }
    else if (CombatMemory.TimeSinceLastSeen > MemoryDuration)
    {
        SetCombatState(EDinosaurCombatState::Idle);
        ForgetTarget(CombatMemory.LastKnownTarget);
    }
}

void ADinosaurCombatAIController::ExecuteStalkingBehavior()
{
    if (!CombatMemory.LastKnownTarget) return;
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CombatMemory.LastKnownTargetLocation);
    
    if (CanSeeTarget(CombatMemory.LastKnownTarget))
    {
        if (DistanceToTarget <= AttackRange)
        {
            SetCombatState(EDinosaurCombatState::Attacking);
        }
        else if (DistanceToTarget > StalkingDistance * 1.5f)
        {
            SetCombatState(EDinosaurCombatState::Hunting);
        }
    }
}

void ADinosaurCombatAIController::ExecuteAttackingBehavior()
{
    if (!CombatMemory.LastKnownTarget) return;
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CombatMemory.LastKnownTargetLocation);
    
    if (DistanceToTarget > AttackRange * 1.2f)
    {
        SetCombatState(EDinosaurCombatState::Hunting);
    }
    
    if (ShouldFlee())
    {
        SetCombatState(EDinosaurCombatState::Fleeing);
    }
}

void ADinosaurCombatAIController::ExecuteFleeingBehavior()
{
    if (!CombatMemory.LastKnownTarget) return;
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CombatMemory.LastKnownTargetLocation);
    
    if (DistanceToTarget >= FleeDistance)
    {
        SetCombatState(EDinosaurCombatState::Idle);
    }
}

void ADinosaurCombatAIController::ExecuteTerritorialBehavior()
{
    if (!GetPawn()) return;
    
    // Verificar se há intrusos no território
    bool bIntruderFound = false;
    for (AActor* Threat : KnownThreats)
    {
        if (IsValid(Threat))
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Threat->GetActorLocation());
            if (Distance <= TerritorialRadius)
            {
                bIntruderFound = true;
                UpdateCombatMemory(Threat, Threat->GetActorLocation());
                SetCombatState(EDinosaurCombatState::Hunting);
                break;
            }
        }
    }
    
    if (!bIntruderFound)
    {
        SetCombatState(EDinosaurCombatState::Idle);
    }
}

void ADinosaurCombatAIController::ExecuteInvestigatingBehavior()
{
    // Investigar última localização conhecida
    if (CombatMemory.TimeSinceLastSeen > 5.0f)
    {
        SetCombatState(EDinosaurCombatState::Idle);
    }
}

void ADinosaurCombatAIController::SetCombatState(EDinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        
        // Alertar membros do bando se necessário
        if (bIsPackHunter && (NewState == EDinosaurCombatState::Hunting || NewState == EDinosaurCombatState::Attacking))
        {
            AlertPackMembers(CombatMemory.LastKnownTarget);
        }
    }
}

EDinosaurThreatLevel ADinosaurCombatAIController::EvaluateThreatLevel(AActor* Target)
{
    if (!Target || !GetPawn()) return EDinosaurThreatLevel::None;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    
    // Avaliar baseado na distância e tipo de alvo
    if (Distance <= AttackRange)
    {
        return EDinosaurThreatLevel::Critical;
    }
    else if (Distance <= StalkingDistance)
    {
        return EDinosaurThreatLevel::High;
    }
    else if (Distance <= FleeDistance)
    {
        return EDinosaurThreatLevel::Medium;
    }
    else
    {
        return EDinosaurThreatLevel::Low;
    }
}

bool ADinosaurCombatAIController::CanSeeTarget(AActor* Target)
{
    if (!Target || !AIPerceptionComponent) return false;
    
    FActorPerceptionBlueprintInfo Info;
    AIPerceptionComponent->GetActorsPerception(Target, Info);
    
    for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Stimulus.WasSuccessfullySensed())
        {
            return true;
        }
    }
    
    return false;
}

bool ADinosaurCombatAIController::IsInAttackRange(AActor* Target)
{
    if (!Target || !GetPawn()) return false;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

FVector ADinosaurCombatAIController::GetOptimalAttackPosition(AActor* Target)
{
    if (!Target || !GetPawn()) return FVector::ZeroVector;
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector PawnLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - PawnLocation).GetSafeNormal();
    
    // Posição ideal: próximo o suficiente para atacar, mas com espaço para manobrar
    return TargetLocation - (DirectionToTarget * (AttackRange * 0.8f));
}

void ADinosaurCombatAIController::UpdateCombatMemory(AActor* Target, const FVector& LastSeenLocation)
{
    if (!Target) return;
    
    CombatMemory.LastKnownTarget = Target;
    CombatMemory.LastKnownTargetLocation = LastSeenLocation;
    CombatMemory.TimeSinceLastSeen = 0.0f;
    CombatMemory.ThreatLevel = EvaluateThreatLevel(Target);
    
    // Adicionar à lista de ameaças conhecidas
    KnownThreats.AddUnique(Target);
}

void ADinosaurCombatAIController::ForgetTarget(AActor* Target)
{
    if (CombatMemory.LastKnownTarget == Target)
    {
        CombatMemory.LastKnownTarget = nullptr;
        CombatMemory.LastKnownTargetLocation = FVector::ZeroVector;
        CombatMemory.TimeSinceLastSeen = 0.0f;
        CombatMemory.ThreatLevel = EDinosaurThreatLevel::None;
    }
    
    KnownThreats.Remove(Target);
}

bool ADinosaurCombatAIController::ShouldFlee()
{
    // Lógica para determinar se deve fugir
    // Baseado em saúde, número de inimigos, etc.
    
    if (CombatMemory.bHasBeenDamaged && CombatMemory.ThreatLevel >= EDinosaurThreatLevel::High)
    {
        return true;
    }
    
    // Verificar se está sendo atacado por múltiplos inimigos
    int32 NearbyThreats = 0;
    for (AActor* Threat : KnownThreats)
    {
        if (IsValid(Threat) && FVector::Dist(GetPawn()->GetActorLocation(), Threat->GetActorLocation()) <= AttackRange * 2.0f)
        {
            NearbyThreats++;
        }
    }
    
    return NearbyThreats >= 3; // Fugir se houver 3 ou mais ameaças próximas
}

void ADinosaurCombatAIController::AlertPackMembers(AActor* Threat)
{
    if (!bIsPackHunter || !Threat) return;
    
    for (AActor* Member : PackMembers)
    {
        if (IsValid(Member))
        {
            ADinosaurCombatAIController* MemberController = Cast<ADinosaurCombatAIController>(Member->GetInstigatorController());
            if (MemberController)
            {
                MemberController->UpdateCombatMemory(Threat, Threat->GetActorLocation());
                MemberController->SetCombatState(EDinosaurCombatState::Hunting);
            }
        }
    }
}

void ADinosaurCombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (IsValid(Actor))
        {
            FActorPerceptionBlueprintInfo Info;
            AIPerceptionComponent->GetActorsPerception(Actor, Info);
            
            bool bCurrentlySeen = false;
            FVector LastSeenLocation = Actor->GetActorLocation();
            
            for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    bCurrentlySeen = true;
                    LastSeenLocation = Stimulus.StimulusLocation;
                    break;
                }
            }
            
            if (bCurrentlySeen)
            {
                UpdateCombatMemory(Actor, LastSeenLocation);
                
                // Mudar estado baseado na situação
                if (CurrentCombatState == EDinosaurCombatState::Idle)
                {
                    SetCombatState(EDinosaurCombatState::Hunting);
                }
            }
        }
    }
}

void ADinosaurCombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!IsValid(Actor)) return;
    
    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
    {
        CombatMemory.bHasBeenDamaged = true;
        CombatMemory.LastDamageTime = GetWorld()->GetTimeSeconds();
        
        // Reagir ao dano
        if (CurrentCombatState == EDinosaurCombatState::Idle)
        {
            SetCombatState(EDinosaurCombatState::Investigating);
        }
    }
}