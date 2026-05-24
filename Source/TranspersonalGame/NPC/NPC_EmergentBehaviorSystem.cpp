#include "NPC_EmergentBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UNPC_EmergentBehaviorSystem::UNPC_EmergentBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default values
    EmergentBehaviorRadius = 1500.0f;
    SocialInfluenceStrength = 1.0f;
    AdaptationRate = 0.5f;
    MemoryDecayRate = 0.01f;
    
    // Initialize behavior state
    CurrentEmergentState = ENPC_EmergentState::Neutral;
    LastStateChangeTime = 0.0f;
    StateStabilityThreshold = 5.0f;
    
    // Initialize social metrics
    SocialCohesion = 0.5f;
    GroupTension = 0.0f;
    CollectiveStress = 0.0f;
    
    bIsInitialized = false;
}

void UNPC_EmergentBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEmergentBehavior();
    
    // Start emergent behavior update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            EmergentUpdateTimer,
            this,
            &UNPC_EmergentBehaviorSystem::UpdateEmergentBehavior,
            1.0f,
            true
        );
    }
}

void UNPC_EmergentBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized)
        return;
        
    // Update behavior patterns based on environmental factors
    UpdateBehaviorPatterns(DeltaTime);
    
    // Process social influences
    ProcessSocialInfluences(DeltaTime);
    
    // Update memory decay
    DecayBehaviorMemory(DeltaTime);
}

void UNPC_EmergentBehaviorSystem::InitializeEmergentBehavior()
{
    if (!GetOwner())
        return;
        
    // Initialize behavior patterns
    BehaviorPatterns.Empty();
    
    // Add basic behavior patterns
    FNPC_BehaviorPattern ExplorationPattern;
    ExplorationPattern.PatternName = TEXT("Exploration");
    ExplorationPattern.Priority = 0.3f;
    ExplorationPattern.Duration = 300.0f;
    ExplorationPattern.Conditions.Add(TEXT("Low_Stress"));
    BehaviorPatterns.Add(ExplorationPattern);
    
    FNPC_BehaviorPattern SocialPattern;
    SocialPattern.PatternName = TEXT("Social_Interaction");
    SocialPattern.Priority = 0.5f;
    SocialPattern.Duration = 180.0f;
    SocialPattern.Conditions.Add(TEXT("Near_Others"));
    BehaviorPatterns.Add(SocialPattern);
    
    FNPC_BehaviorPattern DefensivePattern;
    DefensivePattern.PatternName = TEXT("Defensive");
    DefensivePattern.Priority = 0.8f;
    DefensivePattern.Duration = 120.0f;
    DefensivePattern.Conditions.Add(TEXT("High_Threat"));
    BehaviorPatterns.Add(DefensivePattern);
    
    // Initialize social connections
    SocialConnections.Empty();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("EmergentBehaviorSystem initialized for %s"), *GetOwner()->GetName());
}

void UNPC_EmergentBehaviorSystem::UpdateEmergentBehavior()
{
    if (!GetOwner() || !GetWorld())
        return;
        
    // Analyze current situation
    AnalyzeEnvironmentalContext();
    
    // Update social dynamics
    UpdateSocialDynamics();
    
    // Determine emergent state
    DetermineEmergentState();
    
    // Adapt behavior based on emergent state
    AdaptBehaviorToState();
}

void UNPC_EmergentBehaviorSystem::UpdateBehaviorPatterns(float DeltaTime)
{
    // Update pattern priorities based on current conditions
    for (FNPC_BehaviorPattern& Pattern : BehaviorPatterns)
    {
        float NewPriority = CalculatePatternPriority(Pattern);
        Pattern.Priority = FMath::Lerp(Pattern.Priority, NewPriority, AdaptationRate * DeltaTime);
        
        // Update pattern duration based on success
        if (Pattern.PatternName == CurrentActiveBehavior)
        {
            Pattern.Duration += DeltaTime;
        }
    }
}

