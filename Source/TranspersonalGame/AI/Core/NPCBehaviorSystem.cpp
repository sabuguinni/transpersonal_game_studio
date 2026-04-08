#include "NPCBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UNPCBehaviorSystem::UNPCBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second for performance
}

void UNPCBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize daily routine if empty
    if (DailyRoutine.ScheduledActivities.Num() == 0)
    {
        // Create a basic routine based on diet and personality
        GenerateDefaultRoutine();
    }
    
    // Set initial activity
    UpdateCurrentActivity();
}

void UNPCBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateNeeds(DeltaTime);
    UpdateMemoryDecay(DeltaTime);
    UpdateCurrentActivity();
    UpdateDomesticationLevel();
}

void UNPCBehaviorSystem::UpdateCurrentActivity()
{
    // Check if urgent needs override routine
    if (CurrentHunger > 0.8f)
    {
        CurrentActivity = (Diet == EDinosaurDiet::Carnivore) ? EDinosaurActivity::Hunting : EDinosaurActivity::Foraging;
        return;
    }
    
    if (CurrentThirst > 0.8f)
    {
        CurrentActivity = EDinosaurActivity::Drinking;
        return;
    }
    
    if (CurrentStress > 0.7f)
    {
        CurrentActivity = EDinosaurActivity::Fleeing;
        return;
    }
    
    // Follow daily routine
    EDinosaurActivity RoutineActivity = DetermineActivityFromTime();
    if (RoutineActivity != CurrentActivity)
    {
        // Personality affects how likely they are to change activities
        float ChangeChance = 0.1f + (Personality.Curiosity * 0.3f);
        if (FMath::RandRange(0.0f, 1.0f) < ChangeChance)
        {
            CurrentActivity = RoutineActivity;
        }
    }
}

void UNPCBehaviorSystem::ProcessPlayerInteraction(bool bPositive, float Intensity)
{
    if (!bCanBeDomesticated) return;
    
    float TrustChange = bPositive ? (DomesticationRate * Intensity) : (-DomesticationRate * Intensity * 2.0f);
    
    // Personality affects trust building
    if (bPositive)
    {
        TrustChange *= (1.0f + Personality.Curiosity - Personality.Fearfulness);
        Memory.PositiveInteractions++;
    }
    else
    {
        TrustChange *= (1.0f + Personality.Fearfulness);
        Memory.NegativeInteractions++;
        UpdateStressLevel(0.3f * Intensity);
    }
    
    Memory.PlayerTrustLevel = FMath::Clamp(Memory.PlayerTrustLevel + TrustChange, -1.0f, 1.0f);
    Memory.LastPlayerEncounter = FDateTime::Now();
    
    // Store player location
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        Memory.LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
    }
}

void UNPCBehaviorSystem::UpdateDomesticationLevel()
{
    if (!bCanBeDomesticated) return;
    
    EDomesticationStage NewLevel = DomesticationLevel;
    
    if (Memory.PlayerTrustLevel >= 0.8f)
        NewLevel = EDomesticationStage::Companion;
    else if (Memory.PlayerTrustLevel >= 0.6f)
        NewLevel = EDomesticationStage::Bonded;
    else if (Memory.PlayerTrustLevel >= 0.4f)
        NewLevel = EDomesticationStage::Friendly;
    else if (Memory.PlayerTrustLevel >= 0.2f)
        NewLevel = EDomesticationStage::Tolerant;
    else if (Memory.PlayerTrustLevel >= 0.0f)
        NewLevel = EDomesticationStage::Curious;
    else if (Memory.PlayerTrustLevel >= -0.3f)
        NewLevel = EDomesticationStage::Wary;
    else
        NewLevel = EDomesticationStage::Wild;
    
    DomesticationLevel = NewLevel;
}

bool UNPCBehaviorSystem::ShouldFleeFromPlayer() const
{
    // Base flee chance based on threat level and personality
    float FleeChance = 0.0f;
    
    switch (ThreatLevel)
    {
        case EDinosaurThreatLevel::Passive:
            FleeChance = 0.8f;
            break;
        case EDinosaurThreatLevel::Defensive:
            FleeChance = 0.6f;
            break;
        case EDinosaurThreatLevel::Territorial:
            FleeChance = 0.3f;
            break;
        case EDinosaurThreatLevel::Aggressive:
            FleeChance = 0.1f;
            break;
        case EDinosaurThreatLevel::Apex:
            FleeChance = 0.0f;
            break;
    }
    
    // Modify by personality and domestication
    FleeChance += Personality.Fearfulness * 0.5f;
    FleeChance -= (Memory.PlayerTrustLevel + 1.0f) * 0.3f; // Trust reduces flee chance
    FleeChance += CurrentStress * 0.4f;
    
    return FMath::RandRange(0.0f, 1.0f) < FMath::Clamp(FleeChance, 0.0f, 1.0f);
}

