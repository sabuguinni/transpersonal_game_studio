#include "DinosaurBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/CapsuleComponent.h"

UNPC_DinosaurBehaviorComponent::UNPC_DinosaurBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Actualizar 10 vezes por segundo

    // Criar componente de percepção
    PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
    if (PawnSensingComponent)
    {
        PawnSensingComponent->SightRadius = 3000.0f;
        PawnSensingComponent->HearingThreshold = 1200.0f;
        PawnSensingComponent->LOSHearingThreshold = 1500.0f;
        PawnSensingComponent->SensingInterval = 0.2f;
        PawnSensingComponent->SetPeripheralVisionAngle(90.0f);
        PawnSensingComponent->bSeePawns = true;
        PawnSensingComponent->bHearNoises = true;
    }
}

void UNPC_DinosaurBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Configurar stats por espécie
    ConfigureSpeciesStats();
    
    // Definir localização inicial como "casa"
    HomeLocation = GetOwner()->GetActorLocation();
    PatrolTarget = HomeLocation;
    
    // Conectar callbacks de percepção
    if (PawnSensingComponent)
    {
        PawnSensingComponent->OnSeePawn.AddDynamic(this, &UNPC_DinosaurBehaviorComponent::OnPlayerSeen);
        PawnSensingComponent->OnHearNoise.AddDynamic(this, &UNPC_DinosaurBehaviorComponent::OnPlayerHeard);
    }
    
    // Inicializar comportamento
    InitializeDinosaurBehavior();
}

void UNPC_DinosaurBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Actualizar stats (fome, stamina, etc.)
    UpdateStats(DeltaTime);
    
    // Actualizar timers
    StateTimer += DeltaTime;
    
    // Lógica de estado baseada no estado actual
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            IdleTimer += DeltaTime;
            if (IdleTimer > 5.0f) // Após 5 segundos idle, começar patrulha
            {
                StartPatrolling();
            }
            break;
            
        case ENPC_DinosaurState::Patrolling:
            PatrolTimer += DeltaTime;
            
            // Verificar se chegou ao destino de patrulha
            float DistanceToPatrol = FVector::Dist(GetOwner()->GetActorLocation(), PatrolTarget);
            if (DistanceToPatrol < 200.0f || PatrolTimer > 15.0f)
            {
                // Escolher novo ponto de patrulha
                FVector RandomOffset = FVector(
                    FMath::RandRange(-2000.0f, 2000.0f),
                    FMath::RandRange(-2000.0f, 2000.0f),
                    0.0f
                );
                PatrolTarget = HomeLocation + RandomOffset;
                PatrolTimer = 0.0f;
            }
            break;
            
        case ENPC_DinosaurState::Hunting:
            HuntTimer += DeltaTime;
            
            if (CurrentTarget)
            {
                float DistanceToTarget = GetDistanceToPlayer();
                
                // Se o alvo está muito longe, desistir da caça
                if (DistanceToTarget > Stats.DetectionRadius * 2.0f || HuntTimer > 30.0f)
                {
                    CurrentTarget = nullptr;
                    ReturnToIdle();
                }
                // Se está perto o suficiente, atacar
                else if (DistanceToTarget < 300.0f)
                {
                    // Aplicar dano ao jogador (implementar depois)
                    UE_LOG(LogTemp, Warning, TEXT("Dinossauro %s atacou o jogador!"), *GetOwner()->GetName());
                    
                    // Voltar ao idle após ataque
                    FTimerHandle TimerHandle;
                    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
                    {
                        ReturnToIdle();
                    }, 2.0f, false);
                }
            }
            else
            {
                ReturnToIdle();
            }
            break;
            
        case ENPC_DinosaurState::Fleeing:
            // Fugir por 10 segundos, depois voltar ao idle
            if (StateTimer > 10.0f)
            {
                ReturnToIdle();
            }
            break;
            
        default:
            break;
    }
    
    // Actualizar blackboard se disponível
    UpdateBlackboard();
}

void UNPC_DinosaurBehaviorComponent::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Dinossauro %s mudou para estado: %d"), 
               *GetOwner()->GetName(), (int32)CurrentState);
    }
}

