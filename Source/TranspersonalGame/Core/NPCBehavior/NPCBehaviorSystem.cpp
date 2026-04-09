#include "NPCBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Components/CapsuleComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize personality with default values
    InitializeDefaultPersonality();
    
    // Initialize needs with default values
    InitializeDefaultNeeds();
    
    // Initialize daily routine
    InitializeDefaultRoutine();
    
    // Memory settings
    MaxMemories = 100;
    MemoryDecayRate = 0.01f;
    
    // Behavior settings
    DecisionUpdateInterval = 1.0f;
    LastDecisionTime = 0.0f;
    
    // State
    CurrentEmotionalState = EEmotionalState::Calm;
    CurrentActivity = EActivityType::Observing;
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Constructor completed"));
}

// Called when the game starts
void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get references to important components
    OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        AIController = Cast<AAIController>(OwnerPawn->GetController());
        if (AIController)
        {
            BlackboardComp = AIController->GetBlackboardComponent();
            BehaviorTreeComp = AIController->GetBrainComponent();
            PerceptionComp = AIController->GetAIPerceptionComponent();
            
            // Initialize blackboard values
            InitializeBlackboard();
        }
    }
    
    // Start the behavior system
    StartBehaviorSystem();
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: BeginPlay completed for %s"), 
           OwnerPawn ? *OwnerPawn->GetName() : TEXT("Unknown"));
}

// Called every frame
void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerPawn || !AIController)
    {
        return;
    }
    
    // Update needs over time
    UpdateNeeds(DeltaTime);
    
    // Update emotional state based on needs and environment
    UpdateEmotionalState(DeltaTime);
    
    // Update memories (decay old ones)
    UpdateMemories(DeltaTime);
    
    // Make behavioral decisions periodically
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastDecisionTime >= DecisionUpdateInterval)
    {
        MakeBehavioralDecision();
        LastDecisionTime = CurrentTime;
    }
    
    // Update current activity based on routine and needs
    UpdateCurrentActivity(DeltaTime);
}

void UNPCBehaviorComponent::InitializeDefaultPersonality()
{
    // Set random but reasonable personality traits
    PersonalityProfile.TraitLevels.Empty();
    
    // Big Five traits (0.3 to 0.7 range for balanced NPCs)
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::Openness, FMath::RandRange(0.3f, 0.7f));
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::Conscientiousness, FMath::RandRange(0.4f, 0.8f));
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::Extraversion, FMath::RandRange(0.2f, 0.8f));
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::Agreeableness, FMath::RandRange(0.4f, 0.8f));
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::Neuroticism, FMath::RandRange(0.2f, 0.6f));
    
    // Survival-specific traits
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::Cautiousness, FMath::RandRange(0.5f, 0.9f));
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::Resourcefulness, FMath::RandRange(0.4f, 0.8f));
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::Leadership, FMath::RandRange(0.2f, 0.7f));
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::Adaptability, FMath::RandRange(0.4f, 0.8f));
    PersonalityProfile.TraitLevels.Add(EPersonalityTrait::RiskTolerance, FMath::RandRange(0.2f, 0.6f));
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Personality initialized"));
}

