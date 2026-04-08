#include "NPCBehaviorManager.h"
#include "DinosaurNPC.h"
#include "NPCMemoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UNPCBehaviorManager::UNPCBehaviorManager()
{
    // Set default values
    TimeOfDay = 12.0f;
    WeatherIntensity = 0.0f;
    FoodAvailability = 1.0f;
    WaterAvailability = 1.0f;
    MaxActiveDetailedBehaviors = 20;
    DetailedBehaviorRange = 2000.0f;
    MemoryUpdateInterval = 0.5f;
}

void UNPCBehaviorManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorManager initialized"));
    
    // Initialize timers
    MemoryUpdateTimer = 0.0f;
    EcosystemUpdateTimer = 0.0f;
    InteractionUpdateTimer = 0.0f;
}

void UNPCBehaviorManager::Deinitialize()
{
    // Clean up registered dinosaurs
    RegisteredDinosaurs.Empty();
    DinosaurStates.Empty();
    DinosaurMemories.Empty();
    DomesticationLevels.Empty();
    
    Super::Deinitialize();
}

void UNPCBehaviorManager::Tick(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }

    // Update ecosystem state
    UpdateEcosystemState(DeltaTime);
    
    // Update dinosaur needs and behaviors
    UpdateDinosaurNeeds(DeltaTime);
    
    // Update daily routines
    UpdateDailyRoutines(DeltaTime);
    
    // Process interactions between dinosaurs
    ProcessDinosaurInteractions(DeltaTime);
    
    // Manage behavior LOD for performance
    ManageBehaviorLOD(DeltaTime);
    
    // Update memory system
    MemoryUpdateTimer += DeltaTime;
    if (MemoryUpdateTimer >= MemoryUpdateInterval)
    {
        CleanupOldMemories(DeltaTime);
        UpdateMemoryFamiliarity(DeltaTime);
        MemoryUpdateTimer = 0.0f;
    }
}

void UNPCBehaviorManager::RegisterDinosaur(ADinosaurNPC* Dinosaur)
{
    if (!Dinosaur || RegisteredDinosaurs.Contains(Dinosaur))
    {
        return;
    }
    
    RegisteredDinosaurs.Add(Dinosaur);
    DinosaurStates.Add(Dinosaur, EDinosaurBehaviorState::Idle);
    DinosaurMemories.Add(Dinosaur, TArray<FDinosaurMemoryEntry>());
    DomesticationLevels.Add(Dinosaur, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Registered dinosaur: %s"), *Dinosaur->GetName());
}

void UNPCBehaviorManager::UnregisterDinosaur(ADinosaurNPC* Dinosaur)
{
    if (!Dinosaur)
    {
        return;
    }
    
    RegisteredDinosaurs.Remove(Dinosaur);
    DinosaurStates.Remove(Dinosaur);
    DinosaurMemories.Remove(Dinosaur);
    DomesticationLevels.Remove(Dinosaur);
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered dinosaur: %s"), *Dinosaur->GetName());
}

void UNPCBehaviorManager::SetDinosaurBehaviorState(ADinosaurNPC* Dinosaur, EDinosaurBehaviorState NewState)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return;
    }
    
    EDinosaurBehaviorState* CurrentState = DinosaurStates.Find(Dinosaur);
    if (CurrentState && *CurrentState != NewState)
    {
        *CurrentState = NewState;
        
        // Log state change for debugging
        UE_LOG(LogTemp, VeryVerbose, TEXT("Dinosaur %s changed state to %d"), 
               *Dinosaur->GetName(), (int32)NewState);
    }
}

EDinosaurBehaviorState UNPCBehaviorManager::GetDinosaurBehaviorState(ADinosaurNPC* Dinosaur) const
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return EDinosaurBehaviorState::Idle;
    }
    
    return DinosaurStates[Dinosaur];
}

