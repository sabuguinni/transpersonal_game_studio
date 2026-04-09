#include "EmotionalQuestDesigner.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogEmotionalQuest);

UEmotionalQuestDesigner::UEmotionalQuestDesigner()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize emotional states
    CurrentEmotionalState = EEmotionalState::Neutral;
    EmotionalIntensity = 0.5f;
    EmotionalMomentum = 0.0f;
    
    // Initialize quest emotional parameters
    QuestEmotionalWeight = 1.0f;
    EmotionalTransitionSpeed = 0.1f;
    MaxEmotionalIntensity = 1.0f;
    MinEmotionalIntensity = 0.1f;
    
    // Initialize audio components
    EmotionalAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EmotionalAudioComponent"));
    QuestMusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("QuestMusicComponent"));
    
    // Set default values
    bAutoActivate = true;
    bTickInEditor = false;
}

void UEmotionalQuestDesigner::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogEmotionalQuest, Log, TEXT("Emotional Quest Designer V43_001 - Initializing"));
    
    // Initialize emotional state tracking
    InitializeEmotionalStates();
    
    // Setup quest emotional triggers
    SetupQuestEmotionalTriggers();
    
    // Start emotional monitoring
    StartEmotionalMonitoring();
    
    UE_LOG(LogEmotionalQuest, Log, TEXT("Emotional Quest Designer - Ready for prehistoric storytelling"));
}

void UEmotionalQuestDesigner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update emotional state based on player actions and environment
    UpdateEmotionalState(DeltaTime);
    
    // Process emotional quest generation
    ProcessEmotionalQuestGeneration(DeltaTime);
    
    // Update quest emotional atmosphere
    UpdateQuestEmotionalAtmosphere(DeltaTime);
}

void UEmotionalQuestDesigner::InitializeEmotionalStates()
{
    // Define emotional state transitions for prehistoric survival
    EmotionalStateMap.Empty();
    
    // Wonder - discovering the prehistoric world
    FEmotionalStateData WonderState;
    WonderState.StateName = "Wonder";
    WonderState.BaseIntensity = 0.7f;
    WonderState.TransitionSpeed = 0.15f;
    WonderState.QuestInfluence = 0.8f;
    EmotionalStateMap.Add(EEmotionalState::Wonder, WonderState);
    
    // Fear - encountering dangerous predators
    FEmotionalStateData FearState;
    FearState.StateName = "Fear";
    FearState.BaseIntensity = 0.9f;
    FearState.TransitionSpeed = 0.3f;
    FearState.QuestInfluence = 1.0f;
    EmotionalStateMap.Add(EEmotionalState::Fear, FearState);
    
    // Discovery - finding important items or locations
    FEmotionalStateData DiscoveryState;
    DiscoveryState.StateName = "Discovery";
    DiscoveryState.BaseIntensity = 0.8f;
    DiscoveryState.TransitionSpeed = 0.2f;
    DiscoveryState.QuestInfluence = 0.9f;
    EmotionalStateMap.Add(EEmotionalState::Discovery, DiscoveryState);
    
    // Triumph - completing challenging objectives
    FEmotionalStateData TriumphState;
    TriumphState.StateName = "Triumph";
    TriumphState.BaseIntensity = 0.85f;
    TriumphState.TransitionSpeed = 0.25f;
    TriumphState.QuestInfluence = 0.7f;
    EmotionalStateMap.Add(EEmotionalState::Triumph, TriumphState);
    
    // Reflection - contemplating experiences
    FEmotionalStateData ReflectionState;
    ReflectionState.StateName = "Reflection";
    ReflectionState.BaseIntensity = 0.6f;
    ReflectionState.TransitionSpeed = 0.1f;
    ReflectionState.QuestInfluence = 0.6f;
    EmotionalStateMap.Add(EEmotionalState::Reflection, ReflectionState);
    
    UE_LOG(LogEmotionalQuest, Log, TEXT("Emotional states initialized: %d states"), EmotionalStateMap.Num());
}

