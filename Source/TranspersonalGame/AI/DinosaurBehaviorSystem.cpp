#include "DinosaurBehaviorSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

UDinosaurBehaviorSystem::UDinosaurBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick a cada 100ms para performance
    
    // Gerar nome único para o dinossauro
    if (IndividualName.IsEmpty())
    {
        int32 RandomSuffix = FMath::RandRange(1000, 9999);
        IndividualName = FString::Printf(TEXT("Dino_%d"), RandomSuffix);
    }
}

void UDinosaurBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar necessidades com base na espécie
    InitializeSpeciesDefaults();
    
    // Randomizar ligeiramente as necessidades iniciais para variação
    Needs.Hunger += FMath::RandRange(-10.0f, 10.0f);
    Needs.Thirst += FMath::RandRange(-10.0f, 10.0f);
    Needs.Energy += FMath::RandRange(-20.0f, 0.0f);
    
    // Clamp values
    Needs.Hunger = FMath::Clamp(Needs.Hunger, 0.0f, 100.0f);
    Needs.Thirst = FMath::Clamp(Needs.Thirst, 0.0f, 100.0f);
    Needs.Energy = FMath::Clamp(Needs.Energy, 0.0f, 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s (%s) initialized with Hunger: %.1f, Thirst: %.1f, Energy: %.1f"), 
           *IndividualName, 
           *UEnum::GetValueAsString(Species), 
           Needs.Hunger, 
           Needs.Thirst, 
           Needs.Energy);
}

void UDinosaurBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update needs gradually over time
    UpdateNeeds(DeltaTime);
    
    // Check daily schedule every 30 seconds
    if (GetWorld()->GetTimeSeconds() - LastScheduleCheckTime > 30.0f)
    {
        ProcessDailySchedule();
        LastScheduleCheckTime = GetWorld()->GetTimeSeconds();
    }
    
    // Determine new behavior state
    EDinosaurBehaviorState NewState = DetermineBehaviorState();
    if (NewState != CurrentState)
    {
        EDinosaurBehaviorState OldState = CurrentState;
        CurrentState = NewState;
        OnBehaviorStateChanged.Broadcast(OldState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed state from %s to %s"), 
               *IndividualName,
               *UEnum::GetValueAsString(OldState),
               *UEnum::GetValueAsString(NewState));
    }
    
    // Decay memories over time
    DecayMemories(DeltaTime);
}

void UDinosaurBehaviorSystem::UpdateNeeds(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update needs based on time passed and current activity
    float HungerDecayRate = 2.0f; // Points per hour
    float ThirstDecayRate = 3.0f;
    float EnergyDecayRate = 1.5f;
    
    // Modify decay rates based on current state
    switch (CurrentState)
    {
        case EDinosaurBehaviorState::Hunting:
        case EDinosaurBehaviorState::Fleeing:
            EnergyDecayRate *= 3.0f;
            ThirstDecayRate *= 2.0f;
            break;
        case EDinosaurBehaviorState::Resting:
            EnergyDecayRate *= -2.0f; // Recover energy while resting
            HungerDecayRate *= 0.5f;
            break;
        case EDinosaurBehaviorState::Foraging:
            HungerDecayRate *= -1.0f; // Reduce hunger while eating
            break;
        case EDinosaurBehaviorState::Drinking:
            ThirstDecayRate *= -3.0f; // Reduce thirst while drinking
            break;
    }
    
    // Apply personality modifiers
    HungerDecayRate = CalculatePersonalityModifier(PrimaryPersonality, HungerDecayRate);
    
    // Update needs
    float HourlyDelta = DeltaTime / 3600.0f; // Convert to hours
    
    Needs.Hunger = FMath::Clamp(Needs.Hunger + (HungerDecayRate * HourlyDelta), 0.0f, 100.0f);
    Needs.Thirst = FMath::Clamp(Needs.Thirst + (ThirstDecayRate * HourlyDelta), 0.0f, 100.0f);
    Needs.Energy = FMath::Clamp(Needs.Energy + (EnergyDecayRate * HourlyDelta), 0.0f, 100.0f);
    
    // Stress increases when needs are not met
    float StressIncrease = 0.0f;
    if (Needs.Hunger > 80.0f) StressIncrease += 5.0f * HourlyDelta;
    if (Needs.Thirst > 85.0f) StressIncrease += 8.0f * HourlyDelta;
    if (Needs.Energy < 20.0f) StressIncrease += 3.0f * HourlyDelta;
    
    Needs.Stress = FMath::Clamp(Needs.Stress + StressIncrease, 0.0f, 100.0f);
    
    // Stress naturally decays when needs are met
    if (Needs.Hunger < 50.0f && Needs.Thirst < 50.0f && Needs.Energy > 50.0f)
    {
        Needs.Stress = FMath::Clamp(Needs.Stress - (2.0f * HourlyDelta), 0.0f, 100.0f);
    }
}

