#include "NPCBehaviorAgent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogNPCBehavior);

UNPCBehaviorAgent::UNPCBehaviorAgent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default values
    BehaviorUpdateInterval = 0.5f;
    MemoryUpdateInterval = 1.0f;
    RoutineUpdateInterval = 5.0f;
    
    // Initialize behavior weights
    BehaviorWeights.Add(ENPCBehaviorType::Idle, 1.0f);
    BehaviorWeights.Add(ENPCBehaviorType::Patrol, 0.8f);
    BehaviorWeights.Add(ENPCBehaviorType::Investigate, 0.6f);
    BehaviorWeights.Add(ENPCBehaviorType::Socialize, 0.4f);
    BehaviorWeights.Add(ENPCBehaviorType::Work, 0.7f);
    BehaviorWeights.Add(ENPCBehaviorType::Rest, 0.5f);
    BehaviorWeights.Add(ENPCBehaviorType::Flee, 2.0f);
    BehaviorWeights.Add(ENPCBehaviorType::Follow, 0.9f);
    BehaviorWeights.Add(ENPCBehaviorType::Hunt, 1.5f);
    BehaviorWeights.Add(ENPCBehaviorType::Forage, 0.6f);
}

void UNPCBehaviorAgent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the NPC behavior system
    InitializeBehaviorSystem();
    
    // Set up timers for periodic updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            BehaviorUpdateTimer,
            this,
            &UNPCBehaviorAgent::UpdateBehaviorDecision,
            BehaviorUpdateInterval,
            true
        );
        
        World->GetTimerManager().SetTimer(
            MemoryUpdateTimer,
            this,
            &UNPCBehaviorAgent::UpdateMemorySystem,
            MemoryUpdateInterval,
            true
        );
        
        World->GetTimerManager().SetTimer(
            RoutineUpdateTimer,
            this,
            &UNPCBehaviorAgent::UpdateDailyRoutine,
            RoutineUpdateInterval,
            true
        );
    }
    
    UE_LOG(LogNPCBehavior, Log, TEXT("NPCBehaviorAgent initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPCBehaviorAgent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update needs and emotional state
    UpdateNeeds(DeltaTime);
    UpdateEmotionalState(DeltaTime);
    
    // Update behavior tree blackboard with current state
    UpdateBlackboardValues();
}

void UNPCBehaviorAgent::InitializeBehaviorSystem()
{
    // Initialize personality with some variation
    if (Personality.Traits.Num() == 0)
    {
        GenerateRandomPersonality();
    }
    
    // Initialize memory system
    Memory.KnownLocations.Empty();
    Memory.KnownActors.Empty();
    Memory.RecentEvents.Empty();
    
    // Initialize needs
    InitializeNeeds();
    
    // Initialize daily routine
    if (DailyRoutine.ScheduledActivities.Num() == 0)
    {
        GenerateDefaultDailyRoutine();
    }
    
    // Set initial behavior state
    CurrentBehaviorType = ENPCBehaviorType::Idle;
    BehaviorStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UNPCBehaviorAgent::GenerateRandomPersonality()
{
    Personality.Traits.Empty();
    
    // Generate core personality traits
    Personality.Traits.Add(TEXT("Curiosity"), FMath::RandRange(0.1f, 0.9f));
    Personality.Traits.Add(TEXT("Aggressiveness"), FMath::RandRange(0.0f, 0.8f));
    Personality.Traits.Add(TEXT("Sociability"), FMath::RandRange(0.2f, 1.0f));
    Personality.Traits.Add(TEXT("Fearfulness"), FMath::RandRange(0.1f, 0.7f));
    Personality.Traits.Add(TEXT("Intelligence"), FMath::RandRange(0.3f, 1.0f));
    Personality.Traits.Add(TEXT("Loyalty"), FMath::RandRange(0.2f, 0.9f));
    Personality.Traits.Add(TEXT("Territoriality"), FMath::RandRange(0.0f, 0.8f));
    
    // Set archetype based on dominant traits
    float Aggressiveness = Personality.Traits.FindRef(TEXT("Aggressiveness"));
    float Sociability = Personality.Traits.FindRef(TEXT("Sociability"));
    float Fearfulness = Personality.Traits.FindRef(TEXT("Fearfulness"));
    
    if (Aggressiveness > 0.6f)
    {
        Personality.Archetype = ENPCArchetype::Aggressive;
    }
    else if (Fearfulness > 0.6f)
    {
        Personality.Archetype = ENPCArchetype::Timid;
    }
    else if (Sociability > 0.7f)
    {
        Personality.Archetype = ENPCArchetype::Social;
    }
    else
    {
        Personality.Archetype = ENPCArchetype::Neutral;
    }
    
    UE_LOG(LogNPCBehavior, Log, TEXT("Generated personality for %s: Archetype=%d, Aggressiveness=%.2f, Sociability=%.2f"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
           (int32)Personality.Archetype, Aggressiveness, Sociability);
}

void UNPCBehaviorAgent::InitializeNeeds()
{
    Needs.PhysicalNeeds.Empty();
    Needs.SocialNeeds.Empty();
    Needs.EmotionalNeeds.Empty();
    
    // Initialize physical needs
    Needs.PhysicalNeeds.Add(TEXT("Hunger"), FMath::RandRange(0.2f, 0.6f));
    Needs.PhysicalNeeds.Add(TEXT("Thirst"), FMath::RandRange(0.1f, 0.4f));
    Needs.PhysicalNeeds.Add(TEXT("Rest"), FMath::RandRange(0.0f, 0.3f));
    Needs.PhysicalNeeds.Add(TEXT("Safety"), FMath::RandRange(0.3f, 0.7f));
    
    // Initialize social needs
    Needs.SocialNeeds.Add(TEXT("Companionship"), FMath::RandRange(0.2f, 0.8f));
    Needs.SocialNeeds.Add(TEXT("Territory"), FMath::RandRange(0.1f, 0.6f));
    Needs.SocialNeeds.Add(TEXT("Dominance"), FMath::RandRange(0.0f, 0.5f));
    
    // Initialize emotional needs
    Needs.EmotionalNeeds.Add(TEXT("Stimulation"), FMath::RandRange(0.3f, 0.7f));
    Needs.EmotionalNeeds.Add(TEXT("Comfort"), FMath::RandRange(0.2f, 0.6f));
    Needs.EmotionalNeeds.Add(TEXT("Purpose"), FMath::RandRange(0.4f, 0.8f));
}

void UNPCBehaviorAgent::GenerateDefaultDailyRoutine()
{
    DailyRoutine.ScheduledActivities.Empty();
    
    // Morning routine (6:00 - 12:00)
    FNPCScheduledActivity MorningActivity;
    MorningActivity.ActivityType = ENPCBehaviorType::Forage;
    MorningActivity.StartTime = 6.0f;
    MorningActivity.Duration = 4.0f;
    MorningActivity.Priority = 0.8f;
    MorningActivity.Location = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    DailyRoutine.ScheduledActivities.Add(MorningActivity);
    
    // Afternoon routine (12:00 - 18:00)
    FNPCScheduledActivity AfternoonActivity;
    AfternoonActivity.ActivityType = ENPCBehaviorType::Socialize;
    AfternoonActivity.StartTime = 12.0f;
    AfternoonActivity.Duration = 3.0f;
    AfternoonActivity.Priority = 0.6f;
    AfternoonActivity.Location = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    DailyRoutine.ScheduledActivities.Add(AfternoonActivity);
    
    // Evening routine (18:00 - 22:00)
    FNPCScheduledActivity EveningActivity;
    EveningActivity.ActivityType = ENPCBehaviorType::Patrol;
    EveningActivity.StartTime = 18.0f;
    EveningActivity.Duration = 2.0f;
    EveningActivity.Priority = 0.7f;
    EveningActivity.Location = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    DailyRoutine.ScheduledActivities.Add(EveningActivity);
    
    // Night routine (22:00 - 6:00)
    FNPCScheduledActivity NightActivity;
    NightActivity.ActivityType = ENPCBehaviorType::Rest;
    NightActivity.StartTime = 22.0f;
    NightActivity.Duration = 8.0f;
    NightActivity.Priority = 0.9f;
    NightActivity.Location = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    DailyRoutine.ScheduledActivities.Add(NightActivity);
}

void UNPCBehaviorAgent::UpdateBehaviorDecision()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Calculate behavior scores based on needs, personality, and context
    TMap<ENPCBehaviorType, float> BehaviorScores;
    
    for (const auto& BehaviorWeight : BehaviorWeights)
    {
        float Score = CalculateBehaviorScore(BehaviorWeight.Key);
        BehaviorScores.Add(BehaviorWeight.Key, Score);
    }
    
    // Find the highest scoring behavior
    ENPCBehaviorType BestBehavior = ENPCBehaviorType::Idle;
    float HighestScore = 0.0f;
    
    for (const auto& BehaviorScore : BehaviorScores)
    {
        if (BehaviorScore.Value > HighestScore)
        {
            HighestScore = BehaviorScore.Value;
            BestBehavior = BehaviorScore.Key;
        }
    }
    
    // Change behavior if needed
    if (BestBehavior != CurrentBehaviorType)
    {
        SetBehaviorType(BestBehavior);
    }
}

float UNPCBehaviorAgent::CalculateBehaviorScore(ENPCBehaviorType BehaviorType)
{
    float BaseScore = BehaviorWeights.FindRef(BehaviorType);
    
    // Modify score based on needs
    float NeedModifier = 1.0f;
    switch (BehaviorType)
    {
        case ENPCBehaviorType::Rest:
            NeedModifier = Needs.PhysicalNeeds.FindRef(TEXT("Rest")) * 2.0f;
            break;
        case ENPCBehaviorType::Forage:
            NeedModifier = (Needs.PhysicalNeeds.FindRef(TEXT("Hunger")) + 
                          Needs.PhysicalNeeds.FindRef(TEXT("Thirst"))) * 1.5f;
            break;
        case ENPCBehaviorType::Socialize:
            NeedModifier = Needs.SocialNeeds.FindRef(TEXT("Companionship")) * 1.5f;
            break;
        case ENPCBehaviorType::Flee:
            NeedModifier = (1.0f - Needs.PhysicalNeeds.FindRef(TEXT("Safety"))) * 3.0f;
            break;
        default:
            break;
    }
    
    // Modify score based on personality
    float PersonalityModifier = 1.0f;
    switch (BehaviorType)
    {
        case ENPCBehaviorType::Investigate:
            PersonalityModifier = Personality.Traits.FindRef(TEXT("Curiosity"));
            break;
        case ENPCBehaviorType::Hunt:
            PersonalityModifier = Personality.Traits.FindRef(TEXT("Aggressiveness"));
            break;
        case ENPCBehaviorType::Socialize:
            PersonalityModifier = Personality.Traits.FindRef(TEXT("Sociability"));
            break;
        case ENPCBehaviorType::Flee:
            PersonalityModifier = Personality.Traits.FindRef(TEXT("Fearfulness"));
            break;
        default:
            break;
    }
    
    return BaseScore * NeedModifier * PersonalityModifier;
}

void UNPCBehaviorAgent::UpdateNeeds(float DeltaTime)
{
    // Update physical needs
    for (auto& Need : Needs.PhysicalNeeds)
    {
        if (Need.Key == TEXT("Hunger"))
        {
            Need.Value = FMath::Clamp(Need.Value + (DeltaTime * 0.01f), 0.0f, 1.0f);
        }
        else if (Need.Key == TEXT("Thirst"))
        {
            Need.Value = FMath::Clamp(Need.Value + (DeltaTime * 0.015f), 0.0f, 1.0f);
        }
        else if (Need.Key == TEXT("Rest"))
        {
            if (CurrentBehaviorType == ENPCBehaviorType::Rest)
            {
                Need.Value = FMath::Clamp(Need.Value - (DeltaTime * 0.02f), 0.0f, 1.0f);
            }
            else
            {
                Need.Value = FMath::Clamp(Need.Value + (DeltaTime * 0.005f), 0.0f, 1.0f);
            }
        }
    }
    
    // Update social needs
    for (auto& Need : Needs.SocialNeeds)
    {
        if (Need.Key == TEXT("Companionship"))
        {
            if (CurrentBehaviorType == ENPCBehaviorType::Socialize)
            {
                Need.Value = FMath::Clamp(Need.Value - (DeltaTime * 0.01f), 0.0f, 1.0f);
            }
            else
            {
                Need.Value = FMath::Clamp(Need.Value + (DeltaTime * 0.003f), 0.0f, 1.0f);
            }
        }
    }
}

void UNPCBehaviorAgent::UpdateEmotionalState(float DeltaTime)
{
    // Update emotional state based on current situation
    float StressLevel = 0.0f;
    
    // Calculate stress from unmet needs
    for (const auto& Need : Needs.PhysicalNeeds)
    {
        StressLevel += Need.Value * 0.3f;
    }
    
    for (const auto& Need : Needs.SocialNeeds)
    {
        StressLevel += Need.Value * 0.2f;
    }
    
    // Update emotional state
    EmotionalState.Stress = FMath::Clamp(StressLevel, 0.0f, 1.0f);
    EmotionalState.Happiness = FMath::Clamp(1.0f - StressLevel * 0.8f, 0.0f, 1.0f);
    
    // Emotional recovery over time
    EmotionalState.Stress = FMath::Clamp(EmotionalState.Stress - (DeltaTime * 0.01f), 0.0f, 1.0f);
    EmotionalState.Happiness = FMath::Clamp(EmotionalState.Happiness + (DeltaTime * 0.005f), 0.0f, 1.0f);
}

void UNPCBehaviorAgent::UpdateMemorySystem()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Update memory of nearby actors
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && Actor != GetOwner())
        {
            float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
            if (Distance <= MemoryRange)
            {
                // Update or add to memory
                FNPCMemoryEntry* ExistingEntry = Memory.KnownActors.Find(Actor);
                if (ExistingEntry)
                {
                    ExistingEntry->LastSeenTime = GetWorld()->GetTimeSeconds();
                    ExistingEntry->LastKnownLocation = Actor->GetActorLocation();
                    ExistingEntry->Familiarity = FMath::Clamp(ExistingEntry->Familiarity + 0.1f, 0.0f, 1.0f);
                }
                else
                {
                    FNPCMemoryEntry NewEntry;
                    NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
                    NewEntry.LastKnownLocation = Actor->GetActorLocation();
                    NewEntry.ThreatLevel = CalculateThreatLevel(Actor);
                    NewEntry.Familiarity = 0.1f;
                    Memory.KnownActors.Add(Actor, NewEntry);
                }
            }
        }
    }
    
    // Clean up old memories
    CleanupOldMemories();
}

