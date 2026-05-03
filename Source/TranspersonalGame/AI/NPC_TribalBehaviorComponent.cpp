#include "NPC_TribalBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

UNPC_TribalBehaviorComponent::UNPC_TribalBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Valores padrão
    CurrentState = ENPC_TribalState::Idle;
    DetectionRadius = 1500.0f;
    FleeRadius = 800.0f;
    PatrolRadius = 2000.0f;
    MovementSpeed = 300.0f;
    CurrentPatrolIndex = 0;
    StateTimer = 0.0f;
    LastBehaviorUpdate = 0.0f;

    // Personalidade padrão
    Personality.Courage = FMath::RandRange(0.3f, 0.8f);
    Personality.Curiosity = FMath::RandRange(0.2f, 0.7f);
    Personality.Sociability = FMath::RandRange(0.4f, 0.9f);
    Personality.Aggression = FMath::RandRange(0.1f, 0.4f);
}

void UNPC_TribalBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Configurar pontos de patrulha iniciais ao redor da posição inicial
    if (GetOwner())
    {
        FVector StartLocation = GetOwner()->GetActorLocation();
        
        // Criar 4 pontos de patrulha em cruz
        PatrolPoints.Empty();
        PatrolPoints.Add(StartLocation + FVector(PatrolRadius, 0, 0));
        PatrolPoints.Add(StartLocation + FVector(0, PatrolRadius, 0));
        PatrolPoints.Add(StartLocation + FVector(-PatrolRadius, 0, 0));
        PatrolPoints.Add(StartLocation + FVector(0, -PatrolRadius, 0));
        
        CurrentPatrolIndex = 0;
        
        UE_LOG(LogTemp, Warning, TEXT("TribalBehavior: %s iniciado com %d pontos de patrulha"), 
               *GetOwner()->GetName(), PatrolPoints.Num());
    }
}

void UNPC_TribalBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    
    // Atualizar comportamento a cada 0.1 segundos
    if (GetWorld()->GetTimeSeconds() - LastBehaviorUpdate >= 0.1f)
    {
        UpdateBehavior(DeltaTime);
        LastBehaviorUpdate = GetWorld()->GetTimeSeconds();
    }

    // Atualizar memória do tempo
    Memory.TimeSincePlayerSeen += DeltaTime;
    Memory.LastMealTime += DeltaTime;
}

void UNPC_TribalBehaviorComponent::UpdateBehavior(float DeltaTime)
{
    APawn* Player = FindNearestPlayer();
    if (Player)
    {
        float DistanceToPlayer = GetDistanceToPlayer();
        bool bPlayerVisible = IsPlayerVisible(Player);
        
        // Atualizar memória se o jogador está visível
        if (bPlayerVisible)
        {
            UpdateMemory(Player->GetActorLocation(), true);
        }

        // Lógica de mudança de estado baseada na distância e personalidade
        if (DistanceToPlayer < FleeRadius && ShouldFlee())
        {
            SetState(ENPC_TribalState::Fleeing);
        }
        else if (DistanceToPlayer < DetectionRadius && ShouldApproachPlayer())
        {
            SetState(ENPC_TribalState::Socializing);
        }
        else if (CurrentState == ENPC_TribalState::Idle && StateTimer > 5.0f)
        {
            // Mudar para patrulha após 5 segundos de inatividade
            SetState(ENPC_TribalState::Patrolling);
        }
    }

    // Executar comportamento baseado no estado atual
    switch (CurrentState)
    {
        case ENPC_TribalState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ENPC_TribalState::Patrolling:
            HandlePatrollingState(DeltaTime);
            break;
        case ENPC_TribalState::Fleeing:
            HandleFleeingState(DeltaTime);
            break;
        case ENPC_TribalState::Gathering:
            HandleGatheringState(DeltaTime);
            break;
        default:
            break;
    }
}

void UNPC_TribalBehaviorComponent::HandleIdleState(float DeltaTime)
{
    // No estado idle, ocasionalmente olhar ao redor
    if (StateTimer > 3.0f)
    {
        // Transição para patrulha
        SetState(ENPC_TribalState::Patrolling);
    }
}

void UNPC_TribalBehaviorComponent::HandlePatrollingState(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    APawn* OwnerPawn = GetOwnerPawn();
    if (!OwnerPawn) return;

    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    FVector TargetPoint = GetNextPatrolPoint();
    
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetPoint);
    
    if (DistanceToTarget < 200.0f)
    {
        // Chegou ao ponto, ir para o próximo
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        
        // Parar um pouco no ponto
        SetState(ENPC_TribalState::Idle);
    }
    else
    {
        // Mover em direção ao ponto de patrulha
        FVector Direction = (TargetPoint - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + Direction * MovementSpeed * DeltaTime;
        OwnerPawn->SetActorLocation(NewLocation);
        
        // Rodar para a direção de movimento
        FRotator NewRotation = Direction.Rotation();
        OwnerPawn->SetActorRotation(NewRotation);
    }
}

