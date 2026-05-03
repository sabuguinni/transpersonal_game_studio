#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ANPC_DinosaurAIController::ANPC_DinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componentes de IA
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configurar percepção visual
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 3000.0f;
        SightConfig->LoseSightRadius = 3500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(10.0f);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // Configurar stats padrão
    DinosaurStats.Health = 100.0f;
    DinosaurStats.MaxHealth = 100.0f;
    DinosaurStats.AttackDamage = 25.0f;
    DinosaurStats.MovementSpeed = 400.0f;
    DinosaurStats.SightRange = 3000.0f;
    DinosaurStats.AttackRange = 200.0f;
    DinosaurStats.PatrolRadius = 1500.0f;

    // Estado inicial
    CurrentState = ENPC_DinosaurState::Idle;
    CurrentTarget = nullptr;
}

void ANPC_DinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    // Guardar posição inicial
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Configurar blackboard
    if (BlackboardAsset && BlackboardComponent)
    {
        UseBlackboard(BlackboardAsset);
    }

    // Iniciar behavior tree
    if (BehaviorTree && BehaviorTreeComponent)
    {
        RunBehaviorTree(BehaviorTree);
    }

    // Conectar callback de percepção
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurAIController::OnPerceptionUpdated);
    }

    // Estado inicial de patrulha
    SetDinosaurState(ENPC_DinosaurState::Patrol);
}

void ANPC_DinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (IsAlive())
    {
        UpdateBehavior(DeltaTime);
    }
}

void ANPC_DinosaurAIController::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        // Actualizar blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("DinosaurState"), static_cast<uint8>(NewState));
        }

        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s mudou para estado: %d"), 
               GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), 
               static_cast<int32>(NewState));
    }
}

void ANPC_DinosaurAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
    }
}

void ANPC_DinosaurAIController::TakeDamage(float DamageAmount)
{
    DinosaurStats.Health = FMath::Max(0.0f, DinosaurStats.Health - DamageAmount);
    
    if (DinosaurStats.Health <= 0.0f)
    {
        SetDinosaurState(ENPC_DinosaurState::Dead);
    }
    else if (DinosaurStats.Health < DinosaurStats.MaxHealth * 0.3f)
    {
        // Fugir se a saúde estiver baixa
        SetDinosaurState(ENPC_DinosaurState::Flee);
    }
}

float ANPC_DinosaurAIController::GetHealthPercentage() const
{
    if (DinosaurStats.MaxHealth > 0.0f)
    {
        return DinosaurStats.Health / DinosaurStats.MaxHealth;
    }
    return 0.0f;
}

void ANPC_DinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            // Verificar se é o jogador
            if (Actor->GetName().Contains(TEXT("Character")) || Actor->GetName().Contains(TEXT("Player")))
            {
                float Distance = GetDistanceToTarget(Actor);
                
                if (Distance <= DinosaurStats.SightRange)
                {
                    SetTarget(Actor);
                    
                    if (Distance <= DinosaurStats.AttackRange)
                    {
                        SetDinosaurState(ENPC_DinosaurState::Attack);
                    }
                    else
                    {
                        SetDinosaurState(ENPC_DinosaurState::Hunt);
                    }
                    break;
                }
            }
        }
    }
}

void ANPC_DinosaurAIController::UpdateBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            HandleIdleState();
            break;
        case ENPC_DinosaurState::Patrol:
            HandlePatrolState();
            break;
        case ENPC_DinosaurState::Hunt:
            HandleHuntState();
            break;
        case ENPC_DinosaurState::Attack:
            HandleAttackState();
            break;
        case ENPC_DinosaurState::Flee:
            HandleFleeState();
            break;
        case ENPC_DinosaurState::Dead:
            // Não fazer nada quando morto
            break;
    }
}

void ANPC_DinosaurAIController::HandleIdleState()
{
    // Transição para patrulha após alguns segundos
    static float IdleTimer = 0.0f;
    IdleTimer += GetWorld()->GetDeltaSeconds();
    
    if (IdleTimer >= 3.0f)
    {
        SetDinosaurState(ENPC_DinosaurState::Patrol);
        IdleTimer = 0.0f;
    }
}

void ANPC_DinosaurAIController::HandlePatrolState()
{
    if (!GetPawn()) return;

    // Verificar se chegou ao destino de patrulha
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    float DistanceToHome = FVector::Dist(CurrentLocation, HomeLocation);
    
    if (DistanceToHome > DinosaurStats.PatrolRadius)
    {
        // Voltar para casa
        MoveToLocation(HomeLocation);
    }
    else
    {
        // Mover para ponto aleatório de patrulha
        FVector PatrolPoint = GetRandomPatrolPoint();
        MoveToLocation(PatrolPoint);
    }
}

void ANPC_DinosaurAIController::HandleHuntState()
{
    if (CurrentTarget && IsValid(CurrentTarget))
    {
        float Distance = GetDistanceToTarget(CurrentTarget);
        
        if (Distance <= DinosaurStats.AttackRange)
        {
            SetDinosaurState(ENPC_DinosaurState::Attack);
        }
        else if (Distance <= DinosaurStats.SightRange)
        {
            // Perseguir o alvo
            MoveToActor(CurrentTarget);
        }
        else
        {
            // Perdeu o alvo
            SetTarget(nullptr);
            SetDinosaurState(ENPC_DinosaurState::Patrol);
        }
    }
    else
    {
        SetDinosaurState(ENPC_DinosaurState::Patrol);
    }
}

void ANPC_DinosaurAIController::HandleAttackState()
{
    if (CurrentTarget && IsValid(CurrentTarget))
    {
        float Distance = GetDistanceToTarget(CurrentTarget);
        
        if (Distance <= DinosaurStats.AttackRange)
        {
            // Executar ataque (placeholder)
            UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s atacou %s!"), 
                   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
                   *CurrentTarget->GetName());
            
            // Voltar para caça após ataque
            SetDinosaurState(ENPC_DinosaurState::Hunt);
        }
        else
        {
            SetDinosaurState(ENPC_DinosaurState::Hunt);
        }
    }
    else
    {
        SetDinosaurState(ENPC_DinosaurState::Patrol);
    }
}

void ANPC_DinosaurAIController::HandleFleeState()
{
    if (!GetPawn()) return;

    // Fugir na direcção oposta ao alvo
    if (CurrentTarget && IsValid(CurrentTarget))
    {
        FVector FleeDirection = GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation();
        FleeDirection.Normalize();
        
        FVector FleeLocation = GetPawn()->GetActorLocation() + (FleeDirection * 2000.0f);
        MoveToLocation(FleeLocation);
    }
    else
    {
        // Voltar para casa se não há alvo
        MoveToLocation(HomeLocation);
        SetDinosaurState(ENPC_DinosaurState::Patrol);
    }
}

bool ANPC_DinosaurAIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !GetPawn()) return false;

    FVector Start = GetPawn()->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, Start, End, ECC_Visibility, QueryParams
    );
    
    return !bHit || HitResult.GetActor() == Target;
}

float ANPC_DinosaurAIController::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn()) return FLT_MAX;
    
    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

FVector ANPC_DinosaurAIController::GetRandomPatrolPoint() const
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Manter no plano horizontal
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(500.0f, DinosaurStats.PatrolRadius);
    
    return HomeLocation + (RandomDirection * RandomDistance);
}