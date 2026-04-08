// CrowdManager.cpp
// Implementação do sistema de gestão de multidões conscientes

#include "CrowdManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACrowdManager::ACrowdManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configurações padrão
    MaxCrowdSize = 100;
    SpawnRadius = 1000.0f;
    MovementSpeed = 150.0f;
    ConsciousnessInfluenceRange = 500.0f;

    // Configurações de flocking
    SeparationWeight = 2.0f;
    AlignmentWeight = 1.0f;
    CohesionWeight = 1.0f;
    FlockingRadius = 100.0f;

    // Estado inicial
    CurrentCrowdState = ECrowdBehaviorState::Peaceful;

    // Otimização
    UpdateInterval = 0.1f; // Atualiza a cada 100ms
    LastUpdateTime = 0.0f;
}

void ACrowdManager::BeginPlay()
{
    Super::BeginPlay();

    // Encontra o sistema de consciência
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        ConsciousnessSystem = PlayerPawn->FindComponentByClass<UConsciousnessComponent>();
    }

    // Spawn inicial da multidão
    SpawnCrowdUnits(MaxCrowdSize / 2, GetActorLocation());
}

void ACrowdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateInterval)
    {
        // Atualiza comportamento baseado na consciência do jogador
        if (ConsciousnessSystem)
        {
            float PlayerConsciousness = ConsciousnessSystem->GetCurrentConsciousnessLevel();
            UpdateCrowdBehavior(PlayerConsciousness);
        }

        // Atualiza movimento da multidão
        UpdateCrowdMovement(DeltaTime);

        // Aplica influência da consciência
        ApplyConsciousnessInfluence();

        LastUpdateTime = 0.0f;
    }
}

void ACrowdManager::SpawnCrowdUnits(int32 Count, FVector CenterLocation)
{
    for (int32 i = 0; i < Count; i++)
    {
        FCrowdUnit NewUnit;
        
        // Posição aleatória dentro do raio de spawn
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, SpawnRadius);
        
        NewUnit.Position = CenterLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );

        // Velocidade inicial aleatória
        NewUnit.Velocity = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal() * MovementSpeed;

        // Nível de consciência inicial aleatório
        NewUnit.ConsciousnessLevel = FMath::RandRange(0.2f, 0.8f);
        NewUnit.BehaviorState = CurrentCrowdState;

        CrowdUnits.Add(NewUnit);

        // Chama evento Blueprint
        OnCrowdUnitSpawned(NewUnit);
    }
}

void ACrowdManager::UpdateCrowdBehavior(float PlayerConsciousnessLevel)
{
    ECrowdBehaviorState NewState = CurrentCrowdState;

    // Determina o estado da multidão baseado na consciência do jogador
    if (PlayerConsciousnessLevel >= 0.8f)
    {
        NewState = ECrowdBehaviorState::Transcendent;
    }
    else if (PlayerConsciousnessLevel >= 0.6f)
    {
        NewState = ECrowdBehaviorState::Peaceful;
    }
    else if (PlayerConsciousnessLevel >= 0.4f)
    {
        NewState = ECrowdBehaviorState::Agitated;
    }
    else if (PlayerConsciousnessLevel >= 0.2f)
    {
        NewState = ECrowdBehaviorState::Hostile;
    }
    else
    {
        NewState = ECrowdBehaviorState::Fearful;
    }

    if (NewState != CurrentCrowdState)
    {
        SetCrowdState(NewState);
    }
}

void ACrowdManager::SetCrowdState(ECrowdBehaviorState NewState)
{
    CurrentCrowdState = NewState;

    // Atualiza comportamento de todas as unidades
    for (FCrowdUnit& Unit : CrowdUnits)
    {
        Unit.BehaviorState = NewState;
        
        // Ajusta propriedades baseado no estado
        switch (NewState)
        {
            case ECrowdBehaviorState::Peaceful:
                Unit.InfluenceRadius = 200.0f;
                break;
            case ECrowdBehaviorState::Agitated:
                Unit.InfluenceRadius = 150.0f;
                break;
            case ECrowdBehaviorState::Hostile:
                Unit.InfluenceRadius = 100.0f;
                break;
            case ECrowdBehaviorState::Transcendent:
                Unit.InfluenceRadius = 300.0f;
                break;
            case ECrowdBehaviorState::Fearful:
                Unit.InfluenceRadius = 50.0f;
                break;
        }
    }

    // Chama evento Blueprint
    OnCrowdStateChanged(NewState);
}

