#include "NPC_DinosaurBehaviorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UNPC_DinosaurBehaviorComponent::UNPC_DinosaurBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick a cada 100ms para performance
}

void UNPC_DinosaurBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        DinosaurAIController = Cast<AAIController>(OwnerPawn->GetController());
        HomeLocation = OwnerPawn->GetActorLocation();
    }
    
    InitializeBehavior();
}

void UNPC_DinosaurBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!IsAlive())
    {
        return;
    }
    
    StateTimer += DeltaTime;
    UpdateBehaviorBySpecies(DeltaTime);
    UpdateBlackboard();
    
    // Reduzir fome gradualmente
    ModifyHunger(-DeltaTime * 2.0f);
}

void UNPC_DinosaurBehaviorComponent::InitializeBehavior()
{
    if (!OwnerPawn)
    {
        return;
    }
    
    // Configurar stats baseado na espécie
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            Stats.MaxHealth = 300.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.Aggression = 80.0f;
            Stats.TerritoryRadius = 8000.0f;
            Stats.DetectionRange = 5000.0f;
            Stats.MovementSpeed = 800.0f;
            Stats.AttackDamage = 100.0f;
            break;
            
        case ENPC_DinosaurSpecies::Raptor:
            Stats.MaxHealth = 120.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.Aggression = 70.0f;
            Stats.TerritoryRadius = 6000.0f;
            Stats.DetectionRange = 4000.0f;
            Stats.MovementSpeed = 1200.0f;
            Stats.AttackDamage = 60.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            Stats.MaxHealth = 500.0f;
            Stats.Health = Stats.MaxHealth;
            Stats.Aggression = 10.0f;
            Stats.TerritoryRadius = 12000.0f;
            Stats.DetectionRange = 3000.0f;
            Stats.MovementSpeed = 400.0f;
            Stats.AttackDamage = 80.0f; // Defesa, não ataque
            break;
            
        default:
            break;
    }
    
    // Inicializar estado baseado na espécie
    if (Species == ENPC_DinosaurSpecies::Brachiosaurus)
    {
        SetDinosaurState(ENPC_DinosaurState::Feeding);
    }
    else
    {
        SetDinosaurState(ENPC_DinosaurState::Patrolling);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Behavior Initialized: Species %d, Health %.1f"), 
           (int32)Species, Stats.Health);
}

void UNPC_DinosaurBehaviorComponent::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur state changed to: %d"), (int32)CurrentState);
    }
}

void UNPC_DinosaurBehaviorComponent::StartPatrolling()
{
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
    PatrolTarget = GeneratePatrolPoint();
}

void UNPC_DinosaurBehaviorComponent::StartHunting(AActor* Target)
{
    if (Target)
    {
        CurrentTarget = Target;
        SetDinosaurState(ENPC_DinosaurState::Hunting);
        
        UE_LOG(LogTemp, Warning, TEXT("Starting hunt for target: %s"), *Target->GetName());
    }
}

void UNPC_DinosaurBehaviorComponent::StartFeeding()
{
    SetDinosaurState(ENPC_DinosaurState::Feeding);
    ModifyHunger(20.0f); // Recuperar fome
}

void UNPC_DinosaurBehaviorComponent::StartFleeing(AActor* Threat)
{
    if (Threat && Species == ENPC_DinosaurSpecies::Brachiosaurus)
    {
        CurrentTarget = Threat;
        SetDinosaurState(ENPC_DinosaurState::Fleeing);
        
        UE_LOG(LogTemp, Warning, TEXT("Brachiosaurus fleeing from threat: %s"), *Threat->GetName());
    }
}

TArray<AActor*> UNPC_DinosaurBehaviorComponent::DetectNearbyActors(float Range, TSubclassOf<AActor> ActorClass)
{
    TArray<AActor*> FoundActors;
    
    if (!OwnerPawn)
    {
        return FoundActors;
    }
    
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, FoundActors);
    
    TArray<AActor*> NearbyActors;
    FVector MyLocation = OwnerPawn->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != OwnerPawn)
        {
            float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
            if (Distance <= Range)
            {
                NearbyActors.Add(Actor);
            }
        }
    }
    
    return NearbyActors;
}

AActor* UNPC_DinosaurBehaviorComponent::FindNearestPlayer()
{
    if (!OwnerPawn)
    {
        return nullptr;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
        if (Distance <= Stats.DetectionRange)
        {
            LastPlayerDetectionTime = GetWorld()->GetTimeSeconds();
            return PlayerPawn;
        }
    }
    
    return nullptr;
}

bool UNPC_DinosaurBehaviorComponent::IsPlayerInRange(float Range)
{
    AActor* Player = FindNearestPlayer();
    if (Player && OwnerPawn)
    {
        float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), Player->GetActorLocation());
        return Distance <= Range;
    }
    return false;
}