void UNPCBehaviorComponent::InitializeDefaultNeeds()
{
    CurrentNeeds.Empty();
    
    // Physical needs
    FNeedState HungerNeed;
    HungerNeed.NeedType = ENeedType::Hunger;
    HungerNeed.CurrentLevel = FMath::RandRange(0.4f, 0.8f);
    HungerNeed.Importance = 1.0f;
    HungerNeed.DecayRate = 0.05f; // Hunger decreases over time
    CurrentNeeds.Add(HungerNeed);
    
    FNeedState ThirstNeed;
    ThirstNeed.NeedType = ENeedType::Thirst;
    ThirstNeed.CurrentLevel = FMath::RandRange(0.5f, 0.9f);
    ThirstNeed.Importance = 1.2f; // Thirst is critical
    ThirstNeed.DecayRate = 0.08f;
    CurrentNeeds.Add(ThirstNeed);
    
    FNeedState SleepNeed;
    SleepNeed.NeedType = ENeedType::Sleep;
    SleepNeed.CurrentLevel = FMath::RandRange(0.6f, 1.0f);
    SleepNeed.Importance = 0.8f;
    SleepNeed.DecayRate = 0.03f;
    CurrentNeeds.Add(SleepNeed);
    
    FNeedState SafetyNeed;
    SafetyNeed.NeedType = ENeedType::Safety;
    SafetyNeed.CurrentLevel = FMath::RandRange(0.3f, 0.7f);
    SafetyNeed.Importance = 0.9f;
    SafetyNeed.DecayRate = 0.02f;
    CurrentNeeds.Add(SafetyNeed);
    
    // Psychological needs
    FNeedState CompanionshipNeed;
    CompanionshipNeed.NeedType = ENeedType::Companionship;
    CompanionshipNeed.CurrentLevel = FMath::RandRange(0.2f, 0.8f);
    CompanionshipNeed.Importance = 0.6f;
    CompanionshipNeed.DecayRate = 0.01f;
    CurrentNeeds.Add(CompanionshipNeed);
    
    FNeedState PurposeNeed;
    PurposeNeed.NeedType = ENeedType::Purpose;
    PurposeNeed.CurrentLevel = FMath::RandRange(0.4f, 0.8f);
    PurposeNeed.Importance = 0.5f;
    PurposeNeed.DecayRate = 0.005f;
    CurrentNeeds.Add(PurposeNeed);
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Needs initialized with %d needs"), CurrentNeeds.Num());
}

void UNPCBehaviorComponent::InitializeDefaultRoutine()
{
    DailyRoutine.Empty();
    
    // Morning routine (6 AM - 8 AM): Wake up and assess situation
    FRoutineEntry MorningAssessment;
    MorningAssessment.StartTime = 0.25f; // 6 AM
    MorningAssessment.Duration = 2.0f;
    MorningAssessment.Activity = EActivityType::Observing;
    MorningAssessment.Priority = 0.7f;
    MorningAssessment.SatisfiedNeeds.Add(ENeedType::Stimulation);
    DailyRoutine.Add(MorningAssessment);
    
    // Morning foraging (8 AM - 11 AM)
    FRoutineEntry MorningForaging;
    MorningForaging.StartTime = 0.33f; // 8 AM
    MorningForaging.Duration = 3.0f;
    MorningForaging.Activity = EActivityType::ForagingFood;
    MorningForaging.Priority = 0.9f;
    MorningForaging.SatisfiedNeeds.Add(ENeedType::Hunger);
    DailyRoutine.Add(MorningForaging);
    
    // Midday rest (11 AM - 1 PM)
    FRoutineEntry MiddayRest;
    MiddayRest.StartTime = 0.46f; // 11 AM
    MiddayRest.Duration = 2.0f;
    MiddayRest.Activity = EActivityType::RestingRecovering;
    MiddayRest.Priority = 0.6f;
    MiddayRest.SatisfiedNeeds.Add(ENeedType::Health);
    DailyRoutine.Add(MiddayRest);
    
    // Afternoon exploration (1 PM - 4 PM)
    FRoutineEntry AfternoonExploration;
    AfternoonExploration.StartTime = 0.54f; // 1 PM
    AfternoonExploration.Duration = 3.0f;
    AfternoonExploration.Activity = EActivityType::ScoutingArea;
    AfternoonExploration.Priority = 0.8f;
    AfternoonExploration.SatisfiedNeeds.Add(ENeedType::Stimulation);
    AfternoonExploration.SatisfiedNeeds.Add(ENeedType::Safety);
    DailyRoutine.Add(AfternoonExploration);
    
    // Evening preparation (4 PM - 6 PM)
    FRoutineEntry EveningPrep;
    EveningPrep.StartTime = 0.67f; // 4 PM
    EveningPrep.Duration = 2.0f;
    EveningPrep.Activity = EActivityType::MaintainingGear;
    EveningPrep.Priority = 0.7f;
    EveningPrep.SatisfiedNeeds.Add(ENeedType::Safety);
    DailyRoutine.Add(EveningPrep);
    
    // Night rest (6 PM - 6 AM)
    FRoutineEntry NightRest;
    NightRest.StartTime = 0.75f; // 6 PM
    NightRest.Duration = 12.0f;
    NightRest.Activity = EActivityType::RestingRecovering;
    NightRest.Priority = 0.9f;
    NightRest.SatisfiedNeeds.Add(ENeedType::Sleep);
    NightRest.SatisfiedNeeds.Add(ENeedType::Health);
    DailyRoutine.Add(NightRest);
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Daily routine initialized with %d activities"), DailyRoutine.Num());
}