void ACrowdManager::UpdateCrowdMovement(float DeltaTime)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;

    for (FCrowdUnit& Unit : CrowdUnits)
    {
        // Obtém unidades próximas para flocking
        TArray<FCrowdUnit> NearbyUnits = GetNearbyUnits(Unit, FlockingRadius);
        
        // Calcula forças de flocking
        FVector FlockingForce = FVector::ZeroVector;
        if (NearbyUnits.Num() > 0)
        {
            CalculateFlocking(Unit, NearbyUnits);
        }

        // Força baseada no estado da multidão
        FVector StateForce = FVector::ZeroVector;
        float DistanceToPlayer = FVector::Dist(Unit.Position, PlayerLocation);

        switch (Unit.BehaviorState)
        {
            case ECrowdBehaviorState::Peaceful:
                // Movimento suave e orgânico
                StateForce = FVector(
                    FMath::Sin(GetWorld()->GetTimeSeconds() + Unit.Position.X * 0.01f),
                    FMath::Cos(GetWorld()->GetTimeSeconds() + Unit.Position.Y * 0.01f),
                    0.0f
                ) * 50.0f;
                break;

            case ECrowdBehaviorState::Agitated:
                // Movimento mais errático
                StateForce = FVector(
                    FMath::RandRange(-1.0f, 1.0f),
                    FMath::RandRange(-1.0f, 1.0f),
                    0.0f
                ) * 100.0f;
                break;

            case ECrowdBehaviorState::Hostile:
                // Move em direção ao jogador se próximo
                if (DistanceToPlayer < ConsciousnessInfluenceRange)
                {
                    StateForce = (PlayerLocation - Unit.Position).GetSafeNormal() * 200.0f;
                }
                break;

            case ECrowdBehaviorState::Transcendent:
                // Movimento harmonioso em círculos
                FVector CenterOffset = Unit.Position - GetActorLocation();
                StateForce = FVector(-CenterOffset.Y, CenterOffset.X, 0.0f).GetSafeNormal() * 75.0f;
                break;

            case ECrowdBehaviorState::Fearful:
                // Foge do jogador
                if (DistanceToPlayer < ConsciousnessInfluenceRange)
                {
                    StateForce = (Unit.Position - PlayerLocation).GetSafeNormal() * 300.0f;
                }
                break;
        }

        // Aplica forças
        Unit.Velocity += StateForce * DeltaTime;
        Unit.Velocity = Unit.Velocity.GetClampedToMaxSize(MovementSpeed);

        // Atualiza posição
        Unit.Position += Unit.Velocity * DeltaTime;

        // Mantém dentro dos limites do mundo
        float WorldBounds = 2000.0f;
        Unit.Position.X = FMath::Clamp(Unit.Position.X, -WorldBounds, WorldBounds);
        Unit.Position.Y = FMath::Clamp(Unit.Position.Y, -WorldBounds, WorldBounds);
    }
}

void ACrowdManager::ApplyConsciousnessInfluence()
{
    if (!ConsciousnessSystem) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float PlayerConsciousness = ConsciousnessSystem->GetCurrentConsciousnessLevel();

    for (FCrowdUnit& Unit : CrowdUnits)
    {
        float DistanceToPlayer = FVector::Dist(Unit.Position, PlayerLocation);
        
        if (DistanceToPlayer <= ConsciousnessInfluenceRange)
        {
            // Calcula influência baseada na distância
            float InfluenceFactor = 1.0f - (DistanceToPlayer / ConsciousnessInfluenceRange);
            
            // Ajusta nível de consciência da unidade
            float TargetConsciousness = FMath::Lerp(Unit.ConsciousnessLevel, PlayerConsciousness, InfluenceFactor * 0.1f);
            Unit.ConsciousnessLevel = FMath::Clamp(TargetConsciousness, 0.0f, 1.0f);
        }
    }
}

void ACrowdManager::CalculateFlocking(FCrowdUnit& Unit, const TArray<FCrowdUnit>& NearbyUnits)
{
    FVector Separation = CalculateSeparation(Unit, NearbyUnits);
    FVector Alignment = CalculateAlignment(Unit, NearbyUnits);
    FVector Cohesion = CalculateCohesion(Unit, NearbyUnits);

    // Aplica pesos e adiciona à velocidade
    FVector FlockingForce = (Separation * SeparationWeight) + 
                           (Alignment * AlignmentWeight) + 
                           (Cohesion * CohesionWeight);

    Unit.Velocity += FlockingForce * 0.1f;
}

FVector ACrowdManager::CalculateSeparation(const FCrowdUnit& Unit, const TArray<FCrowdUnit>& NearbyUnits)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowdUnit& Other : NearbyUnits)
    {
        float Distance = FVector::Dist(Unit.Position, Other.Position);
        if (Distance > 0 && Distance < 50.0f) // Distância mínima de separação
        {
            FVector Diff = (Unit.Position - Other.Position).GetSafeNormal();
            Diff /= Distance; // Peso inversamente proporcional à distância
            SeparationForce += Diff;
            Count++;
        }
    }

    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce = SeparationForce.GetSafeNormal() * MovementSpeed;
        SeparationForce -= Unit.Velocity;
    }

    return SeparationForce;
}

FVector ACrowdManager::CalculateAlignment(const FCrowdUnit& Unit, const TArray<FCrowdUnit>& NearbyUnits)
{
    FVector AverageVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowdUnit& Other : NearbyUnits)
    {
        AverageVelocity += Other.Velocity;
        Count++;
    }

    if (Count > 0)
    {
        AverageVelocity /= Count;
        AverageVelocity = AverageVelocity.GetSafeNormal() * MovementSpeed;
        return AverageVelocity - Unit.Velocity;
    }

    return FVector::ZeroVector;
}

FVector ACrowdManager::CalculateCohesion(const FCrowdUnit& Unit, const TArray<FCrowdUnit>& NearbyUnits)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowdUnit& Other : NearbyUnits)
    {
        CenterOfMass += Other.Position;
        Count++;
    }

    if (Count > 0)
    {
        CenterOfMass /= Count;
        FVector Desired = (CenterOfMass - Unit.Position).GetSafeNormal() * MovementSpeed;
        return Desired - Unit.Velocity;
    }

    return FVector::ZeroVector;
}

TArray<FCrowdUnit> ACrowdManager::GetNearbyUnits(const FCrowdUnit& Unit, float Radius)
{
    TArray<FCrowdUnit> NearbyUnits;

    for (const FCrowdUnit& Other : CrowdUnits)
    {
        if (&Other != &Unit)
        {
            float Distance = FVector::Dist(Unit.Position, Other.Position);
            if (Distance <= Radius)
            {
                NearbyUnits.Add(Other);
            }
        }
    }

    return NearbyUnits;
}

void ACrowdManager::ClearCrowd()
{
    CrowdUnits.Empty();
}