bool UNPCBehaviorSystem::ShouldInvestigatePlayer() const
{
    if (DomesticationLevel == EDomesticationStage::Wild && Memory.PlayerTrustLevel < -0.5f)
        return false;
    
    float InvestigateChance = Personality.Curiosity * 0.7f;
    InvestigateChance += (Memory.PlayerTrustLevel + 1.0f) * 0.2f;
    InvestigateChance -= CurrentStress * 0.3f;
    InvestigateChance -= Personality.Fearfulness * 0.4f;
    
    return FMath::RandRange(0.0f, 1.0f) < FMath::Clamp(InvestigateChance, 0.0f, 1.0f);
}

bool UNPCBehaviorSystem::ShouldAttackPlayer() const
{
    // Only aggressive or territorial dinosaurs attack
    if (ThreatLevel == EDinosaurThreatLevel::Passive) return false;
    
    // Domesticated dinosaurs don't attack unless severely stressed
    if (DomesticationLevel >= EDomesticationStage::Tolerant && CurrentStress < 0.9f) return false;
    
    float AttackChance = 0.0f;
    
    switch (ThreatLevel)
    {
        case EDinosaurThreatLevel::Defensive:
            AttackChance = CurrentStress > 0.7f ? 0.3f : 0.0f;
            break;
        case EDinosaurThreatLevel::Territorial:
            AttackChance = 0.4f;
            break;
        case EDinosaurThreatLevel::Aggressive:
            AttackChance = 0.7f;
            break;
        case EDinosaurThreatLevel::Apex:
            AttackChance = 0.9f;
            break;
    }
    
    // Modify by personality and memory
    AttackChance += Personality.Aggression * 0.3f;
    AttackChance -= (Memory.PlayerTrustLevel + 1.0f) * 0.4f;
    AttackChance += CurrentStress * 0.2f;
    
    return FMath::RandRange(0.0f, 1.0f) < FMath::Clamp(AttackChance, 0.0f, 1.0f);
}

FVector UNPCBehaviorSystem::GetPreferredLocationForActivity(EDinosaurActivity Activity) const
{
    switch (Activity)
    {
        case EDinosaurActivity::Sleeping:
        case EDinosaurActivity::Resting:
            return DailyRoutine.PreferredRestingLocation;
            
        case EDinosaurActivity::Foraging:
        case EDinosaurActivity::Hunting:
            if (Memory.FoodLocations.Num() > 0)
                return Memory.FoodLocations[FMath::RandRange(0, Memory.FoodLocations.Num() - 1)];
            return DailyRoutine.PreferredFeedingLocation;
            
        case EDinosaurActivity::Drinking:
            if (Memory.WaterLocations.Num() > 0)
                return Memory.WaterLocations[FMath::RandRange(0, Memory.WaterLocations.Num() - 1)];
            break;
            
        case EDinosaurActivity::Fleeing:
            if (Memory.SafeLocations.Num() > 0)
                return Memory.SafeLocations[FMath::RandRange(0, Memory.SafeLocations.Num() - 1)];
            break;
    }
    
    // Default: random location within activity radius
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0; // Keep on ground level
    return OwnerLocation + (RandomDirection * FMath::RandRange(100.0f, DailyRoutine.ActivityRadius));
}

void UNPCBehaviorSystem::AddMemoryLocation(FVector Location, bool bSafe, bool bFood, bool bWater)
{
    if (bSafe)
        Memory.SafeLocations.AddUnique(Location);
    if (bFood)
        Memory.FoodLocations.AddUnique(Location);
    if (bWater)
        Memory.WaterLocations.AddUnique(Location);
    
    // Limit memory size for performance
    const int32 MaxMemoryLocations = 20;
    if (Memory.SafeLocations.Num() > MaxMemoryLocations)
        Memory.SafeLocations.RemoveAt(0);
    if (Memory.FoodLocations.Num() > MaxMemoryLocations)
        Memory.FoodLocations.RemoveAt(0);
    if (Memory.WaterLocations.Num() > MaxMemoryLocations)
        Memory.WaterLocations.RemoveAt(0);
}

void UNPCBehaviorSystem::UpdateStressLevel(float StressChange)
{
    CurrentStress = FMath::Clamp(CurrentStress + StressChange, 0.0f, 1.0f);
}

float UNPCBehaviorSystem::GetPersonalityModifier(const FString& TraitName) const
{
    if (TraitName == "Aggression") return Personality.Aggression;
    if (TraitName == "Curiosity") return Personality.Curiosity;
    if (TraitName == "Sociability") return Personality.Sociability;
    if (TraitName == "Fearfulness") return Personality.Fearfulness;
    if (TraitName == "Intelligence") return Personality.Intelligence;
    if (TraitName == "Territoriality") return Personality.Territoriality;
    
    return 0.5f; // Default neutral value
}

bool UNPCBehaviorSystem::MakePersonalityBasedDecision(float BaseChance, const FString& InfluencingTrait, float TraitWeight) const
{
    float Modifier = GetPersonalityModifier(InfluencingTrait);
    float AdjustedChance = BaseChance + ((Modifier - 0.5f) * TraitWeight);
    AdjustedChance = FMath::Clamp(AdjustedChance, 0.0f, 1.0f);
    
    return FMath::RandRange(0.0f, 1.0f) < AdjustedChance;
}

