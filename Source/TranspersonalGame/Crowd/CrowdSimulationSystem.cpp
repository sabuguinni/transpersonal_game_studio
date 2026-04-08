#include "CrowdSimulationSystem.h"
#include "CrowdAgent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

ACrowdSimulationSystem::ACrowdSimulationSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.033f; // 30 FPS for crowd simulation

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Default settings
    MaxCrowdAgents = 200;
    SpawnRadius = 2000.0f;
    AgentInteractionRadius = 150.0f;
    ConsciousnessInfluenceRadius = 500.0f;

    // Behavior weights
    SeparationWeight = 2.0f;
    AlignmentWeight = 1.0f;
    CohesionWeight = 1.0f;
    ConsciousnessWeight = 1.5f;

    // Consciousness settings
    CollectiveThreshold = 0.7f;
    EmergenceRate = 0.1f;
    bEnableCollectiveConsciousness = true;

    CurrentAgentCount = 0;
    LastUpdateTime = 0.0f;
}

void ACrowdSimulationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize spatial grid
    UpdateSpatialGrid();
    
    // Create initial consciousness field
    if (bEnableCollectiveConsciousness)
    {
        CreateCollectiveConsciousnessField(GetActorLocation(), 1000.0f, EConsciousnessLevel::Rational);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationSystem initialized with max agents: %d"), MaxCrowdAgents);
}

void ACrowdSimulationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update crowd behaviors
    UpdateCrowdBehavior(DeltaTime);
    
    // Update flow fields
    UpdateFlowFields(DeltaTime);
    
    // Process consciousness influence
    if (bEnableCollectiveConsciousness)
    {
        ProcessConsciousnessInfluence(DeltaTime);
        UpdateCollectiveConsciousness(DeltaTime);
    }
    
    // Handle emergent behaviors
    HandleEmergentBehaviors(DeltaTime);
    
    // Update spatial partitioning every few frames
    if (FMath::Fmod(LastUpdateTime, 0.1f) < DeltaTime)
    {
        UpdateSpatialGrid();
        CleanupInvalidAgents();
    }
}

