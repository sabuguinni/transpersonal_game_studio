#include "Narr_ContextualNarrativeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/Core/TranspersonalCharacter.h"
#include "TranspersonalGame/Core/TranspersonalGameState.h"

UNarr_ContextualNarrativeSystem::UNarr_ContextualNarrativeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Initialize narrative context tracking
    CurrentNarrativeContext = ENarr_NarrativeContext::Exploration;
    LastContextChangeTime = 0.0f;
    ContextStabilityThreshold = 3.0f;
    
    // Initialize biome-specific narrative triggers
    BiomeNarrativeData.Add(ENarr_BiomeType::Savana, FNarr_BiomeNarrativeData{
        TEXT("The vast grasslands stretch endlessly before you..."),
        TEXT("Wind whispers through the tall grass, carrying distant roars."),
        0.3f
    });
    
    BiomeNarrativeData.Add(ENarr_BiomeType::Forest, FNarr_BiomeNarrativeData{
        TEXT("Dense canopy blocks the sun, shadows dance between ancient trees..."),
        TEXT("Every rustle could hide a predator. Stay alert."),
        0.7f
    });
    
    BiomeNarrativeData.Add(ENarr_BiomeType::Desert, FNarr_BiomeNarrativeData{
        TEXT("The scorching sun beats down on endless dunes..."),
        TEXT("Water is life here. Every drop counts for survival."),
        0.4f
    });
    
    BiomeNarrativeData.Add(ENarr_BiomeType::Swamp, FNarr_BiomeNarrativeData{
        TEXT("Murky waters hide ancient secrets and deadly predators..."),
        TEXT("The air is thick with humidity and the stench of decay."),
        0.8f
    });
    
    BiomeNarrativeData.Add(ENarr_BiomeType::Mountain, FNarr_BiomeNarrativeData{
        TEXT("Rocky peaks pierce the sky, wind howls through stone corridors..."),
        TEXT("The thin air makes every breath precious at this altitude."),
        0.5f
    });
}

void UNarr_ContextualNarrativeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to player character
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerCharacter = Cast<ATranspersonalCharacter>(PC->GetPawn());
        }
        
        // Get game state reference
        GameStateRef = Cast<ATranspersonalGameState>(World->GetGameState());
    }
    
    // Initialize narrative state
    LastContextChangeTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Contextual Narrative System initialized"));
}

void UNarr_ContextualNarrativeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!PlayerCharacter || !GameStateRef)
    {
        return;
    }
    
    // Evaluate current narrative context
    ENarr_NarrativeContext NewContext = EvaluateNarrativeContext();
    
    // Check if context has changed and is stable
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (NewContext != CurrentNarrativeContext)
    {
        if (CurrentTime - LastContextChangeTime > ContextStabilityThreshold)
        {
            // Context change is stable, trigger narrative event
            OnNarrativeContextChanged(CurrentNarrativeContext, NewContext);
            CurrentNarrativeContext = NewContext;
            LastContextChangeTime = CurrentTime;
        }
    }
    else
    {
        // Context is stable, reset timer
        LastContextChangeTime = CurrentTime;
    }
    
    // Check for biome-specific narrative triggers
    CheckBiomeNarrativeTriggers();
}

ENarr_NarrativeContext UNarr_ContextualNarrativeSystem::EvaluateNarrativeContext()
{
    if (!PlayerCharacter || !GameStateRef)
    {
        return ENarr_NarrativeContext::Exploration;
    }
    
    // Check survival stats for context
    float HealthPercent = PlayerCharacter->GetHealthPercent();
    float HungerPercent = PlayerCharacter->GetHungerPercent();
    float ThirstPercent = PlayerCharacter->GetThirstPercent();
    float FearLevel = PlayerCharacter->GetFearLevel();
    
    // Critical survival situation
    if (HealthPercent < 0.2f || HungerPercent < 0.1f || ThirstPercent < 0.1f)
    {
        return ENarr_NarrativeContext::CriticalSurvival;
    }
    
    // High fear/danger situation
    if (FearLevel > 0.7f)
    {
        return ENarr_NarrativeContext::Danger;
    }
    
    // Resource gathering context
    if (HungerPercent < 0.5f || ThirstPercent < 0.5f)
    {
        return ENarr_NarrativeContext::ResourceGathering;
    }
    
    // Check for nearby dinosaurs (danger context)
    if (CheckForNearbyThreats())
    {
        return ENarr_NarrativeContext::Danger;
    }
    
    // Default to exploration
    return ENarr_NarrativeContext::Exploration;
}

bool UNarr_ContextualNarrativeSystem::CheckForNearbyThreats()
{
    if (!PlayerCharacter)
    {
        return false;
    }
    
    // Get player location
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Check for actors with "Dinosaur" or "Predator" tags within threat range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Dinosaur"), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && FVector::Dist(PlayerLocation, Actor->GetActorLocation()) < ThreatDetectionRange)
        {
            return true;
        }
    }
    
    return false;
}