void UNPCBehaviorManager::UpdateDinosaurMemory(ADinosaurNPC* Dinosaur, AActor* PerceivedActor, float ThreatLevel)
{
    if (!Dinosaur || !PerceivedActor || !DinosaurMemories.Contains(Dinosaur))
    {
        return;
    }
    
    TArray<FDinosaurMemoryEntry>& Memories = DinosaurMemories[Dinosaur];
    
    // Find existing memory entry
    FDinosaurMemoryEntry* ExistingEntry = Memories.FindByPredicate(
        [PerceivedActor](const FDinosaurMemoryEntry& Entry)
        {
            return Entry.Actor == PerceivedActor;
        });
    
    if (ExistingEntry)
    {
        // Update existing memory
        ExistingEntry->LastKnownLocation = PerceivedActor->GetActorLocation();
        ExistingEntry->ThreatLevel = ThreatLevel;
        ExistingEntry->LastSeenTime = GetWorld()->GetTimeSeconds();
        ExistingEntry->Familiarity = FMath::Min(ExistingEntry->Familiarity + 0.1f, 1.0f);
    }
    else
    {
        // Create new memory entry
        FDinosaurMemoryEntry NewEntry;
        NewEntry.Actor = PerceivedActor;
        NewEntry.LastKnownLocation = PerceivedActor->GetActorLocation();
        NewEntry.ThreatLevel = ThreatLevel;
        NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewEntry.Familiarity = 0.1f;
        
        // Determine if this is a predator/prey relationship
        if (PerceivedActor->IsA<APawn>())
        {
            // This would be determined by species data in a full implementation
            NewEntry.bIsPredator = ThreatLevel > 0.7f;
            NewEntry.bIsPrey = ThreatLevel < 0.3f;
        }
        
        Memories.Add(NewEntry);
    }
}

FDinosaurMemoryEntry UNPCBehaviorManager::GetMemoryEntry(ADinosaurNPC* Dinosaur, AActor* Actor) const
{
    if (!Dinosaur || !Actor || !DinosaurMemories.Contains(Dinosaur))
    {
        return FDinosaurMemoryEntry();
    }
    
    const TArray<FDinosaurMemoryEntry>& Memories = DinosaurMemories[Dinosaur];
    
    const FDinosaurMemoryEntry* Entry = Memories.FindByPredicate(
        [Actor](const FDinosaurMemoryEntry& MemEntry)
        {
            return MemEntry.Actor == Actor;
        });
    
    return Entry ? *Entry : FDinosaurMemoryEntry();
}

void UNPCBehaviorManager::UpdateEcosystemState(float DeltaTime)
{
    EcosystemUpdateTimer += DeltaTime;
    
    if (EcosystemUpdateTimer >= 1.0f) // Update every second
    {
        // Update time of day (24 hour cycle over 24 minutes real time)
        TimeOfDay += DeltaTime * (24.0f / (24.0f * 60.0f));
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }
        
        // Simple weather simulation
        WeatherIntensity += FMath::RandRange(-0.1f, 0.1f) * DeltaTime;
        WeatherIntensity = FMath::Clamp(WeatherIntensity, 0.0f, 1.0f);
        
        // Resource availability fluctuation
        FoodAvailability += FMath::RandRange(-0.05f, 0.05f) * DeltaTime;
        FoodAvailability = FMath::Clamp(FoodAvailability, 0.1f, 1.0f);
        
        WaterAvailability += FMath::RandRange(-0.02f, 0.02f) * DeltaTime;
        WaterAvailability = FMath::Clamp(WaterAvailability, 0.1f, 1.0f);
        
        EcosystemUpdateTimer = 0.0f;
    }
}

void UNPCBehaviorManager::NotifyPlayerAction(const FVector& Location, float NoiseLevel, bool bIsAggressive)
{
    // Notify nearby dinosaurs of player action
    for (ADinosaurNPC* Dinosaur : RegisteredDinosaurs)
    {
        if (!Dinosaur)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Dinosaur->GetActorLocation(), Location);
        float EffectiveRange = NoiseLevel * 1000.0f; // Convert noise level to range
        
        if (Distance <= EffectiveRange)
        {
            // Calculate threat level based on distance and aggression
            float ThreatLevel = bIsAggressive ? 0.8f : 0.3f;
            ThreatLevel *= (1.0f - (Distance / EffectiveRange));
            
            // Update dinosaur's memory about the player
            if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    UpdateDinosaurMemory(Dinosaur, PlayerPawn, ThreatLevel);
                }
            }
        }
    }
}

float UNPCBehaviorManager::GetDomesticationLevel(ADinosaurNPC* Dinosaur) const
{
    if (!Dinosaur || !DomesticationLevels.Contains(Dinosaur))
    {
        return 0.0f;
    }
    
    return DomesticationLevels[Dinosaur];
}

void UNPCBehaviorManager::UpdateDomesticationLevel(ADinosaurNPC* Dinosaur, float DeltaValue)
{
    if (!Dinosaur || !DomesticationLevels.Contains(Dinosaur))
    {
        return;
    }
    
    float& CurrentLevel = DomesticationLevels[Dinosaur];
    CurrentLevel = FMath::Clamp(CurrentLevel + DeltaValue, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Dinosaur %s domestication level: %f"), 
           *Dinosaur->GetName(), CurrentLevel);
}