void UDinosaurBehaviorSystem::ProcessDailySchedule()
{
    float CurrentTime = GetCurrentTimeOfDay();
    
    // Check if it's time for specific activities
    if (IsTimeForActivity(DailySchedule.SleepTime, 1.0f))
    {
        if (CurrentState != EDinosaurBehaviorState::Resting)
        {
            UE_LOG(LogTemp, Log, TEXT("Dinosaur %s: Time to sleep"), *IndividualName);
        }
    }
    else if (IsTimeForActivity(DailySchedule.WakeUpTime, 0.5f))
    {
        if (CurrentState == EDinosaurBehaviorState::Resting)
        {
            UE_LOG(LogTemp, Log, TEXT("Dinosaur %s: Time to wake up"), *IndividualName);
        }
    }
    
    // Check feeding times
    for (float FeedingTime : DailySchedule.FeedingTimes)
    {
        if (IsTimeForActivity(FeedingTime, 0.5f))
        {
            if (Needs.Hunger > 30.0f) // Only if actually hungry
            {
                UE_LOG(LogTemp, Log, TEXT("Dinosaur %s: Scheduled feeding time"), *IndividualName);
            }
        }
    }
}

EDinosaurBehaviorState UDinosaurBehaviorSystem::DetermineBehaviorState()
{
    float CurrentTime = GetCurrentTimeOfDay();
    
    // Emergency states take priority
    if (Needs.Stress > 80.0f)
    {
        return EDinosaurBehaviorState::Fleeing;
    }
    
    // Critical needs
    if (Needs.Thirst > 90.0f)
    {
        return EDinosaurBehaviorState::Drinking;
    }
    
    if (Needs.Hunger > 85.0f)
    {
        if (IsHerbivore())
        {
            return EDinosaurBehaviorState::Foraging;
        }
        else
        {
            return EDinosaurBehaviorState::Hunting;
        }
    }
    
    // Sleep schedule
    if (IsTimeForActivity(DailySchedule.SleepTime, 2.0f) || Needs.Energy < 15.0f)
    {
        return EDinosaurBehaviorState::Resting;
    }
    
    // Social needs for pack animals
    if (IsSocialSpecies() && Needs.Social > 70.0f)
    {
        return EDinosaurBehaviorState::Socializing;
    }
    
    // Moderate needs
    if (Needs.Thirst > 60.0f)
    {
        return EDinosaurBehaviorState::Drinking;
    }
    
    if (Needs.Hunger > 60.0f)
    {
        return IsHerbivore() ? EDinosaurBehaviorState::Foraging : EDinosaurBehaviorState::Hunting;
    }
    
    // Personality-driven behaviors
    if (PrimaryPersonality == EDinosaurPersonality::Curious)
    {
        return EDinosaurBehaviorState::Investigating;
    }
    
    if (PrimaryPersonality == EDinosaurPersonality::Territorial)
    {
        return EDinosaurBehaviorState::Territorial;
    }
    
    // Default idle behavior
    return EDinosaurBehaviorState::Idle;
}

