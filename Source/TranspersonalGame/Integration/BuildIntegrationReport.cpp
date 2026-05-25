#include "BuildIntegrationReport.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

UBuildIntegrationReportManager::UBuildIntegrationReportManager()
{
    InitializeCoreClassPaths();
}

void UBuildIntegrationReportManager::InitializeCoreClassPaths()
{
    CoreClassPaths.Empty();
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.FoliageManager"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.ProceduralWorldManager"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.BuildIntegrationManager"));
}

FBuild_IntegrationReport UBuildIntegrationReportManager::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    Report.ReportTimestamp = FDateTime::Now();
    Report.ExpectedClassCount = CoreClassPaths.Num();
    
    // Validate core classes
    int32 LoadedClasses = 0;
    for (const FString& ClassPath : CoreClassPaths)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            LoadedClasses++;
        }
        else
        {
            Report.CriticalIssues.Add(FString::Printf(TEXT("Failed to load core class: %s"), *ClassPath));
        }
    }
    
    Report.LoadedClassCount = LoadedClasses;
    
    // Validate systems
    Report.SystemStatuses.Add(ValidateWorldGeneration());
    Report.SystemStatuses.Add(ValidateCharacterSystems());
    Report.SystemStatuses.Add(ValidateDinosaurSystems());
    Report.SystemStatuses.Add(ValidateVFXSystems());
    Report.SystemStatuses.Add(ValidateAudioSystems());
    
    // Calculate overall performance score
    Report.OverallPerformanceScore = CalculatePerformanceScore();
    
    // Count total actors in world
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        Report.TotalActorCount = World->GetActorCount();
    }
    else if (GEditor && GEditor->GetEditorWorldContext().World())
    {
        Report.TotalActorCount = GEditor->GetEditorWorldContext().World()->GetActorCount();
    }
    
    // Determine overall status
    if (Report.CriticalIssues.Num() > 0)
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Critical;
    }
    else if (LoadedClasses < CoreClassPaths.Num())
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Failed;
    }
    else if (Report.Warnings.Num() > 0)
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Warning;
    }
    else
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Success;
    }
    
    LastReport = Report;
    return Report;
}

EBuild_IntegrationStatus UBuildIntegrationReportManager::ValidateSystemIntegration(const FString& SystemName)
{
    if (SystemStatusCache.Contains(SystemName))
    {
        return SystemStatusCache[SystemName];
    }
    
    // Default validation logic
    EBuild_IntegrationStatus Status = EBuild_IntegrationStatus::Unknown;
    
    if (SystemName == TEXT("WorldGeneration"))
    {
        UClass* PCGClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
        Status = PCGClass ? EBuild_IntegrationStatus::Success : EBuild_IntegrationStatus::Failed;
    }
    else if (SystemName == TEXT("Character"))
    {
        UClass* CharClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
        Status = CharClass ? EBuild_IntegrationStatus::Success : EBuild_IntegrationStatus::Failed;
    }
    else if (SystemName == TEXT("Dinosaur"))
    {
        // Check for dinosaur assets and AI systems
        Status = EBuild_IntegrationStatus::Warning; // Placeholder
    }
    
    SystemStatusCache.Add(SystemName, Status);
    return Status;
}

bool UBuildIntegrationReportManager::ValidateCoreClasses()
{
    int32 LoadedCount = 0;
    for (const FString& ClassPath : CoreClassPaths)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            LoadedCount++;
        }
    }
    
    return LoadedCount >= CoreClassPaths.Num() * 0.8f; // 80% success rate
}

float UBuildIntegrationReportManager::CalculatePerformanceScore()
{
    float Score = 100.0f;
    
    // Deduct points for missing classes
    float ClassSuccessRate = (float)LastReport.LoadedClassCount / (float)FMath::Max(1, LastReport.ExpectedClassCount);
    Score *= ClassSuccessRate;
    
    // Deduct points for high actor count
    if (LastReport.TotalActorCount > 1000)
    {
        Score *= 0.9f;
    }
    
    // Deduct points for critical issues
    Score -= (LastReport.CriticalIssues.Num() * 10.0f);
    
    // Deduct points for warnings
    Score -= (LastReport.Warnings.Num() * 2.0f);
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

TArray<FString> UBuildIntegrationReportManager::GetCriticalIssues()
{
    return LastReport.CriticalIssues;
}

FBuild_SystemStatus UBuildIntegrationReportManager::ValidateWorldGeneration()
{
    FBuild_SystemStatus Status;
    Status.SystemName = TEXT("World Generation");
    
    UClass* PCGClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    UClass* FoliageClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.FoliageManager"));
    
    if (PCGClass && FoliageClass)
    {
        Status.Status = EBuild_IntegrationStatus::Success;
        Status.StatusMessage = TEXT("World generation systems loaded successfully");
        Status.PerformanceScore = 95.0f;
    }
    else
    {
        Status.Status = EBuild_IntegrationStatus::Failed;
        Status.StatusMessage = TEXT("Missing world generation classes");
        Status.PerformanceScore = 0.0f;
    }
    
    return Status;
}

FBuild_SystemStatus UBuildIntegrationReportManager::ValidateCharacterSystems()
{
    FBuild_SystemStatus Status;
    Status.SystemName = TEXT("Character Systems");
    
    UClass* CharClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    UClass* GameModeClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
    
    if (CharClass && GameModeClass)
    {
        Status.Status = EBuild_IntegrationStatus::Success;
        Status.StatusMessage = TEXT("Character systems operational");
        Status.PerformanceScore = 90.0f;
    }
    else
    {
        Status.Status = EBuild_IntegrationStatus::Failed;
        Status.StatusMessage = TEXT("Character system classes missing");
        Status.PerformanceScore = 0.0f;
    }
    
    return Status;
}

FBuild_SystemStatus UBuildIntegrationReportManager::ValidateDinosaurSystems()
{
    FBuild_SystemStatus Status;
    Status.SystemName = TEXT("Dinosaur Systems");
    Status.Status = EBuild_IntegrationStatus::Warning;
    Status.StatusMessage = TEXT("Dinosaur AI systems partially implemented");
    Status.PerformanceScore = 60.0f;
    
    return Status;
}

FBuild_SystemStatus UBuildIntegrationReportManager::ValidateVFXSystems()
{
    FBuild_SystemStatus Status;
    Status.SystemName = TEXT("VFX Systems");
    Status.Status = EBuild_IntegrationStatus::Warning;
    Status.StatusMessage = TEXT("VFX systems require validation");
    Status.PerformanceScore = 70.0f;
    
    return Status;
}

FBuild_SystemStatus UBuildIntegrationReportManager::ValidateAudioSystems()
{
    FBuild_SystemStatus Status;
    Status.SystemName = TEXT("Audio Systems");
    Status.Status = EBuild_IntegrationStatus::Warning;
    Status.StatusMessage = TEXT("Audio systems require integration testing");
    Status.PerformanceScore = 65.0f;
    
    return Status;
}