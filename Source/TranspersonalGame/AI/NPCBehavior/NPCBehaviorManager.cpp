#include "NPCBehaviorManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "NPCBehaviorAgent.h"

DEFINE_LOG_CATEGORY(LogNPCBehaviorManager);

UNPCBehaviorManager::UNPCBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update once per second
    
    // Initialize default values
    MaxManagedNPCs = 100;
    GlobalBehaviorUpdateInterval = 5.0f;
    MemoryCleanupInterval = 30.0f;
    NPCInteractionRange = 1000.0f;
    
    // Initialize global modifiers
    GlobalBehaviorModifiers.Add(TEXT("TimeOfDay"), 1.0f);
    GlobalBehaviorModifiers.Add(TEXT("Weather"), 1.0f);
    GlobalBehaviorModifiers.Add(TEXT("PlayerPresence"), 1.0f);
    GlobalBehaviorModifiers.Add(TEXT("ThreatLevel"), 1.0f);
}

void UNPCBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the behavior management system
    InitializeManager();
    
    // Set up global update timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            GlobalUpdateTimer,
            this,
            &UNPCBehaviorManager::UpdateGlobalBehaviorModifiers,
            GlobalBehaviorUpdateInterval,
            true
        );
        
        World->GetTimerManager().SetTimer(
            MemoryCleanupTimer,
            this,
            &UNPCBehaviorManager::CleanupGlobalMemory,
            MemoryCleanupInterval,
            true
        );
    }
    
    UE_LOG(LogNPCBehaviorManager, Log, TEXT("NPCBehaviorManager initialized"));
}

void UNPCBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update managed NPCs
    UpdateManagedNPCs(DeltaTime);
    
    // Process NPC interactions
    ProcessNPCInteractions();
    
    // Update global statistics
    UpdateGlobalStatistics();
}

void UNPCBehaviorManager::InitializeManager()
{
    ManagedNPCs.Empty();
    GlobalMemory.SharedEvents.Empty();
    GlobalMemory.ImportantLocations.Empty();
    
    // Find all existing NPCs in the world
    RegisterExistingNPCs();
    
    UE_LOG(LogNPCBehaviorManager, Log, TEXT("Found and registered %d existing NPCs"), ManagedNPCs.Num());
}

void UNPCBehaviorManager::RegisterExistingNPCs()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Find all actors with NPCBehaviorAgent components
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (UNPCBehaviorAgent* BehaviorAgent = Actor->FindComponentByClass<UNPCBehaviorAgent>())
        {
            RegisterNPC(BehaviorAgent);
        }
    }
}

void UNPCBehaviorManager::RegisterNPC(UNPCBehaviorAgent* NPCBehaviorAgent)
{
    if (!NPCBehaviorAgent || ManagedNPCs.Contains(NPCBehaviorAgent))
    {
        return;
    }
    
    if (ManagedNPCs.Num() >= MaxManagedNPCs)
    {
        UE_LOG(LogNPCBehaviorManager, Warning, TEXT("Cannot register NPC - maximum limit reached (%d)"), MaxManagedNPCs);
        return;
    }
    
    ManagedNPCs.Add(NPCBehaviorAgent);
    
    // Bind to behavior change events
    NPCBehaviorAgent->OnBehaviorChanged.AddDynamic(this, &UNPCBehaviorManager::OnNPCBehaviorChanged);
    
    UE_LOG(LogNPCBehaviorManager, Log, TEXT("Registered NPC: %s (Total: %d)"), 
           NPCBehaviorAgent->GetOwner() ? *NPCBehaviorAgent->GetOwner()->GetName() : TEXT("Unknown"),
           ManagedNPCs.Num());
}

void UNPCBehaviorManager::UnregisterNPC(UNPCBehaviorAgent* NPCBehaviorAgent)
{
    if (!NPCBehaviorAgent)
    {
        return;
    }
    
    if (ManagedNPCs.Remove(NPCBehaviorAgent) > 0)
    {
        // Unbind from behavior change events
        NPCBehaviorAgent->OnBehaviorChanged.RemoveDynamic(this, &UNPCBehaviorManager::OnNPCBehaviorChanged);
        
        UE_LOG(LogNPCBehaviorManager, Log, TEXT("Unregistered NPC: %s (Remaining: %d)"), 
               NPCBehaviorAgent->GetOwner() ? *NPCBehaviorAgent->GetOwner()->GetName() : TEXT("Unknown"),
               ManagedNPCs.Num());
    }
}

void UNPCBehaviorManager::UpdateManagedNPCs(float DeltaTime)
{
    // Clean up invalid NPCs
    ManagedNPCs.RemoveAll([](UNPCBehaviorAgent* NPC) 
    {
        return !IsValid(NPC) || !IsValid(NPC->GetOwner());
    });
    
    // Update global behavior influences
    for (UNPCBehaviorAgent* NPC : ManagedNPCs)
    {
        if (IsValid(NPC))
        {
            ApplyGlobalModifiersToNPC(NPC);
        }
    }
}

