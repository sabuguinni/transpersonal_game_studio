#include "DinosaurBehaviorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UDinosaurBehaviorComponent::UDinosaurBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UDinosaurBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize personality if not already set
    if (Personality.IndividualName.IsEmpty())
    {
        GeneratePersonalityName();
    }
    
    // Initialize daily schedule
    InitializeDailySchedule();
    
    // Set initial mood based on personality
    EDinosaurMood InitialMood = EDinosaurMood::Calm;
    if (Personality.Fearfulness > 0.7f)
    {
        InitialMood = EDinosaurMood::Nervous;
    }
    else if (Personality.Aggression > 0.7f)
    {
        InitialMood = EDinosaurMood::Alert;
    }
    
    SetMood(InitialMood);
    SetActivity(GetScheduledActivity());
}

void UDinosaurBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update game time (simplified - 1 real second = 1 game minute)
    CurrentTime += DeltaTime / 60.0f;
    if (CurrentTime >= 24.0f)
    {
        CurrentTime = 0.0f;
    }
    
    // Update systems
    UpdateMemory(DeltaTime);
    UpdateDomestication(DeltaTime);
    UpdateDailyRoutine();
}

void UDinosaurBehaviorComponent::SetMood(EDinosaurMood NewMood)
{
    if (CurrentMood != NewMood)
    {
        EDinosaurMood OldMood = CurrentMood;
        CurrentMood = NewMood;
        OnMoodChanged.Broadcast(OldMood, NewMood);
    }
}

void UDinosaurBehaviorComponent::SetActivity(EDinosaurActivity NewActivity)
{
    if (CurrentActivity != NewActivity)
    {
        EDinosaurActivity OldActivity = CurrentActivity;
        CurrentActivity = NewActivity;
        OnActivityChanged.Broadcast(OldActivity, NewActivity);
    }
}

void UDinosaurBehaviorComponent::RememberActor(AActor* Actor, float EmotionalValue)
{
    if (!Actor) return;
    
    // Find existing memory entry
    FDinosaurMemoryEntry* ExistingEntry = nullptr;
    for (FDinosaurMemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.RememberedActor == Actor)
        {
            ExistingEntry = &Entry;
            break;
        }
    }
    
    if (ExistingEntry)
    {
        // Update existing memory
        ExistingEntry->LastKnownLocation = Actor->GetActorLocation();
        ExistingEntry->LastSeenTime = CurrentTime;
        ExistingEntry->EncounterCount++;
        
        // Adjust emotional value based on personality and previous encounters
        float PersonalityModifier = 1.0f;
        if (EmotionalValue > 0.0f)
        {
            PersonalityModifier = Personality.Sociability;
        }
        else if (EmotionalValue < 0.0f)
        {
            PersonalityModifier = Personality.Fearfulness;
        }
        
        ExistingEntry->EmotionalValue = FMath::Lerp(ExistingEntry->EmotionalValue, EmotionalValue, 0.1f * PersonalityModifier);
        ExistingEntry->EmotionalValue = FMath::Clamp(ExistingEntry->EmotionalValue, -1.0f, 1.0f);
    }
    else
    {
        // Create new memory entry
        FDinosaurMemoryEntry NewEntry;
        NewEntry.RememberedActor = Actor;
        NewEntry.LastKnownLocation = Actor->GetActorLocation();
        NewEntry.EmotionalValue = EmotionalValue * Personality.Intelligence; // Intelligence affects memory formation
        NewEntry.LastSeenTime = CurrentTime;
        NewEntry.EncounterCount = 1;
        
        MemoryEntries.Add(NewEntry);
        
        // Remove oldest memories if we exceed the limit
        if (MemoryEntries.Num() > MaxMemoryEntries)
        {
            MemoryEntries.RemoveAt(0);
        }
    }
    
    OnMemoryUpdated.Broadcast(Actor, EmotionalValue, ExistingEntry ? ExistingEntry->EncounterCount : 1);
}

FDinosaurMemoryEntry* UDinosaurBehaviorComponent::GetMemoryOfActor(AActor* Actor)
{
    if (!Actor) return nullptr;
    
    for (FDinosaurMemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.RememberedActor == Actor)
        {
            return &Entry;
        }
    }
    
    return nullptr;
}

void UDinosaurBehaviorComponent::ForgetActor(AActor* Actor)
{
    if (!Actor) return;
    
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        if (MemoryEntries[i].RememberedActor == Actor)
        {
            MemoryEntries.RemoveAt(i);
            break;
        }
    }
}

void UDinosaurBehaviorComponent::ModifyDomestication(float Amount)
{
    if (!CanBeDomesticated()) return;
    
    float OldLevel = DomesticationLevel;
    DomesticationLevel = FMath::Clamp(DomesticationLevel + Amount, 0.0f, 1.0f);
    
    // Personality affects domestication rate
    if (Amount > 0.0f)
    {
        DomesticationLevel *= (1.0f + Personality.Sociability * 0.5f);
        DomesticationLevel *= (1.0f - Personality.Fearfulness * 0.3f);
    }
    
    DomesticationLevel = FMath::Clamp(DomesticationLevel, 0.0f, 1.0f);
}