void UNPC_DinosaurBehaviorComponent::InitializeDinosaurBehavior()
{
    // Obter referência do AI Controller
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        AIController = Cast<AAIController>(OwnerPawn->GetController());
        
        if (AIController)
        {
            BlackboardComponent = AIController->GetBlackboardComponent();
            BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
            
            // Configurar Behavior Tree se disponível
            SetupBehaviorTree();
        }
    }
    
    // Começar em estado idle
    SetDinosaurState(ENPC_DinosaurState::Idle);
}

void UNPC_DinosaurBehaviorComponent::StartPatrolling()
{
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
    PatrolTimer = 0.0f;
    
    // Escolher primeiro ponto de patrulha
    FVector RandomOffset = FVector(
        FMath::RandRange(-1500.0f, 1500.0f),
        FMath::RandRange(-1500.0f, 1500.0f),
        0.0f
    );
    PatrolTarget = HomeLocation + RandomOffset;
    
    UE_LOG(LogTemp, Log, TEXT("Dinossauro %s começou a patrulhar"), *GetOwner()->GetName());
}

void UNPC_DinosaurBehaviorComponent::StartHunting(APawn* Target)
{
    if (Target)
    {
        CurrentTarget = Target;
        SetDinosaurState(ENPC_DinosaurState::Hunting);
        HuntTimer = 0.0f;
        LastPlayerSightTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Warning, TEXT("Dinossauro %s começou a caçar %s!"), 
               *GetOwner()->GetName(), *Target->GetName());
    }
}

void UNPC_DinosaurBehaviorComponent::StartFleeing()
{
    SetDinosaurState(ENPC_DinosaurState::Fleeing);
    CurrentTarget = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Dinossauro %s está a fugir"), *GetOwner()->GetName());
}

void UNPC_DinosaurBehaviorComponent::ReturnToIdle()
{
    SetDinosaurState(ENPC_DinosaurState::Idle);
    CurrentTarget = nullptr;
    IdleTimer = 0.0f;
    PatrolTimer = 0.0f;
    HuntTimer = 0.0f;
}

void UNPC_DinosaurBehaviorComponent::OnPlayerSeen(APawn* SeenPawn)
{
    if (SeenPawn && SeenPawn->IsA<ACharacter>())
    {
        LastPlayerSightTime = GetWorld()->GetTimeSeconds();
        
        // Comportamento baseado na espécie e agressividade
        if (Stats.Aggression > 50.0f)
        {
            // Espécies agressivas atacam
            StartHunting(SeenPawn);
        }
        else if (Stats.Fear > 30.0f)
        {
            // Espécies medrosas fogem
            StartFleeing();
        }
        else
        {
            // Espécies neutras observam
            UE_LOG(LogTemp, Log, TEXT("Dinossauro %s viu o jogador mas permanece neutro"), 
                   *GetOwner()->GetName());
        }
    }
}

void UNPC_DinosaurBehaviorComponent::OnPlayerHeard(APawn* HeardPawn, const FVector& Location, float Volume)
{
    if (HeardPawn && HeardPawn->IsA<ACharacter>())
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Location);
        
        // Se o som está muito perto, reagir
        if (Distance < 1000.0f && Volume > 0.5f)
        {
            if (Stats.Aggression > 40.0f)
            {
                StartHunting(HeardPawn);
            }
            else
            {
                // Ficar alerta (implementar estado de alerta depois)
                UE_LOG(LogTemp, Log, TEXT("Dinossauro %s ouviu ruído suspeito"), *GetOwner()->GetName());
            }
        }
    }
}

float UNPC_DinosaurBehaviorComponent::GetDistanceToPlayer() const
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            return FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
        }
    }
    return 10000.0f; // Distância muito grande se não encontrar jogador
}

bool UNPC_DinosaurBehaviorComponent::IsPlayerInTerritory() const
{
    float DistanceToPlayer = GetDistanceToPlayer();
    return DistanceToPlayer <= Stats.TerritorialRadius;
}