void UNPCBehaviorManager::ProcessNPCInteractions()
{
    // Process interactions between nearby NPCs
    for (int32 i = 0; i < ManagedNPCs.Num(); ++i)
    {
        UNPCBehaviorAgent* NPC1 = ManagedNPCs[i];
        if (!IsValid(NPC1) || !IsValid(NPC1->GetOwner()))
        {
            continue;
        }
        
        for (int32 j = i + 1; j < ManagedNPCs.Num(); ++j)
        {
            UNPCBehaviorAgent* NPC2 = ManagedNPCs[j];
            if (!IsValid(NPC2) || !IsValid(NPC2->GetOwner()))
            {
                continue;
            }
            
            float Distance = FVector::Dist(
                NPC1->GetOwner()->GetActorLocation(),
                NPC2->GetOwner()->GetActorLocation()
            );
            
            if (Distance <= NPCInteractionRange)
            {
                ProcessNPCPairInteraction(NPC1, NPC2, Distance);
            }
        }
    }
}

void UNPCBehaviorManager::ProcessNPCPairInteraction(UNPCBehaviorAgent* NPC1, UNPCBehaviorAgent* NPC2, float Distance)
{
    if (!NPC1 || !NPC2)
    {
        return;
    }
    
    // Check if NPCs should interact based on their personalities and current behaviors
    bool ShouldInteract = false;
    
    // Social NPCs are more likely to interact
    float NPC1Sociability = NPC1->GetPersonality().Traits.FindRef(TEXT("Sociability"));
    float NPC2Sociability = NPC2->GetPersonality().Traits.FindRef(TEXT("Sociability"));
    
    float InteractionChance = (NPC1Sociability + NPC2Sociability) * 0.5f;
    InteractionChance *= (1.0f - (Distance / NPCInteractionRange)); // Closer = more likely
    
    if (FMath::RandRange(0.0f, 1.0f) < InteractionChance * 0.1f) // 10% base chance per update
    {
        ShouldInteract = true;
    }
    
    if (ShouldInteract)
    {
        // Create interaction event
        FNPCInteractionEvent InteractionEvent;
        InteractionEvent.NPC1 = NPC1;
        InteractionEvent.NPC2 = NPC2;
        InteractionEvent.InteractionType = DetermineInteractionType(NPC1, NPC2);
        InteractionEvent.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        InteractionEvent.Location = (NPC1->GetOwner()->GetActorLocation() + NPC2->GetOwner()->GetActorLocation()) * 0.5f;
        
        // Process the interaction
        ExecuteNPCInteraction(InteractionEvent);
        
        // Add to global memory
        GlobalMemory.SharedEvents.Add(InteractionEvent);
        
        // Limit global memory size
        if (GlobalMemory.SharedEvents.Num() > 100)
        {
            GlobalMemory.SharedEvents.RemoveAt(0);
        }
    }
}

ENPCInteractionType UNPCBehaviorManager::DetermineInteractionType(UNPCBehaviorAgent* NPC1, UNPCBehaviorAgent* NPC2)
{
    if (!NPC1 || !NPC2)
    {
        return ENPCInteractionType::Neutral;
    }
    
    // Determine interaction type based on personalities and current states
    float NPC1Aggressiveness = NPC1->GetPersonality().Traits.FindRef(TEXT("Aggressiveness"));
    float NPC2Aggressiveness = NPC2->GetPersonality().Traits.FindRef(TEXT("Aggressiveness"));
    
    float NPC1Sociability = NPC1->GetPersonality().Traits.FindRef(TEXT("Sociability"));
    float NPC2Sociability = NPC2->GetPersonality().Traits.FindRef(TEXT("Sociability"));
    
    float CombinedAggressiveness = (NPC1Aggressiveness + NPC2Aggressiveness) * 0.5f;
    float CombinedSociability = (NPC1Sociability + NPC2Sociability) * 0.5f;
    
    if (CombinedAggressiveness > 0.7f)
    {
        return ENPCInteractionType::Hostile;
    }
    else if (CombinedSociability > 0.7f)
    {
        return ENPCInteractionType::Friendly;
    }
    else if (CombinedSociability > 0.4f)
    {
        return ENPCInteractionType::Social;
    }
    else
    {
        return ENPCInteractionType::Neutral;
    }
}

