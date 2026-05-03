#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componente de percepção
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configurar visão
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 3000.0f;
        SightConfig->LoseSightRadius = 3500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        
        // Detectar jogadores
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configurar Blackboard
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    
    // Valores padrão
    PatrolRadius = 5000.0f;
    MovementSpeed = 300.0f;
    PlayerDetectionRange = 3000.0f;
    bIsChasing = false;
    bIsPatrolling = false;
    CurrentTarget = nullptr;
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
    }
    
    // Guardar posição inicial
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }
    
    // Iniciar patrulha
    StartPatrolling();
}

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    // Iniciar Behavior Tree se definido
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
        
        // Configurar valores iniciais no Blackboard
        BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
        BlackboardComponent->SetValueAsFloat(TEXT("PatrolRadius"), PatrolRadius);
        BlackboardComponent->SetValueAsBool(TEXT("IsChasing"), false);
    }
}

void ADinosaurAIController::StartChasing(AActor* Target)
{
    if (!Target)
        return;
        
    CurrentTarget = Target;
    bIsChasing = true;
    bIsPatrolling = false;
    
    // Parar timer de patrulha
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PatrolTimerHandle);
    }
    
    // Actualizar Blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
        BlackboardComponent->SetValueAsBool(TEXT("IsChasing"), true);
        BlackboardComponent->SetValueAsBool(TEXT("IsPatrolling"), false);
    }
    
    // Mover para o target
    MoveToActor(Target, 100.0f);
}

void ADinosaurAIController::StopChasing()
{
    CurrentTarget = nullptr;
    bIsChasing = false;
    
    // Actualizar Blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
        BlackboardComponent->SetValueAsBool(TEXT("IsChasing"), false);
    }
    
    // Voltar à patrulha
    StartPatrolling();
}

void ADinosaurAIController::SetPatrolLocation(FVector NewLocation)
{
    HomeLocation = NewLocation;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
    }
}

void ADinosaurAIController::StartPatrolling()
{
    if (bIsChasing)
        return;
        
    bIsPatrolling = true;
    
    // Actualizar Blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsPatrolling"), true);
        BlackboardComponent->SetValueAsBool(TEXT("IsChasing"), false);
    }
    
    // Iniciar timer de patrulha
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(PatrolTimerHandle, this, &ADinosaurAIController::PatrolTick, 5.0f, true);
    }
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
            continue;
            
        // Verificar se é o jogador
        if (Actor->IsA<ACharacter>())
        {
            APlayerController* PC = Cast<APlayerController>(Actor->GetInstigatorController());
            if (PC)
            {
                // Jogador detectado - iniciar perseguição
                float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
                
                if (Distance <= PlayerDetectionRange)
                {
                    StartChasing(Actor);
                }
            }
        }
    }
}

void ADinosaurAIController::PatrolTick()
{
    if (bIsChasing || !GetPawn())
        return;
        
    // Gerar posição aleatória dentro do raio de patrulha
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    FVector PatrolTarget = HomeLocation + (RandomDirection * RandomDistance);
    
    // Actualizar Blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolTarget);
    }
    
    // Mover para posição de patrulha
    MoveToLocation(PatrolTarget, 100.0f);
}

void ADinosaurAIController::CheckPlayerDistance()
{
    if (!GetPawn())
        return;
        
    // Encontrar jogador
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player)
        return;
        
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
    
    if (Distance <= PlayerDetectionRange && !bIsChasing)
    {
        // Iniciar perseguição
        StartChasing(Player);
    }
    else if (Distance > PlayerDetectionRange * 1.5f && bIsChasing)
    {
        // Parar perseguição se muito longe
        StopChasing();
    }
}