void UNPCBehaviorComponent::InitializeBlackboard()
{
    if (!BlackboardComp)
    {
        return;
    }
    
    // Set initial blackboard values based on personality and needs
    BlackboardComp->SetValueAsFloat(TEXT("Cautiousness"), 
        PersonalityProfile.TraitLevels.FindRef(EPersonalityTrait::Cautiousness));
    BlackboardComp->SetValueAsFloat(TEXT("Extraversion"), 
        PersonalityProfile.TraitLevels.FindRef(EPersonalityTrait::Extraversion));
    
    // Set current emotional state
    BlackboardComp->SetValueAsEnum(TEXT("EmotionalState"), (uint8)CurrentEmotionalState);
    
    // Set most urgent need
    ENeedType MostUrgentNeed = GetMostUrgentNeed();
    BlackboardComp->SetValueAsEnum(TEXT("MostUrgentNeed"), (uint8)MostUrgentNeed);
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Blackboard initialized"));
}

void UNPCBehaviorComponent::StartBehaviorSystem()
{
    if (!AIController || !BehaviorTreeComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCBehaviorComponent: Cannot start behavior system - missing AI components"));
        return;
    }
    
    // Start the behavior tree if we have one
    if (BehaviorTree)
    {
        AIController->RunBehaviorTree(BehaviorTree);
        UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Behavior tree started"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCBehaviorComponent: No behavior tree assigned"));
    }
}

void UNPCBehaviorComponent::UpdateNeeds(float DeltaTime)
{
    for (FNeedState& Need : CurrentNeeds)
    {
        // Decay needs over time
        Need.CurrentLevel = FMath::Max(0.0f, Need.CurrentLevel - (Need.DecayRate * DeltaTime));
        
        // Update last satisfied time
        if (Need.CurrentLevel > 0.8f)
        {
            Need.LastSatisfiedTime = GetWorld()->GetTimeSeconds();
        }
    }
    
    // Update blackboard with most urgent need
    if (BlackboardComp)
    {
        ENeedType MostUrgentNeed = GetMostUrgentNeed();
        BlackboardComp->SetValueAsEnum(TEXT("MostUrgentNeed"), (uint8)MostUrgentNeed);
    }
}

void UNPCBehaviorComponent::UpdateEmotionalState(float DeltaTime)
{
    // Calculate overall need satisfaction
    float TotalSatisfaction = 0.0f;
    float TotalImportance = 0.0f;
    
    for (const FNeedState& Need : CurrentNeeds)
    {
        TotalSatisfaction += Need.CurrentLevel * Need.Importance;
        TotalImportance += Need.Importance;
    }
    
    float OverallSatisfaction = TotalImportance > 0.0f ? TotalSatisfaction / TotalImportance : 0.5f;
    
    // Determine emotional state based on satisfaction and personality
    float NeuroticismLevel = PersonalityProfile.TraitLevels.FindRef(EPersonalityTrait::Neuroticism);
    float CautiousnessLevel = PersonalityProfile.TraitLevels.FindRef(EPersonalityTrait::Cautiousness);
    
    EEmotionalState NewState = CurrentEmotionalState;
    
    if (OverallSatisfaction < 0.3f)
    {
        // Low satisfaction - negative emotions
        if (NeuroticismLevel > 0.6f)
        {
            NewState = EEmotionalState::Panicked;
        }
        else if (CautiousnessLevel > 0.7f)
        {
            NewState = EEmotionalState::Anxious;
        }
        else
        {
            NewState = EEmotionalState::Desperate;
        }
    }
    else if (OverallSatisfaction < 0.6f)
    {
        // Medium satisfaction - cautious emotions
        if (CautiousnessLevel > 0.6f)
        {
            NewState = EEmotionalState::Anxious;
        }
        else
        {
            NewState = EEmotionalState::Focused;
        }
    }
    else
    {
        // High satisfaction - positive emotions
        float ExtraversionLevel = PersonalityProfile.TraitLevels.FindRef(EPersonalityTrait::Extraversion);
        if (ExtraversionLevel > 0.6f)
        {
            NewState = EEmotionalState::Confident;
        }
        else
        {
            NewState = EEmotionalState::Calm;
        }
    }
    
    // Update emotional state if it changed
    if (NewState != CurrentEmotionalState)
    {
        CurrentEmotionalState = NewState;
        
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(TEXT("EmotionalState"), (uint8)CurrentEmotionalState);
        }
        
        UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Emotional state changed to %d"), (int32)CurrentEmotionalState);
    }
}

