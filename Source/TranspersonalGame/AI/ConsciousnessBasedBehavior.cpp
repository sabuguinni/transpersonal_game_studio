#include "ConsciousnessBasedBehavior.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

UConsciousnessBasedBehavior::UConsciousnessBasedBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentBehaviorState = EBehaviorState::Idle;
    CurrentBehaviorTimer = 0.0f;
    InteractionRange = 300.0f;
    InteractionCooldown = 2.0f;
    LastInteractionTime = 0.0f;
    ResonanceThreshold = 0.3f;
    EmotionalInfluenceRadius = 500.0f;
    LearningRate = 0.1f;
    MaxLearnedBehaviors = 10;
    
    // Setup default emotional affinities
    EmotionalAffinities.Add("Joy", 0.8f);
    EmotionalAffinities.Add("Peace", 0.9f);
    EmotionalAffinities.Add("Curiosity", 0.7f);
    EmotionalAffinities.Add("Compassion", 0.85f);
    EmotionalAffinities.Add("Fear", 0.2f);
    EmotionalAffinities.Add("Anger", 0.1f);
    
    // Initialize default behavior patterns
    FBehaviorPattern IdlePattern;
    IdlePattern.BehaviorType = EBehaviorState::Idle;
    IdlePattern.Duration = 5.0f;
    IdlePattern.ConsciousnessThreshold = 0.0f;
    IdlePattern.Priority = 1.0f;
    AvailableBehaviors.Add(IdlePattern);
    
    FBehaviorPattern SeekingPattern;
    SeekingPattern.BehaviorType = EBehaviorState::Seeking;
    SeekingPattern.Duration = 10.0f;
    SeekingPattern.ConsciousnessThreshold = 0.3f;
    SeekingPattern.RequiredEmotions.Add("Curiosity");
    SeekingPattern.Priority = 2.0f;
    AvailableBehaviors.Add(SeekingPattern);
    
    FBehaviorPattern ContemplatingPattern;
    ContemplatingPattern.BehaviorType = EBehaviorState::Contemplating;
    ContemplatingPattern.Duration = 15.0f;
    ContemplatingPattern.ConsciousnessThreshold = 0.5f;
    ContemplatingPattern.RequiredEmotions.Add("Peace");
    ContemplatingPattern.Priority = 2.5f;
    AvailableBehaviors.Add(ContemplatingPattern);
    
    FBehaviorPattern GuidingPattern;
    GuidingPattern.BehaviorType = EBehaviorState::Guiding;
    GuidingPattern.Duration = 20.0f;
    GuidingPattern.ConsciousnessThreshold = 0.7f;
    GuidingPattern.RequiredEmotions.Add("Compassion");
    GuidingPattern.Priority = 3.0f;
    AvailableBehaviors.Add(GuidingPattern);
}

void UConsciousnessBasedBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    // Find consciousness system component
    ConsciousnessSystem = GetOwner()->FindComponentByClass<UConsciousnessSystem>();
    if (!ConsciousnessSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsciousnessBasedBehavior: No ConsciousnessSystem found on %s"), *GetOwner()->GetName());
    }
    
    // Initialize with idle behavior
    CurrentPattern = AvailableBehaviors[0];
    CurrentBehaviorTimer = CurrentPattern.Duration;
}

void UConsciousnessBasedBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update behavior timer
    CurrentBehaviorTimer -= DeltaTime;
    
    // Check if behavior should change
    if (CurrentBehaviorTimer <= 0.0f)
    {
        UpdateBehaviorState();
    }
    
    // Update consciousness influence
    UpdateConsciousnessInfluence();
    
    // Process emotional resonance with nearby entities
    TArray<AActor*> NearbyNPCs = FindNearbyNPCs();
    if (NearbyNPCs.Num() > 0)
    {
        TArray<FString> NearbyEmotions = GetActiveEmotions();
        RespondToEmotionalField(NearbyEmotions);
    }
    
    // Broadcast emotional state periodically
    static float BroadcastTimer = 0.0f;
    BroadcastTimer += DeltaTime;
    if (BroadcastTimer >= 1.0f)
    {
        BroadcastEmotionalState();
        BroadcastTimer = 0.0f;
    }
}

void UConsciousnessBasedBehavior::UpdateBehaviorState()
{
    EBehaviorState PreviousState = CurrentBehaviorState;
    
    // Select optimal behavior based on current consciousness state
    SelectOptimalBehavior();
    
    // Adapt behavior patterns based on learning
    AdaptBehaviorToConsciousness();
    
    // Update behavior priorities based on recent interactions
    UpdateBehaviorPriorities();
    
    // Notify if behavior changed
    if (PreviousState != CurrentBehaviorState)
    {
        OnBehaviorChanged.Broadcast(PreviousState, CurrentBehaviorState);
    }
}