bool UDinosaurBehaviorComponent::CanBeDomesticated() const
{
    FDinosaurSpeciesData* SpeciesData = GetSpeciesData();
    if (!SpeciesData) return false;
    
    return SpeciesData->DomesticationPotential > 0.0f && 
           SpeciesData->ThreatLevel <= EDinosaurThreatLevel::Defensive &&
           SpeciesData->Size <= EDinosaurSize::Small;
}

FDinosaurSpeciesData* UDinosaurBehaviorComponent::GetSpeciesData() const
{
    if (!SpeciesDataHandle.IsValid()) return nullptr;
    
    return SpeciesDataHandle.GetRow<FDinosaurSpeciesData>(TEXT("Getting Species Data"));
}

EDinosaurActivity UDinosaurBehaviorComponent::GetScheduledActivity() const
{
    // Find the closest scheduled activity for current time
    float ClosestTime = 24.0f;
    EDinosaurActivity ClosestActivity = EDinosaurActivity::Resting;
    
    for (const auto& SchedulePair : DailySchedule.ScheduledActivities)
    {
        float TimeDiff = FMath::Abs(SchedulePair.Key - CurrentTime);
        if (TimeDiff < ClosestTime)
        {
            ClosestTime = TimeDiff;
            ClosestActivity = SchedulePair.Value;
        }
    }
    
    return ClosestActivity;
}

bool UDinosaurBehaviorComponent::ShouldBeActive() const
{
    FDinosaurSpeciesData* SpeciesData = GetSpeciesData();
    if (!SpeciesData) return true;
    
    // Check if current time is in active hours
    for (float ActiveHour : SpeciesData->ActiveHours)
    {
        if (FMath::Abs(CurrentTime - ActiveHour) < 1.0f) // Within 1 hour
        {
            return true;
        }
    }
    
    return false;
}

FVector UDinosaurBehaviorComponent::GetPreferredLocation() const
{
    switch (CurrentActivity)
    {
        case EDinosaurActivity::Foraging:
            if (DailySchedule.FeedingSpots.Num() > 0)
            {
                return DailySchedule.FeedingSpots[FMath::RandRange(0, DailySchedule.FeedingSpots.Num() - 1)];
            }
            break;
            
        case EDinosaurActivity::Drinking:
            if (DailySchedule.WaterSources.Num() > 0)
            {
                return DailySchedule.WaterSources[FMath::RandRange(0, DailySchedule.WaterSources.Num() - 1)];
            }
            break;
            
        case EDinosaurActivity::Patrolling:
            if (DailySchedule.PatrolPoints.Num() > 0)
            {
                return DailySchedule.PatrolPoints[FMath::RandRange(0, DailySchedule.PatrolPoints.Num() - 1)];
            }
            break;
            
        case EDinosaurActivity::Resting:
        case EDinosaurActivity::Sleeping:
            return DailySchedule.HomeLocation;
    }
    
    return DailySchedule.HomeLocation;
}

float UDinosaurBehaviorComponent::GetPersonalityModifier(const FString& TraitName) const
{
    if (TraitName == "Aggression") return Personality.Aggression;
    if (TraitName == "Curiosity") return Personality.Curiosity;
    if (TraitName == "Fearfulness") return Personality.Fearfulness;
    if (TraitName == "Sociability") return Personality.Sociability;
    if (TraitName == "Intelligence") return Personality.Intelligence;
    
    return 0.5f; // Default neutral value
}

void UDinosaurBehaviorComponent::ReactToPlayer(AActor* Player, float Distance)
{
    if (!Player) return;
    
    FDinosaurSpeciesData* SpeciesData = GetSpeciesData();
    if (!SpeciesData) return;
    
    // Calculate reaction based on species, personality, and domestication
    float ReactionIntensity = 1.0f - (Distance / SpeciesData->DetectionRange);
    ReactionIntensity = FMath::Clamp(ReactionIntensity, 0.0f, 1.0f);
    
    // Domesticated dinosaurs react more positively
    float EmotionalValue = -0.3f; // Default: player is a threat
    if (IsDomesticated())
    {
        EmotionalValue = 0.5f + (DomesticationLevel * 0.5f);
    }
    else if (SpeciesData->ThreatLevel == EDinosaurThreatLevel::Passive)
    {
        EmotionalValue = -0.1f; // Less threatening
    }
    
    // Personality modifies reaction
    EmotionalValue *= (1.0f - Personality.Fearfulness * 0.5f);
    EmotionalValue += (Personality.Curiosity * 0.2f);
    
    RememberActor(Player, EmotionalValue);
    
    // Set mood based on reaction
    if (EmotionalValue < -0.5f)
    {
        SetMood(EDinosaurMood::Fearful);
        SetActivity(EDinosaurActivity::Fleeing);
    }
    else if (EmotionalValue < 0.0f)
    {
        SetMood(EDinosaurMood::Alert);
    }
    else if (EmotionalValue > 0.3f)
    {
        SetMood(EDinosaurMood::Curious);
    }
}