void UNPCBehaviorAgent::UpdateDailyRoutine()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetGameTimeOfDay();
    
    // Find current scheduled activity
    FNPCScheduledActivity* CurrentActivity = nullptr;
    for (FNPCScheduledActivity& Activity : DailyRoutine.ScheduledActivities)
    {
        float EndTime = Activity.StartTime + Activity.Duration;
        if (EndTime > 24.0f) EndTime -= 24.0f; // Handle day wrap
        
        if (IsTimeInRange(CurrentTime, Activity.StartTime, EndTime))
        {
            CurrentActivity = &Activity;
            break;
        }
    }
    
    // Update routine-based behavior
    if (CurrentActivity && CurrentActivity->ActivityType != CurrentBehaviorType)
    {
        // Check if we should override current behavior for routine
        float RoutineScore = CurrentActivity->Priority * 1.2f; // Boost routine activities
        float CurrentScore = CalculateBehaviorScore(CurrentBehaviorType);
        
        if (RoutineScore > CurrentScore)
        {
            SetBehaviorType(CurrentActivity->ActivityType);
        }
    }
}

void UNPCBehaviorAgent::SetBehaviorType(ENPCBehaviorType NewBehaviorType)
{
    if (CurrentBehaviorType != NewBehaviorType)
    {
        ENPCBehaviorType OldBehavior = CurrentBehaviorType;
        CurrentBehaviorType = NewBehaviorType;
        BehaviorStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        // Notify behavior change
        OnBehaviorChanged.Broadcast(OldBehavior, NewBehaviorType);
        
        UE_LOG(LogNPCBehavior, Log, TEXT("%s changed behavior from %d to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               (int32)OldBehavior, (int32)NewBehaviorType);
    }
}

void UNPCBehaviorAgent::UpdateBlackboardValues()
{
    // Update blackboard with current behavior state
    if (AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController()))
    {
        if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
        {
            // Update behavior type
            BlackboardComp->SetValueAsEnum(TEXT("CurrentBehaviorType"), (uint8)CurrentBehaviorType);
            
            // Update needs
            BlackboardComp->SetValueAsFloat(TEXT("HungerLevel"), Needs.PhysicalNeeds.FindRef(TEXT("Hunger")));
            BlackboardComp->SetValueAsFloat(TEXT("ThirstLevel"), Needs.PhysicalNeeds.FindRef(TEXT("Thirst")));
            BlackboardComp->SetValueAsFloat(TEXT("RestLevel"), Needs.PhysicalNeeds.FindRef(TEXT("Rest")));
            
            // Update emotional state
            BlackboardComp->SetValueAsFloat(TEXT("StressLevel"), EmotionalState.Stress);
            BlackboardComp->SetValueAsFloat(TEXT("HappinessLevel"), EmotionalState.Happiness);
        }
    }
}

float UNPCBehaviorAgent::CalculateThreatLevel(AActor* Actor)
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    // Basic threat calculation based on actor type and distance
    float ThreatLevel = 0.0f;
    
    if (Actor->IsA<APawn>())
    {
        // Check if it's a player
        if (Actor->IsA<ACharacter>())
        {
            ThreatLevel = 0.3f; // Players are moderately threatening by default
        }
        else
        {
            ThreatLevel = 0.2f; // Other pawns are less threatening
        }
    }
    
    // Modify based on personality
    ThreatLevel *= (1.0f + Personality.Traits.FindRef(TEXT("Fearfulness")));
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

void UNPCBehaviorAgent::CleanupOldMemories()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<AActor*> ActorsToRemove;
    
    for (auto& MemoryPair : Memory.KnownActors)
    {
        if (CurrentTime - MemoryPair.Value.LastSeenTime > MemoryRetentionTime)
        {
            ActorsToRemove.Add(MemoryPair.Key);
        }
    }
    
    for (AActor* Actor : ActorsToRemove)
    {
        Memory.KnownActors.Remove(Actor);
    }
}