void UConsciousnessBasedBehavior::SelectOptimalBehavior()
{
    float HighestPriority = -1.0f;
    FBehaviorPattern BestPattern = CurrentPattern;
    
    // Evaluate all available behaviors
    for (const FBehaviorPattern& Pattern : AvailableBehaviors)
    {
        if (EvaluateBehaviorConditions(Pattern))
        {
            float AdjustedPriority = Pattern.Priority;
            
            // Adjust priority based on consciousness level
            float ConsciousnessLevel = GetConsciousnessLevel();
            if (ConsciousnessLevel >= Pattern.ConsciousnessThreshold)
            {
                AdjustedPriority *= (1.0f + ConsciousnessLevel);
            }
            else
            {
                AdjustedPriority *= 0.5f; // Reduce priority if consciousness requirement not met
            }
            
            // Consider emotional resonance
            TArray<FString> ActiveEmotions = GetActiveEmotions();
            for (const FString& RequiredEmotion : Pattern.RequiredEmotions)
            {
                if (ActiveEmotions.Contains(RequiredEmotion))
                {
                    float Resonance = CalculateEmotionalResonance(RequiredEmotion);
                    AdjustedPriority *= (1.0f + Resonance);
                }
            }
            
            if (AdjustedPriority > HighestPriority)
            {
                HighestPriority = AdjustedPriority;
                BestPattern = Pattern;
            }
        }
    }
    
    // Also check learned behaviors
    for (const FBehaviorPattern& Pattern : LearnedBehaviors)
    {
        if (EvaluateBehaviorConditions(Pattern))
        {
            float AdjustedPriority = Pattern.Priority * 1.2f; // Learned behaviors get bonus
            
            if (AdjustedPriority > HighestPriority)
            {
                HighestPriority = AdjustedPriority;
                BestPattern = Pattern;
            }
        }
    }
    
    // Transition to new behavior if different
    if (BestPattern.BehaviorType != CurrentBehaviorState)
    {
        TransitionToBehavior(BestPattern.BehaviorType);
        CurrentPattern = BestPattern;
        CurrentBehaviorTimer = BestPattern.Duration;
    }
}

bool UConsciousnessBasedBehavior::EvaluateBehaviorConditions(const FBehaviorPattern& Pattern) const
{
    // Check consciousness threshold
    float ConsciousnessLevel = GetConsciousnessLevel();
    if (ConsciousnessLevel < Pattern.ConsciousnessThreshold)
    {
        return false;
    }
    
    // Check required emotions
    if (Pattern.RequiredEmotions.Num() > 0)
    {
        TArray<FString> ActiveEmotions = GetActiveEmotions();
        bool HasRequiredEmotion = false;
        
        for (const FString& RequiredEmotion : Pattern.RequiredEmotions)
        {
            if (ActiveEmotions.Contains(RequiredEmotion))
            {
                HasRequiredEmotion = true;
                break;
            }
        }
        
        if (!HasRequiredEmotion)
        {
            return false;
        }
    }
    
    return true;
}

void UConsciousnessBasedBehavior::TransitionToBehavior(EBehaviorState NewState)
{
    CurrentBehaviorState = NewState;
    
    // Perform state-specific initialization
    switch (NewState)
    {
        case EBehaviorState::Seeking:
            // Look for knowledge sources or interesting objects
            break;
            
        case EBehaviorState::Contemplating:
            // Enter reflective state, reduce movement
            break;
            
        case EBehaviorState::Interacting:
            // Prepare for social interaction
            break;
            
        case EBehaviorState::Guiding:
            // Look for entities that could benefit from guidance
            break;
            
        case EBehaviorState::Meditating:
            // Enter deep contemplative state
            break;
            
        default:
            break;
    }
}

