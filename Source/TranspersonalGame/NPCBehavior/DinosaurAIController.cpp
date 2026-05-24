#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componentes de AI
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configurações padrão
    TerritoryRadius = 5000.0f;
    PatrolSpeed = 200.0f;
    ChaseSpeed = 600.0f;
    SightRange = 3000.0f;
    HearingRange = 1500.0f;

    // Estado inicial
    CurrentBehaviorState = ENPCBehaviorState::Idle;
    DinosaurSpecies = EDinosaurSpecies::TRex;
    HomeLocation = FVector::ZeroVector;
    TargetActor = nullptr;
    LastPlayerSightTime = -1.0f;
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    // Configurar percepção
    SetupAIPerception();

    // Definir localização inicial como território
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    // Configurar comportamento baseado na espécie
    ConfigureBehaviorForSpecies();

    // Iniciar comportamento
    StartPatrolling();
}

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Actualizar estado de comportamento
    UpdateBehaviorState();

    // Actualizar valores do blackboard
    UpdateBlackboardValues();

    // Debug visual (apenas em desenvolvimento)
    if (GetPawn())
    {
        FVector PawnLocation = GetPawn()->GetActorLocation();
        
        // Desenhar território
        DrawDebugCircle(GetWorld(), HomeLocation, TerritoryRadius, 32, FColor::Green, false, 0.1f, 0, 50.0f, FVector(0, 0, 1), FVector(1, 0, 0));
        
        // Desenhar range de visão
        DrawDebugCircle(GetWorld(), PawnLocation, SightRange, 16, FColor::Yellow, false, 0.1f, 0, 25.0f, FVector(0, 0, 1), FVector(1, 0, 0));
    }
}

void ADinosaurAIController::SetupAIPerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configurar visão
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRange;
    SightConfig->LoseSightRadius = SightRange * 1.2f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Configurar audição
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

    // Adicionar configurações ao componente
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind eventos
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
}

void ADinosaurAIController::ConfigureBehaviorForSpecies()
{
    // Configurar baseado na espécie
    switch (DinosaurSpecies)
    {
        case EDinosaurSpecies::TRex:
            TerritoryRadius = 6000.0f;
            PatrolSpeed = 250.0f;
            ChaseSpeed = 800.0f;
            SightRange = 4000.0f;
            if (TRexBehaviorTree)
            {
                RunBehaviorTree(TRexBehaviorTree);
            }
            break;

        case EDinosaurSpecies::Raptor:
            TerritoryRadius = 4000.0f;
            PatrolSpeed = 400.0f;
            ChaseSpeed = 1200.0f;
            SightRange = 3500.0f;
            if (RaptorBehaviorTree)
            {
                RunBehaviorTree(RaptorBehaviorTree);
            }
            break;

        case EDinosaurSpecies::Brachiosaurus:
            TerritoryRadius = 8000.0f;
            PatrolSpeed = 100.0f;
            ChaseSpeed = 200.0f;
            SightRange = 2000.0f;
            if (HerbivoreBehaviorTree)
            {
                RunBehaviorTree(HerbivoreBehaviorTree);
            }
            break;

        default:
            // Configuração padrão
            break;
    }
}

void ADinosaurAIController::UpdateBehaviorState()
{
    if (!GetPawn())
        return;

    // Verificar se pode ver o jogador
    bool bCanSeePlayerNow = CanSeePlayer();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    if (bCanSeePlayerNow)
    {
        LastPlayerSightTime = CurrentTime;
        
        // Se é predador e vê jogador, começar perseguição
        if (DinosaurSpecies == EDinosaurSpecies::TRex || DinosaurSpecies == EDinosaurSpecies::Raptor)
        {
            if (CurrentBehaviorState != ENPCBehaviorState::Chasing)
            {
                ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
                if (PlayerCharacter)
                {
                    StartChasing(PlayerCharacter);
                }
            }
        }
    }
    else
    {
        // Se perdeu de vista há mais de 5 segundos, voltar ao território
        if (CurrentBehaviorState == ENPCBehaviorState::Chasing && 
            (CurrentTime - LastPlayerSightTime) > 5.0f)
        {
            ReturnToTerritory();
        }
    }

    // Executar comportamento actual
    switch (CurrentBehaviorState)
    {
        case ENPCBehaviorState::Patrolling:
            HandlePatrolBehavior();
            break;
        case ENPCBehaviorState::Chasing:
            HandleChaseBehavior();
            break;
        case ENPCBehaviorState::Idle:
            HandleIdleBehavior();
            break;
        default:
            break;
    }
}

