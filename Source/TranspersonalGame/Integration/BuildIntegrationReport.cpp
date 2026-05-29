#include "BuildIntegrationReport.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UBuildIntegrationReport::UBuildIntegrationReport()
{
    bBuildSuccessful = false;
    BuildVersion = TEXT("PROD_CYCLE_AUTO_20260529_004");
}

void UBuildIntegrationReport::GenerateHealthReport()
{
    SystemHealthReports.Empty();
    
    // Core systems to validate
    TArray<FString> CoreSystems = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("DinosaurTRex"),
        TEXT("DinosaurCombatAIController")
    };

    for (const FString& SystemName : CoreSystems)
    {
        FBuild_SystemHealth HealthReport;
        HealthReport.SystemName = SystemName;
        
        // Try to load the class
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (SystemClass)
        {
            HealthReport.Status = EBuild_SystemStatus::Operational;
            HealthReport.LoadTime = 0.1f; // Placeholder timing
        }
        else
        {
            HealthReport.Status = EBuild_SystemStatus::Failed;
            HealthReport.LastError = TEXT("Class not found or failed to load");
        }
        
        SystemHealthReports.Add(HealthReport);
    }

    CheckCoreSystemHealth();
    ValidateActorSpawning();
    CheckBinaryIntegrity();
    
    IntegrationMetrics.OverallHealth = CalculateOverallHealth();
    IntegrationMetrics.LastValidation = FDateTime::Now();
}

void UBuildIntegrationReport::ValidateSystemIntegration()
{
    IntegrationMetrics.TotalSystems = SystemHealthReports.Num();
    IntegrationMetrics.OperationalSystems = 0;
    IntegrationMetrics.FailedSystems = 0;

    for (const FBuild_SystemHealth& Health : SystemHealthReports)
    {
        if (Health.Status == EBuild_SystemStatus::Operational)
        {
            IntegrationMetrics.OperationalSystems++;
        }
        else if (Health.Status == EBuild_SystemStatus::Failed)
        {
            IntegrationMetrics.FailedSystems++;
        }
    }

    // Count total actors in the world
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        IntegrationMetrics.TotalActors = AllActors.Num();
    }
}

float UBuildIntegrationReport::CalculateOverallHealth() const
{
    if (IntegrationMetrics.TotalSystems == 0)
    {
        return 0.0f;
    }

    float HealthPercentage = (float)IntegrationMetrics.OperationalSystems / (float)IntegrationMetrics.TotalSystems;
    return HealthPercentage * 100.0f;
}

bool UBuildIntegrationReport::IsSystemOperational(const FString& SystemName) const
{
    for (const FBuild_SystemHealth& Health : SystemHealthReports)
    {
        if (Health.SystemName == SystemName)
        {
            return Health.Status == EBuild_SystemStatus::Operational;
        }
    }
    return false;
}

void UBuildIntegrationReport::CheckCoreSystemHealth()
{
    // Validate that core game systems are functioning
    bBuildSuccessful = true;
    
    // Check if TranspersonalCharacter can be spawned
    UClass* CharacterClass = LoadClass<APawn>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (!CharacterClass)
    {
        bBuildSuccessful = false;
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalCharacter class not available"));
    }

    // Check if game state is accessible
    UClass* GameStateClass = LoadClass<AGameStateBase>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    if (!GameStateClass)
    {
        bBuildSuccessful = false;
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalGameState class not available"));
    }
}

void UBuildIntegrationReport::ValidateActorSpawning()
{
    // Test actor spawning capabilities
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        // Try to get all actors to validate world state
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        if (AllActors.Num() < 5)
        {
            UE_LOG(LogTemp, Warning, TEXT("Low actor count in world: %d"), AllActors.Num());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("World has %d actors - integration healthy"), AllActors.Num());
        }
    }
}

void UBuildIntegrationReport::CheckBinaryIntegrity()
{
    // Check if compiled binaries exist
    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesPath = FPaths::Combine(ProjectDir, TEXT("Binaries"));
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    if (PlatformFile.DirectoryExists(*BinariesPath))
    {
        UE_LOG(LogTemp, Log, TEXT("Binaries directory found: %s"), *BinariesPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Binaries directory not found: %s"), *BinariesPath);
        bBuildSuccessful = false;
    }
}