float UNPCBehaviorAgent::GetGameTimeOfDay()
{
    // Simple time of day calculation (0-24 hours)
    if (UWorld* World = GetWorld())
    {
        float GameTime = World->GetTimeSeconds();
        return FMath::Fmod(GameTime / 3600.0f, 24.0f); // Convert to hours and wrap at 24
    }
    return 12.0f; // Default to noon
}

bool UNPCBehaviorAgent::IsTimeInRange(float CurrentTime, float StartTime, float EndTime)
{
    if (StartTime <= EndTime)
    {
        return CurrentTime >= StartTime && CurrentTime <= EndTime;
    }
    else
    {
        // Handle day wrap (e.g., 22:00 to 6:00)
        return CurrentTime >= StartTime || CurrentTime <= EndTime;
    }
}

void UNPCBehaviorAgent::RememberLocation(FVector Location, ENPCLocationMemoryType LocationType, float Importance)
{
    FNPCLocationMemory LocationMemory;
    LocationMemory.Location = Location;
    LocationMemory.LocationType = LocationType;
    LocationMemory.Importance = Importance;
    LocationMemory.LastVisited = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    Memory.KnownLocations.Add(LocationMemory);
    
    // Limit memory size
    if (Memory.KnownLocations.Num() > MaxLocationMemories)
    {
        Memory.KnownLocations.RemoveAt(0);
    }
}

void UNPCBehaviorAgent::ForgetActor(AActor* Actor)
{
    if (Actor)
    {
        Memory.KnownActors.Remove(Actor);
    }
}

FVector UNPCBehaviorAgent::GetNearestKnownLocation(ENPCLocationMemoryType LocationType)
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    FVector NearestLocation = FVector::ZeroVector;
    float NearestDistance = FLT_MAX;
    
    for (const FNPCLocationMemory& LocationMemory : Memory.KnownLocations)
    {
        if (LocationMemory.LocationType == LocationType)
        {
            float Distance = FVector::Dist(MyLocation, LocationMemory.Location);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestLocation = LocationMemory.Location;
            }
        }
    }
    
    return NearestLocation;
}

TArray<AActor*> UNPCBehaviorAgent::GetKnownActorsByThreatLevel(float MinThreatLevel, float MaxThreatLevel)
{
    TArray<AActor*> FilteredActors;
    
    for (const auto& MemoryPair : Memory.KnownActors)
    {
        if (MemoryPair.Value.ThreatLevel >= MinThreatLevel && MemoryPair.Value.ThreatLevel <= MaxThreatLevel)
        {
            FilteredActors.Add(MemoryPair.Key);
        }
    }
    
    return FilteredActors;
}