void UNPCBehaviorComponent::UpdateMemories(float DeltaTime)
{
    // Decay memory accuracy over time
    for (FMemoryEntry& Memory : StoredMemories)
    {
        Memory.Accuracy = FMath::Max(0.1f, Memory.Accuracy - (MemoryDecayRate * DeltaTime));
    }
    
    // Remove very old or inaccurate memories if we're at capacity
    if (StoredMemories.Num() > MaxMemories)
    {
        StoredMemories.RemoveAll([](const FMemoryEntry& Memory)
        {
            return Memory.Accuracy < 0.2f;
        });
        
        // If still too many, remove oldest
        if (StoredMemories.Num() > MaxMemories)
        {
            StoredMemories.Sort([](const FMemoryEntry& A, const FMemoryEntry& B)
            {
                return A.Timestamp < B.Timestamp;
            });
            
            int32 ToRemove = StoredMemories.Num() - MaxMemories;
            StoredMemories.RemoveAt(0, ToRemove);
        }
    }
}

void UNPCBehaviorComponent::MakeBehavioralDecision()
{
    if (!OwnerPawn || !AIController)
    {
        return;
    }
    
    // Evaluate current situation
    ENeedType MostUrgentNeed = GetMostUrgentNeed();
    EActivityType IdealActivity = GetIdealActivityForNeed(MostUrgentNeed);
    
    // Check if we should interrupt current activity
    bool ShouldInterrupt = ShouldInterruptCurrentActivity(IdealActivity);
    
    if (ShouldInterrupt || CurrentActivity != IdealActivity)
    {
        // Change activity
        CurrentActivity = IdealActivity;
        
        // Update blackboard
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(TEXT("CurrentActivity"), (uint8)CurrentActivity);
        }
        
        // Find appropriate location for this activity
        FVector TargetLocation = FindLocationForActivity(CurrentActivity);
        if (BlackboardComp && TargetLocation != FVector::ZeroVector)
        {
            BlackboardComp->SetValueAsVector(TEXT("TargetLocation"), TargetLocation);
        }
        
        UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Changed activity to %d"), (int32)CurrentActivity);
    }
}

void UNPCBehaviorComponent::UpdateCurrentActivity(float DeltaTime)
{
    // Activity-specific updates can be added here
    // For now, just log periodically
    static float LogTimer = 0.0f;
    LogTimer += DeltaTime;
    
    if (LogTimer >= 5.0f) // Log every 5 seconds
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("NPCBehaviorComponent: Current activity: %d, Emotional state: %d"), 
               (int32)CurrentActivity, (int32)CurrentEmotionalState);
        LogTimer = 0.0f;
    }
}

ENeedType UNPCBehaviorComponent::GetMostUrgentNeed() const
{
    if (CurrentNeeds.Num() == 0)
    {
        return ENeedType::Safety;
    }
    
    float HighestUrgency = 0.0f;
    ENeedType MostUrgent = ENeedType::Safety;
    
    for (const FNeedState& Need : CurrentNeeds)
    {
        // Calculate urgency as (1 - satisfaction) * importance
        float Urgency = (1.0f - Need.CurrentLevel) * Need.Importance;
        
        if (Urgency > HighestUrgency)
        {
            HighestUrgency = Urgency;
            MostUrgent = Need.NeedType;
        }
    }
    
    return MostUrgent;
}

