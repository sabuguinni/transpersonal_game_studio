// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CombatMemoryComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"

UCombatMemoryComponent::UCombatMemoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    MaxCombatMemories = 50;
    MaxLocationMemories = 100;
    MemoryDecayTime = 300.0f;
    LocationMemoryRadius = 500.0f;
    ThreatLevelUpdateRate = 0.1f;
    LearningRate = 0.1f;
    MemoryStrength = 1.0f;
    bEnableLearning = true;
}

void UCombatMemoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerController = Cast<AAIController>(GetOwner());
    CachedWorld = GetWorld();
    
    if (!OwnerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("CombatMemoryComponent: Owner is not an AIController"));
    }
}

void UCombatMemoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    MemoryUpdateTimer += DeltaTime;
    AnalysisTimer += DeltaTime;
    CleanupTimer += DeltaTime;
    
    // Update memory decay every 5 seconds
    if (MemoryUpdateTimer >= 5.0f)
    {
        UpdateMemoryDecay(DeltaTime);
        UpdateThreatLevels();
        MemoryUpdateTimer = 0.0f;
    }
    
    // Analyze combat patterns every 30 seconds
    if (AnalysisTimer >= 30.0f)
    {
        AnalyzeCombatPatterns();
        ProcessLocationLearning();
        AnalysisTimer = 0.0f;
    }
    
    // Cleanup old memories every 60 seconds
    if (CleanupTimer >= 60.0f)
    {
        CleanupOldMemories();
        OptimizeMemoryStorage();
        CleanupTimer = 0.0f;
    }
}

void UCombatMemoryComponent::RecordCombatEncounter(AActor* Opponent, bool bVictorious, float DamageDealt, float DamageReceived)
{
    if (!Opponent || !bEnableLearning)
    {
        return;
    }
    
    FCombatMemory& Memory = GetOrCreateCombatMemory(Opponent);
    
    Memory.EncounterCount++;
    Memory.TotalDamageDealt += DamageDealt;
    Memory.TotalDamageReceived += DamageReceived;
    Memory.LastSeenTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    
    if (bVictorious)
    {
        Memory.VictoryCount++;
    }
    else
    {
        Memory.DefeatCount++;
    }
    
    // Update threat level based on combat outcome
    float ThreatAdjustment = bVictorious ? -ThreatLevelUpdateRate : ThreatLevelUpdateRate;
    Memory.ThreatLevel = FMath::Clamp(Memory.ThreatLevel + ThreatAdjustment, 0.0f, 1.0f);
    
    // Mark as hostile if damage was exchanged
    if (DamageDealt > 0.0f || DamageReceived > 0.0f)
    {
        Memory.bIsHostile = true;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CombatMemory: Recorded encounter with %s - Victory: %s, Threat Level: %f"), 
           *Opponent->GetName(), bVictorious ? TEXT("Yes") : TEXT("No"), Memory.ThreatLevel);
}

void UCombatMemoryComponent::UpdateActorPosition(AActor* Actor, FVector Position)
{
    if (!Actor)
    {
        return;
    }
    
    if (FCombatMemory* Memory = CombatMemories.Find(Actor))
    {
        Memory->LastKnownPosition = Position;
        Memory->LastSeenTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    }
}

bool UCombatMemoryComponent::HasMemoryOf(AActor* Actor) const
{
    return Actor && CombatMemories.Contains(Actor);
}

FCombatMemory UCombatMemoryComponent::GetCombatMemory(AActor* Actor) const
{
    if (const FCombatMemory* Memory = CombatMemories.Find(Actor))
    {
        return *Memory;
    }
    return FCombatMemory();
}

float UCombatMemoryComponent::GetThreatLevel(AActor* Actor) const
{
    if (const FCombatMemory* Memory = CombatMemories.Find(Actor))
    {
        return Memory->ThreatLevel;
    }
    return 0.5f; // Default threat level for unknown actors
}

FVector UCombatMemoryComponent::GetLastKnownPosition(AActor* Actor) const
{
    if (const FCombatMemory* Memory = CombatMemories.Find(Actor))
    {
        return Memory->LastKnownPosition;
    }
    return FVector::ZeroVector;
}

