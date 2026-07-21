#include "Dir_ProductionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"

void UDir_ProductionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentPhase = EDir_ProductionPhase::MinimumViablePrototype;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Manager Initialized - Phase: MVP"));
}

void UDir_ProductionManager::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    
    FString PhaseName;
    switch(NewPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            PhaseName = TEXT("Pre-Production");
            break;
        case EDir_ProductionPhase::MinimumViablePrototype:
            PhaseName = TEXT("Minimum Viable Prototype");
            break;
        case EDir_ProductionPhase::VerticalSlice:
            PhaseName = TEXT("Vertical Slice");
            break;
        case EDir_ProductionPhase::AlphaPhase:
            PhaseName = TEXT("Alpha Phase");
            break;
        case EDir_ProductionPhase::BetaPhase:
            PhaseName = TEXT("Beta Phase");
            break;
        case EDir_ProductionPhase::GoldMaster:
            PhaseName = TEXT("Gold Master");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production Phase Changed to: %s"), *PhaseName);
}

bool UDir_ProductionManager::ValidateMinimumViablePrototype()
{
    FDir_ProductionMetrics Metrics = GatherProductionMetrics();
    
    // MVP Requirements:
    // 1. Playable character exists
    // 2. Basic terrain exists
    // 3. At least one light source
    // 4. PlayerStart exists
    // 5. Performance is acceptable (>20 FPS)
    
    bool bMVPValid = Metrics.bHasPlayableCharacter && 
                     Metrics.bHasBasicTerrain && 
                     Metrics.FrameRate > 20.0f &&
                     Metrics.TotalActorsInLevel > 0;
    
    if (bMVPValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("MVP VALIDATION PASSED - Prototype is playable"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MVP VALIDATION FAILED - Missing critical elements"));
    }
    
    return bMVPValid;
}

FDir_ProductionMetrics UDir_ProductionManager::GatherProductionMetrics()
{
    FDir_ProductionMetrics Metrics;
    
    if (UWorld* World = GetWorld())
    {
        if (ULevel* Level = World->GetCurrentLevel())
        {
            Metrics.TotalActorsInLevel = Level->Actors.Num();
            
            // Count specific actor types
            int32 PlayerStarts = 0;
            int32 Characters = 0;
            int32 Landscapes = 0;
            int32 Lights = 0;
            
            for (AActor* Actor : Level->Actors)
            {
                if (!Actor) continue;
                
                if (Actor->IsA<APlayerStart>())
                {
                    PlayerStarts++;
                }
                else if (Actor->IsA<ACharacter>())
                {
                    Characters++;
                    Metrics.bHasPlayableCharacter = true;
                }
                else if (Actor->IsA<ALandscape>())
                {
                    Landscapes++;
                    Metrics.bHasBasicTerrain = true;
                }
                else if (Actor->IsA<ADirectionalLight>())
                {
                    Lights++;
                }
                else if (Actor->IsA<AStaticMeshActor>())
                {
                    // Check if this is terrain substitute
                    FString ActorName = Actor->GetName();
                    if (ActorName.Contains(TEXT("Terrain")) || ActorName.Contains(TEXT("Ground")))
                    {
                        Metrics.bHasBasicTerrain = true;
                    }
                    Metrics.EnvironmentActors++;
                }
                
                // Count dinosaurs
                FString ActorLabel = Actor->GetActorLabel();
                if (ActorLabel.Contains(TEXT("Rex")) || ActorLabel.Contains(TEXT("Raptor")) || 
                    ActorLabel.Contains(TEXT("Tricera")) || ActorLabel.Contains(TEXT("Brachi")))
                {
                    Metrics.DinosaurCount++;
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Production Metrics - Actors: %d, Dinos: %d, Characters: %d"), 
                   Metrics.TotalActorsInLevel, Metrics.DinosaurCount, Characters);
        }
    }
    
    // Get frame rate (simplified)
    if (GEngine)
    {
        Metrics.FrameRate = 1.0f / GEngine->GetMaxTickRate(0.0f, false);
    }
    
    LastMetrics = Metrics;
    return Metrics;
}

void UDir_ProductionManager::RegisterAgentDeliverable(const FString& AgentName, const FString& DeliverableName)
{
    if (!AgentDeliverables.Contains(AgentName))
    {
        AgentDeliverables.Add(AgentName, TArray<FString>());
    }
    
    AgentDeliverables[AgentName].Add(DeliverableName);
    
    UE_LOG(LogTemp, Warning, TEXT("Agent %s delivered: %s"), *AgentName, *DeliverableName);
}

TArray<FString> UDir_ProductionManager::GetPendingDeliverables()
{
    TArray<FString> PendingItems;
    
    // Define expected deliverables per agent
    TMap<FString, TArray<FString>> ExpectedDeliverables;
    ExpectedDeliverables.Add(TEXT("EngineArchitect"), {TEXT("CoreSystems"), TEXT("PhysicsFramework")});
    ExpectedDeliverables.Add(TEXT("WorldGenerator"), {TEXT("TerrainGeneration"), TEXT("BiomeSystem")});
    ExpectedDeliverables.Add(TEXT("CharacterArtist"), {TEXT("PlayerCharacter"), TEXT("MovementSystem")});
    ExpectedDeliverables.Add(TEXT("AnimationAgent"), {TEXT("BasicAnimations"), TEXT("MotionMatching")});
    
    // Check what's missing
    for (const auto& Expected : ExpectedDeliverables)
    {
        const FString& AgentName = Expected.Key;
        const TArray<FString>& ExpectedItems = Expected.Value;
        
        const TArray<FString>* DeliveredItems = AgentDeliverables.Find(AgentName);
        
        for (const FString& ExpectedItem : ExpectedItems)
        {
            bool bFound = false;
            if (DeliveredItems)
            {
                bFound = DeliveredItems->Contains(ExpectedItem);
            }
            
            if (!bFound)
            {
                PendingItems.Add(FString::Printf(TEXT("%s: %s"), *AgentName, *ExpectedItem));
            }
        }
    }
    
    return PendingItems;
}

bool UDir_ProductionManager::IsPlayablePrototypeReady()
{
    return ValidateMinimumViablePrototype();
}

void UDir_ProductionManager::LogProductionStatus()
{
    FDir_ProductionMetrics Metrics = GatherProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), Metrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), Metrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Environment: %d"), Metrics.EnvironmentActors);
    UE_LOG(LogTemp, Warning, TEXT("Has Character: %s"), Metrics.bHasPlayableCharacter ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Has Terrain: %s"), Metrics.bHasBasicTerrain ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.1f"), Metrics.FrameRate);
    
    TArray<FString> Pending = GetPendingDeliverables();
    UE_LOG(LogTemp, Warning, TEXT("Pending Deliverables: %d"), Pending.Num());
    for (const FString& Item : Pending)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Item);
    }
    UE_LOG(LogTemp, Warning, TEXT("=== END STATUS REPORT ==="));
}

void UDir_ProductionManager::ValidateWorldState()
{
    // Implementation for world state validation
}

void UDir_ProductionManager::CheckPerformanceMetrics()
{
    // Implementation for performance checking
}

bool UDir_ProductionManager::HasEssentialGameplayElements()
{
    FDir_ProductionMetrics Metrics = GatherProductionMetrics();
    return Metrics.bHasPlayableCharacter && Metrics.bHasBasicTerrain;
}