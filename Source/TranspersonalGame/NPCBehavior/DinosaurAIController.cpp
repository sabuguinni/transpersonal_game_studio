#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componentes
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configurar sight config
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    
    // Valores padrão
    SightRadius = 3000.0f;
    LoseSightRadius = 3500.0f;
    FieldOfViewAngle = 90.0f;
    WalkSpeed = 200.0f;
    RunSpeed = 600.0f;
    PatrolRadius = 1000.0f;
    AttackRange = 300.0f;
    AttackDamage = 50.0f;
    AggressionLevel = 0.7f;
    MaxChaseTime = 10.0f;
    
    DinosaurSpecies = ENPC_DinosaurSpecies::TRex;
    CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
    
    CurrentTarget = nullptr;
    HomeLocation = FVector::ZeroVector;
    CurrentPatrolPoint = FVector::ZeroVector;
    LastTargetSeenTime = 0.0f;
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Configurar perceção
    ConfigurePerceptionForSpecies();
    
    // Configurar comportamento
    ConfigureBehaviorForSpecies();
    
    // Definir localização inicial como home
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
        CurrentPatrolPoint = GetRandomPatrolPoint();
    }
    
    // Iniciar behavior tree
    StartBehaviorTree();
    
    // Configurar timers
    GetWorld()->GetTimerManager().SetTimer(StateCheckTimerHandle, [this]()
    {
        // Verificar estado periodicamente
        if (CurrentTarget && GetWorld()->GetTimeSeconds() - LastTargetSeenTime > MaxChaseTime)
        {
            CurrentTarget = nullptr;
            SetBehaviorState(ENPC_DinosaurBehaviorState::Returning);
        }
    }, 1.0f, true);
}

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Atualizar blackboard com informações atuais
    if (Blackboard)
    {
        Blackboard->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
        Blackboard->SetValueAsVector(TEXT("PatrolPoint"), CurrentPatrolPoint);
        Blackboard->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
        Blackboard->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
    }
}

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
        
        // Configurar movimento baseado na espécie
        if (ACharacter* Character = Cast<ACharacter>(InPawn))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = WalkSpeed;
            }
        }
    }
}

void ADinosaurAIController::ConfigurePerceptionForSpecies()
{
    if (!AIPerceptionComponent || !SightConfig)
        return;
    
    // Configurar baseado na espécie
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            SightRadius = 4000.0f;
            LoseSightRadius = 4500.0f;
            FieldOfViewAngle = 120.0f;
            break;
            
        case ENPC_DinosaurSpecies::Raptor:
            SightRadius = 3500.0f;
            LoseSightRadius = 4000.0f;
            FieldOfViewAngle = 140.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            SightRadius = 2500.0f;
            LoseSightRadius = 3000.0f;
            FieldOfViewAngle = 100.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            SightRadius = 5000.0f;
            LoseSightRadius = 5500.0f;
            FieldOfViewAngle = 180.0f;
            break;
    }
    
    // Configurar sight config
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = FieldOfViewAngle;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Adicionar à perceção
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Bind callbacks
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnTargetPerceptionUpdated);
}

void ADinosaurAIController::ConfigureBehaviorForSpecies()
{
    // Configurar comportamento baseado na espécie
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            WalkSpeed = 300.0f;
            RunSpeed = 800.0f;
            AttackRange = 400.0f;
            AttackDamage = 100.0f;
            AggressionLevel = 0.9f;
            PatrolRadius = 2000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Raptor:
            WalkSpeed = 400.0f;
            RunSpeed = 1000.0f;
            AttackRange = 250.0f;
            AttackDamage = 40.0f;
            AggressionLevel = 0.8f;
            PatrolRadius = 1500.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            WalkSpeed = 200.0f;
            RunSpeed = 500.0f;
            AttackRange = 300.0f;
            AttackDamage = 80.0f;
            AggressionLevel = 0.3f;
            PatrolRadius = 800.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            WalkSpeed = 150.0f;
            RunSpeed = 300.0f;
            AttackRange = 500.0f;
            AttackDamage = 60.0f;
            AggressionLevel = 0.1f;
            PatrolRadius = 1200.0f;
            break;
    }
}

void ADinosaurAIController::StartBehaviorTree()
{
    if (BehaviorTree && BehaviorTreeComponent)
    {
        if (BlackboardAsset)
        {
            UseBlackboard(BlackboardAsset);
        }
        
        BehaviorTreeComponent->StartTree(*BehaviorTree);
    }
}

void ADinosaurAIController::StopBehaviorTree()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ADinosaurAIController::SetBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    CurrentBehaviorState = NewState;
    
    if (Blackboard)
    {
        Blackboard->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
    }
}

void ADinosaurAIController::SetMovementSpeed(float Speed)
{
    if (GetPawn())
    {
        if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = Speed;
            }
        }
    }
}

FVector ADinosaurAIController::GetRandomPatrolPoint()
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f;
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    
    return HomeLocation + (RandomDirection * RandomDistance);
}

bool ADinosaurAIController::IsAtLocation(FVector TargetLocation, float Tolerance)
{
    if (!GetPawn())
        return false;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TargetLocation);
    return Distance <= Tolerance;
}

bool ADinosaurAIController::CanAttackTarget(AActor* Target)
{
    if (!Target || !GetPawn())
        return false;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

void ADinosaurAIController::AttackTarget(AActor* Target)
{
    if (!CanAttackTarget(Target))
        return;
    
    // Implementar lógica de ataque
    // Por agora, apenas log
    UE_LOG(LogTemp, Warning, TEXT("%s attacks %s for %f damage"), 
           *GetPawn()->GetName(), *Target->GetName(), AttackDamage);
}

void ADinosaurAIController::SetBlackboardValue(const FString& KeyName, const FString& Value)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsString(*KeyName, Value);
    }
}

void ADinosaurAIController::SetBlackboardVector(const FString& KeyName, FVector Value)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsVector(*KeyName, Value);
    }
}

void ADinosaurAIController::SetBlackboardActor(const FString& KeyName, AActor* Actor)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsObject(*KeyName, Actor);
    }
}

void ADinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Verificar se é o jogador
            if (Actor->ActorHasTag(TEXT("Player")) || Actor->GetName().Contains(TEXT("Character")))
            {
                FAIStimulus Stimulus;
                if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
                {
                    if (Stimulus.WasSuccessfullySensed())
                    {
                        CurrentTarget = Actor;
                        LastTargetSeenTime = GetWorld()->GetTimeSeconds();
                        
                        // Determinar comportamento baseado na agressividade
                        if (FMath::RandRange(0.0f, 1.0f) < AggressionLevel)
                        {
                            SetBehaviorState(ENPC_DinosaurBehaviorState::Chasing);
                            SetMovementSpeed(RunSpeed);
                        }
                        else
                        {
                            SetBehaviorState(ENPC_DinosaurBehaviorState::Investigating);
                            SetMovementSpeed(WalkSpeed * 1.5f);
                        }
                    }
                }
            }
        }
    }
}

void ADinosaurAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;
    
    if (Stimulus.WasSuccessfullySensed())
    {
        if (Actor->ActorHasTag(TEXT("Player")) || Actor->GetName().Contains(TEXT("Character")))
        {
            CurrentTarget = Actor;
            LastTargetSeenTime = GetWorld()->GetTimeSeconds();
        }
    }
    else
    {
        // Perdeu o alvo de vista
        if (CurrentTarget == Actor)
        {
            SetBehaviorState(ENPC_DinosaurBehaviorState::Searching);
        }
    }
}