FInteractionResponse UConsciousnessBasedBehavior::ProcessPlayerInteraction(const FString& InteractionType)
{
    FInteractionResponse Response;
    
    if (!CanInteract())
    {
        return Response;
    }
    
    LastInteractionTime = GetWorld()->GetTimeSeconds();
    
    // Process interaction based on current behavior state and consciousness level
    float ConsciousnessLevel = GetConsciousnessLevel();
    
    if (InteractionType == "Question")
    {
        if (CurrentBehaviorState == EBehaviorState::Guiding || ConsciousnessLevel > 0.6f)
        {
            Response.DialogueKey = "Wisdom_Response";
            Response.EmotionalImpact = 0.3f;
            Response.ConsciousnessShift = 0.1f;
            Response.UnlockedBehaviors.Add("Teaching");
        }
        else
        {
            Response.DialogueKey = "Basic_Response";
            Response.EmotionalImpact = 0.1f;
            Response.ConsciousnessShift = 0.05f;
        }
    }
    else if (InteractionType == "Emotional_Support")
    {
        if (EmotionalAffinities.Contains("Compassion") && EmotionalAffinities["Compassion"] > 0.7f)
        {
            Response.DialogueKey = "Compassionate_Response";
            Response.EmotionalImpact = 0.5f;
            Response.ConsciousnessShift = 0.15f;
            Response.UnlockedBehaviors.Add("Healing");
        }
    }
    else if (InteractionType == "Knowledge_Sharing")
    {
        if (CurrentBehaviorState == EBehaviorState::Contemplating)
        {
            Response.DialogueKey = "Deep_Knowledge";
            Response.EmotionalImpact = 0.2f;
            Response.ConsciousnessShift = 0.2f;
            Response.UnlockedBehaviors.Add("Advanced_Teaching");
        }
    }
    
    // Learn from this interaction
    LearnFromInteraction(Response);
    
    // Update interaction history
    if (InteractionHistory.Contains(InteractionType))
    {
        InteractionHistory[InteractionType]++;
    }
    else
    {
        InteractionHistory.Add(InteractionType, 1);
    }
    
    OnInteractionCompleted.Broadcast(Response);
    return Response;
}

bool UConsciousnessBasedBehavior::CanInteract() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastInteractionTime) >= InteractionCooldown;
}

void UConsciousnessBasedBehavior::LearnFromInteraction(const FInteractionResponse& Response)
{
    if (LearnedBehaviors.Num() >= MaxLearnedBehaviors)
    {
        return; // Max learned behaviors reached
    }
    
    // Create new behavior pattern based on successful interaction
    if (Response.ConsciousnessShift > 0.1f)
    {
        CreateNewBehaviorPattern(Response);
    }
}

void UConsciousnessBasedBehavior::CreateNewBehaviorPattern(const FInteractionResponse& Response)
{
    FBehaviorPattern NewPattern;
    NewPattern.BehaviorType = EBehaviorState::Interacting;
    NewPattern.Duration = 8.0f + (Response.ConsciousnessShift * 20.0f);
    NewPattern.ConsciousnessThreshold = GetConsciousnessLevel() * 0.8f;
    NewPattern.Priority = 2.0f + Response.EmotionalImpact;
    
    // Add emotional requirements based on current active emotions
    TArray<FString> ActiveEmotions = GetActiveEmotions();
    if (ActiveEmotions.Num() > 0)
    {
        NewPattern.RequiredEmotions.Add(ActiveEmotions[0]);
    }
    
    LearnedBehaviors.Add(NewPattern);
}

void UConsciousnessBasedBehavior::AdaptBehaviorToConsciousness()
{
    float ConsciousnessLevel = GetConsciousnessLevel();
    
    // Adjust behavior patterns based on consciousness evolution
    for (FBehaviorPattern& Pattern : AvailableBehaviors)
    {
        if (ConsciousnessLevel > 0.8f)
        {
            // High consciousness: increase duration and priority of contemplative behaviors
            if (Pattern.BehaviorType == EBehaviorState::Contemplating || 
                Pattern.BehaviorType == EBehaviorState::Guiding)
            {
                Pattern.Priority *= 1.2f;
                Pattern.Duration *= 1.1f;
            }
        }
        else if (ConsciousnessLevel < 0.3f)
        {
            // Low consciousness: favor basic behaviors
            if (Pattern.BehaviorType == EBehaviorState::Idle || 
                Pattern.BehaviorType == EBehaviorState::Seeking)
            {
                Pattern.Priority *= 1.3f;
            }
        }
    }
}

void UConsciousnessBasedBehavior::RespondToEmotionalField(const TArray<FString>& NearbyEmotions)
{
    for (const FString& Emotion : NearbyEmotions)
    {
        float Resonance = CalculateEmotionalResonance(Emotion);
        
        if (Resonance > ResonanceThreshold && ConsciousnessSystem)
        {
            // Influence our own emotional state based on resonance
            ConsciousnessSystem->AddEmotionalInfluence(Emotion, Resonance * 0.1f);
        }
    }
}