void UNarr_ContextualNarrativeSystem::CheckBiomeNarrativeTriggers()
{
    if (!PlayerCharacter)
    {
        return;
    }
    
    // Get current biome based on player location
    ENarr_BiomeType CurrentBiome = DetermineBiomeFromLocation(PlayerCharacter->GetActorLocation());
    
    // Check if biome has changed
    if (CurrentBiome != LastBiome)
    {
        TriggerBiomeNarrative(CurrentBiome);
        LastBiome = CurrentBiome;
    }
}

ENarr_BiomeType UNarr_ContextualNarrativeSystem::DetermineBiomeFromLocation(const FVector& Location)
{
    // Biome determination based on coordinates
    // Savana: center area
    if (FMath::Abs(Location.X) < 20000 && FMath::Abs(Location.Y) < 20000)
    {
        return ENarr_BiomeType::Savana;
    }
    
    // Forest: northwest
    if (Location.X < -15000 && Location.Y > 15000)
    {
        return ENarr_BiomeType::Forest;
    }
    
    // Desert: east
    if (Location.X > 25000 && FMath::Abs(Location.Y) < 30000)
    {
        return ENarr_BiomeType::Desert;
    }
    
    // Swamp: southwest
    if (Location.X < -25000 && Location.Y < -15000)
    {
        return ENarr_BiomeType::Swamp;
    }
    
    // Mountain: northeast
    if (Location.X > 15000 && Location.Y > 20000)
    {
        return ENarr_BiomeType::Mountain;
    }
    
    // Default to Savana
    return ENarr_BiomeType::Savana;
}

void UNarr_ContextualNarrativeSystem::OnNarrativeContextChanged(ENarr_NarrativeContext OldContext, ENarr_NarrativeContext NewContext)
{
    UE_LOG(LogTemp, Warning, TEXT("Narrative context changed from %d to %d"), (int32)OldContext, (int32)NewContext);
    
    // Trigger appropriate narrative response
    FString NarrativeText = GetContextualNarrativeText(NewContext);
    
    if (!NarrativeText.IsEmpty())
    {
        // Broadcast narrative event
        OnNarrativeTriggered.Broadcast(NarrativeText, NewContext);
        
        // Play contextual audio if available
        PlayContextualAudio(NewContext);
    }
}

void UNarr_ContextualNarrativeSystem::TriggerBiomeNarrative(ENarr_BiomeType BiomeType)
{
    if (FNarr_BiomeNarrativeData* BiomeData = BiomeNarrativeData.Find(BiomeType))
    {
        // Choose between entrance and ambient text based on random chance
        FString NarrativeText = (FMath::RandRange(0.0f, 1.0f) < 0.7f) ? 
            BiomeData->EntranceText : BiomeData->AmbientText;
        
        UE_LOG(LogTemp, Warning, TEXT("Biome narrative triggered: %s"), *NarrativeText);
        
        // Broadcast biome narrative event
        OnBiomeNarrativeTriggered.Broadcast(NarrativeText, BiomeType);
        
        // Play biome-specific audio
        PlayBiomeAudio(BiomeType);
    }
}

FString UNarr_ContextualNarrativeSystem::GetContextualNarrativeText(ENarr_NarrativeContext Context)
{
    switch (Context)
    {
        case ENarr_NarrativeContext::Exploration:
            return TEXT("The wilderness stretches before you, full of mystery and danger.");
            
        case ENarr_NarrativeContext::Danger:
            return TEXT("Your heart pounds. Something is watching you from the shadows.");
            
        case ENarr_NarrativeContext::ResourceGathering:
            return TEXT("Survival depends on finding resources. Every decision matters.");
            
        case ENarr_NarrativeContext::CriticalSurvival:
            return TEXT("Death stalks you. Every breath could be your last.");
            
        case ENarr_NarrativeContext::Discovery:
            return TEXT("You've found something remarkable. This changes everything.");
            
        case ENarr_NarrativeContext::Rest:
            return TEXT("A moment of peace in this hostile world. Savor it while you can.");
            
        default:
            return TEXT("");
    }
}

void UNarr_ContextualNarrativeSystem::PlayContextualAudio(ENarr_NarrativeContext Context)
{
    // Implementation for playing context-specific audio
    // This would integrate with the audio system
    UE_LOG(LogTemp, Log, TEXT("Playing contextual audio for context: %d"), (int32)Context);
}

void UNarr_ContextualNarrativeSystem::PlayBiomeAudio(ENarr_BiomeType BiomeType)
{
    // Implementation for playing biome-specific audio
    // This would integrate with the audio system
    UE_LOG(LogTemp, Log, TEXT("Playing biome audio for biome: %d"), (int32)BiomeType);
}

void UNarr_ContextualNarrativeSystem::TriggerManualNarrative(const FString& NarrativeText, ENarr_NarrativeContext Context)
{
    if (!NarrativeText.IsEmpty())
    {
        OnNarrativeTriggered.Broadcast(NarrativeText, Context);
        UE_LOG(LogTemp, Warning, TEXT("Manual narrative triggered: %s"), *NarrativeText);
    }
}

ENarr_NarrativeContext UNarr_ContextualNarrativeSystem::GetCurrentNarrativeContext() const
{
    return CurrentNarrativeContext;
}

ENarr_BiomeType UNarr_ContextualNarrativeSystem::GetCurrentBiome() const
{
    return LastBiome;
}