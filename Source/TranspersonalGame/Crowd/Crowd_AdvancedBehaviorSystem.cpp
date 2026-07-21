#include "Crowd_AdvancedBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCrowd_AdvancedBehaviorSystem::UCrowd_AdvancedBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    CurrentBehaviorState = ECrowd_BehaviorState::Idle;
    CurrentEmotionalState = ECrowd_EmotionalState::Calm;
    BehaviorUpdateInterval = 1.0f;
    EmotionalDecayRate = 0.1f;
    LastBehaviorUpdate = 0.0f;
    EmotionalIntensity = 0.0f;
    
    // Setup default behavior profile
    DefaultProfile.Aggression = 0.3f;
    DefaultProfile.Curiosity = 0.7f;
    DefaultProfile.Sociability = 0.6f;
    DefaultProfile.Fearfulness = 0.4f;
    DefaultProfile.EnergyLevel = 1.0f;
    DefaultProfile.HungerLevel = 0.2f;
    
    CurrentProfile = DefaultProfile;
    
    // Setup default behavior rules
    FCrowd_BehaviorRule IdleToWandering;
    IdleToWandering.TriggerState = ECrowd_BehaviorState::Idle;
    IdleToWandering.TargetState = ECrowd_BehaviorState::Wandering;
    IdleToWandering.Probability = 0.3f;
    IdleToWandering.MinDistance = 0.0f;
    IdleToWandering.MaxDistance = 1000.0f;
    BehaviorRules.Add(IdleToWandering);
    
    FCrowd_BehaviorRule WanderingToSocializing;
    WanderingToSocializing.TriggerState = ECrowd_BehaviorState::Wandering;
    WanderingToSocializing.TargetState = ECrowd_BehaviorState::Socializing;
    WanderingToSocializing.Probability = 0.2f;
    WanderingToSocializing.MinDistance = 50.0f;
    WanderingToSocializing.MaxDistance = 300.0f;
    BehaviorRules.Add(WanderingToSocializing);
    
    FCrowd_BehaviorRule SocializingToResting;
    SocializingToResting.TriggerState = ECrowd_BehaviorState::Socializing;
    SocializingToResting.TargetState = ECrowd_BehaviorState::Resting;
    SocializingToResting.Probability = 0.15f;
    SocializingToResting.MinDistance = 0.0f;
    SocializingToResting.MaxDistance = 200.0f;
    BehaviorRules.Add(SocializingToResting);
}

void UCrowd_AdvancedBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    LastBehaviorUpdate = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Advanced Behavior System initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCrowd_AdvancedBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update emotional state decay
    UpdateEmotionalState(DeltaTime);
    
    // Process behavior transitions at intervals
    if (CurrentTime - LastBehaviorUpdate >= BehaviorUpdateInterval)
    {
        // Get nearby actors for context
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
        
        // Filter to nearby actors within reasonable distance
        CachedNearbyActors.Empty();
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        
        for (AActor* Actor : NearbyActors)
        {
            if (Actor && Actor != GetOwner())
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance <= 1000.0f)
                {
                    CachedNearbyActors.Add(Actor);
                }
            }
        }
        
        ProcessBehaviorTransitions();
        ProcessEnvironmentalStimuli(CachedNearbyActors);
        
        LastBehaviorUpdate = CurrentTime;
    }
}

void UCrowd_AdvancedBehaviorSystem::SetBehaviorState(ECrowd_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        ECrowd_BehaviorState PreviousState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("%s behavior changed from %d to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               (int32)PreviousState, (int32)NewState);
    }
}

void UCrowd_AdvancedBehaviorSystem::SetEmotionalState(ECrowd_EmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        CurrentEmotionalState = NewState;
        EmotionalIntensity = 1.0f;
        
        UE_LOG(LogTemp, Log, TEXT("%s emotional state changed to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               (int32)NewState);
    }
}

void UCrowd_AdvancedBehaviorSystem::UpdateBehaviorProfile(const FCrowd_BehaviorProfile& NewProfile)
{
    CurrentProfile = NewProfile;
    
    // Clamp values to valid ranges
    CurrentProfile.Aggression = FMath::Clamp(CurrentProfile.Aggression, 0.0f, 1.0f);
    CurrentProfile.Curiosity = FMath::Clamp(CurrentProfile.Curiosity, 0.0f, 1.0f);
    CurrentProfile.Sociability = FMath::Clamp(CurrentProfile.Sociability, 0.0f, 1.0f);
    CurrentProfile.Fearfulness = FMath::Clamp(CurrentProfile.Fearfulness, 0.0f, 1.0f);
    CurrentProfile.EnergyLevel = FMath::Clamp(CurrentProfile.EnergyLevel, 0.0f, 1.0f);
    CurrentProfile.HungerLevel = FMath::Clamp(CurrentProfile.HungerLevel, 0.0f, 1.0f);
}

void UCrowd_AdvancedBehaviorSystem::AddBehaviorRule(const FCrowd_BehaviorRule& NewRule)
{
    BehaviorRules.Add(NewRule);
}

ECrowd_BehaviorState UCrowd_AdvancedBehaviorSystem::EvaluateNextBehavior()
{
    ECrowd_BehaviorState NextState = CurrentBehaviorState;
    
    for (const FCrowd_BehaviorRule& Rule : BehaviorRules)
    {
        if (Rule.TriggerState == CurrentBehaviorState)
        {
            if (EvaluateBehaviorRule(Rule, CachedNearbyActors))
            {
                float RandomValue = FMath::FRand();
                if (RandomValue <= Rule.Probability)
                {
                    NextState = Rule.TargetState;
                    break;
                }
            }
        }
    }
    
    return NextState;
}

void UCrowd_AdvancedBehaviorSystem::ProcessEnvironmentalStimuli(const TArray<AActor*>& NearbyActors)
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    int32 NearbyCount = 0;
    bool ThreatDetected = false;
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        // Count nearby social actors
        if (Distance <= 300.0f && Actor->GetName().Contains(TEXT("Crowd")))
        {
            NearbyCount++;
        }
        
        // Detect threats (predators, dangers)
        if (Distance <= 500.0f && 
            (Actor->GetName().Contains(TEXT("Predator")) || 
             Actor->GetName().Contains(TEXT("Danger"))))
        {
            ThreatDetected = true;
        }
    }
    
    // Apply environmental influences
    if (ThreatDetected)
    {
        ApplyEmotionalInfluence(ECrowd_EmotionalState::Fearful, 0.8f);
        if (CurrentBehaviorState != ECrowd_BehaviorState::Fleeing)
        {
            SetBehaviorState(ECrowd_BehaviorState::Fleeing);
        }
    }
    else if (NearbyCount >= 3)
    {
        ApplyEmotionalInfluence(ECrowd_EmotionalState::Social, 0.6f);
        if (CurrentBehaviorState == ECrowd_BehaviorState::Idle)
        {
            SetBehaviorState(ECrowd_BehaviorState::Socializing);
        }
    }
    else if (NearbyCount == 0 && CurrentProfile.Curiosity > 0.6f)
    {
        ApplyEmotionalInfluence(ECrowd_EmotionalState::Curious, 0.4f);
        if (CurrentBehaviorState == ECrowd_BehaviorState::Idle)
        {
            SetBehaviorState(ECrowd_BehaviorState::Wandering);
        }
    }
}

