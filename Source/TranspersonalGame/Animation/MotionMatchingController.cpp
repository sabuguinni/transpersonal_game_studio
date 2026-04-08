#include "MotionMatchingController.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogMotionMatching, Log, All);

UMotionMatchingController::UMotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Configurações padrão
    TrajectoryPredictionTime = 1.0f;
    TrajectoryHistoryCount = 3;
    PoseHistoryTime = 0.5f;
    DefaultBlendTime = 0.2f;
    PanicBlendTime = 0.1f;
    StealthBlendTime = 0.4f;
    QueryCooldown = 0.016f; // ~60fps
    
    // Inicializar pesos padrão
    DatabaseWeights.IdleWeight = 1.0f;
    DatabaseWeights.LocomotionWeight = 1.0f;
    DatabaseWeights.PanicWeight = 1.5f;
    DatabaseWeights.StealthWeight = 0.8f;
    DatabaseWeights.InjuredWeight = 1.2f;
    DatabaseWeights.ExhaustedWeight = 0.9f;
    
    // Reservar espaço para arrays de trajetória
    TrajectoryHistory.Reserve(TrajectoryHistoryCount);
    PredictedTrajectory.Reserve(10); // Predição para próximos frames
}

void UMotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar referência do sistema de animação
    if (AActor* Owner = GetOwner())
    {
        AnimationSystem = Owner->FindComponentByClass<UAnimationSystemCore>();
        if (!AnimationSystem)
        {
            UE_LOG(LogMotionMatching, Warning, TEXT("No AnimationSystemCore found on %s"), *Owner->GetName());
        }
    }
    
    // Inicializar histórico de trajetória
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        FVector CurrentLocation = Character->GetActorLocation();
        for (int32 i = 0; i < TrajectoryHistoryCount; i++)
        {
            TrajectoryHistory.Add(CurrentLocation);
        }
    }
    
    UE_LOG(LogMotionMatching, Log, TEXT("Motion Matching Controller initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UMotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Atualizar trajetória
    UpdateTrajectoryHistory(DeltaTime);
    UpdateTrajectoryPrediction(DeltaTime);
    
    // Otimizar frequência de queries
    OptimizeQueryFrequency(DeltaTime);
}

FMotionMatchingQuery UMotionMatchingController::BuildQuery() const
{
    FMotionMatchingQuery Query;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return Query;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return Query;
    }
    
    // Velocidades atuais
    Query.CurrentVelocity = MovementComp->Velocity;
    Query.CurrentSpeed = Query.CurrentVelocity.Size();
    Query.CurrentRotation = Character->GetActorRotation();
    
    // Velocidades desejadas (baseadas no input)
    FVector InputVector = MovementComp->GetLastInputVector();
    Query.DesiredVelocity = InputVector * MovementComp->GetMaxSpeed();
    Query.DesiredSpeed = Query.DesiredVelocity.Size();
    
    if (!InputVector.IsZero())
    {
        Query.DesiredRotation = UKismetMathLibrary::FindLookAtRotation(
            Character->GetActorLocation(),
            Character->GetActorLocation() + InputVector
        );
    }
    else
    {
        Query.DesiredRotation = Query.CurrentRotation;
    }
    
    // Determinar estado de movimento
    if (Query.CurrentSpeed < 10.0f)
    {
        Query.MovementState = ECharacterMovementState::Idle;
    }
    else if (Query.CurrentSpeed < 300.0f)
    {
        Query.MovementState = ECharacterMovementState::Walking;
    }
    else if (Query.CurrentSpeed < 600.0f)
    {
        Query.MovementState = ECharacterMovementState::Running;
    }
    else
    {
        Query.MovementState = ECharacterMovementState::Sprinting;
    }
    
    // Obter dados contextuais do sistema de animação
    if (AnimationSystem)
    {
        Query.ThreatLevel = AnimationSystem->GetCurrentThreatLevel();
        Query.FearLevel = AnimationSystem->GetCurrentFearLevel();
        Query.StaminaLevel = AnimationSystem->GetCurrentStaminaLevel();
    }
    
    return Query;
}

UPoseSearchDatabase* UMotionMatchingController::SelectOptimalDatabase(const FMotionMatchingQuery& Query) const
{
    UPoseSearchDatabase* BestDatabase = nullptr;
    float BestScore = -1.0f;
    
    // Avaliar databases por estado de movimento
    if (UPoseSearchDatabase** StateDB = StateDatabases.Find(Query.MovementState))
    {
        if (*StateDB)
        {
            float Score = CalculateDatabaseScore(*StateDB, Query);
            if (Score > BestScore)
            {
                BestScore = Score;
                BestDatabase = *StateDB;
            }
        }
    }
    
    // Avaliar databases por nível de ameaça
    if (UPoseSearchDatabase** ThreatDB = ThreatDatabases.Find(Query.ThreatLevel))
    {
        if (*ThreatDB)
        {
            float Score = CalculateDatabaseScore(*ThreatDB, Query);
            if (Score > BestScore)
            {
                BestScore = Score;
                BestDatabase = *ThreatDB;
            }
        }
    }
    
    // Fallback para database padrão de locomoção
    if (!BestDatabase)
    {
        if (UPoseSearchDatabase** DefaultDB = StateDatabases.Find(ECharacterMovementState::Walking))
        {
            BestDatabase = *DefaultDB;
        }
    }
    
    return BestDatabase;
}