void UNPC_EmergentBehaviorSystem::ProcessSocialInfluences(float DeltaTime)
{
    if (!GetWorld())
        return;
        
    // Find nearby NPCs with emergent behavior systems
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
    
    float TotalInfluence = 0.0f;
    int32 InfluencingActors = 0;
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == GetOwner())
            continue;
            
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Distance > EmergentBehaviorRadius)
            continue;
            
        // Check if actor has emergent behavior system
        UNPC_EmergentBehaviorSystem* OtherSystem = Actor->FindComponentByClass<UNPC_EmergentBehaviorSystem>();
        if (!OtherSystem)
            continue;
            
        // Calculate influence based on distance and state
        float InfluenceStrength = 1.0f - (Distance / EmergentBehaviorRadius);
        InfluenceStrength *= SocialInfluenceStrength;
        
        // Apply state-based influence
        if (OtherSystem->CurrentEmergentState == ENPC_EmergentState::Aggressive)
        {
            GroupTension += InfluenceStrength * DeltaTime;
        }
        else if (OtherSystem->CurrentEmergentState == ENPC_EmergentState::Cooperative)
        {
            SocialCohesion += InfluenceStrength * DeltaTime;
        }
        
        TotalInfluence += InfluenceStrength;
        InfluencingActors++;
    }
    
    // Normalize social metrics
    SocialCohesion = FMath::Clamp(SocialCohesion, 0.0f, 1.0f);
    GroupTension = FMath::Clamp(GroupTension, 0.0f, 1.0f);
    CollectiveStress = (GroupTension + (1.0f - SocialCohesion)) * 0.5f;
}

void UNPC_EmergentBehaviorSystem::DecayBehaviorMemory(float DeltaTime)
{
    // Decay behavior memories over time
    for (auto& Memory : BehaviorMemories)
    {
        Memory.Value *= (1.0f - MemoryDecayRate * DeltaTime);
        
        // Remove very weak memories
        if (Memory.Value < 0.1f)
        {
            Memory.Value = 0.0f;
        }
    }
    
    // Clean up empty memories
    BehaviorMemories.RemoveAll([](const TPair<FString, float>& Memory) {
        return Memory.Value <= 0.0f;
    });
}

float UNPC_EmergentBehaviorSystem::CalculatePatternPriority(const FNPC_BehaviorPattern& Pattern)
{
    float Priority = 0.5f; // Base priority
    
    // Adjust based on environmental conditions
    if (Pattern.PatternName == TEXT("Defensive") && CollectiveStress > 0.7f)
    {
        Priority += 0.4f;
    }
    else if (Pattern.PatternName == TEXT("Social_Interaction") && SocialCohesion > 0.6f)
    {
        Priority += 0.3f;
    }
    else if (Pattern.PatternName == TEXT("Exploration") && CollectiveStress < 0.3f)
    {
        Priority += 0.2f;
    }
    
    // Adjust based on behavior memory
    if (BehaviorMemories.Contains(Pattern.PatternName))
    {
        float MemoryStrength = BehaviorMemories[Pattern.PatternName];
        Priority += MemoryStrength * 0.2f;
    }
    
    return FMath::Clamp(Priority, 0.0f, 1.0f);
}

void UNPC_EmergentBehaviorSystem::AnalyzeEnvironmentalContext()
{
    if (!GetWorld())
        return;
        
    // Analyze threat level
    float ThreatLevel = 0.0f;
    
    // Check for predators or dangerous entities nearby
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == GetOwner())
            continue;
            
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
        if (Distance < EmergentBehaviorRadius)
        {
            // Simple threat assessment based on size and proximity
            float ThreatContribution = (EmergentBehaviorRadius - Distance) / EmergentBehaviorRadius;
            ThreatLevel += ThreatContribution * 0.1f;
        }
    }
    
    // Store environmental context
    EnvironmentalContext.ThreatLevel = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
    EnvironmentalContext.ResourceAvailability = 0.7f; // Placeholder
    EnvironmentalContext.SocialDensity = FMath::Clamp(static_cast<float>(NearbyActors.Num()) / 10.0f, 0.0f, 1.0f);
}

void UNPC_EmergentBehaviorSystem::UpdateSocialDynamics()
{
    // Update social connections based on recent interactions
    for (auto& Connection : SocialConnections)
    {
        // Decay connection strength over time
        Connection.Value *= 0.99f;
        
        // Remove weak connections
        if (Connection.Value < 0.1f)
        {
            Connection.Value = 0.0f;
        }
    }
    
    // Clean up empty connections
    SocialConnections.RemoveAll([](const TPair<FString, float>& Connection) {
        return Connection.Value <= 0.0f;
    });
}