void UCrowd_AdvancedBehaviorSystem::ApplyEmotionalInfluence(ECrowd_EmotionalState Influence, float Intensity)
{
    SetEmotionalState(Influence);
    EmotionalIntensity = FMath::Clamp(EmotionalIntensity + Intensity, 0.0f, 1.0f);
    
    // Modify behavior profile based on emotional state
    switch (Influence)
    {
        case ECrowd_EmotionalState::Fearful:
            CurrentProfile.Fearfulness = FMath::Min(CurrentProfile.Fearfulness + Intensity * 0.3f, 1.0f);
            CurrentProfile.Aggression = FMath::Max(CurrentProfile.Aggression - Intensity * 0.2f, 0.0f);
            break;
            
        case ECrowd_EmotionalState::Aggressive:
            CurrentProfile.Aggression = FMath::Min(CurrentProfile.Aggression + Intensity * 0.4f, 1.0f);
            CurrentProfile.Sociability = FMath::Max(CurrentProfile.Sociability - Intensity * 0.2f, 0.0f);
            break;
            
        case ECrowd_EmotionalState::Social:
            CurrentProfile.Sociability = FMath::Min(CurrentProfile.Sociability + Intensity * 0.3f, 1.0f);
            CurrentProfile.Fearfulness = FMath::Max(CurrentProfile.Fearfulness - Intensity * 0.1f, 0.0f);
            break;
            
        case ECrowd_EmotionalState::Curious:
            CurrentProfile.Curiosity = FMath::Min(CurrentProfile.Curiosity + Intensity * 0.2f, 1.0f);
            break;
            
        default:
            break;
    }
}

void UCrowd_AdvancedBehaviorSystem::UpdateEmotionalState(float DeltaTime)
{
    // Decay emotional intensity over time
    EmotionalIntensity = FMath::Max(EmotionalIntensity - EmotionalDecayRate * DeltaTime, 0.0f);
    
    // Return to calm state when intensity is low
    if (EmotionalIntensity <= 0.1f && CurrentEmotionalState != ECrowd_EmotionalState::Calm)
    {
        SetEmotionalState(ECrowd_EmotionalState::Calm);
    }
    
    // Gradually return behavior profile to default
    float DecayRate = EmotionalDecayRate * DeltaTime * 0.5f;
    CurrentProfile.Aggression = FMath::Lerp(CurrentProfile.Aggression, DefaultProfile.Aggression, DecayRate);
    CurrentProfile.Curiosity = FMath::Lerp(CurrentProfile.Curiosity, DefaultProfile.Curiosity, DecayRate);
    CurrentProfile.Sociability = FMath::Lerp(CurrentProfile.Sociability, DefaultProfile.Sociability, DecayRate);
    CurrentProfile.Fearfulness = FMath::Lerp(CurrentProfile.Fearfulness, DefaultProfile.Fearfulness, DecayRate);
}

void UCrowd_AdvancedBehaviorSystem::ProcessBehaviorTransitions()
{
    ECrowd_BehaviorState NextState = EvaluateNextBehavior();
    
    if (NextState != CurrentBehaviorState)
    {
        SetBehaviorState(NextState);
    }
}

bool UCrowd_AdvancedBehaviorSystem::EvaluateBehaviorRule(const FCrowd_BehaviorRule& Rule, const TArray<AActor*>& Context)
{
    if (!GetOwner())
    {
        return false;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Check distance constraints
    for (AActor* Actor : Context)
    {
        if (!Actor)
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance >= Rule.MinDistance && Distance <= Rule.MaxDistance)
        {
            return true;
        }
    }
    
    // If no distance constraints, rule can be applied
    if (Rule.MinDistance == 0.0f && Rule.MaxDistance >= 1000.0f)
    {
        return true;
    }
    
    return false;
}