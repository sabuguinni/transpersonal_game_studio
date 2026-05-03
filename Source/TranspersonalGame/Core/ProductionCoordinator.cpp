#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Landscape/Landscape.h"
#include "Engine/PlayerStart.h"

UProductionCoordinator::UProductionCoordinator()
{
    // Inicializar estado do Milestone 1
    bCharacterMovementReady = false;
    bLandscapeReady = false;
    bDinosaursSpawned = false;
    bLightingConfigured = false;
    TotalActorsInMap = 0;

    // Tarefas iniciais para Milestone 1
    PendingTasks.Add(TEXT("Agent #3: Implement TranspersonalCharacter movement"));
    PendingTasks.Add(TEXT("Agent #5: Expand landscape to 200km²"));
    PendingTasks.Add(TEXT("Agent #9: Create dinosaur actors with collision"));
    PendingTasks.Add(TEXT("Agent #8: Configure lighting system"));
    PendingTasks.Add(TEXT("Agent #12: Implement survival HUD"));
}

void UProductionCoordinator::CheckMilestone1Progress()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Checking Milestone 1 progress..."));

    // Verificar cada componente crítico
    bCharacterMovementReady = ValidateCharacterController();
    bLandscapeReady = ValidateLandscape();
    bDinosaursSpawned = ValidateDinosaurActors();
    bLightingConfigured = ValidateLightingSetup();

    // Actualizar estado geral
    UpdateMilestone1Status();
    LogProductionState();
}

void UProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription)
{
    FString FullTask = FString::Printf(TEXT("%s: %s"), *AgentName, *TaskDescription);
    
    if (!PendingTasks.Contains(FullTask))
    {
        PendingTasks.Add(FullTask);
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Assigned task to %s - %s"), *AgentName, *TaskDescription);
    }
}

void UProductionCoordinator::MarkTaskCompleted(const FString& TaskDescription)
{
    for (int32 i = PendingTasks.Num() - 1; i >= 0; i--)
    {
        if (PendingTasks[i].Contains(TaskDescription))
        {
            CompletedTasks.Add(PendingTasks[i]);
            PendingTasks.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Task completed - %s"), *TaskDescription);
            break;
        }
    }
}

float UProductionCoordinator::GetMilestone1CompletionPercentage()
{
    int32 CompletedComponents = 0;
    int32 TotalComponents = 4; // Character, Landscape, Dinosaurs, Lighting

    if (bCharacterMovementReady) CompletedComponents++;
    if (bLandscapeReady) CompletedComponents++;
    if (bDinosaursSpawned) CompletedComponents++;
    if (bLightingConfigured) CompletedComponents++;

    return (float)CompletedComponents / (float)TotalComponents * 100.0f;
}

void UProductionCoordinator::ValidateMinPlayableMap()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Validating MinPlayableMap..."));

    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }

    if (World)
    {
        // Contar todos os actores
        TotalActorsInMap = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            TotalActorsInMap++;
        }

        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Found %d actors in MinPlayableMap"), TotalActorsInMap);

        // Verificar duplicados críticos
        CleanupDuplicateActors();
    }
}

void UProductionCoordinator::CleanupDuplicateActors()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Cleaning up duplicate actors..."));

    // Limpar DirectionalLights duplicados
    RemoveDuplicateActorsOfType(ADirectionalLight::StaticClass(), 1);
    
    // Limpar SkyLights duplicados
    RemoveDuplicateActorsOfType(ASkyLight::StaticClass(), 1);
    
    // Limpar SkyAtmosphere duplicados
    RemoveDuplicateActorsOfType(ASkyAtmosphere::StaticClass(), 1);
    
    // Limpar ExponentialHeightFog duplicados
    RemoveDuplicateActorsOfType(AExponentialHeightFog::StaticClass(), 1);
}

bool UProductionCoordinator::ValidateCharacterController()
{
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }

    if (World)
    {
        // Verificar se existe PlayerStart
        int32 PlayerStartCount = CountActorsByClass(APlayerStart::StaticClass());
        
        // Verificar se existe GameMode configurado
        AGameModeBase* GameMode = World->GetAuthGameMode();
        
        bool bHasPlayerStart = PlayerStartCount > 0;
        bool bHasGameMode = GameMode != nullptr;
        
        UE_LOG(LogTemp, Warning, TEXT("Character Validation - PlayerStarts: %d, GameMode: %s"), 
               PlayerStartCount, bHasGameMode ? TEXT("Yes") : TEXT("No"));
        
        return bHasPlayerStart && bHasGameMode;
    }
    
    return false;
}

bool UProductionCoordinator::ValidateLandscape()
{
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }

    if (World)
    {
        int32 LandscapeCount = CountActorsByClass(ALandscape::StaticClass());
        
        UE_LOG(LogTemp, Warning, TEXT("Landscape Validation - Count: %d"), LandscapeCount);
        
        return LandscapeCount > 0;
    }
    
    return false;
}

bool UProductionCoordinator::ValidateDinosaurActors()
{
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }

    if (World)
    {
        int32 DinosaurCount = 0;
        
        // Contar actores que possam ser dinossauros (com mesh components)
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                FString ActorName = Actor->GetName();
                if (ActorName.Contains(TEXT("Dinosaur")) || 
                    ActorName.Contains(TEXT("TRex")) || 
                    ActorName.Contains(TEXT("Raptor")) || 
                    ActorName.Contains(TEXT("Brachio")))
                {
                    DinosaurCount++;
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur Validation - Count: %d"), DinosaurCount);
        
        return DinosaurCount >= 3; // Mínimo 3 dinossauros para Milestone 1
    }
    
    return false;
}