void UNPCBehaviorManager::ExecuteNPCInteraction(const FNPCInteractionEvent& InteractionEvent)
{
    if (!InteractionEvent.NPC1 || !InteractionEvent.NPC2)
    {
        return;
    }
    
    // Apply interaction effects based on type
    switch (InteractionEvent.InteractionType)
    {
        case ENPCInteractionType::Friendly:
            // Reduce stress, increase happiness
            InteractionEvent.NPC1->GetEmotionalState().Stress *= 0.9f;
            InteractionEvent.NPC1->GetEmotionalState().Happiness *= 1.1f;
            InteractionEvent.NPC2->GetEmotionalState().Stress *= 0.9f;
            InteractionEvent.NPC2->GetEmotionalState().Happiness *= 1.1f;
            break;
            
        case ENPCInteractionType::Hostile:
            // Increase stress, reduce happiness
            InteractionEvent.NPC1->GetEmotionalState().Stress *= 1.2f;
            InteractionEvent.NPC1->GetEmotionalState().Happiness *= 0.8f;
            InteractionEvent.NPC2->GetEmotionalState().Stress *= 1.2f;
            InteractionEvent.NPC2->GetEmotionalState().Happiness *= 0.8f;
            break;
            
        case ENPCInteractionType::Social:
            // Moderate positive effect
            InteractionEvent.NPC1->GetEmotionalState().Happiness *= 1.05f;
            InteractionEvent.NPC2->GetEmotionalState().Happiness *= 1.05f;
            break;
            
        default:
            // Neutral interaction - no effect
            break;
    }
    
    // Update memory of each other
    if (InteractionEvent.NPC1->GetOwner() && InteractionEvent.NPC2->GetOwner())
    {
        // NPC1 remembers NPC2
        FNPCMemoryEntry Memory1;
        Memory1.LastSeenTime = InteractionEvent.Timestamp;
        Memory1.LastKnownLocation = InteractionEvent.NPC2->GetOwner()->GetActorLocation();
        Memory1.ThreatLevel = (InteractionEvent.InteractionType == ENPCInteractionType::Hostile) ? 0.8f : 0.2f;
        Memory1.Familiarity = 0.5f;
        InteractionEvent.NPC1->GetMemory().KnownActors.Add(InteractionEvent.NPC2->GetOwner(), Memory1);
        
        // NPC2 remembers NPC1
        FNPCMemoryEntry Memory2;
        Memory2.LastSeenTime = InteractionEvent.Timestamp;
        Memory2.LastKnownLocation = InteractionEvent.NPC1->GetOwner()->GetActorLocation();
        Memory2.ThreatLevel = (InteractionEvent.InteractionType == ENPCInteractionType::Hostile) ? 0.8f : 0.2f;
        Memory2.Familiarity = 0.5f;
        InteractionEvent.NPC2->GetMemory().KnownActors.Add(InteractionEvent.NPC1->GetOwner(), Memory2);
    }
    
    UE_LOG(LogNPCBehaviorManager, Log, TEXT("Executed %s interaction between %s and %s"), 
           *UEnum::GetValueAsString(InteractionEvent.InteractionType),
           InteractionEvent.NPC1->GetOwner() ? *InteractionEvent.NPC1->GetOwner()->GetName() : TEXT("Unknown"),
           InteractionEvent.NPC2->GetOwner() ? *InteractionEvent.NPC2->GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPCBehaviorManager::UpdateGlobalBehaviorModifiers()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Update time of day modifier
    float TimeOfDay = GetGameTimeOfDay();
    float TimeModifier = 1.0f;
    
    // Different behaviors are more active at different times
    if (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f) // Daytime
    {
        TimeModifier = 1.2f; // More active during day
    }
    else if (TimeOfDay >= 22.0f || TimeOfDay <= 4.0f) // Night
    {
        TimeModifier = 0.6f; // Less active at night
    }
    else // Dawn/Dusk
    {
        TimeModifier = 1.0f; // Normal activity
    }
    
    GlobalBehaviorModifiers[TEXT("TimeOfDay")] = TimeModifier;
    
    // Update player presence modifier
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    float PlayerPresenceModifier = 1.0f;
    
    if (PlayerPawn)
    {
        // Count NPCs near player
        int32 NPCsNearPlayer = 0;
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        for (UNPCBehaviorAgent* NPC : ManagedNPCs)
        {
            if (IsValid(NPC) && IsValid(NPC->GetOwner()))
            {
                float Distance = FVector::Dist(PlayerLocation, NPC->GetOwner()->GetActorLocation());
                if (Distance <= 2000.0f) // 20 meter radius
                {
                    NPCsNearPlayer++;
                }
            }
        }
        
        // More NPCs near player = more cautious behavior
        PlayerPresenceModifier = 1.0f + (NPCsNearPlayer * 0.1f);
    }
    
    GlobalBehaviorModifiers[TEXT("PlayerPresence")] = PlayerPresenceModifier;
    
    // Update threat level modifier based on recent hostile interactions
    float ThreatModifier = 1.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    int32 RecentHostileInteractions = 0;
    
    for (const FNPCInteractionEvent& Event : GlobalMemory.SharedEvents)
    {
        if (CurrentTime - Event.Timestamp <= 300.0f && // Within last 5 minutes
            Event.InteractionType == ENPCInteractionType::Hostile)
        {
            RecentHostileInteractions++;
        }
    }
    
    ThreatModifier = 1.0f + (RecentHostileInteractions * 0.2f);
    GlobalBehaviorModifiers[TEXT("ThreatLevel")] = ThreatModifier;
}

void UNPCBehaviorManager::ApplyGlobalModifiersToNPC(UNPCBehaviorAgent* NPC)
{
    if (!NPC)
    {
        return;
    }
    
    // Apply global modifiers to NPC behavior weights
    for (const auto& Modifier : GlobalBehaviorModifiers)
    {
        // This would ideally modify the NPC's behavior weights
        // For now, we'll just log the application
        UE_LOG(LogNPCBehaviorManager, VeryVerbose, TEXT("Applied %s modifier (%.2f) to %s"), 
               *Modifier.Key, Modifier.Value,
               NPC->GetOwner() ? *NPC->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UNPCBehaviorManager::UpdateGlobalStatistics()
{
    Statistics.TotalManagedNPCs = ManagedNPCs.Num();
    Statistics.ActiveNPCs = 0;
    Statistics.IdleNPCs = 0;
    Statistics.SocialInteractions = 0;
    Statistics.HostileInteractions = 0;
    
    // Count NPCs by behavior state
    for (UNPCBehaviorAgent* NPC : ManagedNPCs)
    {
        if (IsValid(NPC))
        {
            if (NPC->GetCurrentBehaviorType() != ENPCBehaviorType::Idle)
            {
                Statistics.ActiveNPCs++;
            }
            else
            {
                Statistics.IdleNPCs++;
            }
        }
    }
    
    // Count recent interactions
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    for (const FNPCInteractionEvent& Event : GlobalMemory.SharedEvents)
    {
        if (CurrentTime - Event.Timestamp <= 60.0f) // Last minute
        {
            if (Event.InteractionType == ENPCInteractionType::Social || 
                Event.InteractionType == ENPCInteractionType::Friendly)
            {
                Statistics.SocialInteractions++;
            }
            else if (Event.InteractionType == ENPCInteractionType::Hostile)
            {
                Statistics.HostileInteractions++;
            }
        }
    }
}

void UNPCBehaviorManager::CleanupGlobalMemory()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old events (older than 1 hour)
    GlobalMemory.SharedEvents.RemoveAll([CurrentTime](const FNPCInteractionEvent& Event)
    {
        return CurrentTime - Event.Timestamp > 3600.0f;
    });
    
    UE_LOG(LogNPCBehaviorManager, Log, TEXT("Cleaned up global memory. Events remaining: %d"), 
           GlobalMemory.SharedEvents.Num());
}

float UNPCBehaviorManager::GetGameTimeOfDay()
{
    if (UWorld* World = GetWorld())
    {
        float GameTime = World->GetTimeSeconds();
        return FMath::Fmod(GameTime / 3600.0f, 24.0f); // Convert to hours and wrap at 24
    }
    return 12.0f; // Default to noon
}

void UNPCBehaviorManager::OnNPCBehaviorChanged(ENPCBehaviorType OldBehavior, ENPCBehaviorType NewBehavior)
{
    // Log behavior changes for debugging
    UE_LOG(LogNPCBehaviorManager, VeryVerbose, TEXT("NPC behavior changed from %d to %d"), 
           (int32)OldBehavior, (int32)NewBehavior);
    
    // Could trigger global events based on behavior changes
    // For example, if many NPCs start fleeing, increase global threat level
}

TArray<UNPCBehaviorAgent*> UNPCBehaviorManager::GetNPCsInRadius(FVector Center, float Radius)
{
    TArray<UNPCBehaviorAgent*> NPCsInRadius;
    
    for (UNPCBehaviorAgent* NPC : ManagedNPCs)
    {
        if (IsValid(NPC) && IsValid(NPC->GetOwner()))
        {
            float Distance = FVector::Dist(Center, NPC->GetOwner()->GetActorLocation());
            if (Distance <= Radius)
            {
                NPCsInRadius.Add(NPC);
            }
        }
    }
    
    return NPCsInRadius;
}

TArray<UNPCBehaviorAgent*> UNPCBehaviorManager::GetNPCsByBehaviorType(ENPCBehaviorType BehaviorType)
{
    TArray<UNPCBehaviorAgent*> FilteredNPCs;
    
    for (UNPCBehaviorAgent* NPC : ManagedNPCs)
    {
        if (IsValid(NPC) && NPC->GetCurrentBehaviorType() == BehaviorType)
        {
            FilteredNPCs.Add(NPC);
        }
    }
    
    return FilteredNPCs;
}