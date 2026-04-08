#include "DinosaurBehaviorComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UDinosaurBehaviorComponent::UDinosaurBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Default values
    DinosaurName = TEXT("Unnamed Dinosaur");
    PrimaryPersonality = EDinosaurPersonality::Cautious;
    SecondaryPersonality = EDinosaurPersonality::Curious;
    PersonalityStrength = 0.7f;
    
    CurrentState = EDinosaurState::Idle;
    DomesticationLevel = EDomesticationLevel::Wild;
    
    MemoryDuration = 300.0f; // 5 minutes
    MaxMemoryEntries = 20;
    
    bCanBeDomesticated = false;
    DomesticationProgress = 0.0f;
    DomesticationDecayRate = 1.0f; // 1% per minute
    BondedPlayer = nullptr;
    
    LastNeedsUpdate = 0.0f;
    LastMemoryUpdate = 0.0f;
    LastRoutineUpdate = 0.0f;
}

void UDinosaurBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePersonality();
    InitializeDailyRoutines();
    
    // Find blackboard component
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AController* Controller = OwnerPawn->GetController())
        {
            BlackboardComponent = Controller->FindComponentByClass<UBlackboardComponent>();
        }
    }
}

void UDinosaurBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update needs every 5 seconds
    if (CurrentTime - LastNeedsUpdate >= 5.0f)
    {
        UpdateNeeds(CurrentTime - LastNeedsUpdate);
        LastNeedsUpdate = CurrentTime;
    }
    
    // Update memory every 2 seconds
    if (CurrentTime - LastMemoryUpdate >= 2.0f)
    {
        UpdateMemory(CurrentTime - LastMemoryUpdate);
        LastMemoryUpdate = CurrentTime;
    }
    
    // Update routine every 30 seconds
    if (CurrentTime - LastRoutineUpdate >= 30.0f)
    {
        UpdateDailyRoutine();
        LastRoutineUpdate = CurrentTime;
    }
    
    // Decay domestication
    if (bCanBeDomesticated && DomesticationProgress > 0.0f)
    {
        DecayDomestication(DeltaTime);
    }
    
    // Update blackboard
    UpdateBlackboard();
}

void UDinosaurBehaviorComponent::UpdateNeeds(float DeltaTime)
{
    float TimeMultiplier = DeltaTime / 60.0f; // Per minute
    
    // Hunger increases over time
    CurrentNeeds.Hunger = FMath::Clamp(CurrentNeeds.Hunger + (2.0f * TimeMultiplier), 0.0f, 100.0f);
    
    // Thirst increases faster than hunger
    CurrentNeeds.Thirst = FMath::Clamp(CurrentNeeds.Thirst + (3.0f * TimeMultiplier), 0.0f, 100.0f);
    
    // Energy decreases during day, increases during rest
    float TimeOfDay = GetCurrentTimeOfDay();
    bool bIsNight = (TimeOfDay < 6.0f || TimeOfDay > 20.0f);
    
    if (CurrentState == EDinosaurState::Sleeping || CurrentState == EDinosaurState::Resting)
    {
        CurrentNeeds.Energy = FMath::Clamp(CurrentNeeds.Energy + (10.0f * TimeMultiplier), 0.0f, 100.0f);
    }
    else
    {
        float EnergyDecay = bIsNight ? 1.0f : 2.0f;
        CurrentNeeds.Energy = FMath::Clamp(CurrentNeeds.Energy - (EnergyDecay * TimeMultiplier), 0.0f, 100.0f);
    }
    
    // Social needs depend on personality
    if (PrimaryPersonality == EDinosaurPersonality::Social)
    {
        if (CurrentState != EDinosaurState::Socializing)
        {
            CurrentNeeds.Social = FMath::Clamp(CurrentNeeds.Social + (1.5f * TimeMultiplier), 0.0f, 100.0f);
        }
    }
    else if (PrimaryPersonality == EDinosaurPersonality::Solitary)
    {
        if (CurrentState == EDinosaurState::Socializing)
        {
            CurrentNeeds.Social = FMath::Clamp(CurrentNeeds.Social + (1.0f * TimeMultiplier), 0.0f, 100.0f);
        }
        else
        {
            CurrentNeeds.Social = FMath::Clamp(CurrentNeeds.Social - (0.5f * TimeMultiplier), 0.0f, 100.0f);
        }
    }
    
    // Safety decreases when threats are nearby
    bool bThreatsNearby = false;
    for (const FDinosaurMemoryEntry& Memory : MemoryEntries)
    {
        if (Memory.ThreatLevel > 0.5f && GetWorld()->GetTimeSeconds() - Memory.LastSeenTime < 60.0f)
        {
            bThreatsNearby = true;
            break;
        }
    }
    
    if (bThreatsNearby)
    {
        CurrentNeeds.Safety = FMath::Clamp(CurrentNeeds.Safety + (5.0f * TimeMultiplier), 0.0f, 100.0f);
    }
    else
    {
        CurrentNeeds.Safety = FMath::Clamp(CurrentNeeds.Safety - (1.0f * TimeMultiplier), 0.0f, 100.0f);
    }
}