EActivityType UNPCBehaviorComponent::GetIdealActivityForNeed(ENeedType NeedType) const
{
    switch (NeedType)
    {
        case ENeedType::Hunger:
            return EActivityType::ForagingFood;
        case ENeedType::Thirst:
            return EActivityType::CollectingWater;
        case ENeedType::Sleep:
            return EActivityType::RestingRecovering;
        case ENeedType::Safety:
            return EActivityType::ScoutingArea;
        case ENeedType::Companionship:
            return EActivityType::Conversing;
        case ENeedType::Purpose:
            return EActivityType::MakingTools;
        case ENeedType::Stimulation:
            return EActivityType::StudyingEnvironment;
        default:
            return EActivityType::Observing;
    }
}

bool UNPCBehaviorComponent::ShouldInterruptCurrentActivity(EActivityType NewActivity) const
{
    // Get priority of current and new activities
    float CurrentPriority = GetActivityPriority(CurrentActivity);
    float NewPriority = GetActivityPriority(NewActivity);
    
    // Interrupt if new activity is significantly more important
    return NewPriority > CurrentPriority + 0.2f;
}

float UNPCBehaviorComponent::GetActivityPriority(EActivityType Activity) const
{
    // Base priorities for different activities
    switch (Activity)
    {
        case EActivityType::CollectingWater:
            return 1.0f; // Highest priority
        case EActivityType::ForagingFood:
            return 0.9f;
        case EActivityType::BuildingShelter:
            return 0.8f;
        case EActivityType::ScoutingArea:
            return 0.7f;
        case EActivityType::RestingRecovering:
            return 0.6f;
        case EActivityType::MakingTools:
            return 0.5f;
        case EActivityType::Conversing:
            return 0.4f;
        case EActivityType::Observing:
            return 0.3f;
        default:
            return 0.2f;
    }
}

FVector UNPCBehaviorComponent::FindLocationForActivity(EActivityType Activity) const
{
    if (!OwnerPawn)
    {
        return FVector::ZeroVector;
    }
    
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    
    // For now, return a location near the NPC
    // In a full implementation, this would use EQS or other systems to find appropriate locations
    FVector RandomOffset = FVector(
        FMath::RandRange(-500.0f, 500.0f),
        FMath::RandRange(-500.0f, 500.0f),
        0.0f
    );
    
    return CurrentLocation + RandomOffset;
}

void UNPCBehaviorComponent::AddMemory(const FMemoryEntry& NewMemory)
{
    StoredMemories.Add(NewMemory);
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Added memory of type %d at location %s"), 
           (int32)NewMemory.MemoryType, *NewMemory.Location.ToString());
}

TArray<FMemoryEntry> UNPCBehaviorComponent::GetMemoriesOfType(EMemoryType MemoryType) const
{
    TArray<FMemoryEntry> FilteredMemories;
    
    for (const FMemoryEntry& Memory : StoredMemories)
    {
        if (Memory.MemoryType == MemoryType)
        {
            FilteredMemories.Add(Memory);
        }
    }
    
    return FilteredMemories;
}

void UNPCBehaviorComponent::SatisfyNeed(ENeedType NeedType, float Amount)
{
    for (FNeedState& Need : CurrentNeeds)
    {
        if (Need.NeedType == NeedType)
        {
            Need.CurrentLevel = FMath::Min(1.0f, Need.CurrentLevel + Amount);
            Need.LastSatisfiedTime = GetWorld()->GetTimeSeconds();
            
            UE_LOG(LogTemp, Log, TEXT("NPCBehaviorComponent: Satisfied need %d by %f, new level: %f"), 
                   (int32)NeedType, Amount, Need.CurrentLevel);
            break;
        }
    }
}

float UNPCBehaviorComponent::GetNeedLevel(ENeedType NeedType) const
{
    for (const FNeedState& Need : CurrentNeeds)
    {
        if (Need.NeedType == NeedType)
        {
            return Need.CurrentLevel;
        }
    }
    
    return 0.5f; // Default if need not found
}

float UNPCBehaviorComponent::GetPersonalityTrait(EPersonalityTrait Trait) const
{
    return PersonalityProfile.TraitLevels.FindRef(Trait);
}