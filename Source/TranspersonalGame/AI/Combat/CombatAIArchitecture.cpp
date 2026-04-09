#include "CombatAIArchitecture.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"

UCombatAIArchitecture::UCombatAIArchitecture()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for AI architecture updates
    
    // Initialize default values
    MaxCombatDistance = 2000.0f;
    MinCombatDistance = 100.0f;
    TacticalUpdateInterval = 0.5f;
    MaxSimultaneousCombatants = 8;
    
    // Initialize tactical layers
    InitializeTacticalLayers();
}

void UCombatAIArchitecture::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with global combat manager
    RegisterWithCombatManager();
    
    // Initialize tactical grid
    InitializeTacticalGrid();
    
    // Start tactical analysis timer
    GetWorld()->GetTimerManager().SetTimer(
        TacticalAnalysisTimer,
        this,
        &UCombatAIArchitecture::PerformTacticalAnalysis,
        TacticalUpdateInterval,
        true
    );
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIArchitecture initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombatAIArchitecture::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update combat state machine
    UpdateCombatStateMachine(DeltaTime);
    
    // Update tactical awareness
    UpdateTacticalAwareness(DeltaTime);
    
    // Process combat decisions
    ProcessCombatDecisions(DeltaTime);
}

void UCombatAIArchitecture::InitializeTacticalLayers()
{
    // Layer 1: Immediate Threat Assessment
    FTacticalLayer ImmediateLayer;
    ImmediateLayer.LayerName = TEXT("Immediate");
    ImmediateLayer.Priority = 10;
    ImmediateLayer.UpdateFrequency = 0.1f;
    ImmediateLayer.InfluenceRadius = 500.0f;
    TacticalLayers.Add(ImmediateLayer);
    
    // Layer 2: Tactical Positioning
    FTacticalLayer TacticalLayer;
    TacticalLayer.LayerName = TEXT("Tactical");
    TacticalLayer.Priority = 7;
    TacticalLayer.UpdateFrequency = 0.3f;
    TacticalLayer.InfluenceRadius = 1000.0f;
    TacticalLayers.Add(TacticalLayer);
    
    // Layer 3: Strategic Overview
    FTacticalLayer StrategicLayer;
    StrategicLayer.LayerName = TEXT("Strategic");
    StrategicLayer.Priority = 5;
    StrategicLayer.UpdateFrequency = 1.0f;
    StrategicLayer.InfluenceRadius = 2000.0f;
    TacticalLayers.Add(StrategicLayer);
}