float UCombatMemoryComponent::GetTimeSinceLastSeen(AActor* Actor) const
{
    if (const FCombatMemory* Memory = CombatMemories.Find(Actor))
    {
        float CurrentTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
        return CurrentTime - Memory->LastSeenTime;
    }
    return FLT_MAX;
}

void UCombatMemoryComponent::RecordTacticUsage(FGameplayTag TacticTag, bool bSuccessful, AActor* TargetActor)
{
    if (!TacticTag.IsValid() || !bEnableLearning)
    {
        return;
    }
    
    FTacticalMemory& Memory = GetOrCreateTacticalMemory(TacticTag);
    
    Memory.UsageCount++;
    Memory.LastUsedTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    
    if (bSuccessful)
    {
        Memory.SuccessCount++;
        
        if (TargetActor)
        {
            Memory.MostEffectiveAgainst = TargetActor;
        }
    }
    else if (TargetActor)
    {
        Memory.LeastEffectiveAgainst = TargetActor;
    }
    
    UpdateTacticEffectiveness(Memory);
    
    UE_LOG(LogTemp, Log, TEXT("CombatMemory: Recorded tactic %s usage - Success: %s, Success Rate: %f"), 
           *TacticTag.ToString(), bSuccessful ? TEXT("Yes") : TEXT("No"), Memory.SuccessRate);
}

float UCombatMemoryComponent::GetTacticSuccessRate(FGameplayTag TacticTag) const
{
    if (const FTacticalMemory* Memory = TacticalMemories.Find(TacticTag))
    {
        return Memory->SuccessRate;
    }
    return 0.5f; // Default success rate for unknown tactics
}

FGameplayTag UCombatMemoryComponent::GetMostSuccessfulTactic() const
{
    FGameplayTag BestTactic;
    float BestSuccessRate = 0.0f;
    
    for (const auto& Pair : TacticalMemories)
    {
        if (Pair.Value.SuccessRate > BestSuccessRate && Pair.Value.UsageCount >= 3)
        {
            BestSuccessRate = Pair.Value.SuccessRate;
            BestTactic = Pair.Key;
        }
    }
    
    return BestTactic;
}

FGameplayTag UCombatMemoryComponent::GetBestTacticAgainst(AActor* Opponent) const
{
    if (!Opponent)
    {
        return FGameplayTag();
    }
    
    FGameplayTag BestTactic;
    float BestSuccessRate = 0.0f;
    
    for (const auto& Pair : TacticalMemories)
    {
        if (Pair.Value.MostEffectiveAgainst.Get() == Opponent && 
            Pair.Value.SuccessRate > BestSuccessRate)
        {
            BestSuccessRate = Pair.Value.SuccessRate;
            BestTactic = Pair.Key;
        }
    }
    
    return BestTactic;
}