void UEmotionalQuestDesigner::SetupQuestEmotionalTriggers()
{
    // Setup emotional triggers for different quest scenarios
    EmotionalTriggers.Empty();
    
    // Dinosaur encounter triggers
    FQuestEmotionalTrigger DinoEncounter;
    DinoEncounter.TriggerName = "DinosaurEncounter";
    DinoEncounter.EmotionalState = EEmotionalState::Fear;
    DinoEncounter.IntensityModifier = 0.8f;
    DinoEncounter.Duration = 10.0f;
    EmotionalTriggers.Add(DinoEncounter);
    
    // Discovery triggers
    FQuestEmotionalTrigger ItemDiscovery;
    ItemDiscovery.TriggerName = "ImportantItemFound";
    ItemDiscovery.EmotionalState = EEmotionalState::Discovery;
    ItemDiscovery.IntensityModifier = 0.7f;
    ItemDiscovery.Duration = 8.0f;
    EmotionalTriggers.Add(ItemDiscovery);
    
    // Wonder triggers
    FQuestEmotionalTrigger NewLocation;
    NewLocation.TriggerName = "NewLocationDiscovered";
    NewLocation.EmotionalState = EEmotionalState::Wonder;
    NewLocation.IntensityModifier = 0.6f;
    NewLocation.Duration = 15.0f;
    EmotionalTriggers.Add(NewLocation);
    
    UE_LOG(LogEmotionalQuest, Log, TEXT("Quest emotional triggers configured: %d triggers"), EmotionalTriggers.Num());
}

void UEmotionalQuestDesigner::StartEmotionalMonitoring()
{
    // Begin monitoring player emotional state for quest generation
    bEmotionalMonitoringActive = true;
    EmotionalMonitoringTimer = 0.0f;
    
    // Initialize quest emotional history
    QuestEmotionalHistory.Empty();
    
    UE_LOG(LogEmotionalQuest, Log, TEXT("Emotional monitoring started for quest generation"));
}

void UEmotionalQuestDesigner::UpdateEmotionalState(float DeltaTime)
{
    if (!bEmotionalMonitoringActive) return;
    
    EmotionalMonitoringTimer += DeltaTime;
    
    // Update emotional intensity based on current state
    if (EmotionalStateMap.Contains(CurrentEmotionalState))
    {
        const FEmotionalStateData& StateData = EmotionalStateMap[CurrentEmotionalState];
        
        // Calculate target intensity
        float TargetIntensity = StateData.BaseIntensity;
        
        // Apply momentum and transitions
        float IntensityDelta = (TargetIntensity - EmotionalIntensity) * StateData.TransitionSpeed * DeltaTime;
        EmotionalIntensity = FMath::Clamp(EmotionalIntensity + IntensityDelta, MinEmotionalIntensity, MaxEmotionalIntensity);
        
        // Update momentum
        EmotionalMomentum = FMath::Lerp(EmotionalMomentum, IntensityDelta, 0.1f);
    }
    
    // Record emotional state for quest generation
    if (EmotionalMonitoringTimer >= 2.0f)
    {
        RecordEmotionalMoment();
        EmotionalMonitoringTimer = 0.0f;
    }
}

void UEmotionalQuestDesigner::ProcessEmotionalQuestGeneration(float DeltaTime)
{
    // Generate quests based on emotional state patterns
    QuestGenerationTimer += DeltaTime;
    
    if (QuestGenerationTimer >= QuestGenerationInterval)
    {
        AnalyzeEmotionalPatternsForQuests();
        QuestGenerationTimer = 0.0f;
    }
}

void UEmotionalQuestDesigner::UpdateQuestEmotionalAtmosphere(float DeltaTime)
{
    // Update audio and visual atmosphere based on quest emotional state
    if (EmotionalAudioComponent && QuestMusicComponent)
    {
        // Adjust audio intensity based on emotional state
        float AudioIntensity = EmotionalIntensity * QuestEmotionalWeight;
        
        EmotionalAudioComponent->SetVolumeMultiplier(AudioIntensity);
        QuestMusicComponent->SetVolumeMultiplier(AudioIntensity * 0.7f);
    }
}

