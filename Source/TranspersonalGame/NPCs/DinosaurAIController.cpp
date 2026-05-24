#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componente de percepção
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Configurações padrão de percepção
    SightRadius = 3000.0f;
    LoseSightRadius = 3500.0f;
    PeripheralVisionAngleDegrees = 90.0f;
    MaxAge = 5.0f;
    HearingRange = 2000.0f;

    // Estado inicial
    DinosaurType = ENPC_DinosaurType::TRex;
    CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
    CurrentTarget = nullptr;
    CurrentPatrolIndex = 0;
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    // Configurar percepção baseada no tipo de dinossauro
    SetupPerceptionForDinosaurType();

    // Bind do callback de percepção
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
    }

    // Definir localização home
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Iniciar timer de update de comportamento
    GetWorld()->GetTimerManager().SetTimer(
        BehaviorUpdateTimer,
        this,
        &ADinosaurAIController::UpdateBehavior,
        1.0f,
        true
    );
}

void ADinosaurAIController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);

    // Iniciar Behavior Tree se disponível
    if (BehaviorTree && BlackboardAsset)
    {
        RunBehaviorTree(BehaviorTree);
        
        if (Blackboard)
        {
            Blackboard->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
            Blackboard->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
        }
    }
}

void ADinosaurAIController::SetupPerceptionForDinosaurType()
{
    if (!AIPerceptionComponent)
        return;

    // Configurar visão
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        // Ajustar parâmetros baseados no tipo de dinossauro
        switch (DinosaurType)
        {
        case ENPC_DinosaurType::TRex:
            SightConfig->SightRadius = 4000.0f;
            SightConfig->LoseSightRadius = 4500.0f;
            SightConfig->PeripheralVisionAngleDegrees = 60.0f;
            break;
        case ENPC_DinosaurType::Raptor:
            SightConfig->SightRadius = 3500.0f;
            SightConfig->LoseSightRadius = 4000.0f;
            SightConfig->PeripheralVisionAngleDegrees = 120.0f;
            break;
        case ENPC_DinosaurType::Herbivore:
            SightConfig->SightRadius = 2500.0f;
            SightConfig->LoseSightRadius = 3000.0f;
            SightConfig->PeripheralVisionAngleDegrees = 180.0f;
            break;
        }

        SightConfig->SetMaxAge(MaxAge);
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configurar audição
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(MaxAge);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Definir sentido dominante
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ADinosaurAIController::SetBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        
        if (Blackboard)
        {
            Blackboard->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }
    }
}

void ADinosaurAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (Blackboard)
    {
        Blackboard->SetValueAsObject(TEXT("TargetActor"), NewTarget);
    }
}

void ADinosaurAIController::SetPatrolPoints(const TArray<FVector>& Points)
{
    PatrolPoints = Points;
    CurrentPatrolIndex = 0;
}

FVector ADinosaurAIController::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
        return HomeLocation;

    FVector NextPoint = PatrolPoints[CurrentPatrolIndex];
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    
    return NextPoint;
}

bool ADinosaurAIController::IsPlayerInSight() const
{
    if (!AIPerceptionComponent)
        return false;

    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    for (AActor* Actor : PerceivedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Verificar se é o jogador
            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC && PC->GetPawn() == Actor)
            {
                return true;
            }
        }
    }

    return false;
}

float ADinosaurAIController::GetDistanceToPlayer() const
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn() && GetPawn())
    {
        return FVector::Dist(GetPawn()->GetActorLocation(), PC->GetPawn()->GetActorLocation());
    }
    
    return -1.0f;
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Verificar se é o jogador
            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC && PC->GetPawn() == Actor)
            {
                float Distance = GetDistanceToPlayer();
                
                // Lógica baseada no tipo de dinossauro
                switch (DinosaurType)
                {
                case ENPC_DinosaurType::TRex:
                    if (Distance < 3000.0f && CurrentBehaviorState != ENPC_DinosaurBehaviorState::Attacking)
                    {
                        SetBehaviorState(ENPC_DinosaurBehaviorState::Chasing);
                        SetTarget(Actor);
                    }
                    break;
                case ENPC_DinosaurType::Raptor:
                    if (Distance < 2500.0f)
                    {
                        SetBehaviorState(ENPC_DinosaurBehaviorState::Hunting);
                        SetTarget(Actor);
                    }
                    break;
                case ENPC_DinosaurType::Herbivore:
                    if (Distance < 2000.0f)
                    {
                        SetBehaviorState(ENPC_DinosaurBehaviorState::Fleeing);
                        SetTarget(Actor);
                    }
                    break;
                }
            }
        }
    }
}

void ADinosaurAIController::UpdateBehavior()
{
    switch (DinosaurType)
    {
    case ENPC_DinosaurType::TRex:
        HandleTRexBehavior();
        break;
    case ENPC_DinosaurType::Raptor:
        HandleRaptorBehavior();
        break;
    case ENPC_DinosaurType::Herbivore:
        HandleHerbivoreBehavior();
        break;
    }
}

void ADinosaurAIController::HandleTRexBehavior()
{
    if (!IsPlayerInSight() && CurrentBehaviorState == ENPC_DinosaurBehaviorState::Chasing)
    {
        // Perder interesse no jogador
        SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
        SetTarget(nullptr);
    }
    
    if (CurrentBehaviorState == ENPC_DinosaurBehaviorState::Idle)
    {
        // Começar patrulha
        SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
    }
}

void ADinosaurAIController::HandleRaptorBehavior()
{
    // Raptors são mais persistentes na caça
    if (CurrentBehaviorState == ENPC_DinosaurBehaviorState::Hunting)
    {
        float Distance = GetDistanceToPlayer();
        if (Distance > 4000.0f)
        {
            // Desistir da caça se muito longe
            SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
            SetTarget(nullptr);
        }
    }
}

void ADinosaurAIController::HandleHerbivoreBehavior()
{
    if (CurrentBehaviorState == ENPC_DinosaurBehaviorState::Fleeing)
    {
        float Distance = GetDistanceToPlayer();
        if (Distance > 3000.0f)
        {
            // Voltar ao comportamento normal
            SetBehaviorState(ENPC_DinosaurBehaviorState::Grazing);
            SetTarget(nullptr);
        }
    }
    
    if (CurrentBehaviorState == ENPC_DinosaurBehaviorState::Idle)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Grazing);
    }
}