void UDinosaurBehaviorSystem::ReactToPlayer(AActor* Player, float Distance, bool bPlayerVisible)
{
    if (!Player) return;
    
    float PreviousTrust = Memory.PlayerTrustLevel;
    
    // Base reaction depends on species and personality
    float ReactionModifier = 0.0f;
    
    // Distance affects reaction
    if (Distance < 200.0f) // Very close
    {
        if (Memory.PlayerTrustLevel < 0.0f)
        {
            ReactionModifier -= 2.0f; // Increase fear/aggression
            Needs.Stress = FMath::Min(Needs.Stress + 10.0f, 100.0f);
        }
        else if (Memory.PlayerTrustLevel > 50.0f)
        {
            ReactionModifier += 1.0f; // Friendly approach
        }
    }
    
    // Personality modifiers
    switch (PrimaryPersonality)
    {
        case EDinosaurPersonality::Timid:
            ReactionModifier -= 1.0f;
            break;
        case EDinosaurPersonality::Curious:
            ReactionModifier += 0.5f;
            break;
        case EDinosaurPersonality::Aggressive:
            if (Memory.PlayerTrustLevel < 25.0f)
                ReactionModifier -= 2.0f;
            break;
    }
    
    // Update memory
    Memory.PlayerInteractions++;
    Memory.LastPlayerEncounterTime = GetWorld()->GetTimeSeconds();
    
    // Apply trust change
    Memory.PlayerTrustLevel = FMath::Clamp(Memory.PlayerTrustLevel + ReactionModifier, -100.0f, 100.0f);
    
    // Broadcast trust level change if significant
    if (FMath::Abs(Memory.PlayerTrustLevel - PreviousTrust) > 1.0f)
    {
        OnDomesticationLevelChanged.Broadcast(Memory.PlayerTrustLevel);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s reacted to player. Distance: %.1f, Trust: %.1f -> %.1f"), 
           *IndividualName, Distance, PreviousTrust, Memory.PlayerTrustLevel);
}

void UDinosaurBehaviorSystem::LearnFromExperience(AActor* Target, bool bPositiveExperience, float Intensity)
{
    if (!Target) return;
    
    float ExperienceValue = bPositiveExperience ? Intensity : -Intensity;
    
    // Update relationship memory
    if (Memory.KnownCreatures.Contains(Target))
    {
        float CurrentRelationship = Memory.KnownCreatures[Target];
        Memory.KnownCreatures[Target] = FMath::Clamp(CurrentRelationship + ExperienceValue, -100.0f, 100.0f);
    }
    else
    {
        Memory.KnownCreatures.Add(Target, ExperienceValue);
    }
    
    // If target is player, update trust
    APlayerController* PC = Cast<APlayerController>(Target);
    if (PC || Target->IsA<APawn>())
    {
        float PreviousTrust = Memory.PlayerTrustLevel;
        Memory.PlayerTrustLevel = FMath::Clamp(Memory.PlayerTrustLevel + ExperienceValue, -100.0f, 100.0f);
        
        if (FMath::Abs(Memory.PlayerTrustLevel - PreviousTrust) > 0.5f)
        {
            OnDomesticationLevelChanged.Broadcast(Memory.PlayerTrustLevel);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s learned from experience with %s. Positive: %s, Intensity: %.1f"), 
           *IndividualName, 
           Target ? *Target->GetName() : TEXT("Unknown"),
           bPositiveExperience ? TEXT("Yes") : TEXT("No"), 
           Intensity);
}

bool UDinosaurBehaviorSystem::IsDomesticated() const
{
    return bCanBeDomesticated && Memory.PlayerTrustLevel >= DomesticationThreshold;
}

FVector UDinosaurBehaviorSystem::FindNearestResourceLocation(const TArray<FVector>& ResourceLocations) const
{
    if (ResourceLocations.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector NearestLocation = ResourceLocations[0];
    float NearestDistance = FVector::Dist(OwnerLocation, NearestLocation);
    
    for (int32 i = 1; i < ResourceLocations.Num(); i++)
    {
        float Distance = FVector::Dist(OwnerLocation, ResourceLocations[i]);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestLocation = ResourceLocations[i];
        }
    }
    
    return NearestLocation;
}

float UDinosaurBehaviorSystem::GetCurrentTimeOfDay() const
{
    // This should be connected to the game's day/night cycle
    // For now, using a simple time-based calculation
    float GameTime = GetWorld()->GetTimeSeconds();
    float DayLength = 1200.0f; // 20 minutes = 1 day
    float TimeOfDay = FMath::Fmod(GameTime, DayLength) / DayLength * 24.0f;
    return TimeOfDay;
}

bool UDinosaurBehaviorSystem::IsTimeForActivity(float ScheduledTime, float Tolerance) const
{
    float CurrentTime = GetCurrentTimeOfDay();
    float TimeDiff = FMath::Abs(CurrentTime - ScheduledTime);
    
    // Handle wrap-around (e.g., 23.5 to 0.5)
    if (TimeDiff > 12.0f)
    {
        TimeDiff = 24.0f - TimeDiff;
    }
    
    return TimeDiff <= Tolerance;
}

void UDinosaurBehaviorSystem::DecayMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MemoryDecayRate = 1.0f / (MemoryRetentionDays * 86400.0f); // Per second
    
    // Decay player trust slowly if no recent interactions
    if (CurrentTime - Memory.LastPlayerEncounterTime > 3600.0f) // 1 hour
    {
        float DecayAmount = MemoryDecayRate * DeltaTime * 10.0f; // Faster decay
        if (Memory.PlayerTrustLevel > 0.0f)
        {
            Memory.PlayerTrustLevel = FMath::Max(Memory.PlayerTrustLevel - DecayAmount, 0.0f);
        }
        else if (Memory.PlayerTrustLevel < 0.0f)
        {
            Memory.PlayerTrustLevel = FMath::Min(Memory.PlayerTrustLevel + DecayAmount, 0.0f);
        }
    }
}