void UNPCBehaviorManager::UpdateDinosaurNeeds(float DeltaTime)
{
    // This would be implemented with access to dinosaur need components
    // For now, this is a placeholder for the need decay system
    
    for (ADinosaurNPC* Dinosaur : RegisteredDinosaurs)
    {
        if (!Dinosaur)
        {
            continue;
        }
        
        // Needs would decay over time and influence behavior decisions
        // This would integrate with the Behavior Tree system
    }
}

void UNPCBehaviorManager::UpdateDailyRoutines(float DeltaTime)
{
    // Update dinosaur daily routines based on time of day
    // This would trigger different behaviors at different times
    
    for (ADinosaurNPC* Dinosaur : RegisteredDinosaurs)
    {
        if (!Dinosaur)
        {
            continue;
        }
        
        // Check if dinosaur should change behavior based on time
        // This would be species-specific and personality-dependent
    }
}

void UNPCBehaviorManager::ProcessDinosaurInteractions(float DeltaTime)
{
    InteractionUpdateTimer += DeltaTime;
    
    if (InteractionUpdateTimer >= 0.1f) // Update 10 times per second
    {
        // Process interactions between dinosaurs
        for (int32 i = 0; i < RegisteredDinosaurs.Num(); ++i)
        {
            for (int32 j = i + 1; j < RegisteredDinosaurs.Num(); ++j)
            {
                ADinosaurNPC* DinoA = RegisteredDinosaurs[i];
                ADinosaurNPC* DinoB = RegisteredDinosaurs[j];
                
                if (!DinoA || !DinoB)
                {
                    continue;
                }
                
                float Distance = FVector::Dist(DinoA->GetActorLocation(), DinoB->GetActorLocation());
                
                // Only process if dinosaurs are close enough to interact
                if (Distance <= 2000.0f)
                {
                    // Update mutual memories
                    UpdateDinosaurMemory(DinoA, DinoB, 0.2f); // Neutral threat level
                    UpdateDinosaurMemory(DinoB, DinoA, 0.2f);
                }
            }
        }
        
        InteractionUpdateTimer = 0.0f;
    }
}

void UNPCBehaviorManager::ManageBehaviorLOD(float DeltaTime)
{
    // Manage Level of Detail for behavior processing
    // Dinosaurs closer to the player get more detailed AI processing
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Sort dinosaurs by distance to player
            RegisteredDinosaurs.Sort([PlayerLocation](const ADinosaurNPC& A, const ADinosaurNPC& B)
            {
                float DistA = FVector::DistSquared(A.GetActorLocation(), PlayerLocation);
                float DistB = FVector::DistSquared(B.GetActorLocation(), PlayerLocation);
                return DistA < DistB;
            });
            
            // Enable detailed behavior for closest dinosaurs
            for (int32 i = 0; i < RegisteredDinosaurs.Num(); ++i)
            {
                ADinosaurNPC* Dinosaur = RegisteredDinosaurs[i];
                if (!Dinosaur)
                {
                    continue;
                }
                
                float Distance = FVector::Dist(Dinosaur->GetActorLocation(), PlayerLocation);
                bool bShouldUseDetailedBehavior = (i < MaxActiveDetailedBehaviors) && 
                                                  (Distance <= DetailedBehaviorRange);
                
                // This would enable/disable detailed Behavior Tree processing
                // For distant dinosaurs, use simplified Mass Entity simulation
            }
        }
    }
}

void UNPCBehaviorManager::CleanupOldMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MemoryRetentionTime = 300.0f; // 5 minutes
    
    for (auto& MemoryPair : DinosaurMemories)
    {
        TArray<FDinosaurMemoryEntry>& Memories = MemoryPair.Value;
        
        // Remove old memories
        Memories.RemoveAll([CurrentTime, MemoryRetentionTime](const FDinosaurMemoryEntry& Entry)
        {
            return (CurrentTime - Entry.LastSeenTime) > MemoryRetentionTime;
        });
    }
}

void UNPCBehaviorManager::UpdateMemoryFamiliarity(float DeltaTime)
{
    // Decay familiarity over time for memories that aren't being refreshed
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float FamiliarityDecayRate = 0.1f; // Per minute
    
    for (auto& MemoryPair : DinosaurMemories)
    {
        TArray<FDinosaurMemoryEntry>& Memories = MemoryPair.Value;
        
        for (FDinosaurMemoryEntry& Memory : Memories)
        {
            float TimeSinceLastSeen = CurrentTime - Memory.LastSeenTime;
            if (TimeSinceLastSeen > 60.0f) // Start decaying after 1 minute
            {
                float DecayAmount = FamiliarityDecayRate * (TimeSinceLastSeen / 60.0f) * DeltaTime;
                Memory.Familiarity = FMath::Max(Memory.Familiarity - DecayAmount, 0.0f);
            }
        }
    }
}