void UNPC_DinosaurBehaviorComponent::ExecuteTRexBehavior(float DeltaTime)
{
    AActor* Player = FindNearestPlayer();
    
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Patrolling:
            if (Player)
            {
                StartHunting(Player);
            }
            else if (StateTimer > 10.0f) // Patrulha por 10 segundos
            {
                PatrolTarget = GeneratePatrolPoint();
                StateTimer = 0.0f;
            }
            break;
            
        case ENPC_DinosaurState::Hunting:
            if (!Player)
            {
                StartPatrolling();
            }
            else if (IsPlayerInRange(200.0f)) // Distância de ataque
            {
                // Atacar jogador
                UE_LOG(LogTemp, Warning, TEXT("T-Rex attacking player!"));
                SetDinosaurState(ENPC_DinosaurState::Territorial);
            }
            break;
            
        case ENPC_DinosaurState::Territorial:
            if (StateTimer > 3.0f) // Ataque dura 3 segundos
            {
                if (Player)
                {
                    StartHunting(Player);
                }
                else
                {
                    StartPatrolling();
                }
            }
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorComponent::ExecuteRaptorBehavior(float DeltaTime)
{
    AActor* Player = FindNearestPlayer();
    
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Patrolling:
            if (Player)
            {
                SetDinosaurState(ENPC_DinosaurState::PackHunting);
            }
            break;
            
        case ENPC_DinosaurState::PackHunting:
            if (!Player)
            {
                StartPatrolling();
            }
            else if (IsPlayerInRange(150.0f)) // Raptors atacam em grupo
            {
                UE_LOG(LogTemp, Warning, TEXT("Raptor pack attacking player!"));
            }
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorComponent::ExecuteBrachiosaurusBehavior(float DeltaTime)
{
    AActor* Player = FindNearestPlayer();
    
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Feeding:
            if (Player && IsPlayerInRange(1000.0f)) // Muito próximo
            {
                StartFleeing(Player);
            }
            else if (StateTimer > 15.0f) // Alimenta por 15 segundos
            {
                SetDinosaurState(ENPC_DinosaurState::Idle);
            }
            break;
            
        case ENPC_DinosaurState::Fleeing:
            if (!Player || !IsPlayerInRange(2000.0f)) // Fugiu o suficiente
            {
                StartFeeding();
            }
            break;
            
        case ENPC_DinosaurState::Idle:
            if (StateTimer > 5.0f)
            {
                StartFeeding();
            }
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorComponent::TakeDamage(float Damage)
{
    Stats.Health = FMath::Max(0.0f, Stats.Health - Damage);
    
    if (Stats.Health <= 0.0f)
    {
        SetDinosaurState(ENPC_DinosaurState::Idle);
        UE_LOG(LogTemp, Error, TEXT("Dinosaur died!"));
    }
    else
    {
        // Aumentar agressividade quando ferido
        Stats.Aggression = FMath::Min(100.0f, Stats.Aggression + 10.0f);
    }
}

void UNPC_DinosaurBehaviorComponent::RestoreHealth(float Amount)
{
    Stats.Health = FMath::Min(Stats.MaxHealth, Stats.Health + Amount);
}

void UNPC_DinosaurBehaviorComponent::ModifyHunger(float Amount)
{
    Stats.Hunger = FMath::Clamp(Stats.Hunger + Amount, 0.0f, 100.0f);
    
    // Fome afeta comportamento
    if (Stats.Hunger < 20.0f && CurrentState != ENPC_DinosaurState::Feeding)
    {
        if (Species != ENPC_DinosaurSpecies::Brachiosaurus)
        {
            Stats.Aggression += 5.0f; // Mais agressivo quando com fome
        }
    }
}

bool UNPC_DinosaurBehaviorComponent::IsAlive() const
{
    return Stats.Health > 0.0f;
}

void UNPC_DinosaurBehaviorComponent::UpdateBehaviorBySpecies(float DeltaTime)
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            ExecuteTRexBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurSpecies::Raptor:
            ExecuteRaptorBehavior(DeltaTime);
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            ExecuteBrachiosaurusBehavior(DeltaTime);
            break;
            
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorComponent::UpdateBlackboard()
{
    if (BlackboardComponent && DinosaurAIController)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("Health"), Stats.Health);
        BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), Stats.Hunger);
        BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), Stats.Aggression);
        BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), (uint8)CurrentState);
        
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget);
        }
    }
}

FVector UNPC_DinosaurBehaviorComponent::GeneratePatrolPoint()
{
    if (!OwnerPawn)
    {
        return FVector::ZeroVector;
    }
    
    // Gerar ponto de patrulha dentro do território
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Distance = FMath::RandRange(Stats.TerritoryRadius * 0.3f, Stats.TerritoryRadius);
    
    FVector Offset = FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
        0.0f
    );
    
    return HomeLocation + Offset;
}

void UNPC_DinosaurBehaviorComponent::HandleSpeciesSpecificLogic(float DeltaTime)
{
    // Lógica adicional específica por espécie pode ser adicionada aqui
    // Por exemplo: sons específicos, animações, efeitos visuais
}