void UNPCBehaviorSystem::UpdateNeeds(float DeltaTime)
{
    // Hunger increases over time
    float HungerRate = 0.01f; // Base rate per second
    if (Size == EDinosaurSize::Massive) HungerRate *= 2.0f;
    if (Size == EDinosaurSize::Tiny) HungerRate *= 0.5f;
    
    CurrentHunger = FMath::Clamp(CurrentHunger + (HungerRate * DeltaTime), 0.0f, 1.0f);
    
    // Thirst increases faster than hunger
    float ThirstRate = HungerRate * 1.5f;
    CurrentThirst = FMath::Clamp(CurrentThirst + (ThirstRate * DeltaTime), 0.0f, 1.0f);
    
    // Stress naturally decreases when safe
    if (CurrentStress > 0.0f)
    {
        float StressDecay = 0.05f * DeltaTime;
        if (DomesticationLevel >= EDomesticationStage::Tolerant)
            StressDecay *= 2.0f; // Domesticated animals recover faster
            
        CurrentStress = FMath::Clamp(CurrentStress - StressDecay, 0.0f, 1.0f);
    }
}

void UNPCBehaviorSystem::UpdateMemoryDecay(float DeltaTime)
{
    // Trust decays over time without player interaction
    FTimespan TimeSinceLastEncounter = FDateTime::Now() - Memory.LastPlayerEncounter;
    float HoursSinceEncounter = TimeSinceLastEncounter.GetTotalHours();
    
    if (HoursSinceEncounter > 1.0f) // Start decay after 1 hour
    {
        float DecayAmount = TrustDecayRate * DeltaTime * (HoursSinceEncounter / 24.0f);
        
        if (Memory.PlayerTrustLevel > 0.0f)
        {
            Memory.PlayerTrustLevel = FMath::Max(0.0f, Memory.PlayerTrustLevel - DecayAmount);
        }
        else if (Memory.PlayerTrustLevel < 0.0f)
        {
            // Negative memories fade slower but still fade
            Memory.PlayerTrustLevel = FMath::Min(0.0f, Memory.PlayerTrustLevel + (DecayAmount * 0.5f));
        }
    }
}

EDinosaurActivity UNPCBehaviorSystem::DetermineActivityFromTime() const
{
    float CurrentTime = GetCurrentTimeOfDay();
    
    // Find the closest scheduled activity
    float ClosestTime = -1.0f;
    EDinosaurActivity ClosestActivity = EDinosaurActivity::Resting;
    
    for (const auto& Schedule : DailyRoutine.ScheduledActivities)
    {
        float ScheduledTime = Schedule.Key;
        if (ClosestTime < 0.0f || FMath::Abs(CurrentTime - ScheduledTime) < FMath::Abs(CurrentTime - ClosestTime))
        {
            ClosestTime = ScheduledTime;
            ClosestActivity = Schedule.Value;
        }
    }
    
    return ClosestActivity;
}

float UNPCBehaviorSystem::GetCurrentTimeOfDay() const
{
    if (UWorld* World = GetWorld())
    {
        // Get time from world (assuming 24-hour cycle)
        return FMath::Fmod(World->GetTimeSeconds() / 3600.0f, 24.0f); // Convert to hours
    }
    return 12.0f; // Default to noon
}

void UNPCBehaviorSystem::GenerateDefaultRoutine()
{
    DailyRoutine.ScheduledActivities.Empty();
    
    // Create routine based on diet and whether nocturnal
    if (DailyRoutine.bNocturnal)
    {
        // Nocturnal routine
        DailyRoutine.ScheduledActivities.Add(6.0f, EDinosaurActivity::Sleeping);
        DailyRoutine.ScheduledActivities.Add(18.0f, EDinosaurActivity::Foraging);
        DailyRoutine.ScheduledActivities.Add(22.0f, Diet == EDinosaurDiet::Carnivore ? EDinosaurActivity::Hunting : EDinosaurActivity::Foraging);
        DailyRoutine.ScheduledActivities.Add(2.0f, EDinosaurActivity::Drinking);
    }
    else
    {
        // Diurnal routine
        DailyRoutine.ScheduledActivities.Add(22.0f, EDinosaurActivity::Sleeping);
        DailyRoutine.ScheduledActivities.Add(6.0f, EDinosaurActivity::Drinking);
        DailyRoutine.ScheduledActivities.Add(8.0f, Diet == EDinosaurDiet::Carnivore ? EDinosaurActivity::Hunting : EDinosaurActivity::Foraging);
        DailyRoutine.ScheduledActivities.Add(12.0f, EDinosaurActivity::Resting);
        DailyRoutine.ScheduledActivities.Add(16.0f, EDinosaurActivity::Foraging);
        DailyRoutine.ScheduledActivities.Add(19.0f, EDinosaurActivity::Socializing);
    }
}