void UCombatMemoryComponent::LearnOpponentAbility(AActor* Opponent, FGameplayTag AbilityTag)
{
    if (!Opponent || !AbilityTag.IsValid())
    {
        return;
    }
    
    FCombatMemory& Memory = GetOrCreateCombatMemory(Opponent);
    Memory.KnownAbilities.AddUnique(AbilityTag);
    
    // Update combat characteristics based on learned abilities
    if (AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Combat.Ability.Ranged"))))
    {
        Memory.bUsesRangedAttacks = true;
        Memory.PreferredEngagementDistance = 800.0f;
    }
    else if (AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Combat.Ability.Stealth"))))
    {
        Memory.bCanBeFlanked = false;
        Memory.ThreatLevel = FMath::Min(Memory.ThreatLevel + 0.2f, 1.0f);
    }
}

void UCombatMemoryComponent::RecordLocationEvent(FVector Location, FGameplayTag EventType, float DangerLevel)
{
    FLocationMemory* ExistingMemory = FindLocationMemory(Location);
    
    if (ExistingMemory)
    {
        ExistingMemory->LastVisitTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
        ExistingMemory->DangerLevel = FMath::Max(ExistingMemory->DangerLevel, DangerLevel);
        
        if (EventType.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Combat.Event"))))
        {
            ExistingMemory->CombatEventsCount++;
        }
    }
    else
    {
        // Create new location memory
        FLocationMemory NewMemory;
        NewMemory.Location = Location;
        NewMemory.LocationType = EventType;
        NewMemory.DangerLevel = DangerLevel;
        NewMemory.LastVisitTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
        
        if (EventType.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Combat.Event"))))
        {
            NewMemory.CombatEventsCount = 1;
        }
        
        // Analyze location characteristics
        NewMemory.bHasGoodCover = AnalyzeLocationCover(Location);
        NewMemory.bHasEscapeRoutes = AnalyzeEscapeRoutes(Location);
        NewMemory.bIsAmbushSite = AnalyzeAmbushPotential(Location);
        
        LocationMemories.Add(NewMemory);
        
        // Remove oldest if we exceed max capacity
        if (LocationMemories.Num() > MaxLocationMemories)
        {
            LocationMemories.RemoveAt(0);
        }
    }
}

float UCombatMemoryComponent::GetLocationDangerLevel(FVector Location) const
{
    if (const FLocationMemory* Memory = FindLocationMemory(Location))
    {
        return Memory->DangerLevel;
    }
    return 0.0f;
}

TArray<FVector> UCombatMemoryComponent::GetSafeLocations(float MaxDangerLevel) const
{
    TArray<FVector> SafeLocations;
    
    for (const FLocationMemory& Memory : LocationMemories)
    {
        if (Memory.DangerLevel <= MaxDangerLevel)
        {
            SafeLocations.Add(Memory.Location);
        }
    }
    
    return SafeLocations;
}

TArray<FVector> UCombatMemoryComponent::GetAmbushLocations() const
{
    TArray<FVector> AmbushLocations;
    
    for (const FLocationMemory& Memory : LocationMemories)
    {
        if (Memory.bIsAmbushSite && Memory.bHasGoodCover)
        {
            AmbushLocations.Add(Memory.Location);
        }
    }
    
    return AmbushLocations;
}

FVector UCombatMemoryComponent::GetNearestSafeLocation(FVector FromLocation) const
{
    FVector NearestLocation = FVector::ZeroVector;
    float NearestDistance = FLT_MAX;
    
    for (const FLocationMemory& Memory : LocationMemories)
    {
        if (Memory.DangerLevel <= 0.3f)
        {
            float Distance = FVector::Dist(FromLocation, Memory.Location);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestLocation = Memory.Location;
            }
        }
    }
    
    return NearestLocation;
}

AActor* UCombatMemoryComponent::GetMostDangerousOpponent() const
{
    AActor* MostDangerous = nullptr;
    float HighestThreat = 0.0f;
    
    for (const auto& Pair : CombatMemories)
    {
        if (Pair.Value.ThreatLevel > HighestThreat && Pair.Key.IsValid())
        {
            HighestThreat = Pair.Value.ThreatLevel;
            MostDangerous = Pair.Key.Get();
        }
    }
    
    return MostDangerous;
}

AActor* UCombatMemoryComponent::GetWeakestOpponent() const
{
    AActor* Weakest = nullptr;
    float LowestThreat = 1.0f;
    
    for (const auto& Pair : CombatMemories)
    {
        if (Pair.Value.ThreatLevel < LowestThreat && Pair.Key.IsValid())
        {
            LowestThreat = Pair.Value.ThreatLevel;
            Weakest = Pair.Key.Get();
        }
    }
    
    return Weakest;
}

float UCombatMemoryComponent::GetOverallCombatExperience() const
{
    int32 TotalEncounters = 0;
    int32 TotalVictories = 0;
    
    for (const auto& Pair : CombatMemories)
    {
        TotalEncounters += Pair.Value.EncounterCount;
        TotalVictories += Pair.Value.VictoryCount;
    }
    
    return TotalEncounters > 0 ? static_cast<float>(TotalVictories) / TotalEncounters : 0.0f;
}

bool UCombatMemoryComponent::ShouldAvoidOpponent(AActor* Opponent) const
{
    if (const FCombatMemory* Memory = CombatMemories.Find(Opponent))
    {
        // Avoid if threat level is high and we have a poor win rate
        float WinRate = Memory->EncounterCount > 0 ? 
            static_cast<float>(Memory->VictoryCount) / Memory->EncounterCount : 0.0f;
        
        return Memory->ThreatLevel > 0.7f && WinRate < 0.3f;
    }
    
    return false;
}

bool UCombatMemoryComponent::CanDefeatOpponent(AActor* Opponent) const
{
    if (const FCombatMemory* Memory = CombatMemories.Find(Opponent))
    {
        float WinRate = Memory->EncounterCount > 0 ? 
            static_cast<float>(Memory->VictoryCount) / Memory->EncounterCount : 0.5f;
        
        return WinRate > 0.6f && Memory->ThreatLevel < 0.8f;
    }
    
    return true; // Assume we can defeat unknown opponents
}

void UCombatMemoryComponent::CleanupOldMemories()
{
    if (!CachedWorld)
    {
        return;
    }
    
    float CurrentTime = CachedWorld->GetTimeSeconds();
    
    // Clean up combat memories
    TArray<AActor*> ActorsToRemove;
    for (const auto& Pair : CombatMemories)
    {
        if (!Pair.Key.IsValid() || 
            (CurrentTime - Pair.Value.LastSeenTime) > MemoryDecayTime)
        {
            ActorsToRemove.Add(Pair.Key.Get());
        }
    }
    
    for (AActor* Actor : ActorsToRemove)
    {
        CombatMemories.Remove(Actor);
    }
    
    // Clean up location memories
    LocationMemories.RemoveAll([CurrentTime, this](const FLocationMemory& Memory)
    {
        return (CurrentTime - Memory.LastVisitTime) > MemoryDecayTime;
    });
}

void UCombatMemoryComponent::ForgetActor(AActor* Actor)
{
    if (Actor)
    {
        CombatMemories.Remove(Actor);
    }
}

void UCombatMemoryComponent::ClearAllMemories()
{
    CombatMemories.Empty();
    TacticalMemories.Empty();
    LocationMemories.Empty();
}

void UCombatMemoryComponent::AnalyzeCombatPatterns()
{
    if (!bEnableLearning)
    {
        return;
    }
    
    // Analyze overall combat performance
    float OverallWinRate = GetOverallCombatExperience();
    
    // Adjust learning rate based on performance
    if (OverallWinRate > 0.7f)
    {
        LearningRate = FMath::Max(LearningRate * 0.9f, 0.01f); // Reduce learning rate when doing well
    }
    else if (OverallWinRate < 0.3f)
    {
        LearningRate = FMath::Min(LearningRate * 1.1f, 0.5f); // Increase learning rate when struggling
    }
    
    // Update tactic preferences based on success rates
    for (auto& Pair : TacticalMemories)
    {
        UpdateTacticEffectiveness(Pair.Value);
    }
}

void UCombatMemoryComponent::AdaptToOpponent(AActor* Opponent)
{
    if (!Opponent || !bEnableLearning)
    {
        return;
    }
    
    FCombatMemory& Memory = GetOrCreateCombatMemory(Opponent);
    
    // Adapt engagement distance based on opponent's abilities
    if (Memory.bUsesRangedAttacks)
    {
        Memory.PreferredEngagementDistance = FMath::Max(Memory.PreferredEngagementDistance, 300.0f);
    }
    
    // Learn from successful tactics
    FGameplayTag BestTactic = GetBestTacticAgainst(Opponent);
    if (BestTactic.IsValid())
    {
        Memory.SuccessfulTactics.AddUnique(BestTactic);
    }
}

bool UCombatMemoryComponent::IsLearningFromExperience() const
{
    return bEnableLearning && CombatMemories.Num() > 0;
}

void UCombatMemoryComponent::DebugPrintMemoryStats() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== Combat Memory Stats ==="));
    UE_LOG(LogTemp, Warning, TEXT("Combat Memories: %d"), CombatMemories.Num());
    UE_LOG(LogTemp, Warning, TEXT("Tactical Memories: %d"), TacticalMemories.Num());
    UE_LOG(LogTemp, Warning, TEXT("Location Memories: %d"), LocationMemories.Num());
    UE_LOG(LogTemp, Warning, TEXT("Overall Win Rate: %f"), GetOverallCombatExperience());
    UE_LOG(LogTemp, Warning, TEXT("Learning Rate: %f"), LearningRate);
}

void UCombatMemoryComponent::DebugDrawMemoryData() const
{
    if (!CachedWorld)
    {
        return;
    }
    
    // Draw location memories
    for (const FLocationMemory& Memory : LocationMemories)
    {
        FColor Color = Memory.DangerLevel > 0.5f ? FColor::Red : FColor::Green;
        DrawDebugSphere(CachedWorld, Memory.Location, 50.0f, 8, Color, false, 1.0f);
        
        if (Memory.bIsAmbushSite)
        {
            DrawDebugSphere(CachedWorld, Memory.Location + FVector(0, 0, 100), 30.0f, 8, FColor::Purple, false, 1.0f);
        }
    }
}

// Protected Methods

void UCombatMemoryComponent::UpdateMemoryDecay(float DeltaTime)
{
    if (!CachedWorld)
    {
        return;
    }
    
    float CurrentTime = CachedWorld->GetTimeSeconds();
    
    for (auto& Pair : CombatMemories)
    {
        FCombatMemory& Memory = Pair.Value;
        float TimeSinceLastSeen = CurrentTime - Memory.LastSeenTime;
        
        // Gradually reduce threat level for unseen opponents
        if (TimeSinceLastSeen > 60.0f) // 1 minute
        {
            float DecayRate = 0.01f * DeltaTime;
            Memory.ThreatLevel = FMath::Max(Memory.ThreatLevel - DecayRate, 0.1f);
        }
    }
}

void UCombatMemoryComponent::UpdateThreatLevels()
{
    for (auto& Pair : CombatMemories)
    {
        FCombatMemory& Memory = Pair.Value;
        Memory.ThreatLevel = CalculateThreatLevel(Memory);
    }
}

void UCombatMemoryComponent::ProcessLocationLearning()
{
    // Update location danger levels based on combat events
    for (FLocationMemory& Memory : LocationMemories)
    {
        Memory.DangerLevel = CalculateLocationDanger(Memory);
    }
}

void UCombatMemoryComponent::OptimizeMemoryStorage()
{
    // Remove excess memories if we're over capacity
    while (CombatMemories.Num() > MaxCombatMemories)
    {
        RemoveOldestMemory();
    }
}

FCombatMemory& UCombatMemoryComponent::GetOrCreateCombatMemory(AActor* Actor)
{
    if (FCombatMemory* ExistingMemory = CombatMemories.Find(Actor))
    {
        return *ExistingMemory;
    }
    
    FCombatMemory NewMemory;
    NewMemory.Actor = Actor;
    NewMemory.LastSeenTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    
    return CombatMemories.Add(Actor, NewMemory);
}

FTacticalMemory& UCombatMemoryComponent::GetOrCreateTacticalMemory(FGameplayTag TacticTag)
{
    if (FTacticalMemory* ExistingMemory = TacticalMemories.Find(TacticTag))
    {
        return *ExistingMemory;
    }
    
    FTacticalMemory NewMemory;
    NewMemory.TacticTag = TacticTag;
    NewMemory.LastUsedTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    
    return TacticalMemories.Add(TacticTag, NewMemory);
}

FLocationMemory* UCombatMemoryComponent::FindLocationMemory(FVector Location)
{
    for (FLocationMemory& Memory : LocationMemories)
    {
        if (FVector::Dist(Memory.Location, Location) <= LocationMemoryRadius)
        {
            return &Memory;
        }
    }
    return nullptr;
}

void UCombatMemoryComponent::RemoveOldestMemory()
{
    if (CombatMemories.Num() == 0)
    {
        return;
    }
    
    AActor* OldestActor = nullptr;
    float OldestTime = FLT_MAX;
    
    for (const auto& Pair : CombatMemories)
    {
        if (Pair.Value.LastSeenTime < OldestTime)
        {
            OldestTime = Pair.Value.LastSeenTime;
            OldestActor = Pair.Key.Get();
        }
    }
    
    if (OldestActor)
    {
        CombatMemories.Remove(OldestActor);
    }
}

float UCombatMemoryComponent::CalculateThreatLevel(const FCombatMemory& Memory) const
{
    float BaseThreat = 0.5f;
    
    // Factor in win/loss ratio
    if (Memory.EncounterCount > 0)
    {
        float WinRate = static_cast<float>(Memory.VictoryCount) / Memory.EncounterCount;
        BaseThreat += (1.0f - WinRate) * 0.3f;
    }
    
    // Factor in damage ratio
    float TotalDamage = Memory.TotalDamageDealt + Memory.TotalDamageReceived;
    if (TotalDamage > 0.0f)
    {
        float DamageRatio = Memory.TotalDamageReceived / TotalDamage;
        BaseThreat += DamageRatio * 0.2f;
    }
    
    // Factor in known abilities
    BaseThreat += Memory.KnownAbilities.Num() * 0.05f;
    
    return FMath::Clamp(BaseThreat, 0.0f, 1.0f);
}

float UCombatMemoryComponent::CalculateLocationDanger(const FLocationMemory& Memory) const
{
    float BaseDanger = Memory.DangerLevel;
    
    // Increase danger based on combat events
    BaseDanger += Memory.CombatEventsCount * 0.1f;
    
    // Reduce danger if location has good defensive features
    if (Memory.bHasGoodCover)
    {
        BaseDanger *= 0.8f;
    }
    
    if (Memory.bHasEscapeRoutes)
    {
        BaseDanger *= 0.9f;
    }
    
    return FMath::Clamp(BaseDanger, 0.0f, 1.0f);
}

void UCombatMemoryComponent::UpdateTacticEffectiveness(FTacticalMemory& Memory)
{
    if (Memory.UsageCount > 0)
    {
        Memory.SuccessRate = static_cast<float>(Memory.SuccessCount) / Memory.UsageCount;
    }
    else
    {
        Memory.SuccessRate = 0.5f; // Default for unused tactics
    }
}

bool UCombatMemoryComponent::AnalyzeLocationCover(FVector Location) const
{
    // Simple cover analysis - check for nearby walls or obstacles
    if (!CachedWorld)
    {
        return false;
    }
    
    FHitResult HitResult;
    FVector TraceEnd = Location + FVector(0, 0, 200); // Check above
    
    bool bHit = CachedWorld->LineTraceSingleByChannel(
        HitResult,
        Location,
        TraceEnd,
        ECollisionChannel::ECC_WorldStatic
    );
    
    return bHit; // If we hit something above, there's potential cover
}

bool UCombatMemoryComponent::AnalyzeEscapeRoutes(FVector Location) const
{
    // Simple escape route analysis - check for multiple directions without obstacles
    if (!CachedWorld)
    {
        return false;
    }
    
    int32 ClearDirections = 0;
    TArray<FVector> Directions = {
        FVector(1, 0, 0), FVector(-1, 0, 0),
        FVector(0, 1, 0), FVector(0, -1, 0)
    };
    
    for (const FVector& Direction : Directions)
    {
        FHitResult HitResult;
        FVector TraceEnd = Location + Direction * 500.0f; // Check 5 meters in each direction
        
        bool bHit = CachedWorld->LineTraceSingleByChannel(
            HitResult,
            Location,
            TraceEnd,
            ECollisionChannel::ECC_WorldStatic
        );
        
        if (!bHit)
        {
            ClearDirections++;
        }
    }
    
    return ClearDirections >= 2; // Need at least 2 clear escape routes
}

bool UCombatMemoryComponent::AnalyzeAmbushPotential(FVector Location) const
{
    // Simple ambush analysis - check for concealment and good sightlines
    return AnalyzeLocationCover(Location) && !AnalyzeEscapeRoutes(Location);
}