void UCombatAIArchitecture::RegisterWithCombatManager()
{
    // Find or create global combat manager
    if (UWorld* World = GetWorld())
    {
        // Implementation would register this AI with a global combat coordination system
        UE_LOG(LogTemp, Log, TEXT("Registering %s with Combat Manager"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UCombatAIArchitecture::InitializeTacticalGrid()
{
    // Create a spatial grid for tactical analysis
    GridCellSize = 200.0f;
    GridRadius = 1500.0f;
    
    int32 GridSize = FMath::CeilToInt((GridRadius * 2.0f) / GridCellSize);
    TacticalGrid.SetNum(GridSize * GridSize);
    
    // Initialize grid cells
    for (int32 i = 0; i < TacticalGrid.Num(); i++)
    {
        FTacticalGridCell& Cell = TacticalGrid[i];
        Cell.CellIndex = i;
        Cell.ThreatLevel = 0.0f;
        Cell.CoverValue = 0.0f;
        Cell.FlankingValue = 0.0f;
        Cell.bIsOccupied = false;
        Cell.OccupyingActor = nullptr;
        
        // Calculate world position for this cell
        int32 X = i % GridSize;
        int32 Y = i / GridSize;
        
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        Cell.WorldPosition = OwnerLocation + FVector(
            (X - GridSize/2) * GridCellSize,
            (Y - GridSize/2) * GridCellSize,
            0.0f
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tactical grid initialized with %d cells"), TacticalGrid.Num());
}

void UCombatAIArchitecture::PerformTacticalAnalysis()
{
    if (!GetOwner())
        return;
        
    // Update threat assessment
    UpdateThreatAssessment();
    
    // Analyze tactical positions
    AnalyzeTacticalPositions();
    
    // Update combat priorities
    UpdateCombatPriorities();
    
    // Broadcast tactical update to behavior tree
    BroadcastTacticalUpdate();
}

void UCombatAIArchitecture::UpdateThreatAssessment()
{
    CurrentThreats.Empty();
    
    // Scan for threats within combat range
    TArray<AActor*> NearbyActors;
    UWorld* World = GetWorld();
    if (!World) return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Simple sphere overlap to find nearby actors
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && Pawn != GetOwner())
        {
            float Distance = FVector::Dist(OwnerLocation, Pawn->GetActorLocation());
            if (Distance <= MaxCombatDistance)
            {
                FCombatThreat Threat;
                Threat.ThreatActor = Pawn;
                Threat.ThreatLevel = CalculateThreatLevel(Pawn, Distance);
                Threat.Distance = Distance;
                Threat.LastSeenLocation = Pawn->GetActorLocation();
                Threat.LastSeenTime = World->GetTimeSeconds();
                
                CurrentThreats.Add(Threat);
            }
        }
    }
    
    // Sort threats by priority
    CurrentThreats.Sort([](const FCombatThreat& A, const FCombatThreat& B) {
        return A.ThreatLevel > B.ThreatLevel;
    });
}

float UCombatAIArchitecture::CalculateThreatLevel(AActor* Actor, float Distance)
{
    if (!Actor) return 0.0f;
    
    float ThreatLevel = 0.0f;
    
    // Base threat from distance (closer = more threatening)
    float DistanceThreat = FMath::Clamp(1.0f - (Distance / MaxCombatDistance), 0.0f, 1.0f);
    ThreatLevel += DistanceThreat * 0.4f;
    
    // Check if it's a player character (higher threat)
    if (Actor->IsA<APawn>())
    {
        APawn* Pawn = Cast<APawn>(Actor);
        if (Pawn && Pawn->IsPlayerControlled())
        {
            ThreatLevel += 0.6f; // Players are high priority threats
        }
    }
    
    // Factor in line of sight
    if (HasLineOfSight(Actor))
    {
        ThreatLevel += 0.2f;
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

bool UCombatAIArchitecture::HasLineOfSight(AActor* Target)
{
    if (!Target || !GetOwner()) return false;
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Target->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // No hit means clear line of sight
}

void UCombatAIArchitecture::AnalyzeTacticalPositions()
{
    // Update tactical grid based on current threats
    for (FTacticalGridCell& Cell : TacticalGrid)
    {
        AnalyzeCellTacticalValue(Cell);
    }
}

void UCombatAIArchitecture::AnalyzeCellTacticalValue(FTacticalGridCell& Cell)
{
    Cell.ThreatLevel = 0.0f;
    Cell.CoverValue = 0.0f;
    Cell.FlankingValue = 0.0f;
    
    // Analyze threat exposure for this cell
    for (const FCombatThreat& Threat : CurrentThreats)
    {
        if (Threat.ThreatActor)
        {
            float DistanceToThreat = FVector::Dist(Cell.WorldPosition, Threat.ThreatActor->GetActorLocation());
            float ThreatInfluence = FMath::Clamp(1.0f - (DistanceToThreat / MaxCombatDistance), 0.0f, 1.0f);
            Cell.ThreatLevel += ThreatInfluence * Threat.ThreatLevel;
        }
    }
    
    // Analyze cover opportunities (simplified - would use actual cover analysis in full implementation)
    Cell.CoverValue = AnalyzeCoverAtPosition(Cell.WorldPosition);
    
    // Analyze flanking opportunities
    Cell.FlankingValue = AnalyzeFlankingAtPosition(Cell.WorldPosition);
}

float UCombatAIArchitecture::AnalyzeCoverAtPosition(const FVector& Position)
{
    // Simplified cover analysis - in full implementation would check for actual cover objects
    float CoverValue = 0.0f;
    
    // Check for nearby walls or obstacles
    TArray<FVector> CheckDirections = {
        FVector::ForwardVector,
        FVector::RightVector,
        FVector::BackwardVector,
        -FVector::RightVector
    };
    
    for (const FVector& Direction : CheckDirections)
    {
        FVector CheckEnd = Position + Direction * 100.0f;
        FHitResult HitResult;
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Position,
            CheckEnd,
            ECollisionChannel::ECC_WorldStatic
        );
        
        if (bHit)
        {
            CoverValue += 0.25f; // Each direction with cover adds value
        }
    }
    
    return FMath::Clamp(CoverValue, 0.0f, 1.0f);
}

float UCombatAIArchitecture::AnalyzeFlankingAtPosition(const FVector& Position)
{
    // Simplified flanking analysis
    float FlankingValue = 0.0f;
    
    for (const FCombatThreat& Threat : CurrentThreats)
    {
        if (Threat.ThreatActor)
        {
            FVector ThreatLocation = Threat.ThreatActor->GetActorLocation();
            FVector ThreatForward = Threat.ThreatActor->GetActorForwardVector();
            
            FVector ToPosition = (Position - ThreatLocation).GetSafeNormal();
            float DotProduct = FVector::DotProduct(ThreatForward, ToPosition);
            
            // Positions behind or to the side of threats have higher flanking value
            if (DotProduct < 0.0f)
            {
                FlankingValue += FMath::Abs(DotProduct) * 0.5f;
            }
        }
    }
    
    return FMath::Clamp(FlankingValue, 0.0f, 1.0f);
}

void UCombatAIArchitecture::UpdateCombatPriorities()
{
    // Update current tactical priorities based on analysis
    CurrentTacticalPriorities.Empty();
    
    if (CurrentThreats.Num() > 0)
    {
        // Primary threat engagement
        FTacticalPriority EngagePriority;
        EngagePriority.PriorityType = ETacticalPriorityType::Engage;
        EngagePriority.Priority = 8.0f;
        EngagePriority.TargetActor = CurrentThreats[0].ThreatActor;
        CurrentTacticalPriorities.Add(EngagePriority);
        
        // Positioning priority
        FTacticalPriority PositionPriority;
        PositionPriority.PriorityType = ETacticalPriorityType::Position;
        PositionPriority.Priority = 6.0f;
        PositionPriority.TargetLocation = FindOptimalTacticalPosition();
        CurrentTacticalPriorities.Add(PositionPriority);
    }
    else
    {
        // Patrol priority when no threats
        FTacticalPriority PatrolPriority;
        PatrolPriority.PriorityType = ETacticalPriorityType::Patrol;
        PatrolPriority.Priority = 3.0f;
        CurrentTacticalPriorities.Add(PatrolPriority);
    }
}

FVector UCombatAIArchitecture::FindOptimalTacticalPosition()
{
    FVector BestPosition = GetOwner()->GetActorLocation();
    float BestScore = -1.0f;
    
    for (const FTacticalGridCell& Cell : TacticalGrid)
    {
        // Calculate tactical score for this cell
        float Score = 0.0f;
        
        // Prefer positions with good cover
        Score += Cell.CoverValue * 0.4f;
        
        // Prefer positions with flanking opportunities
        Score += Cell.FlankingValue * 0.3f;
        
        // Avoid high threat areas
        Score -= Cell.ThreatLevel * 0.5f;
        
        // Prefer positions not too far from current location
        float DistanceFromCurrent = FVector::Dist(Cell.WorldPosition, GetOwner()->GetActorLocation());
        float DistancePenalty = FMath::Clamp(DistanceFromCurrent / 1000.0f, 0.0f, 0.3f);
        Score -= DistancePenalty;
        
        if (Score > BestScore && !Cell.bIsOccupied)
        {
            BestScore = Score;
            BestPosition = Cell.WorldPosition;
        }
    }
    
    return BestPosition;
}

void UCombatAIArchitecture::BroadcastTacticalUpdate()
{
    // Update blackboard with tactical information
    if (AAIController* AIController = Cast<AAIController>(GetOwner()))
    {
        if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
        {
            // Update primary threat
            if (CurrentThreats.Num() > 0)
            {
                Blackboard->SetValueAsObject(TEXT("PrimaryThreat"), CurrentThreats[0].ThreatActor);
                Blackboard->SetValueAsVector(TEXT("ThreatLocation"), CurrentThreats[0].LastSeenLocation);
                Blackboard->SetValueAsFloat(TEXT("ThreatLevel"), CurrentThreats[0].ThreatLevel);
            }
            else
            {
                Blackboard->ClearValue(TEXT("PrimaryThreat"));
            }
            
            // Update tactical position
            Blackboard->SetValueAsVector(TEXT("OptimalPosition"), FindOptimalTacticalPosition());
            
            // Update combat state
            Blackboard->SetValueAsBool(TEXT("InCombat"), CurrentThreats.Num() > 0);
        }
    }
}

void UCombatAIArchitecture::UpdateCombatStateMachine(float DeltaTime)
{
    // Simple state machine update
    switch (CurrentCombatState)
    {
        case ECombatAIState::Idle:
            if (CurrentThreats.Num() > 0)
            {
                TransitionToState(ECombatAIState::Engaging);
            }
            break;
            
        case ECombatAIState::Engaging:
            if (CurrentThreats.Num() == 0)
            {
                TransitionToState(ECombatAIState::Idle);
            }
            break;
            
        case ECombatAIState::Flanking:
            // Flanking logic would be implemented here
            break;
            
        case ECombatAIState::Retreating:
            // Retreat logic would be implemented here
            break;
    }
}

void UCombatAIArchitecture::TransitionToState(ECombatAIState NewState)
{
    if (CurrentCombatState != NewState)
    {
        ECombatAIState PreviousState = CurrentCombatState;
        CurrentCombatState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("%s transitioning from %d to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               (int32)PreviousState, (int32)NewState);
        
        // Trigger state change events
        OnCombatStateChanged.Broadcast(PreviousState, NewState);
    }
}

void UCombatAIArchitecture::UpdateTacticalAwareness(float DeltaTime)
{
    // Update awareness of tactical situation
    TacticalAwarenessLevel = 0.0f;
    
    // Factor in number of known threats
    if (CurrentThreats.Num() > 0)
    {
        TacticalAwarenessLevel += FMath::Clamp(CurrentThreats.Num() / 3.0f, 0.0f, 0.5f);
    }
    
    // Factor in recent combat activity
    float TimeSinceLastCombat = GetWorld()->GetTimeSeconds() - LastCombatTime;
    if (TimeSinceLastCombat < 30.0f)
    {
        TacticalAwarenessLevel += 0.3f;
    }
    
    // Factor in environmental complexity
    TacticalAwarenessLevel += CalculateEnvironmentalComplexity() * 0.2f;
    
    TacticalAwarenessLevel = FMath::Clamp(TacticalAwarenessLevel, 0.0f, 1.0f);
}

float UCombatAIArchitecture::CalculateEnvironmentalComplexity()
{
    // Simplified environmental complexity calculation
    float Complexity = 0.0f;
    
    // Count nearby objects that could provide cover or tactical advantage
    int32 NearbyObjects = 0;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor != GetOwner())
        {
            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            if (Distance <= 500.0f && Actor->GetRootComponent() && Actor->GetRootComponent()->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                NearbyObjects++;
            }
        }
    }
    
    Complexity = FMath::Clamp(NearbyObjects / 20.0f, 0.0f, 1.0f);
    return Complexity;
}

void UCombatAIArchitecture::ProcessCombatDecisions(float DeltaTime)
{
    // Process high-level combat decisions based on tactical analysis
    if (CurrentThreats.Num() > 0)
    {
        // Update last combat time
        LastCombatTime = GetWorld()->GetTimeSeconds();
        
        // Make combat decisions based on current priorities
        for (const FTacticalPriority& Priority : CurrentTacticalPriorities)
        {
            ProcessTacticalPriority(Priority);
        }
    }
}

void UCombatAIArchitecture::ProcessTacticalPriority(const FTacticalPriority& Priority)
{
    switch (Priority.PriorityType)
    {
        case ETacticalPriorityType::Engage:
            // Signal behavior tree to engage target
            if (AAIController* AIController = Cast<AAIController>(GetOwner()))
            {
                if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
                {
                    Blackboard->SetValueAsObject(TEXT("EngageTarget"), Priority.TargetActor);
                }
            }
            break;
            
        case ETacticalPriorityType::Position:
            // Signal behavior tree to move to tactical position
            if (AAIController* AIController = Cast<AAIController>(GetOwner()))
            {
                if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
                {
                    Blackboard->SetValueAsVector(TEXT("TacticalMoveLocation"), Priority.TargetLocation);
                }
            }
            break;
            
        case ETacticalPriorityType::Patrol:
            // Continue patrol behavior
            break;
            
        case ETacticalPriorityType::Retreat:
            // Signal retreat
            if (AAIController* AIController = Cast<AAIController>(GetOwner()))
            {
                if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
                {
                    Blackboard->SetValueAsBool(TEXT("ShouldRetreat"), true);
                }
            }
            break;
    }
}