void UEmotionalQuestDesigner::TriggerEmotionalState(EEmotionalState NewState, float Intensity, float Duration)
{
    UE_LOG(LogEmotionalQuest, Log, TEXT("Triggering emotional state: %s (Intensity: %.2f, Duration: %.2f)"), 
           *GetEmotionalStateName(NewState), Intensity, Duration);
    
    // Set new emotional state
    PreviousEmotionalState = CurrentEmotionalState;
    CurrentEmotionalState = NewState;
    
    // Apply intensity modifier
    EmotionalIntensity = FMath::Clamp(Intensity, MinEmotionalIntensity, MaxEmotionalIntensity);
    
    // Set duration timer
    EmotionalStateDuration = Duration;
    EmotionalStateTimer = 0.0f;
    
    // Trigger quest generation based on emotional change
    OnEmotionalStateChanged.Broadcast(CurrentEmotionalState, EmotionalIntensity);
    
    // Generate appropriate quest content
    GenerateEmotionalQuest(NewState, Intensity);
}

void UEmotionalQuestDesigner::GenerateEmotionalQuest(EEmotionalState EmotionalState, float Intensity)
{
    UE_LOG(LogEmotionalQuest, Log, TEXT("Generating emotional quest for state: %s"), *GetEmotionalStateName(EmotionalState));
    
    // Create quest based on emotional state
    FQuestEmotionalData QuestData;
    QuestData.EmotionalState = EmotionalState;
    QuestData.Intensity = Intensity;
    QuestData.Timestamp = GetWorld()->GetTimeSeconds();
    
    switch (EmotionalState)
    {
        case EEmotionalState::Fear:
            GenerateFearBasedQuest(QuestData);
            break;
        case EEmotionalState::Wonder:
            GenerateWonderBasedQuest(QuestData);
            break;
        case EEmotionalState::Discovery:
            GenerateDiscoveryBasedQuest(QuestData);
            break;
        case EEmotionalState::Triumph:
            GenerateTriumphBasedQuest(QuestData);
            break;
        case EEmotionalState::Reflection:
            GenerateReflectionBasedQuest(QuestData);
            break;
        default:
            GenerateNeutralQuest(QuestData);
            break;
    }
}

void UEmotionalQuestDesigner::GenerateFearBasedQuest(const FQuestEmotionalData& QuestData)
{
    // Generate survival-focused quests during fear states
    TArray<FString> FearQuests = {
        "Find immediate shelter from approaching predator",
        "Create defensive barriers using available materials",
        "Locate elevated position for safety observation",
        "Craft emergency signaling device",
        "Establish escape route through dangerous territory"
    };
    
    FString SelectedQuest = FearQuests[FMath::RandRange(0, FearQuests.Num() - 1)];
    UE_LOG(LogEmotionalQuest, Log, TEXT("Fear-based quest generated: %s"), *SelectedQuest);
    
    // Broadcast quest generation event
    OnQuestGenerated.Broadcast(SelectedQuest, QuestData.EmotionalState, QuestData.Intensity);
}

void UEmotionalQuestDesigner::GenerateWonderBasedQuest(const FQuestEmotionalData& QuestData)
{
    // Generate exploration and discovery quests during wonder states
    TArray<FString> WonderQuests = {
        "Observe and document unique dinosaur behavior",
        "Explore the mysterious crystal formations",
        "Investigate ancient geological structures",
        "Study the prehistoric ecosystem interactions",
        "Map the territorial boundaries of local fauna"
    };
    
    FString SelectedQuest = WonderQuests[FMath::RandRange(0, WonderQuests.Num() - 1)];
    UE_LOG(LogEmotionalQuest, Log, TEXT("Wonder-based quest generated: %s"), *SelectedQuest);
    
    OnQuestGenerated.Broadcast(SelectedQuest, QuestData.EmotionalState, QuestData.Intensity);
}