void UNPC_DinosaurBehaviorComponent::UpdateStats(float DeltaTime)
{
    // Fome aumenta com o tempo
    Stats.Hunger = FMath::Clamp(Stats.Hunger + (DeltaTime * 0.5f), 0.0f, 100.0f);
    
    // Stamina regenera quando não está a caçar
    if (CurrentState != ENPC_DinosaurState::Hunting && CurrentState != ENPC_DinosaurState::Fleeing)
    {
        Stats.Stamina = FMath::Clamp(Stats.Stamina + (DeltaTime * 10.0f), 0.0f, 100.0f);
    }
    else
    {
        Stats.Stamina = FMath::Clamp(Stats.Stamina - (DeltaTime * 15.0f), 0.0f, 100.0f);
    }
    
    // Medo diminui com o tempo
    Stats.Fear = FMath::Clamp(Stats.Fear - (DeltaTime * 2.0f), 0.0f, 100.0f);
    
    // Regeneração de saúde lenta
    if (Stats.Health < Stats.MaxHealth && CurrentState == ENPC_DinosaurState::Idle)
    {
        Stats.Health = FMath::Clamp(Stats.Health + (DeltaTime * 1.0f), 0.0f, Stats.MaxHealth);
    }
}

void UNPC_DinosaurBehaviorComponent::TakeDamage(float Damage)
{
    Stats.Health = FMath::Clamp(Stats.Health - Damage, 0.0f, Stats.MaxHealth);
    Stats.Fear = FMath::Clamp(Stats.Fear + (Damage * 0.5f), 0.0f, 100.0f);
    
    if (Stats.Health <= 0.0f)
    {
        // Dinossauro morreu
        UE_LOG(LogTemp, Warning, TEXT("Dinossauro %s morreu!"), *GetOwner()->GetName());
        GetOwner()->Destroy();
    }
    else if (Stats.Fear > 70.0f)
    {
        // Começar a fugir se estiver muito assustado
        StartFleeing();
    }
}

void UNPC_DinosaurBehaviorComponent::RestoreHealth(float Amount)
{
    Stats.Health = FMath::Clamp(Stats.Health + Amount, 0.0f, Stats.MaxHealth);
}

void UNPC_DinosaurBehaviorComponent::ConfigureSpeciesStats()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Stats.MaxHealth = 200.0f;
            Stats.Health = 200.0f;
            Stats.Aggression = 80.0f;
            Stats.Fear = 10.0f;
            Stats.MovementSpeed = 800.0f;
            Stats.AttackDamage = 50.0f;
            Stats.DetectionRadius = 4000.0f;
            Stats.TerritorialRadius = 6000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Raptor:
            Stats.MaxHealth = 80.0f;
            Stats.Health = 80.0f;
            Stats.Aggression = 70.0f;
            Stats.Fear = 25.0f;
            Stats.MovementSpeed = 1200.0f;
            Stats.AttackDamage = 30.0f;
            Stats.DetectionRadius = 3500.0f;
            Stats.TerritorialRadius = 4000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            Stats.MaxHealth = 150.0f;
            Stats.Health = 150.0f;
            Stats.Aggression = 40.0f;
            Stats.Fear = 20.0f;
            Stats.MovementSpeed = 500.0f;
            Stats.AttackDamage = 35.0f;
            Stats.DetectionRadius = 2500.0f;
            Stats.TerritorialRadius = 3000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            Stats.MaxHealth = 300.0f;
            Stats.Health = 300.0f;
            Stats.Aggression = 10.0f;
            Stats.Fear = 15.0f;
            Stats.MovementSpeed = 300.0f;
            Stats.AttackDamage = 20.0f;
            Stats.DetectionRadius = 2000.0f;
            Stats.TerritorialRadius = 5000.0f;
            break;
            
        default:
            // Manter valores padrão
            break;
    }
}

void UNPC_DinosaurBehaviorComponent::SetupBehaviorTree()
{
    if (BehaviorTree && AIController)
    {
        AIController->RunBehaviorTree(BehaviorTree);
        UE_LOG(LogTemp, Log, TEXT("Behavior Tree iniciado para %s"), *GetOwner()->GetName());
    }
}

void UNPC_DinosaurBehaviorComponent::UpdateBlackboard()
{
    if (BlackboardComponent)
    {
        // Actualizar valores no blackboard para uso da Behavior Tree
        BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), (uint8)CurrentState);
        BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
        BlackboardComponent->SetValueAsVector(TEXT("PatrolTarget"), PatrolTarget);
        
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
        }
        else
        {
            BlackboardComponent->ClearValue(TEXT("TargetActor"));
        }
        
        BlackboardComponent->SetValueAsFloat(TEXT("Health"), Stats.Health);
        BlackboardComponent->SetValueAsFloat(TEXT("Fear"), Stats.Fear);
        BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), Stats.Aggression);
    }
}