void UNPC_EmergentBehaviorSystem::DetermineEmergentState()
{
    ENPC_EmergentState NewState = CurrentEmergentState;
    
    // Determine state based on environmental and social factors
    if (EnvironmentalContext.ThreatLevel > 0.7f || GroupTension > 0.8f)
    {
        NewState = ENPC_EmergentState::Aggressive;
    }
    else if (SocialCohesion > 0.7f && EnvironmentalContext.ThreatLevel < 0.3f)
    {
        NewState = ENPC_EmergentState::Cooperative;
    }
    else if (CollectiveStress > 0.6f)
    {
        NewState = ENPC_EmergentState::Defensive;
    }
    else if (EnvironmentalContext.SocialDensity < 0.2f)
    {
        NewState = ENPC_EmergentState::Exploratory;
    }
    else
    {
        NewState = ENPC_EmergentState::Neutral;
    }
    
    // Only change state if it's been stable for a minimum time
    if (NewState != CurrentEmergentState)
    {
        float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        if (CurrentTime - LastStateChangeTime > StateStabilityThreshold)
        {
            CurrentEmergentState = NewState;
            LastStateChangeTime = CurrentTime;
            
            OnEmergentStateChanged();
        }
    }
}

void UNPC_EmergentBehaviorSystem::AdaptBehaviorToState()
{
    // Adapt behavior patterns based on current emergent state
    switch (CurrentEmergentState)
    {
        case ENPC_EmergentState::Aggressive:
            CurrentActiveBehavior = TEXT("Aggressive_Territorial");
            break;
            
        case ENPC_EmergentState::Cooperative:
            CurrentActiveBehavior = TEXT("Social_Interaction");
            break;
            
        case ENPC_EmergentState::Defensive:
            CurrentActiveBehavior = TEXT("Defensive");
            break;
            
        case ENPC_EmergentState::Exploratory:
            CurrentActiveBehavior = TEXT("Exploration");
            break;
            
        default:
            CurrentActiveBehavior = TEXT("Neutral_Patrol");
            break;
    }
    
    // Store behavior memory
    if (!CurrentActiveBehavior.IsEmpty())
    {
        if (BehaviorMemories.Contains(CurrentActiveBehavior))
        {
            BehaviorMemories[CurrentActiveBehavior] += 0.1f;
        }
        else
        {
            BehaviorMemories.Add(CurrentActiveBehavior, 0.5f);
        }
    }
}

void UNPC_EmergentBehaviorSystem::OnEmergentStateChanged()
{
    UE_LOG(LogTemp, Log, TEXT("%s emergent state changed to: %d"), 
           *GetOwner()->GetName(), 
           static_cast<int32>(CurrentEmergentState));
           
    // Broadcast state change to other systems
    if (OnStateChanged.IsBound())
    {
        OnStateChanged.Broadcast(CurrentEmergentState);
    }
}

void UNPC_EmergentBehaviorSystem::AddSocialConnection(const FString& ActorName, float Strength)
{
    SocialConnections.Add(ActorName, FMath::Clamp(Strength, 0.0f, 1.0f));
}

void UNPC_EmergentBehaviorSystem::RemoveSocialConnection(const FString& ActorName)
{
    SocialConnections.Remove(ActorName);
}

float UNPC_EmergentBehaviorSystem::GetSocialConnectionStrength(const FString& ActorName) const
{
    if (SocialConnections.Contains(ActorName))
    {
        return SocialConnections[ActorName];
    }
    return 0.0f;
}

void UNPC_EmergentBehaviorSystem::AddBehaviorMemory(const FString& BehaviorName, float Strength)
{
    BehaviorMemories.Add(BehaviorName, FMath::Clamp(Strength, 0.0f, 1.0f));
}

ENPC_EmergentState UNPC_EmergentBehaviorSystem::GetCurrentEmergentState() const
{
    return CurrentEmergentState;
}

float UNPC_EmergentBehaviorSystem::GetSocialCohesion() const
{
    return SocialCohesion;
}

float UNPC_EmergentBehaviorSystem::GetGroupTension() const
{
    return GroupTension;
}

float UNPC_EmergentBehaviorSystem::GetCollectiveStress() const
{
    return CollectiveStress;
}