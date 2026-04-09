// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "NPCBehaviorCore.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UNPCBehaviorCore::UNPCBehaviorCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    Personality = ENPCPersonality::Peaceful;
    CurrentMood = ENPCMoodState::Calm;
    CurrentActivity = ENPCActivity::Idle;
    
    StressLevel = 0.0f;
    EnergyLevel = 1.0f;
    HungerLevel = 0.0f;
    SocialNeed = 0.5f;
    
    MaxMemoryEntries = 50;
    MemoryDecayRate = 0.1f;
    
    CurrentDayTime = 8.0f; // Start at 8 AM
    bFollowDailyRoutine = true;
    
    TerritoryRadius = 1000.0f;
    bDefendTerritory = false;
    SocialRadius = 500.0f;
    
    LastMoodUpdateTime = 0.0f;
    LastRoutineCheckTime = 0.0f;

    // Setup default daily routine
    FNPCDailyRoutine MorningForage;
    MorningForage.StartTime = 7.0f;
    MorningForage.Duration = 2.0f;
    MorningForage.Activity = ENPCActivity::Foraging;
    MorningForage.Priority = 0.8f;
    DailyRoutines.Add(MorningForage);

    FNPCDailyRoutine MidDayRest;
    MidDayRest.StartTime = 12.0f;
    MidDayRest.Duration = 2.0f;
    MidDayRest.Activity = ENPCActivity::Resting;
    MidDayRest.Priority = 0.6f;
    DailyRoutines.Add(MidDayRest);

    FNPCDailyRoutine EveningDrink;
    EveningDrink.StartTime = 18.0f;
    EveningDrink.Duration = 1.0f;
    EveningDrink.Activity = ENPCActivity::Drinking;
    EveningDrink.Priority = 0.9f;
    DailyRoutines.Add(EveningDrink);
}

void UNPCBehaviorCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize territory center to current location
    if (AActor* Owner = GetOwner())
    {
        TerritoryCenter = Owner->GetActorLocation();
    }

    // Setup AI Controller integration
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
        {
            if (BehaviorTreeAsset)
            {
                AIController->RunBehaviorTree(BehaviorTreeAsset);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("NPC Behavior Core initialized for %s"), *GetOwner()->GetName());
}

void UNPCBehaviorCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update all systems
    UpdateMemoryDecay(DeltaTime);
    UpdateDailyRoutine(DeltaTime);
    UpdateNaturalNeeds(DeltaTime);
    ProcessNaturalNeeds(DeltaTime);
    
    // Periodic mood evaluation
    if (GetWorld()->GetTimeSeconds() - LastMoodUpdateTime > 5.0f)
    {
        EvaluateMoodChanges();
        LastMoodUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void UNPCBehaviorCore::AddMemoryEntry(FVector Location, const FString& Description, float EmotionalWeight, bool bPositive)
{
    FNPCMemoryEntry NewEntry;
    NewEntry.Location = Location;
    NewEntry.Timestamp = GetWorld()->GetTimeSeconds();
    NewEntry.EmotionalWeight = EmotionalWeight;
    NewEntry.EventDescription = Description;
    NewEntry.bIsPositive = bPositive;

    MemoryEntries.Add(NewEntry);

    // Remove oldest entries if we exceed max capacity
    while (MemoryEntries.Num() > MaxMemoryEntries)
    {
        MemoryEntries.RemoveAt(0);
    }

    // Adjust stress based on memory
    if (!bPositive)
    {
        StressLevel = FMath::Clamp(StressLevel + EmotionalWeight * 0.1f, 0.0f, 1.0f);
    }
    else
    {
        StressLevel = FMath::Clamp(StressLevel - EmotionalWeight * 0.05f, 0.0f, 1.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("NPC %s added memory: %s (Weight: %f, Positive: %s)"), 
           *GetOwner()->GetName(), *Description, EmotionalWeight, bPositive ? TEXT("Yes") : TEXT("No"));
}

void UNPCBehaviorCore::UpdateMood(ENPCMoodState NewMood)
{
    if (CurrentMood != NewMood)
    {
        ENPCMoodState PreviousMood = CurrentMood;
        CurrentMood = NewMood;

        // Update blackboard if available
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
            {
                if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
                {
                    BlackboardComp->SetValueAsEnum(TEXT("CurrentMood"), static_cast<uint8>(CurrentMood));
                }
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("NPC %s mood changed from %d to %d"), 
               *GetOwner()->GetName(), static_cast<int32>(PreviousMood), static_cast<int32>(CurrentMood));
    }
}

void UNPCBehaviorCore::ChangeActivity(ENPCActivity NewActivity)
{
    if (CurrentActivity != NewActivity)
    {
        ENPCActivity PreviousActivity = CurrentActivity;
        CurrentActivity = NewActivity;

        // Update blackboard
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (AAIController* AIController = Cast<AAIController>(Character->GetController()))
            {
                if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
                {
                    BlackboardComp->SetValueAsEnum(TEXT("CurrentActivity"), static_cast<uint8>(CurrentActivity));
                }
            }
        }

        UE_LOG(LogTemp, Log, TEXT("NPC %s activity changed from %d to %d"), 
               *GetOwner()->GetName(), static_cast<int32>(PreviousActivity), static_cast<int32>(CurrentActivity));
    }
}

FNPCDailyRoutine UNPCBehaviorCore::GetCurrentRoutineActivity()
{
    if (!bFollowDailyRoutine || DailyRoutines.Num() == 0)
    {
        return FNPCDailyRoutine();
    }

    // Find the current routine based on time of day
    for (const FNPCDailyRoutine& Routine : DailyRoutines)
    {
        float EndTime = Routine.StartTime + Routine.Duration;
        if (CurrentDayTime >= Routine.StartTime && CurrentDayTime <= EndTime)
        {
            return Routine;
        }
    }

    // Return idle routine if no specific routine is active
    return FNPCDailyRoutine();
}

bool UNPCBehaviorCore::IsInTerritory(FVector Location)
{
    float Distance = FVector::Dist(Location, TerritoryCenter);
    return Distance <= TerritoryRadius;
}

void UNPCBehaviorCore::UpdateSocialRelationship(AActor* OtherActor, float RelationshipChange)
{
    if (!OtherActor) return;

    float* CurrentRelationship = SocialRelationships.Find(OtherActor);
    if (CurrentRelationship)
    {
        *CurrentRelationship = FMath::Clamp(*CurrentRelationship + RelationshipChange, -1.0f, 1.0f);
    }
    else
    {
        SocialRelationships.Add(OtherActor, FMath::Clamp(RelationshipChange, -1.0f, 1.0f));
    }

    UE_LOG(LogTemp, Log, TEXT("NPC %s relationship with %s changed by %f (now: %f)"), 
           *GetOwner()->GetName(), *OtherActor->GetName(), RelationshipChange, GetRelationshipWith(OtherActor));
}

float UNPCBehaviorCore::GetRelationshipWith(AActor* OtherActor)
{
    if (!OtherActor) return 0.0f;

    float* Relationship = SocialRelationships.Find(OtherActor);
    return Relationship ? *Relationship : 0.0f;
}

void UNPCBehaviorCore::ReactToPlayerPresence(AActor* Player, float Distance)
{
    if (!Player) return;

    // Different reactions based on personality and distance
    float ReactionThreshold = 0.0f;
    
    switch (Personality)
    {
        case ENPCPersonality::Timid:
            ReactionThreshold = 800.0f;
            if (Distance < ReactionThreshold)
            {
                UpdateMood(ENPCMoodState::Frightened);
                ChangeActivity(ENPCActivity::Fleeing);
                AddMemoryEntry(Player->GetActorLocation(), TEXT("Player approached - felt threatened"), 0.7f, false);
            }
            break;
            
        case ENPCPersonality::Curious:
            ReactionThreshold = 600.0f;
            if (Distance < ReactionThreshold)
            {
                UpdateMood(ENPCMoodState::Curious);
                AddMemoryEntry(Player->GetActorLocation(), TEXT("Interesting creature spotted"), 0.5f, true);
            }
            break;
            
        case ENPCPersonality::Aggressive:
            ReactionThreshold = 500.0f;
            if (Distance < ReactionThreshold && bDefendTerritory)
            {
                UpdateMood(ENPCMoodState::Aggressive);
                AddMemoryEntry(Player->GetActorLocation(), TEXT("Territory intruder detected"), 0.8f, false);
            }
            break;
            
        case ENPCPersonality::Social:
            ReactionThreshold = 400.0f;
            if (Distance < ReactionThreshold)
            {
                UpdateMood(ENPCMoodState::Curious);
                ChangeActivity(ENPCActivity::Socializing);
                UpdateSocialRelationship(Player, 0.1f);
            }
            break;
            
        default:
            break;
    }

    // Update stress based on proximity
    if (Distance < 300.0f)
    {
        StressLevel = FMath::Clamp(StressLevel + 0.1f, 0.0f, 1.0f);
    }
}

void UNPCBehaviorCore::ProcessNaturalNeeds(float DeltaTime)
{
    // Process hunger
    if (HungerLevel > 0.7f && CurrentActivity != ENPCActivity::Foraging)
    {
        ChangeActivity(ENPCActivity::Foraging);
        UpdateMood(ENPCMoodState::Hungry);
    }

    // Process energy/tiredness
    if (EnergyLevel < 0.3f && CurrentActivity != ENPCActivity::Resting)
    {
        ChangeActivity(ENPCActivity::Resting);
        UpdateMood(ENPCMoodState::Tired);
    }

    // Process social needs
    if (SocialNeed > 0.8f && Personality == ENPCPersonality::Social)
    {
        ChangeActivity(ENPCActivity::Socializing);
    }
}

void UNPCBehaviorCore::UpdateMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        FNPCMemoryEntry& Entry = MemoryEntries[i];
        float Age = CurrentTime - Entry.Timestamp;
        
        // Decay emotional weight over time
        Entry.EmotionalWeight = FMath::Max(0.0f, Entry.EmotionalWeight - MemoryDecayRate * DeltaTime);
        
        // Remove very old or completely decayed memories
        if (Age > 3600.0f || Entry.EmotionalWeight <= 0.01f) // 1 hour or negligible weight
        {
            MemoryEntries.RemoveAt(i);
        }
    }
}