void UDinosaurBehaviorComponent::UpdateMemory(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove expired memories
    MemoryEntries.RemoveAll([this, CurrentTime](const FDinosaurMemoryEntry& Entry)
    {
        return CurrentTime - Entry.LastSeenTime > MemoryDuration;
    });
    
    // Limit memory entries
    if (MemoryEntries.Num() > MaxMemoryEntries)
    {
        // Remove oldest entries
        MemoryEntries.Sort([](const FDinosaurMemoryEntry& A, const FDinosaurMemoryEntry& B)
        {
            return A.LastSeenTime > B.LastSeenTime;
        });
        
        MemoryEntries.SetNum(MaxMemoryEntries);
    }
}

void UDinosaurBehaviorComponent::UpdateDailyRoutine()
{
    float CurrentTime = GetCurrentTimeOfDay();
    
    // Find the best routine for current time
    FDailyRoutine* BestRoutine = nullptr;
    float BestPriority = 0.0f;
    
    for (FDailyRoutine& Routine : DailyRoutines)
    {
        bool bInTimeRange = false;
        
        if (Routine.StartTime <= Routine.EndTime)
        {
            // Normal time range (e.g., 8:00 to 18:00)
            bInTimeRange = (CurrentTime >= Routine.StartTime && CurrentTime <= Routine.EndTime);
        }
        else
        {
            // Overnight range (e.g., 22:00 to 6:00)
            bInTimeRange = (CurrentTime >= Routine.StartTime || CurrentTime <= Routine.EndTime);
        }
        
        if (bInTimeRange && Routine.Priority > BestPriority)
        {
            BestRoutine = &Routine;
            BestPriority = Routine.Priority;
        }
    }
    
    if (BestRoutine)
    {
        CurrentRoutine = *BestRoutine;
        
        // Adjust routine based on needs
        if (CurrentNeeds.Hunger > 70.0f && CurrentRoutine.Activity != EDinosaurState::Foraging)
        {
            CurrentRoutine.Activity = EDinosaurState::Foraging;
            CurrentRoutine.Priority += 2.0f;
        }
        else if (CurrentNeeds.Thirst > 70.0f && CurrentRoutine.Activity != EDinosaurState::Drinking)
        {
            CurrentRoutine.Activity = EDinosaurState::Drinking;
            CurrentRoutine.Priority += 2.0f;
        }
        else if (CurrentNeeds.Energy < 20.0f && CurrentRoutine.Activity != EDinosaurState::Resting)
        {
            CurrentRoutine.Activity = EDinosaurState::Resting;
            CurrentRoutine.Priority += 1.5f;
        }
    }
}

