#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componentes
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configuração inicial
    DinosaurSpecies = ENPCDinosaurSpecies::Generic;
    PatrolRadius = 2500.0f;
    AggressionLevel = 0.5f;
    MovementSpeed = 250.0f;
    DetectionRange = 1500.0f;
    AttackRange = 300.0f;
    CurrentBehaviorState = ENPCBehaviorState::Idle;
    CurrentTarget = nullptr;
    PatrolCenter = FVector::ZeroVector;
    CurrentPatrolIndex = 0;

    SetupPerception();
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    SetupBehaviorTree();
    ConfigureForSpecies();
    GeneratePatrolPoints();
}

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (InPawn)
    {
        PatrolCenter = InPawn->GetActorLocation();
        
        // Configurar blackboard
        if (BlackboardComponent && BlackboardAsset)
        {
            BlackboardComponent->InitializeBlackboard(*BlackboardAsset);
            
            // Definir valores iniciais
            BlackboardComponent->SetValueAsVector(TEXT("PatrolCenter"), PatrolCenter);
            BlackboardComponent->SetValueAsFloat(TEXT("PatrolRadius"), PatrolRadius);
            BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
            BlackboardComponent->SetValueAsFloat(TEXT("MovementSpeed"), MovementSpeed);
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
        }

        // Iniciar behavior tree
        if (BehaviorTree)
        {
            RunBehaviorTree(BehaviorTree);
        }
    }
}

void ADinosaurAIController::OnUnPossess()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
    
    Super::OnUnPossess();
}

void ADinosaurAIController::SetupPerception()
{
    // Configurar visão
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = DetectionRange;
        SightConfig->LoseSightRadius = DetectionRange * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(3.0f);
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
        HearingConfig->HearingRange = DetectionRange * 0.8f;
        HearingConfig->SetMaxAge(2.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Definir sentido dominante
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind callbacks
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnTargetPerceptionUpdated);
}

void ADinosaurAIController::SetupBehaviorTree()
{
    // Por agora, usar behavior tree genérico
    // TODO: Carregar behavior trees específicos por espécie
    if (!BehaviorTree)
    {
        // Tentar carregar behavior tree padrão
        BehaviorTree = LoadObject<UBehaviorTree>(nullptr, TEXT("/Game/AI/BT_DinosaurBase"));
        if (!BehaviorTree)
        {
            UE_LOG(LogTemp, Warning, TEXT("Behavior Tree não encontrado para dinossauro"));
        }
    }

    if (!BlackboardAsset)
    {
        // Tentar carregar blackboard padrão
        BlackboardAsset = LoadObject<UBlackboardData>(nullptr, TEXT("/Game/AI/BB_DinosaurBase"));
        if (!BlackboardAsset)
        {
            UE_LOG(LogTemp, Warning, TEXT("Blackboard não encontrado para dinossauro"));
        }
    }
}

void ADinosaurAIController::ConfigureForSpecies()
{
    switch (DinosaurSpecies)
    {
        case ENPCDinosaurSpecies::TRex:
            PatrolRadius = 5000.0f;
            AggressionLevel = 0.9f;
            MovementSpeed = 300.0f;
            DetectionRange = 2000.0f;
            AttackRange = 500.0f;
            break;

        case ENPCDinosaurSpecies::Raptor:
            PatrolRadius = 3000.0f;
            AggressionLevel = 0.8f;
            MovementSpeed = 500.0f;
            DetectionRange = 1800.0f;
            AttackRange = 200.0f;
            break;

        case ENPCDinosaurSpecies::Brachiosaurus:
            PatrolRadius = 2000.0f;
            AggressionLevel = 0.1f;
            MovementSpeed = 150.0f;
            DetectionRange = 1200.0f;
            AttackRange = 400.0f;
            break;

        case ENPCDinosaurSpecies::Triceratops:
            PatrolRadius = 2500.0f;
            AggressionLevel = 0.6f;
            MovementSpeed = 200.0f;
            DetectionRange = 1500.0f;
            AttackRange = 350.0f;
            break;

        default:
            // Manter valores padrão
            break;
    }

    // Atualizar configuração de percepção
    if (SightConfig)
    {
        SightConfig->SightRadius = DetectionRange;
        SightConfig->LoseSightRadius = DetectionRange * 1.2f;
    }

    if (HearingConfig)
    {
        HearingConfig->HearingRange = DetectionRange * 0.8f;
    }
}

void ADinosaurAIController::SetBehaviorState(ENPCBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }

        UE_LOG(LogTemp, Log, TEXT("Dinossauro mudou estado para: %d"), static_cast<int32>(NewState));
    }
}

void ADinosaurAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
    }

    if (NewTarget)
    {
        SetBehaviorState(ENPCBehaviorState::Chasing);
        UE_LOG(LogTemp, Log, TEXT("Dinossauro definiu novo alvo: %s"), *NewTarget->GetName());
    }
    else
    {
        SetBehaviorState(ENPCBehaviorState::Patrolling);
    }
}

void ADinosaurAIController::GeneratePatrolPoints()
{
    PatrolPoints.Empty();

    if (PatrolCenter.IsZero() && GetPawn())
    {
        PatrolCenter = GetPawn()->GetActorLocation();
    }

    // Gerar 4-6 pontos de patrulha em círculo
    int32 NumPoints = FMath::RandRange(4, 6);
    float AngleStep = 360.0f / NumPoints;

    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = AngleStep * i;
        float RadiusVariation = FMath::RandRange(0.5f, 1.0f) * PatrolRadius;
        
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * RadiusVariation,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * RadiusVariation,
            0.0f
        );

        FVector PatrolPoint = PatrolCenter + Offset;
        PatrolPoints.Add(PatrolPoint);
    }

    CurrentPatrolIndex = 0;

    if (BlackboardComponent && PatrolPoints.Num() > 0)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolPoint"), PatrolPoints[0]);
    }

    UE_LOG(LogTemp, Log, TEXT("Gerados %d pontos de patrulha para dinossauro"), PatrolPoints.Num());
}

FVector ADinosaurAIController::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        GeneratePatrolPoints();
    }

    if (PatrolPoints.Num() > 0)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        FVector NextPoint = PatrolPoints[CurrentPatrolIndex];

        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("PatrolPoint"), NextPoint);
        }

        return NextPoint;
    }

    return PatrolCenter;
}

bool ADinosaurAIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent)
    {
        return false;
    }

    FActorPerceptionBlueprintInfo Info;
    AIPerceptionComponent->GetActorsPerception(Target, Info);
    
    return Info.LastSensedStimuli.Num() > 0 && Info.LastSensedStimuli[0].WasSuccessfullySensed();
}

float ADinosaurAIController::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return -1.0f;
    }

    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<APawn>())
        {
            float Distance = GetDistanceToTarget(Actor);
            
            if (Distance <= DetectionRange && CanSeeTarget(Actor))
            {
                // Decidir se deve perseguir baseado na agressividade
                float ChaseChance = AggressionLevel * 0.8f + FMath::RandRange(0.0f, 0.2f);
                
                if (FMath::RandRange(0.0f, 1.0f) < ChaseChance)
                {
                    SetTarget(Actor);
                }
            }
        }
    }
}

void ADinosaurAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // Alvo detectado
        if (Actor == CurrentTarget || !CurrentTarget)
        {
            SetTarget(Actor);
        }
    }
    else
    {
        // Alvo perdido
        if (Actor == CurrentTarget)
        {
            SetTarget(nullptr);
        }
    }
}