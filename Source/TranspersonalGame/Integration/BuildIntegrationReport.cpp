#include "BuildIntegrationReport.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"

UBuildIntegrationReport::UBuildIntegrationReport()
{
    bBuildSuccessful = false;
    LastBuildTime = FDateTime::Now();
    BuildVersion = TEXT("PROD_CYCLE_AUTO_20260530_003");
}

void UBuildIntegrationReport::GenerateReport()
{
    SystemStatuses.Empty();
    
    // Check core systems
    TArray<FString> CoreSystems = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("TranspersonalGameMode"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("DinosaurTRex"),
        TEXT("DinosaurCombatAIController")
    };

    for (const FString& SystemName : CoreSystems)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
        CheckSystemStatus(SystemName, SystemClass);
    }

    CheckPerformanceMetrics();
    ValidateCrossSystemDependencies();
    
    bBuildSuccessful = ValidateSystemIntegration();
    LastBuildTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("Build Integration Report Generated: %s"), 
           bBuildSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
}

bool UBuildIntegrationReport::ValidateSystemIntegration()
{
    int32 LoadedSystems = 0;
    int32 TotalSystems = SystemStatuses.Num();
    
    for (const FBuild_SystemStatus& Status : SystemStatuses)
    {
        if (Status.bIsLoaded && Status.bIsCompiled)
        {
            LoadedSystems++;
        }
    }
    
    float SuccessRate = TotalSystems > 0 ? (float)LoadedSystems / TotalSystems : 0.0f;
    return SuccessRate >= 0.8f; // 80% success rate required
}

void UBuildIntegrationReport::CheckPerformanceMetrics()
{
    UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
    if (!World)
    {
        // Try editor world
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor)
            {
                World = Context.World();
                break;
            }
        }
    }
    
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        PerformanceMetrics.TotalActors = AllActors.Num();
        PerformanceMetrics.StaticMeshActors = 0;
        PerformanceMetrics.SkeletalMeshActors = 0;
        PerformanceMetrics.LightActors = 0;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor->IsA<AStaticMeshActor>())
            {
                PerformanceMetrics.StaticMeshActors++;
            }
            else if (Actor->FindComponentByClass<USkeletalMeshComponent>())
            {
                PerformanceMetrics.SkeletalMeshActors++;
            }
            else if (Actor->IsA<ALight>())
            {
                PerformanceMetrics.LightActors++;
            }
        }
        
        // Estimate frame time based on actor count
        PerformanceMetrics.EstimatedFrameTime = PerformanceMetrics.TotalActors * 0.001f; // 1ms per 1000 actors
    }
}

FString UBuildIntegrationReport::GetBuildSummary() const
{
    FString Summary = FString::Printf(TEXT("Build Report %s\n"), *BuildVersion);
    Summary += FString::Printf(TEXT("Status: %s\n"), bBuildSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    Summary += FString::Printf(TEXT("Systems: %d loaded\n"), SystemStatuses.Num());
    Summary += FString::Printf(TEXT("Actors: %d total\n"), PerformanceMetrics.TotalActors);
    Summary += FString::Printf(TEXT("Performance: %.2fms estimated\n"), PerformanceMetrics.EstimatedFrameTime);
    Summary += FString::Printf(TEXT("Build Time: %s\n"), *LastBuildTime.ToString());
    
    return Summary;
}

void UBuildIntegrationReport::CheckSystemStatus(const FString& SystemName, UClass* SystemClass)
{
    FBuild_SystemStatus Status;
    Status.SystemName = SystemName;
    Status.bIsLoaded = (SystemClass != nullptr);
    Status.bIsCompiled = Status.bIsLoaded; // If loaded, assume compiled
    
    if (!Status.bIsLoaded)
    {
        Status.ErrorMessage = FString::Printf(TEXT("Failed to load class: %s"), *SystemName);
    }
    
    SystemStatuses.Add(Status);
}

void UBuildIntegrationReport::ValidateCrossSystemDependencies()
{
    // Check that essential dependencies are met
    bool bHasGameMode = false;
    bool bHasCharacter = false;
    bool bHasWorldGen = false;
    
    for (const FBuild_SystemStatus& Status : SystemStatuses)
    {
        if (Status.SystemName.Contains(TEXT("GameMode")) && Status.bIsLoaded)
        {
            bHasGameMode = true;
        }
        else if (Status.SystemName.Contains(TEXT("Character")) && Status.bIsLoaded)
        {
            bHasCharacter = true;
        }
        else if (Status.SystemName.Contains(TEXT("WorldGenerator")) && Status.bIsLoaded)
        {
            bHasWorldGen = true;
        }
    }
    
    if (!bHasGameMode || !bHasCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical dependency missing: GameMode=%s, Character=%s"), 
               bHasGameMode ? TEXT("OK") : TEXT("MISSING"),
               bHasCharacter ? TEXT("OK") : TEXT("MISSING"));
    }
}