void ADinosaurAIController::HandlePatrolBehavior()
{
    if (!GetPawn())
        return;

    // Verificar se chegou ao destino
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    if (GetPathFollowingComponent() && GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle)
    {
        // Escolher novo ponto de patrulha
        FVector NewPatrolPoint = GetRandomPatrolPoint();
        MoveToLocation(NewPatrolPoint, 100.0f);
    }
}

void ADinosaurAIController::HandleChaseBehavior()
{
    if (!TargetActor || !GetPawn())
        return;

    // Continuar a perseguir o alvo
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
    
    if (DistanceToTarget > TerritoryRadius * 2.0f)
    {
        // Muito longe do território, desistir da perseguição
        ReturnToTerritory();
    }
    else
    {
        // Continuar perseguição
        MoveToActor(TargetActor, 200.0f);
    }
}

void ADinosaurAIController::HandleIdleBehavior()
{
    // Após um tempo idle, começar a patrulhar
    static float IdleTime = 0.0f;
    IdleTime += GetWorld()->GetDeltaSeconds();
    
    if (IdleTime > 3.0f)
    {
        StartPatrolling();
        IdleTime = 0.0f;
    }
}

FVector ADinosaurAIController::GetRandomPatrolPoint() const
{
    // Gerar ponto aleatório dentro do território
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(TerritoryRadius * 0.3f, TerritoryRadius * 0.8f);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return HomeLocation + RandomOffset;
}

bool ADinosaurAIController::IsInTerritory(const FVector& Location) const
{
    return FVector::Dist(HomeLocation, Location) <= TerritoryRadius;
}

void ADinosaurAIController::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
        return;

    // Actualizar valores do blackboard para behavior trees
    BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
    BlackboardComponent->SetValueAsFloat(TEXT("TerritoryRadius"), TerritoryRadius);
    BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), TargetActor);
    BlackboardComponent->SetValueAsBool(TEXT("CanSeePlayer"), CanSeePlayer());
    BlackboardComponent->SetValueAsFloat(TEXT("DistanceToPlayer"), GetDistanceToPlayer());
}

void ADinosaurAIController::SetDinosaurSpecies(EDinosaurSpecies Species)
{
    DinosaurSpecies = Species;
    ConfigureBehaviorForSpecies();
}

void ADinosaurAIController::SetTerritoryRadius(float Radius)
{
    TerritoryRadius = FMath::Max(Radius, 1000.0f); // Mínimo 1km
}

void ADinosaurAIController::StartPatrolling()
{
    CurrentBehaviorState = ENPCBehaviorState::Patrolling;
    TargetActor = nullptr;
    
    // Ir para ponto de patrulha
    FVector PatrolPoint = GetRandomPatrolPoint();
    MoveToLocation(PatrolPoint, 100.0f);
}

void ADinosaurAIController::StartChasing(AActor* Target)
{
    if (!Target)
        return;

    CurrentBehaviorState = ENPCBehaviorState::Chasing;
    TargetActor = Target;
    
    // Começar perseguição
    MoveToActor(Target, 200.0f);
}

void ADinosaurAIController::ReturnToTerritory()
{
    CurrentBehaviorState = ENPCBehaviorState::Returning;
    TargetActor = nullptr;
    
    // Voltar para casa
    MoveToLocation(HomeLocation, 200.0f);
    
    // Após chegar, começar patrulha
    FTimerHandle ReturnTimer;
    GetWorldTimerManager().SetTimer(ReturnTimer, [this]()
    {
        StartPatrolling();
    }, 2.0f, false);
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // Processar actores detectados
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Jogador detectado
            if (DinosaurSpecies == EDinosaurSpecies::TRex || DinosaurSpecies == EDinosaurSpecies::Raptor)
            {
                // Predadores reagem agressivamente
                StartChasing(Actor);
            }
            else if (DinosaurSpecies == EDinosaurSpecies::Brachiosaurus)
            {
                // Herbívoros fogem
                CurrentBehaviorState = ENPCBehaviorState::Fleeing;
                // Implementar fuga (mover na direcção oposta)
                FVector FleeDirection = (GetPawn()->GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal();
                FVector FleeLocation = GetPawn()->GetActorLocation() + (FleeDirection * 2000.0f);
                MoveToLocation(FleeLocation, 100.0f);
            }
        }
    }
}

bool ADinosaurAIController::CanSeePlayer() const
{
    if (!AIPerceptionComponent)
        return false;

    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter)
        return false;

    return AIPerceptionComponent->HasActiveStimulus(*PlayerCharacter, UAISightConfig::StaticClass());
}

float ADinosaurAIController::GetDistanceToPlayer() const
{
    if (!GetPawn())
        return -1.0f;

    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter)
        return -1.0f;

    return FVector::Dist(GetPawn()->GetActorLocation(), PlayerCharacter->GetActorLocation());
}