float UConsciousnessBasedBehavior::CalculateEmotionalResonance(const FString& Emotion) const
{
    if (EmotionalAffinities.Contains(Emotion))
    {
        return EmotionalAffinities[Emotion];
    }
    return 0.5f; // Default neutral resonance
}

void UConsciousnessBasedBehavior::UpdateConsciousnessInfluence()
{
    if (!ConsciousnessSystem)
        return;
    
    // Behavior influences consciousness development
    switch (CurrentBehaviorState)
    {
        case EBehaviorState::Contemplating:
            ConsciousnessSystem->AddConsciousnessGrowth(0.001f);
            break;
            
        case EBehaviorState::Guiding:
            ConsciousnessSystem->AddConsciousnessGrowth(0.002f);
            break;
            
        case EBehaviorState::Meditating:
            ConsciousnessSystem->AddConsciousnessGrowth(0.003f);
            break;
            
        default:
            break;
    }
}

float UConsciousnessBasedBehavior::GetConsciousnessLevel() const
{
    if (ConsciousnessSystem)
    {
        return ConsciousnessSystem->GetConsciousnessLevel();
    }
    return 0.5f; // Default middle consciousness
}

TArray<FString> UConsciousnessBasedBehavior::GetActiveEmotions() const
{
    if (ConsciousnessSystem)
    {
        return ConsciousnessSystem->GetActiveEmotions();
    }
    return TArray<FString>();
}

void UConsciousnessBasedBehavior::UpdateBehaviorPriorities()
{
    // Adjust priorities based on interaction history
    for (FBehaviorPattern& Pattern : AvailableBehaviors)
    {
        if (Pattern.BehaviorType == EBehaviorState::Interacting)
        {
            int32 TotalInteractions = 0;
            for (const auto& HistoryPair : InteractionHistory)
            {
                TotalInteractions += HistoryPair.Value;
            }
            
            if (TotalInteractions > 5)
            {
                Pattern.Priority *= 1.1f; // Increase social behavior priority
            }
        }
    }
}

AActor* UConsciousnessBasedBehavior::FindNearestPlayer() const
{
    return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

TArray<AActor*> UConsciousnessBasedBehavior::FindNearbyNPCs() const
{
    TArray<AActor*> NearbyNPCs;
    
    UWorld* World = GetWorld();
    if (!World)
        return NearbyNPCs;
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (TActorIterator<APawn> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        APawn* Pawn = *ActorIterator;
        if (Pawn && Pawn != GetOwner() && !Pawn->IsPlayerControlled())
        {
            float Distance = FVector::Dist(MyLocation, Pawn->GetActorLocation());
            if (Distance <= EmotionalInfluenceRadius)
            {
                NearbyNPCs.Add(Pawn);
            }
        }
    }
    
    return NearbyNPCs;
}

void UConsciousnessBasedBehavior::BroadcastEmotionalState()
{
    if (!ConsciousnessSystem)
        return;
    
    // Broadcast current emotional state to nearby NPCs
    TArray<AActor*> NearbyNPCs = FindNearbyNPCs();
    TArray<FString> MyEmotions = GetActiveEmotions();
    
    for (AActor* NPC : NearbyNPCs)
    {
        UConsciousnessBasedBehavior* OtherBehavior = NPC->FindComponentByClass<UConsciousnessBasedBehavior>();
        if (OtherBehavior)
        {
            OtherBehavior->RespondToEmotionalField(MyEmotions);
        }
    }
}

void UConsciousnessBasedBehavior::StartGuidingBehavior(AActor* TargetActor)
{
    if (!TargetActor)
        return;
    
    // Transition to guiding behavior focused on target
    TransitionToBehavior(EBehaviorState::Guiding);
    
    // Create specialized guiding pattern
    FBehaviorPattern GuidingPattern;
    GuidingPattern.BehaviorType = EBehaviorState::Guiding;
    GuidingPattern.Duration = 30.0f;
    GuidingPattern.ConsciousnessThreshold = 0.6f;
    GuidingPattern.Priority = 4.0f;
    GuidingPattern.RequiredEmotions.Add("Compassion");
    
    CurrentPattern = GuidingPattern;
    CurrentBehaviorTimer = GuidingPattern.Duration;
}

void UConsciousnessBasedBehavior::SetBehaviorPattern(const FBehaviorPattern& NewPattern)
{
    CurrentPattern = NewPattern;
    CurrentBehaviorTimer = NewPattern.Duration;
    TransitionToBehavior(NewPattern.BehaviorType);
}