void UEmotionalQuestDesigner::GenerateDiscoveryBasedQuest(const FQuestEmotionalData& QuestData)
{
    // Generate investigation and collection quests during discovery states
    TArray<FString> DiscoveryQuests = {
        "Collect rare minerals for tool crafting",
        "Locate the source of the mysterious energy signature",
        "Uncover clues about the time displacement phenomenon",
        "Find additional crystal fragments",
        "Discover hidden cave systems"
    };
    
    FString SelectedQuest = DiscoveryQuests[FMath::RandRange(0, DiscoveryQuests.Num() - 1)];
    UE_LOG(LogEmotionalQuest, Log, TEXT("Discovery-based quest generated: %s"), *SelectedQuest);
    
    OnQuestGenerated.Broadcast(SelectedQuest, QuestData.EmotionalState, QuestData.Intensity);
}

void UEmotionalQuestDesigner::GenerateTriumphBasedQuest(const FQuestEmotionalData& QuestData)
{
    // Generate achievement and mastery quests during triumph states
    TArray<FString> TriumphQuests = {
        "Establish permanent base camp",
        "Successfully domesticate a herbivorous dinosaur",
        "Master advanced tool crafting techniques",
        "Create comprehensive survival guide",
        "Achieve harmony with local ecosystem"
    };
    
    FString SelectedQuest = TriumphQuests[FMath::RandRange(0, TriumphQuests.Num() - 1)];
    UE_LOG(LogEmotionalQuest, Log, TEXT("Triumph-based quest generated: %s"), *SelectedQuest);
    
    OnQuestGenerated.Broadcast(SelectedQuest, QuestData.EmotionalState, QuestData.Intensity);
}

void UEmotionalQuestDesigner::GenerateReflectionBasedQuest(const FQuestEmotionalData& QuestData)
{
    // Generate contemplative and philosophical quests during reflection states
    TArray<FString> ReflectionQuests = {
        "Document personal survival journey",
        "Contemplate the implications of time travel",
        "Study the balance of prehistoric nature",
        "Reflect on humanity's place in natural history",
        "Consider the responsibility of knowledge"
    };
    
    FString SelectedQuest = ReflectionQuests[FMath::RandRange(0, ReflectionQuests.Num() - 1)];
    UE_LOG(LogEmotionalQuest, Log, TEXT("Reflection-based quest generated: %s"), *SelectedQuest);
    
    OnQuestGenerated.Broadcast(SelectedQuest, QuestData.EmotionalState, QuestData.Intensity);
}

void UEmotionalQuestDesigner::GenerateNeutralQuest(const FQuestEmotionalData& QuestData)
{
    // Generate balanced general quests during neutral states
    TArray<FString> NeutralQuests = {
        "Gather basic survival resources",
        "Maintain equipment and tools",
        "Scout nearby territories for opportunities",
        "Practice essential survival skills",
        "Plan next exploration objectives"
    };
    
    FString SelectedQuest = NeutralQuests[FMath::RandRange(0, NeutralQuests.Num() - 1)];
    UE_LOG(LogEmotionalQuest, Log, TEXT("Neutral quest generated: %s"), *SelectedQuest);
    
    OnQuestGenerated.Broadcast(SelectedQuest, QuestData.EmotionalState, QuestData.Intensity);
}

void UEmotionalQuestDesigner::RecordEmotionalMoment()
{
    // Record current emotional state for pattern analysis
    FQuestEmotionalData EmotionalMoment;
    EmotionalMoment.EmotionalState = CurrentEmotionalState;
    EmotionalMoment.Intensity = EmotionalIntensity;
    EmotionalMoment.Timestamp = GetWorld()->GetTimeSeconds();
    
    QuestEmotionalHistory.Add(EmotionalMoment);
    
    // Maintain history size limit
    if (QuestEmotionalHistory.Num() > 100)
    {
        QuestEmotionalHistory.RemoveAt(0);
    }
}