void UNPC_TribalBehaviorComponent::HandleFleeingState(float DeltaTime)
{
    APawn* OwnerPawn = GetOwnerPawn();
    if (!OwnerPawn) return;

    FVector FleeDirection = GetFleeDirection();
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    FVector NewLocation = CurrentLocation + FleeDirection * MovementSpeed * 1.5f * DeltaTime; // Correr mais rápido

    OwnerPawn->SetActorLocation(NewLocation);
    OwnerPawn->SetActorRotation(FleeDirection.Rotation());

    // Parar de fugir após algum tempo
    if (StateTimer > 5.0f)
    {
        SetState(ENPC_TribalState::Idle);
    }
}

void UNPC_TribalBehaviorComponent::HandleGatheringState(float DeltaTime)
{
    // Simular coleta de recursos
    if (StateTimer > 10.0f)
    {
        Memory.LastMealTime = 0.0f; // "Comeu"
        SetState(ENPC_TribalState::Idle);
    }
}

void UNPC_TribalBehaviorComponent::SetState(ENPC_TribalState NewState)
{
    if (CurrentState != NewState)
    {
        UE_LOG(LogTemp, Log, TEXT("TribalBehavior: %s mudou estado de %d para %d"), 
               *GetOwner()->GetName(), (int32)CurrentState, (int32)NewState);
        
        CurrentState = NewState;
        StateTimer = 0.0f;
    }
}

void UNPC_TribalBehaviorComponent::UpdateMemory(const FVector& PlayerLocation, bool bPlayerVisible)
{
    if (bPlayerVisible)
    {
        Memory.LastKnownPlayerLocation = PlayerLocation;
        Memory.TimeSincePlayerSeen = 0.0f;
    }
}

void UNPC_TribalBehaviorComponent::AddDangerousLocation(const FVector& Location)
{
    Memory.DangerousLocations.AddUnique(Location);
    
    // Limitar a 10 localizações perigosas
    if (Memory.DangerousLocations.Num() > 10)
    {
        Memory.DangerousLocations.RemoveAt(0);
    }
}

void UNPC_TribalBehaviorComponent::AddSafeLocation(const FVector& Location)
{
    Memory.SafeLocations.AddUnique(Location);
    
    // Limitar a 10 localizações seguras
    if (Memory.SafeLocations.Num() > 10)
    {
        Memory.SafeLocations.RemoveAt(0);
    }
}

FVector UNPC_TribalBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner()->GetActorLocation();
    }
    
    return PatrolPoints[CurrentPatrolIndex];
}

bool UNPC_TribalBehaviorComponent::ShouldFlee() const
{
    // Fugir baseado na coragem e se viu dinossauros
    float FleeThreshold = 1.0f - Personality.Courage;
    if (Memory.bHasSeenDinosaur)
    {
        FleeThreshold += 0.3f; // Mais propenso a fugir se já viu dinossauros
    }
    
    return FMath::RandRange(0.0f, 1.0f) < FleeThreshold;
}

bool UNPC_TribalBehaviorComponent::ShouldApproachPlayer() const
{
    // Aproximar baseado na sociabilidade e curiosidade
    float ApproachThreshold = (Personality.Sociability + Personality.Curiosity) * 0.5f;
    
    // Menos provável se viu o jogador recentemente
    if (Memory.TimeSincePlayerSeen < 30.0f)
    {
        ApproachThreshold *= 0.5f;
    }
    
    return FMath::RandRange(0.0f, 1.0f) < ApproachThreshold;
}

FVector UNPC_TribalBehaviorComponent::GetFleeDirection() const
{
    APawn* Player = FindNearestPlayer();
    if (Player && GetOwner())
    {
        FVector ToPlayer = Player->GetActorLocation() - GetOwner()->GetActorLocation();
        return -ToPlayer.GetSafeNormal(); // Direção oposta ao jogador
    }
    
    // Direção aleatória se não há jogador
    return FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
}

APawn* UNPC_TribalBehaviorComponent::GetOwnerPawn() const
{
    return Cast<APawn>(GetOwner());
}

APawn* UNPC_TribalBehaviorComponent::FindNearestPlayer() const
{
    if (!GetWorld()) return nullptr;
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        return PC->GetPawn();
    }
    
    return nullptr;
}

bool UNPC_TribalBehaviorComponent::IsPlayerVisible(APawn* Player) const
{
    if (!Player || !GetOwner()) return false;
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Player->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Player);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Se não bateu em nada, o jogador está visível
}

float UNPC_TribalBehaviorComponent::GetDistanceToPlayer() const
{
    APawn* Player = FindNearestPlayer();
    if (Player && GetOwner())
    {
        return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    }
    
    return 999999.0f;
}