float UMotionMatchingController::GetBlendTimeForState(ECharacterMovementState State) const
{
    switch (State)
    {
        case ECharacterMovementState::Idle:
            return DefaultBlendTime;
        case ECharacterMovementState::Walking:
            return DefaultBlendTime;
        case ECharacterMovementState::Running:
            return DefaultBlendTime * 0.8f;
        case ECharacterMovementState::Sprinting:
            return PanicBlendTime;
        case ECharacterMovementState::Crouching:
            return StealthBlendTime;
        case ECharacterMovementState::Crawling:
            return StealthBlendTime * 1.5f;
        default:
            return DefaultBlendTime;
    }
}

void UMotionMatchingController::UpdateTrajectoryPrediction(float DeltaTime)
{
    PredictFutureTrajectory();
}

bool UMotionMatchingController::ShouldUseContextualAnimation(const FMotionMatchingQuery& Query) const
{
    // Usar animação contextual em situações específicas
    if (Query.ThreatLevel >= EThreatLevel::High)
    {
        return true;
    }
    
    if (Query.FearLevel > 0.7f)
    {
        return true;
    }
    
    if (Query.StaminaLevel < 0.3f)
    {
        return true;
    }
    
    // Mudanças bruscas de direção
    if (TrajectoryHistory.Num() >= 2)
    {
        FVector RecentDirection = (TrajectoryHistory.Last() - TrajectoryHistory[TrajectoryHistory.Num() - 2]).GetSafeNormal();
        FVector DesiredDirection = Query.DesiredVelocity.GetSafeNormal();
        
        float DirectionChange = FVector::DotProduct(RecentDirection, DesiredDirection);
        if (DirectionChange < 0.0f) // Mudança de mais de 90 graus
        {
            return true;
        }
    }
    
    return false;
}

void UMotionMatchingController::UpdateTrajectoryHistory(float DeltaTime)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }
    
    FVector CurrentLocation = Character->GetActorLocation();
    
    // Adicionar posição atual ao histórico
    TrajectoryHistory.Add(CurrentLocation);
    
    // Manter tamanho do histórico
    if (TrajectoryHistory.Num() > TrajectoryHistoryCount)
    {
        TrajectoryHistory.RemoveAt(0);
    }
}

void UMotionMatchingController::PredictFutureTrajectory()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return;
    }
    
    PredictedTrajectory.Empty();
    
    FVector CurrentLocation = Character->GetActorLocation();
    FVector CurrentVelocity = MovementComp->Velocity;
    FVector InputVector = MovementComp->GetLastInputVector();
    
    // Predizer trajetória baseada na velocidade atual e input
    const float PredictionSteps = 10;
    const float StepTime = TrajectoryPredictionTime / PredictionSteps;
    
    for (int32 i = 0; i < PredictionSteps; i++)
    {
        // Simular movimento futuro
        FVector PredictedVelocity = FMath::VInterpTo(
            CurrentVelocity,
            InputVector * MovementComp->GetMaxSpeed(),
            StepTime,
            5.0f // Velocidade de interpolação
        );
        
        CurrentLocation += PredictedVelocity * StepTime;
        CurrentVelocity = PredictedVelocity;
        
        PredictedTrajectory.Add(CurrentLocation);
    }
}

float UMotionMatchingController::CalculateDatabaseScore(UPoseSearchDatabase* Database, const FMotionMatchingQuery& Query) const
{
    if (!Database)
    {
        return 0.0f;
    }
    
    float Score = 1.0f;
    
    // Aplicar pesos baseados no estado
    switch (Query.MovementState)
    {
        case ECharacterMovementState::Idle:
            Score *= DatabaseWeights.IdleWeight;
            break;
        case ECharacterMovementState::Walking:
        case ECharacterMovementState::Running:
        case ECharacterMovementState::Sprinting:
            Score *= DatabaseWeights.LocomotionWeight;
            break;
        case ECharacterMovementState::Crouching:
        case ECharacterMovementState::Crawling:
            Score *= DatabaseWeights.StealthWeight;
            break;
    }
    
    // Aplicar modificadores baseados no contexto
    if (Query.ThreatLevel >= EThreatLevel::High)
    {
        Score *= DatabaseWeights.PanicWeight;
    }
    
    if (Query.StaminaLevel < 0.5f)
    {
        Score *= DatabaseWeights.ExhaustedWeight;
    }
    
    if (Query.FearLevel > 0.6f)
    {
        Score *= DatabaseWeights.PanicWeight;
    }
    
    return Score;
}

void UMotionMatchingController::OptimizeQueryFrequency(float DeltaTime)
{
    LastQueryTime += DeltaTime;
    
    // Ajustar frequência de query baseada na situação
    float TargetCooldown = QueryCooldown;
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character && AnimationSystem)
    {
        // Queries mais frequentes em situações dinâmicas
        if (AnimationSystem->GetCurrentThreatLevel() >= EThreatLevel::High)
        {
            TargetCooldown *= 0.5f; // Dobrar frequência
        }
        
        // Queries menos frequentes quando parado
        if (Character->GetVelocity().SizeSquared() < 100.0f)
        {
            TargetCooldown *= 2.0f; // Reduzir frequência
        }
    }
    
    QueryCooldown = FMath::FInterpTo(QueryCooldown, TargetCooldown, DeltaTime, 2.0f);
}