void ACrowdSimulationSystem::SpawnCrowdAgents(int32 Count, FVector SpawnCenter)
{
    if (Count <= 0 || CurrentAgentCount >= MaxCrowdAgents)
    {
        return;
    }
    
    int32 ActualSpawnCount = FMath::Min(Count, MaxCrowdAgents - CurrentAgentCount);
    
    for (int32 i = 0; i < ActualSpawnCount; i++)
    {
        // Random position within spawn radius
        FVector RandomOffset = FMath::VRand() * FMath::RandRange(50.0f, SpawnRadius);
        RandomOffset.Z = 0.0f; // Keep on ground level
        FVector SpawnLocation = SpawnCenter + RandomOffset;
        
        SpawnSingleAgent(SpawnLocation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d crowd agents. Total: %d"), ActualSpawnCount, CurrentAgentCount);
}

void ACrowdSimulationSystem::SpawnSingleAgent(FVector Location)
{
    if (!GetWorld())
    {
        return;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ACrowdAgent* NewAgent = GetWorld()->SpawnActor<ACrowdAgent>(ACrowdAgent::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (NewAgent)
    {
        CrowdAgents.Add(NewAgent);
        CurrentAgentCount++;
        
        // Initialize agent with random consciousness level
        if (UConsciousnessComponent* ConsciousnessComp = NewAgent->FindComponentByClass<UConsciousnessComponent>())
        {
            EConsciousnessLevel RandomLevel = static_cast<EConsciousnessLevel>(FMath::RandRange(0, 6));
            ConsciousnessComp->SetConsciousnessLevel(RandomLevel);
        }
    }
}

void ACrowdSimulationSystem::DespawnAllAgents()
{
    for (ACrowdAgent* Agent : CrowdAgents)
    {
        if (IsValid(Agent))
        {
            Agent->Destroy();
        }
    }
    
    CrowdAgents.Empty();
    CurrentAgentCount = 0;
    SpatialGrid.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Despawned all crowd agents"));
}

void ACrowdSimulationSystem::SetCrowdBehaviorState(ECrowdBehaviorState NewState)
{
    CurrentFlowData.BehaviorState = NewState;
    
    // Apply state-specific behavior modifications
    for (ACrowdAgent* Agent : CrowdAgents)
    {
        if (IsValid(Agent))
        {
            Agent->SetBehaviorState(NewState);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Set crowd behavior state to: %d"), static_cast<int32>(NewState));
}

void ACrowdSimulationSystem::UpdateCrowdBehavior(float DeltaTime)
{
    for (ACrowdAgent* Agent : CrowdAgents)
    {
        if (!IsValid(Agent))
        {
            continue;
        }
        
        FVector Separation = CalculateSeparation(Agent);
        FVector Alignment = CalculateAlignment(Agent);
        FVector Cohesion = CalculateCohesion(Agent);
        FVector ConsciousnessInfluence = CalculateConsciousnessInfluence(Agent);
        
        // Combine all forces
        FVector TotalForce = (Separation * SeparationWeight) +
                           (Alignment * AlignmentWeight) +
                           (Cohesion * CohesionWeight) +
                           (ConsciousnessInfluence * ConsciousnessWeight);
        
        // Apply flow field influence
        FCrowdFlowData FlowData = GetFlowDataAtLocation(Agent->GetActorLocation());
        TotalForce += FlowData.FlowDirection * FlowData.FlowIntensity;
        
        // Update agent movement
        Agent->ApplyMovementForce(TotalForce, DeltaTime);
    }
}

FVector ACrowdSimulationSystem::CalculateSeparation(ACrowdAgent* Agent)
{
    if (!IsValid(Agent))
    {
        return FVector::ZeroVector;
    }
    
    FVector SeparationForce = FVector::ZeroVector;
    FVector AgentLocation = Agent->GetActorLocation();
    int32 NeighborCount = 0;
    
    TArray<ACrowdAgent*> NearbyAgents = GetNearbyAgents(AgentLocation, AgentInteractionRadius);
    
    for (ACrowdAgent* OtherAgent : NearbyAgents)
    {
        if (OtherAgent == Agent || !IsValid(OtherAgent))
        {
            continue;
        }
        
        FVector ToOther = OtherAgent->GetActorLocation() - AgentLocation;
        float Distance = ToOther.Size();
        
        if (Distance > 0.0f && Distance < AgentInteractionRadius)
        {
            FVector AwayFromOther = -ToOther.GetSafeNormal();
            AwayFromOther *= (AgentInteractionRadius - Distance) / AgentInteractionRadius;
            SeparationForce += AwayFromOther;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
        SeparationForce.Normalize();
    }
    
    return SeparationForce;
}

FVector ACrowdSimulationSystem::CalculateAlignment(ACrowdAgent* Agent)
{
    if (!IsValid(Agent))
    {
        return FVector::ZeroVector;
    }
    
    FVector AverageVelocity = FVector::ZeroVector;
    FVector AgentLocation = Agent->GetActorLocation();
    int32 NeighborCount = 0;
    
    TArray<ACrowdAgent*> NearbyAgents = GetNearbyAgents(AgentLocation, AgentInteractionRadius);
    
    for (ACrowdAgent* OtherAgent : NearbyAgents)
    {
        if (OtherAgent == Agent || !IsValid(OtherAgent))
        {
            continue;
        }
        
        float Distance = FVector::Dist(OtherAgent->GetActorLocation(), AgentLocation);
        
        if (Distance < AgentInteractionRadius)
        {
            AverageVelocity += OtherAgent->GetVelocity();
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        AverageVelocity /= NeighborCount;
        AverageVelocity.Normalize();
    }
    
    return AverageVelocity;
}

FVector ACrowdSimulationSystem::CalculateCohesion(ACrowdAgent* Agent)
{
    if (!IsValid(Agent))
    {
        return FVector::ZeroVector;
    }
    
    FVector CenterOfMass = FVector::ZeroVector;
    FVector AgentLocation = Agent->GetActorLocation();
    int32 NeighborCount = 0;
    
    TArray<ACrowdAgent*> NearbyAgents = GetNearbyAgents(AgentLocation, AgentInteractionRadius);
    
    for (ACrowdAgent* OtherAgent : NearbyAgents)
    {
        if (OtherAgent == Agent || !IsValid(OtherAgent))
        {
            continue;
        }
        
        float Distance = FVector::Dist(OtherAgent->GetActorLocation(), AgentLocation);
        
        if (Distance < AgentInteractionRadius)
        {
            CenterOfMass += OtherAgent->GetActorLocation();
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        CenterOfMass /= NeighborCount;
        FVector ToCenterOfMass = CenterOfMass - AgentLocation;
        ToCenterOfMass.Normalize();
        return ToCenterOfMass;
    }
    
    return FVector::ZeroVector;
}

FVector ACrowdSimulationSystem::CalculateConsciousnessInfluence(ACrowdAgent* Agent)
{
    if (!IsValid(Agent) || !bEnableCollectiveConsciousness)
    {
        return FVector::ZeroVector;
    }
    
    FVector ConsciousnessForce = FVector::ZeroVector;
    FVector AgentLocation = Agent->GetActorLocation();
    
    UConsciousnessComponent* AgentConsciousness = Agent->FindComponentByClass<UConsciousnessComponent>();
    if (!AgentConsciousness)
    {
        return FVector::ZeroVector;
    }
    
    EConsciousnessLevel AgentLevel = AgentConsciousness->GetCurrentLevel();
    
    // Find the strongest consciousness field affecting this agent
    for (const FCollectiveConsciousnessField& Field : ConsciousnessFields)
    {
        float Distance = FVector::Dist(AgentLocation, Field.Center);
        
        if (Distance < Field.Radius)
        {
            float Influence = (Field.Radius - Distance) / Field.Radius * Field.Intensity;
            
            // Different consciousness levels create different movement patterns
            FVector ToCenter = Field.Center - AgentLocation;
            ToCenter.Normalize();
            
            switch (Field.DominantLevel)
            {
                case EConsciousnessLevel::Transpersonal:
                case EConsciousnessLevel::Unity:
                    // Higher levels create harmonious gathering
                    ConsciousnessForce += ToCenter * Influence * 0.5f;
                    break;
                    
                case EConsciousnessLevel::Integral:
                case EConsciousnessLevel::Psychic:
                    // Mid levels create organized movement
                    FVector CircularForce = FVector::CrossProduct(ToCenter, FVector::UpVector);
                    ConsciousnessForce += CircularForce * Influence * 0.3f;
                    break;
                    
                case EConsciousnessLevel::Rational:
                case EConsciousnessLevel::Mythic:
                    // Lower levels create more chaotic patterns
                    FVector RandomForce = FMath::VRand();
                    RandomForce.Z = 0.0f;
                    ConsciousnessForce += RandomForce * Influence * 0.2f;
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    return ConsciousnessForce;
}

void ACrowdSimulationSystem::CreateCollectiveConsciousnessField(FVector Center, float Radius, EConsciousnessLevel Level)
{
    FCollectiveConsciousnessField NewField;
    NewField.Center = Center;
    NewField.Radius = Radius;
    NewField.Intensity = 1.0f;
    NewField.DominantLevel = Level;
    
    // Initialize level distribution
    NewField.LevelDistribution.Add(Level, 1.0f);
    
    ConsciousnessFields.Add(NewField);
    
    UE_LOG(LogTemp, Log, TEXT("Created consciousness field at %s with radius %f and level %d"), 
           *Center.ToString(), Radius, static_cast<int32>(Level));
}

void ACrowdSimulationSystem::UpdateCollectiveConsciousness(float DeltaTime)
{
    for (FCollectiveConsciousnessField& Field : ConsciousnessFields)
    {
        // Reset level distribution
        Field.LevelDistribution.Empty();
        
        // Count agents in this field
        int32 AgentsInField = 0;
        
        for (ACrowdAgent* Agent : CrowdAgents)
        {
            if (!IsValid(Agent))
            {
                continue;
            }
            
            float Distance = FVector::Dist(Agent->GetActorLocation(), Field.Center);
            
            if (Distance < Field.Radius)
            {
                AgentsInField++;
                
                UConsciousnessComponent* AgentConsciousness = Agent->FindComponentByClass<UConsciousnessComponent>();
                if (AgentConsciousness)
                {
                    EConsciousnessLevel AgentLevel = AgentConsciousness->GetCurrentLevel();
                    
                    if (Field.LevelDistribution.Contains(AgentLevel))
                    {
                        Field.LevelDistribution[AgentLevel] += 1.0f;
                    }
                    else
                    {
                        Field.LevelDistribution.Add(AgentLevel, 1.0f);
                    }
                }
            }
        }
        
        // Normalize distribution and find dominant level
        if (AgentsInField > 0)
        {
            float MaxCount = 0.0f;
            EConsciousnessLevel NewDominantLevel = Field.DominantLevel;
            
            for (auto& LevelPair : Field.LevelDistribution)
            {
                LevelPair.Value /= AgentsInField;
                
                if (LevelPair.Value > MaxCount)
                {
                    MaxCount = LevelPair.Value;
                    NewDominantLevel = LevelPair.Key;
                }
            }
            
            // Gradual transition to new dominant level
            if (NewDominantLevel != Field.DominantLevel && MaxCount > CollectiveThreshold)
            {
                Field.DominantLevel = NewDominantLevel;
                Field.Intensity = FMath::Lerp(Field.Intensity, MaxCount, EmergenceRate * DeltaTime);
            }
        }
    }
}

void ACrowdSimulationSystem::UpdateSpatialGrid()
{
    SpatialGrid.Empty();
    
    const float GridSize = AgentInteractionRadius;
    
    for (ACrowdAgent* Agent : CrowdAgents)
    {
        if (!IsValid(Agent))
        {
            continue;
        }
        
        FVector Location = Agent->GetActorLocation();
        FIntVector GridCoord = FIntVector(
            FMath::FloorToInt(Location.X / GridSize),
            FMath::FloorToInt(Location.Y / GridSize),
            FMath::FloorToInt(Location.Z / GridSize)
        );
        
        if (!SpatialGrid.Contains(GridCoord))
        {
            SpatialGrid.Add(GridCoord, TArray<ACrowdAgent*>());
        }
        
        SpatialGrid[GridCoord].Add(Agent);
    }
}

TArray<ACrowdAgent*> ACrowdSimulationSystem::GetNearbyAgents(FVector Location, float Radius)
{
    TArray<ACrowdAgent*> NearbyAgents;
    
    const float GridSize = AgentInteractionRadius;
    int32 GridRadius = FMath::CeilToInt(Radius / GridSize);
    
    FIntVector CenterGrid = FIntVector(
        FMath::FloorToInt(Location.X / GridSize),
        FMath::FloorToInt(Location.Y / GridSize),
        FMath::FloorToInt(Location.Z / GridSize)
    );
    
    for (int32 x = -GridRadius; x <= GridRadius; x++)
    {
        for (int32 y = -GridRadius; y <= GridRadius; y++)
        {
            for (int32 z = -GridRadius; z <= GridRadius; z++)
            {
                FIntVector GridCoord = CenterGrid + FIntVector(x, y, z);
                
                if (SpatialGrid.Contains(GridCoord))
                {
                    for (ACrowdAgent* Agent : SpatialGrid[GridCoord])
                    {
                        if (IsValid(Agent))
                        {
                            float Distance = FVector::Dist(Agent->GetActorLocation(), Location);
                            if (Distance <= Radius)
                            {
                                NearbyAgents.Add(Agent);
                            }
                        }
                    }
                }
            }
        }
    }
    
    return NearbyAgents;
}

void ACrowdSimulationSystem::CleanupInvalidAgents()
{
    CrowdAgents.RemoveAll([](ACrowdAgent* Agent) {
        return !IsValid(Agent);
    });
    
    CurrentAgentCount = CrowdAgents.Num();
}

void ACrowdSimulationSystem::HandleEmergentBehaviors(float DeltaTime)
{
    // Analyze crowd patterns and trigger emergent behaviors
    float TotalDensity = 0.0f;
    FVector AveragePosition = FVector::ZeroVector;
    
    for (ACrowdAgent* Agent : CrowdAgents)
    {
        if (IsValid(Agent))
        {
            AveragePosition += Agent->GetActorLocation();
        }
    }
    
    if (CurrentAgentCount > 0)
    {
        AveragePosition /= CurrentAgentCount;
        TotalDensity = GetCrowdDensity(AveragePosition, 500.0f);
        
        // Trigger emergent behaviors based on density and consciousness
        if (TotalDensity > 0.8f)
        {
            // High density - potential panic or celebration
            EConsciousnessLevel DominantLevel = GetDominantConsciousnessLevel(AveragePosition);
            
            if (DominantLevel <= EConsciousnessLevel::Mythic)
            {
                SetCrowdBehaviorState(ECrowdBehaviorState::Panicking);
            }
            else if (DominantLevel >= EConsciousnessLevel::Integral)
            {
                SetCrowdBehaviorState(ECrowdBehaviorState::Celebrating);
            }
        }
        else if (TotalDensity < 0.2f)
        {
            // Low density - return to neutral
            SetCrowdBehaviorState(ECrowdBehaviorState::Neutral);
        }
    }
}

FCrowdFlowData ACrowdSimulationSystem::GetFlowDataAtLocation(FVector Location)
{
    // For now, return the current global flow data
    // In a more complex system, this would sample from flow field textures
    return CurrentFlowData;
}

void ACrowdSimulationSystem::SetFlowDirection(FVector Direction, float Intensity)
{
    CurrentFlowData.FlowDirection = Direction.GetSafeNormal();
    CurrentFlowData.FlowIntensity = Intensity;
    
    UE_LOG(LogTemp, Log, TEXT("Set flow direction to %s with intensity %f"), 
           *Direction.ToString(), Intensity);
}

float ACrowdSimulationSystem::GetCrowdDensity(FVector Location, float Radius)
{
    if (Radius <= 0.0f)
    {
        return 0.0f;
    }
    
    int32 AgentsInRadius = 0;
    
    for (ACrowdAgent* Agent : CrowdAgents)
    {
        if (IsValid(Agent))
        {
            float Distance = FVector::Dist(Agent->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                AgentsInRadius++;
            }
        }
    }
    
    float Area = PI * Radius * Radius;
    return AgentsInRadius / Area;
}

EConsciousnessLevel ACrowdSimulationSystem::GetDominantConsciousnessLevel(FVector Location)
{
    TMap<EConsciousnessLevel, int32> LevelCounts;
    
    for (ACrowdAgent* Agent : CrowdAgents)
    {
        if (IsValid(Agent))
        {
            UConsciousnessComponent* ConsciousnessComp = Agent->FindComponentByClass<UConsciousnessComponent>();
            if (ConsciousnessComp)
            {
                EConsciousnessLevel Level = ConsciousnessComp->GetCurrentLevel();
                
                if (LevelCounts.Contains(Level))
                {
                    LevelCounts[Level]++;
                }
                else
                {
                    LevelCounts.Add(Level, 1);
                }
            }
        }
    }
    
    EConsciousnessLevel DominantLevel = EConsciousnessLevel::Rational;
    int32 MaxCount = 0;
    
    for (const auto& LevelPair : LevelCounts)
    {
        if (LevelPair.Value > MaxCount)
        {
            MaxCount = LevelPair.Value;
            DominantLevel = LevelPair.Key;
        }
    }
    
    return DominantLevel;
}

void ACrowdSimulationSystem::OnPlayerConsciousnessChanged(EConsciousnessLevel NewLevel)
{
    // Create a consciousness field around the player
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        CreateCollectiveConsciousnessField(PlayerPawn->GetActorLocation(), ConsciousnessInfluenceRadius, NewLevel);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Player consciousness changed to level %d, updating crowd influence"), static_cast<int32>(NewLevel));
}

void ACrowdSimulationSystem::OnCombatStarted(FVector CombatLocation)
{
    // Create panic behavior around combat zones
    for (ACrowdAgent* Agent : CrowdAgents)
    {
        if (IsValid(Agent))
        {
            float Distance = FVector::Dist(Agent->GetActorLocation(), CombatLocation);
            if (Distance < 1000.0f) // Panic radius
            {
                Agent->SetBehaviorState(ECrowdBehaviorState::Panicking);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Combat started at %s, triggering crowd panic response"), *CombatLocation.ToString());
}

void ACrowdSimulationSystem::OnMeditationEvent(FVector MeditationCenter, float Radius)
{
    // Create peaceful gathering behavior
    CreateCollectiveConsciousnessField(MeditationCenter, Radius, EConsciousnessLevel::Transpersonal);
    
    for (ACrowdAgent* Agent : CrowdAgents)
    {
        if (IsValid(Agent))
        {
            float Distance = FVector::Dist(Agent->GetActorLocation(), MeditationCenter);
            if (Distance < Radius)
            {
                Agent->SetBehaviorState(ECrowdBehaviorState::Meditating);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Meditation event at %s with radius %f, creating peaceful gathering"), 
           *MeditationCenter.ToString(), Radius);
}