float UDinosaurBehaviorSystem::CalculatePersonalityModifier(EDinosaurPersonality Personality, float BaseValue) const
{
    float Modifier = 1.0f;
    
    switch (Personality)
    {
        case EDinosaurPersonality::Aggressive:
            if (BaseValue > 0) Modifier = 1.2f; // More intense positive reactions
            else Modifier = 1.5f; // Much more intense negative reactions
            break;
        case EDinosaurPersonality::Timid:
            Modifier = 0.7f; // Less intense overall
            break;
        case EDinosaurPersonality::Curious:
            Modifier = 1.1f; // Slightly more responsive
            break;
        case EDinosaurPersonality::Social:
            if (CurrentState == EDinosaurBehaviorState::Socializing) Modifier = 1.3f;
            break;
    }
    
    return BaseValue * Modifier * PersonalityInfluence;
}

void UDinosaurBehaviorSystem::InitializeSpeciesDefaults()
{
    // Set species-specific defaults
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
            DailySchedule.WakeUpTime = 5.0f;
            DailySchedule.SleepTime = 22.0f;
            TerritoryRadius = 300.0f;
            bCanBeDomesticated = true;
            DomesticationThreshold = 60.0f;
            break;
            
        case EDinosaurSpecies::TyrannosaurusRex:
            DailySchedule.WakeUpTime = 7.0f;
            DailySchedule.SleepTime = 20.0f;
            TerritoryRadius = 2000.0f;
            bCanBeDomesticated = false;
            break;
            
        case EDinosaurSpecies::Triceratops:
            DailySchedule.WakeUpTime = 6.0f;
            DailySchedule.SleepTime = 21.0f;
            TerritoryRadius = 800.0f;
            bCanBeDomesticated = true;
            DomesticationThreshold = 80.0f;
            break;
            
        // Add more species configurations...
    }
}

bool UDinosaurBehaviorSystem::IsHerbivore() const
{
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
        case EDinosaurSpecies::Pachycephalosaurus:
        case EDinosaurSpecies::Parasaurolophus:
        case EDinosaurSpecies::Triceratops:
        case EDinosaurSpecies::Stegosaurus:
        case EDinosaurSpecies::Ankylosaurus:
        case EDinosaurSpecies::Brachiosaurus:
        case EDinosaurSpecies::Diplodocus:
            return true;
        default:
            return false;
    }
}

bool UDinosaurBehaviorSystem::IsSocialSpecies() const
{
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
        case EDinosaurSpecies::Parasaurolophus:
        case EDinosaurSpecies::Velociraptor:
        case EDinosaurSpecies::Triceratops:
            return true;
        default:
            return false;
    }
}