void UDinosaurBehaviorComponent::AddMemoryEntry(AActor* Actor, float ThreatLevel, float FamiliarityLevel)
{
    if (!Actor) return;
    
    // Check if we already have a memory of this actor
    FDinosaurMemoryEntry* ExistingEntry = GetMemoryEntry(Actor);
    
    if (ExistingEntry)
    {
        // Update existing memory
        ExistingEntry->LastKnownLocation = Actor->GetActorLocation();
        ExistingEntry->ThreatLevel = FMath::Lerp(ExistingEntry->ThreatLevel, ThreatLevel, 0.3f);
        ExistingEntry->FamiliaryLevel = FMath::Lerp(ExistingEntry->FamiliaryLevel, FamiliarityLevel, 0.2f);
        ExistingEntry->LastSeenTime = GetWorld()->GetTimeSeconds();
    }
    else
    {
        // Create new memory
        FDinosaurMemoryEntry NewEntry;
        NewEntry.Actor = Actor;
        NewEntry.LastKnownLocation = Actor->GetActorLocation();
        NewEntry.ThreatLevel = ThreatLevel;
        NewEntry.FamiliaryLevel = FamiliarityLevel;
        NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewEntry.bIsPlayer = Actor->IsA<APawn>() && Cast<APawn>(Actor)->IsPlayerControlled();
        
        MemoryEntries.Add(NewEntry);
    }
}

FDinosaurMemoryEntry* UDinosaurBehaviorComponent::GetMemoryEntry(AActor* Actor)
{
    if (!Actor) return nullptr;
    
    for (FDinosaurMemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.Actor == Actor)
        {
            return &Entry;
        }
    }
    
    return nullptr;
}

void UDinosaurBehaviorComponent::InteractWithPlayer(AActor* Player, float InteractionStrength)
{
    if (!Player || !bCanBeDomesticated) return;
    
    FDinosaurMemoryEntry* PlayerMemory = GetMemoryEntry(Player);
    
    if (PlayerMemory)
    {
        // Positive interaction increases familiarity and decreases threat
        PlayerMemory->FamiliaryLevel = FMath::Clamp(PlayerMemory->FamiliaryLevel + InteractionStrength, 0.0f, 100.0f);
        PlayerMemory->ThreatLevel = FMath::Clamp(PlayerMemory->ThreatLevel - (InteractionStrength * 0.5f), 0.0f, 100.0f);
        
        // Increase domestication progress based on familiarity
        if (PlayerMemory->FamiliaryLevel > 50.0f)
        {
            float DomesticationGain = InteractionStrength * (PlayerMemory->FamiliaryLevel / 100.0f);
            DomesticationProgress = FMath::Clamp(DomesticationProgress + DomesticationGain, 0.0f, 100.0f);
            
            // Update domestication level
            if (DomesticationProgress >= 90.0f)
            {
                DomesticationLevel = EDomesticationLevel::Domesticated;
                BondedPlayer = Player;
            }
            else if (DomesticationProgress >= 75.0f)
            {
                DomesticationLevel = EDomesticationLevel::Bonded;
            }
            else if (DomesticationProgress >= 60.0f)
            {
                DomesticationLevel = EDomesticationLevel::Friendly;
            }
            else if (DomesticationProgress >= 40.0f)
            {
                DomesticationLevel = EDomesticationLevel::Curious;
            }
            else if (DomesticationProgress >= 20.0f)
            {
                DomesticationLevel = EDomesticationLevel::Neutral;
            }
            else
            {
                DomesticationLevel = EDomesticationLevel::Wary;
            }
        }
    }
}

bool UDinosaurBehaviorComponent::ShouldFleeFrom(AActor* Threat)
{
    if (!Threat) return false;
    
    FDinosaurMemoryEntry* ThreatMemory = GetMemoryEntry(Threat);
    
    if (!ThreatMemory) return false;
    
    // Base flee threshold depends on personality
    float FleeThreshold = 50.0f;
    
    if (PrimaryPersonality == EDinosaurPersonality::Aggressive)
    {
        FleeThreshold = 80.0f;
    }
    else if (PrimaryPersonality == EDinosaurPersonality::Cautious)
    {
        FleeThreshold = 30.0f;
    }
    
    // Adjust for safety needs
    FleeThreshold -= (CurrentNeeds.Safety * 0.3f);
    
    // Adjust for familiarity (familiar threats are less scary)
    FleeThreshold += (ThreatMemory->FamiliaryLevel * 0.2f);
    
    return ThreatMemory->ThreatLevel > FleeThreshold;
}