void UDinosaurBehaviorComponent::ReactToThreat(AActor* Threat)
{
    if (!Threat) return;
    
    RememberActor(Threat, -0.8f);
    SetMood(EDinosaurMood::Fearful);
    SetActivity(EDinosaurActivity::Fleeing);
}

void UDinosaurBehaviorComponent::ReactToFood(AActor* Food)
{
    if (!Food) return;
    
    if (CurrentMood == EDinosaurMood::Hungry)
    {
        RememberActor(Food, 0.6f);
        SetActivity(EDinosaurActivity::Foraging);
    }
}

void UDinosaurBehaviorComponent::UpdateMemory(float DeltaTime)
{
    LastMemoryUpdate += DeltaTime;
    if (LastMemoryUpdate < 3600.0f) return; // Update every hour
    
    LastMemoryUpdate = 0.0f;
    
    // Decay memories over time
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        FDinosaurMemoryEntry& Entry = MemoryEntries[i];
        
        float TimeSinceLastSeen = CurrentTime - Entry.LastSeenTime;
        if (TimeSinceLastSeen > 24.0f) TimeSinceLastSeen -= 24.0f; // Handle day wrap
        
        // Decay emotional value towards neutral
        float DecayAmount = MemoryDecayRate * TimeSinceLastSeen * (1.0f - Personality.Intelligence);
        Entry.EmotionalValue = FMath::Lerp(Entry.EmotionalValue, 0.0f, DecayAmount);
        
        // Remove very old or neutral memories
        if (TimeSinceLastSeen > 72.0f || FMath::Abs(Entry.EmotionalValue) < 0.1f)
        {
            MemoryEntries.RemoveAt(i);
        }
    }
}

void UDinosaurBehaviorComponent::UpdateDomestication(float DeltaTime)
{
    if (!CanBeDomesticated()) return;
    
    LastDomesticationUpdate += DeltaTime;
    if (LastDomesticationUpdate < 60.0f) return; // Update every minute
    
    LastDomesticationUpdate = 0.0f;
    
    // Natural decay of domestication without positive interaction
    ModifyDomestication(-DomesticationLossRate);
}

void UDinosaurBehaviorComponent::UpdateDailyRoutine()
{
    EDinosaurActivity ScheduledActivity = GetScheduledActivity();
    
    // Only change activity if significantly different or if current activity is completed
    if (ScheduledActivity != CurrentActivity)
    {
        // Check if we should switch based on personality and current mood
        bool ShouldSwitch = true;
        
        if (CurrentMood == EDinosaurMood::Fearful || CurrentMood == EDinosaurMood::Aggressive)
        {
            ShouldSwitch = false; // Don't interrupt fear or aggression
        }
        
        if (ShouldSwitch)
        {
            SetActivity(ScheduledActivity);
        }
    }
}

void UDinosaurBehaviorComponent::GeneratePersonalityName()
{
    // Simple name generation based on personality traits
    TArray<FString> Prefixes = {
        TEXT("Bold"), TEXT("Shy"), TEXT("Wise"), TEXT("Quick"), TEXT("Gentle"),
        TEXT("Fierce"), TEXT("Calm"), TEXT("Wild"), TEXT("Noble"), TEXT("Swift")
    };
    
    TArray<FString> Suffixes = {
        TEXT("claw"), TEXT("horn"), TEXT("tail"), TEXT("eye"), TEXT("scale"),
        TEXT("tooth"), TEXT("wing"), TEXT("foot"), TEXT("back"), TEXT("neck")
    };
    
    FString Prefix = Prefixes[FMath::RandRange(0, Prefixes.Num() - 1)];
    FString Suffix = Suffixes[FMath::RandRange(0, Suffixes.Num() - 1)];
    
    Personality.IndividualName = FString::Printf(TEXT("%s%s"), *Prefix, *Suffix);
}

void UDinosaurBehaviorComponent::InitializeDailySchedule()
{
    FDinosaurSpeciesData* SpeciesData = GetSpeciesData();
    if (!SpeciesData) return;
    
    // Set home location to current location
    if (AActor* Owner = GetOwner())
    {
        DailySchedule.HomeLocation = Owner->GetActorLocation();
    }
    
    // Generate basic schedule based on species data
    DailySchedule.ScheduledActivities.Empty();
    
    // Add feeding times
    for (float FeedingHour : SpeciesData->FeedingHours)
    {
        DailySchedule.ScheduledActivities.Add(FeedingHour, EDinosaurActivity::Foraging);
    }
    
    // Add resting times
    for (float RestingHour : SpeciesData->RestingHours)
    {
        DailySchedule.ScheduledActivities.Add(RestingHour, EDinosaurActivity::Resting);
    }
    
    // Add some variety based on personality
    if (Personality.Curiosity > 0.6f)
    {
        DailySchedule.ScheduledActivities.Add(12.0f, EDinosaurActivity::Patrolling); // Midday exploration
    }
    
    if (Personality.Sociability > 0.7f)
    {
        DailySchedule.ScheduledActivities.Add(18.0f, EDinosaurActivity::Socializing); // Evening social time
    }
}