bool UProductionCoordinator::ValidateLightingSetup()
{
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }

    if (World)
    {
        int32 DirectionalLightCount = CountActorsByClass(ADirectionalLight::StaticClass());
        int32 SkyLightCount = CountActorsByClass(ASkyLight::StaticClass());
        int32 SkyAtmosphereCount = CountActorsByClass(ASkyAtmosphere::StaticClass());
        
        bool bHasBasicLighting = (DirectionalLightCount >= 1) && (SkyLightCount >= 1);
        
        UE_LOG(LogTemp, Warning, TEXT("Lighting Validation - DirectionalLight: %d, SkyLight: %d, SkyAtmosphere: %d"), 
               DirectionalLightCount, SkyLightCount, SkyAtmosphereCount);
        
        return bHasBasicLighting;
    }
    
    return false;
}

FString UProductionCoordinator::GenerateProgressReport()
{
    FString Report = TEXT("=== MILESTONE 1 PROGRESS REPORT ===\n");
    
    Report += FString::Printf(TEXT("Completion: %.1f%%\n"), GetMilestone1CompletionPercentage());
    Report += FString::Printf(TEXT("Character Movement: %s\n"), bCharacterMovementReady ? TEXT("READY") : TEXT("PENDING"));
    Report += FString::Printf(TEXT("Landscape: %s\n"), bLandscapeReady ? TEXT("READY") : TEXT("PENDING"));
    Report += FString::Printf(TEXT("Dinosaurs: %s\n"), bDinosaursSpawned ? TEXT("READY") : TEXT("PENDING"));
    Report += FString::Printf(TEXT("Lighting: %s\n"), bLightingConfigured ? TEXT("READY") : TEXT("PENDING"));
    Report += FString::Printf(TEXT("Total Actors: %d\n"), TotalActorsInMap);
    
    Report += TEXT("\nPENDING TASKS:\n");
    for (const FString& Task : PendingTasks)
    {
        Report += FString::Printf(TEXT("- %s\n"), *Task);
    }
    
    Report += TEXT("\nCOMPLETED TASKS:\n");
    for (const FString& Task : CompletedTasks)
    {
        Report += FString::Printf(TEXT("✓ %s\n"), *Task);
    }
    
    return Report;
}

TArray<FString> UProductionCoordinator::GetCriticalIssues()
{
    TArray<FString> Issues;
    
    if (!bCharacterMovementReady)
    {
        Issues.Add(TEXT("CRITICAL: Character movement system not implemented"));
    }
    
    if (!bLandscapeReady)
    {
        Issues.Add(TEXT("CRITICAL: Landscape not configured or too small"));
    }
    
    if (!bDinosaursSpawned)
    {
        Issues.Add(TEXT("HIGH: No dinosaur actors found in map"));
    }
    
    if (!bLightingConfigured)
    {
        Issues.Add(TEXT("MEDIUM: Basic lighting setup incomplete"));
    }
    
    if (TotalActorsInMap < 10)
    {
        Issues.Add(TEXT("LOW: Map appears empty - very few actors"));
    }
    
    return Issues;
}

void UProductionCoordinator::UpdateMilestone1Status()
{
    float CompletionPercentage = GetMilestone1CompletionPercentage();
    
    if (CompletionPercentage >= 100.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: MILESTONE 1 COMPLETED!"));
    }
    else if (CompletionPercentage >= 75.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Milestone 1 near completion (%.1f%%)"), CompletionPercentage);
    }
    else if (CompletionPercentage >= 25.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Milestone 1 in progress (%.1f%%)"), CompletionPercentage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Milestone 1 just started (%.1f%%)"), CompletionPercentage);
    }
}

void UProductionCoordinator::LogProductionState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Character Ready: %s"), bCharacterMovementReady ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Landscape Ready: %s"), bLandscapeReady ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs Spawned: %s"), bDinosaursSpawned ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Lighting Configured: %s"), bLightingConfigured ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorsInMap);
    UE_LOG(LogTemp, Warning, TEXT("Pending Tasks: %d"), PendingTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), CompletedTasks.Num());
}

int32 UProductionCoordinator::CountActorsByClass(UClass* ActorClass)
{
    if (!ActorClass)
    {
        return 0;
    }
    
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }

    if (World)
    {
        int32 Count = 0;
        for (TActorIterator<AActor> ActorItr(World, ActorClass); ActorItr; ++ActorItr)
        {
            Count++;
        }
        return Count;
    }
    
    return 0;
}

void UProductionCoordinator::RemoveDuplicateActorsOfType(UClass* ActorClass, int32 MaxAllowed)
{
    if (!ActorClass)
    {
        return;
    }
    
    UWorld* World = GEngine->GetCurrentPlayWorld();
    if (!World)
    {
        World = GEditor->GetEditorWorldContext().World();
    }

    if (World)
    {
        TArray<AActor*> ActorsOfType;
        
        for (TActorIterator<AActor> ActorItr(World, ActorClass); ActorItr; ++ActorItr)
        {
            ActorsOfType.Add(*ActorItr);
        }
        
        if (ActorsOfType.Num() > MaxAllowed)
        {
            int32 ToRemove = ActorsOfType.Num() - MaxAllowed;
            
            for (int32 i = MaxAllowed; i < ActorsOfType.Num(); i++)
            {
                if (ActorsOfType[i])
                {
                    ActorsOfType[i]->Destroy();
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Removed %d duplicate actors of type %s"), 
                   ToRemove, *ActorClass->GetName());
        }
    }
}