bool UDinosaurBehaviorComponent::ShouldInvestigate(AActor* Target)
{
    if (!Target) return false;
    
    // Curious dinosaurs investigate more
    if (PrimaryPersonality == EDinosaurPersonality::Curious || SecondaryPersonality == EDinosaurPersonality::Curious)
    {
        return FMath::RandRange(0.0f, 1.0f) < 0.7f;
    }
    
    // Others investigate based on safety
    return CurrentNeeds.Safety < 30.0f && FMath::RandRange(0.0f, 1.0f) < 0.3f;
}

FVector UDinosaurBehaviorComponent::GetPreferredLocation()
{
    if (CurrentRoutine.PreferredLocation != FVector::ZeroVector)
    {
        return CurrentRoutine.PreferredLocation;
    }
    
    // Default to current location if no preferred location set
    return GetOwner()->GetActorLocation();
}

void UDinosaurBehaviorComponent::SetCurrentState(EDinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        // Update needs based on state change
        switch (NewState)
        {
            case EDinosaurState::Foraging:
                CurrentNeeds.Hunger = FMath::Clamp(CurrentNeeds.Hunger - 20.0f, 0.0f, 100.0f);
                break;
            case EDinosaurState::Drinking:
                CurrentNeeds.Thirst = FMath::Clamp(CurrentNeeds.Thirst - 25.0f, 0.0f, 100.0f);
                break;
            case EDinosaurState::Socializing:
                if (PrimaryPersonality == EDinosaurPersonality::Social)
                {
                    CurrentNeeds.Social = FMath::Clamp(CurrentNeeds.Social - 15.0f, 0.0f, 100.0f);
                }
                break;
            case EDinosaurState::Resting:
            case EDinosaurState::Sleeping:
                CurrentNeeds.Energy = FMath::Clamp(CurrentNeeds.Energy + 10.0f, 0.0f, 100.0f);
                break;
        }
    }
}

void UDinosaurBehaviorComponent::UpdateBlackboard()
{
    if (!BlackboardComponent) return;
    
    // Update blackboard keys for behavior tree
    BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), static_cast<uint8>(CurrentState));
    BlackboardComponent->SetValueAsEnum(TEXT("DomesticationLevel"), static_cast<uint8>(DomesticationLevel));
    BlackboardComponent->SetValueAsFloat(TEXT("HungerLevel"), CurrentNeeds.Hunger);
    BlackboardComponent->SetValueAsFloat(TEXT("ThirstLevel"), CurrentNeeds.Thirst);
    BlackboardComponent->SetValueAsFloat(TEXT("EnergyLevel"), CurrentNeeds.Energy);
    BlackboardComponent->SetValueAsFloat(TEXT("SafetyLevel"), CurrentNeeds.Safety);
    BlackboardComponent->SetValueAsVector(TEXT("PreferredLocation"), GetPreferredLocation());
    
    // Find most threatening actor
    AActor* MostThreatening = nullptr;
    float HighestThreat = 0.0f;
    
    for (const FDinosaurMemoryEntry& Memory : MemoryEntries)
    {
        if (Memory.ThreatLevel > HighestThreat && GetWorld()->GetTimeSeconds() - Memory.LastSeenTime < 30.0f)
        {
            MostThreatening = Memory.Actor;
            HighestThreat = Memory.ThreatLevel;
        }
    }
    
    BlackboardComponent->SetValueAsObject(TEXT("ThreatTarget"), MostThreatening);
    BlackboardComponent->SetValueAsFloat(TEXT("ThreatLevel"), HighestThreat);
    
    // Set player reference if bonded
    if (BondedPlayer)
    {
        BlackboardComponent->SetValueAsObject(TEXT("BondedPlayer"), BondedPlayer);
    }
}