void UEmotionalQuestDesigner::AnalyzeEmotionalPatternsForQuests()
{
    if (QuestEmotionalHistory.Num() < 5) return;
    
    // Analyze recent emotional patterns
    TMap<EEmotionalState, int32> StateFrequency;
    float AverageIntensity = 0.0f;
    
    // Analyze last 10 emotional moments
    int32 AnalysisCount = FMath::Min(10, QuestEmotionalHistory.Num());
    for (int32 i = QuestEmotionalHistory.Num() - AnalysisCount; i < QuestEmotionalHistory.Num(); i++)
    {
        const FQuestEmotionalData& Moment = QuestEmotionalHistory[i];
        
        if (StateFrequency.Contains(Moment.EmotionalState))
        {
            StateFrequency[Moment.EmotionalState]++;
        }
        else
        {
            StateFrequency.Add(Moment.EmotionalState, 1);
        }
        
        AverageIntensity += Moment.Intensity;
    }
    
    AverageIntensity /= AnalysisCount;
    
    // Generate quest based on dominant emotional pattern
    EEmotionalState DominantState = EEmotionalState::Neutral;
    int32 MaxFrequency = 0;
    
    for (const auto& StatePair : StateFrequency)
    {
        if (StatePair.Value > MaxFrequency)
        {
            MaxFrequency = StatePair.Value;
            DominantState = StatePair.Key;
        }
    }
    
    UE_LOG(LogEmotionalQuest, Log, TEXT("Emotional pattern analysis: Dominant state %s, Average intensity %.2f"), 
           *GetEmotionalStateName(DominantState), AverageIntensity);
    
    // Generate contextual quest based on pattern
    GenerateContextualQuest(DominantState, AverageIntensity);
}

void UEmotionalQuestDesigner::GenerateContextualQuest(EEmotionalState DominantState, float AverageIntensity)
{
    // Generate quest that responds to player's emotional journey
    FQuestEmotionalData ContextualData;
    ContextualData.EmotionalState = DominantState;
    ContextualData.Intensity = AverageIntensity;
    ContextualData.Timestamp = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogEmotionalQuest, Log, TEXT("Generating contextual quest for emotional pattern"));
    
    // Create more complex, story-driven quest based on emotional journey
    GenerateEmotionalQuest(DominantState, AverageIntensity);
}

FString UEmotionalQuestDesigner::GetEmotionalStateName(EEmotionalState State) const
{
    switch (State)
    {
        case EEmotionalState::Fear: return TEXT("Fear");
        case EEmotionalState::Wonder: return TEXT("Wonder");
        case EEmotionalState::Discovery: return TEXT("Discovery");
        case EEmotionalState::Triumph: return TEXT("Triumph");
        case EEmotionalState::Reflection: return TEXT("Reflection");
        default: return TEXT("Neutral");
    }
}

float UEmotionalQuestDesigner::GetCurrentEmotionalIntensity() const
{
    return EmotionalIntensity;
}

EEmotionalState UEmotionalQuestDesigner::GetCurrentEmotionalState() const
{
    return CurrentEmotionalState;
}

void UEmotionalQuestDesigner::SetQuestEmotionalWeight(float NewWeight)
{
    QuestEmotionalWeight = FMath::Clamp(NewWeight, 0.0f, 2.0f);
    UE_LOG(LogEmotionalQuest, Log, TEXT("Quest emotional weight set to: %.2f"), QuestEmotionalWeight);
}

void UEmotionalQuestDesigner::SetEmotionalTransitionSpeed(float NewSpeed)
{
    EmotionalTransitionSpeed = FMath::Clamp(NewSpeed, 0.01f, 1.0f);
    UE_LOG(LogEmotionalQuest, Log, TEXT("Emotional transition speed set to: %.2f"), EmotionalTransitionSpeed);
}