void UNPCBehaviorCore::UpdateDailyRoutine(float DeltaTime)
{
    // Simulate day progression (24 hours = 24 minutes real time)
    CurrentDayTime += DeltaTime / 60.0f; // 1 minute real = 1 hour game
    
    if (CurrentDayTime >= 24.0f)
    {
        CurrentDayTime = 0.0f; // Reset to midnight
    }

    // Check if we should change activity based on routine
    if (bFollowDailyRoutine && GetWorld()->GetTimeSeconds() - LastRoutineCheckTime > 30.0f)
    {
        FNPCDailyRoutine NewRoutine = GetCurrentRoutineActivity();
        if (NewRoutine.Activity != ENPCActivity::Idle && NewRoutine.Activity != CurrentActivity)
        {
            ChangeActivity(NewRoutine.Activity);
        }
        LastRoutineCheckTime = GetWorld()->GetTimeSeconds();
    }
}

void UNPCBehaviorCore::UpdateNaturalNeeds(float DeltaTime)
{
    // Increase hunger over time
    HungerLevel = FMath::Clamp(HungerLevel + DeltaTime * 0.01f, 0.0f, 1.0f);
    
    // Decrease energy over time (faster when active)
    float EnergyDrain = (CurrentActivity == ENPCActivity::Resting) ? 0.005f : 0.02f;
    EnergyLevel = FMath::Clamp(EnergyLevel - DeltaTime * EnergyDrain, 0.0f, 1.0f);
    
    // Social need increases over time for social personalities
    if (Personality == ENPCPersonality::Social)
    {
        SocialNeed = FMath::Clamp(SocialNeed + DeltaTime * 0.015f, 0.0f, 1.0f);
    }

    // Recover from activities
    if (CurrentActivity == ENPCActivity::Foraging && HungerLevel > 0.0f)
    {
        HungerLevel = FMath::Clamp(HungerLevel - DeltaTime * 0.05f, 0.0f, 1.0f);
    }
    
    if (CurrentActivity == ENPCActivity::Resting && EnergyLevel < 1.0f)
    {
        EnergyLevel = FMath::Clamp(EnergyLevel + DeltaTime * 0.03f, 0.0f, 1.0f);
    }

    if (CurrentActivity == ENPCActivity::Socializing && SocialNeed > 0.0f)
    {
        SocialNeed = FMath::Clamp(SocialNeed - DeltaTime * 0.02f, 0.0f, 1.0f);
    }
}

void UNPCBehaviorCore::EvaluateMoodChanges()
{
    // Evaluate mood based on current state
    ENPCMoodState NewMood = CurrentMood;

    // High stress leads to fear or aggression
    if (StressLevel > 0.7f)
    {
        NewMood = (Personality == ENPCPersonality::Aggressive) ? ENPCMoodState::Aggressive : ENPCMoodState::Frightened;
    }
    // Low energy leads to tiredness
    else if (EnergyLevel < 0.3f)
    {
        NewMood = ENPCMoodState::Tired;
    }
    // High hunger leads to hunger mood
    else if (HungerLevel > 0.7f)
    {
        NewMood = ENPCMoodState::Hungry;
    }
    // Alert when moderately stressed
    else if (StressLevel > 0.4f)
    {
        NewMood = ENPCMoodState::Alert;
    }
    // Default to calm when needs are met
    else if (StressLevel < 0.2f && EnergyLevel > 0.6f && HungerLevel < 0.4f)
    {
        NewMood = ENPCMoodState::Calm;
    }

    UpdateMood(NewMood);
}

FNPCMemoryEntry* UNPCBehaviorCore::FindMemoryNear(FVector Location, float Radius)
{
    for (FNPCMemoryEntry& Entry : MemoryEntries)
    {
        if (FVector::Dist(Entry.Location, Location) <= Radius)
        {
            return &Entry;
        }
    }
    return nullptr;
}