void UDinosaurBehaviorComponent::InitializePersonality()
{
    // Generate random personality traits if not set
    if (PrimaryPersonality == EDinosaurPersonality::Cautious && SecondaryPersonality == EDinosaurPersonality::Curious)
    {
        // Random primary personality
        int32 PersonalityIndex = FMath::RandRange(0, 7);
        PrimaryPersonality = static_cast<EDinosaurPersonality>(PersonalityIndex);
        
        // Random secondary personality (different from primary)
        do {
            PersonalityIndex = FMath::RandRange(0, 7);
            SecondaryPersonality = static_cast<EDinosaurPersonality>(PersonalityIndex);
        } while (SecondaryPersonality == PrimaryPersonality);
        
        PersonalityStrength = FMath::RandRange(0.3f, 0.9f);
    }
    
    // Adjust base needs based on personality
    if (PrimaryPersonality == EDinosaurPersonality::Aggressive)
    {
        CurrentNeeds.Safety = 20.0f; // Less concerned about safety
    }
    else if (PrimaryPersonality == EDinosaurPersonality::Cautious)
    {
        CurrentNeeds.Safety = 80.0f; // Very concerned about safety
    }
    
    if (PrimaryPersonality == EDinosaurPersonality::Social)
    {
        CurrentNeeds.Social = 70.0f; // High social needs
    }
    else if (PrimaryPersonality == EDinosaurPersonality::Solitary)
    {
        CurrentNeeds.Social = 10.0f; // Low social needs
    }
}

void UDinosaurBehaviorComponent::InitializeDailyRoutines()
{
    if (DailyRoutines.Num() == 0)
    {
        // Create default routines based on personality
        FDailyRoutine MorningForage;
        MorningForage.StartTime = 6.0f;
        MorningForage.EndTime = 10.0f;
        MorningForage.Activity = EDinosaurState::Foraging;
        MorningForage.Priority = 2.0f;
        DailyRoutines.Add(MorningForage);
        
        FDailyRoutine MidDayRest;
        MidDayRest.StartTime = 12.0f;
        MidDayRest.EndTime = 15.0f;
        MidDayRest.Activity = EDinosaurState::Resting;
        MidDayRest.Priority = 1.5f;
        DailyRoutines.Add(MidDayRest);
        
        FDailyRoutine EveningDrink;
        EveningDrink.StartTime = 17.0f;
        EveningDrink.EndTime = 19.0f;
        EveningDrink.Activity = EDinosaurState::Drinking;
        EveningDrink.Priority = 2.0f;
        DailyRoutines.Add(EveningDrink);
        
        FDailyRoutine NightSleep;
        NightSleep.StartTime = 22.0f;
        NightSleep.EndTime = 5.0f;
        NightSleep.Activity = EDinosaurState::Sleeping;
        NightSleep.Priority = 3.0f;
        DailyRoutines.Add(NightSleep);
    }
}

float UDinosaurBehaviorComponent::GetCurrentTimeOfDay()
{
    // Get time from world (this would need to be connected to day/night cycle)
    // For now, use a simple calculation based on world time
    float WorldTime = GetWorld()->GetTimeSeconds();
    float DayLength = 1200.0f; // 20 minutes per day
    float TimeOfDay = FMath::Fmod(WorldTime, DayLength) / DayLength * 24.0f;
    
    return TimeOfDay;
}

void UDinosaurBehaviorComponent::DecayDomestication(float DeltaTime)
{
    if (BondedPlayer)
    {
        // Check if bonded player is nearby
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), BondedPlayer->GetActorLocation());
        
        if (DistanceToPlayer > 5000.0f) // 50 meters
        {
            // Player is far away, decay domestication slowly
            float DecayAmount = DomesticationDecayRate * (DeltaTime / 60.0f) * 0.1f;
            DomesticationProgress = FMath::Clamp(DomesticationProgress - DecayAmount, 0.0f, 100.0f);
        }
    }
    else
    {
        // No bonded player, decay faster
        float DecayAmount = DomesticationDecayRate * (DeltaTime / 60.0f);
        DomesticationProgress = FMath::Clamp(DomesticationProgress - DecayAmount, 0.0f, 100.0f);
    }
    
    // Update domestication level based on progress
    if (DomesticationProgress < 20.0f)
    {
        DomesticationLevel = EDomesticationLevel::Wild;
        BondedPlayer = nullptr;
    }
    else if (DomesticationProgress < 40.0f)
    {
        DomesticationLevel = EDomesticationLevel::Wary;
    }
    else if (DomesticationProgress < 60.0f)
    {
        DomesticationLevel = EDomesticationLevel::Neutral;
    }
}