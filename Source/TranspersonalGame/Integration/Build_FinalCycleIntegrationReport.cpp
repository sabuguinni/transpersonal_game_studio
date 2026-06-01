#include "Build_FinalCycleIntegrationReport.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UBuild_FinalCycleIntegrationReport::UBuild_FinalCycleIntegrationReport()
{
    PrimaryComponentTick.bCanEverTick = false;
    bReadyForHandoff = false;
    CycleID = TEXT("PROD_CYCLE_AUTO_20260601_010");
    ReportTimestamp = FDateTime::Now();
    
    InitializeReport();
}

void UBuild_FinalCycleIntegrationReport::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-generate report on begin play
    GenerateIntegrationReport();
}

void UBuild_FinalCycleIntegrationReport::InitializeReport()
{
    // Clear previous data
    ModuleData.Empty();
    CriticalIssues.Empty();
    Recommendations.Empty();
    
    // Initialize metrics
    Metrics = FBuild_CycleIntegrationMetrics();
    Metrics.Status = EBuild_IntegrationStatus::InProgress;
    
    UE_LOG(LogTemp, Log, TEXT("Build_FinalCycleIntegrationReport: Initialized for cycle %s"), *CycleID);
}

void UBuild_FinalCycleIntegrationReport::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Log, TEXT("Build_FinalCycleIntegrationReport: Generating comprehensive integration report"));
    
    InitializeReport();
    ValidateWorldState();
    CheckModuleDependencies();
    CalculateIntegrationMetrics();
    CheckSystemLimits();
    AnalyzePerformanceMetrics();
    GenerateRecommendations();
    
    // Determine final status
    if (CriticalIssues.Num() == 0 && Metrics.bWithinActorLimits && Metrics.bWithinDinosaurLimits)
    {
        Metrics.Status = EBuild_IntegrationStatus::Success;
        bReadyForHandoff = true;
    }
    else if (CriticalIssues.Num() > 0)
    {
        Metrics.Status = EBuild_IntegrationStatus::Critical;
        bReadyForHandoff = false;
    }
    else
    {
        Metrics.Status = EBuild_IntegrationStatus::Failed;
        bReadyForHandoff = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Integration Report Complete - Status: %s, Ready for Handoff: %s"), 
           *UEnum::GetValueAsString(Metrics.Status), bReadyForHandoff ? TEXT("YES") : TEXT("NO"));
}

void UBuild_FinalCycleIntegrationReport::ValidateModuleIntegration(const FString& ModuleName)
{
    FBuild_ModuleIntegrationData ModuleInfo;
    ModuleInfo.ModuleName = ModuleName;
    
    // Try to load the module class
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
    UClass* ModuleClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (ModuleClass)
    {
        ModuleInfo.bIsLoaded = true;
        ModuleInfo.bHasValidation = true;
        ModuleInfo.IntegrationScore = 100.0f;
        UE_LOG(LogTemp, Log, TEXT("Module %s: LOADED successfully"), *ModuleName);
    }
    else
    {
        ModuleInfo.bIsLoaded = false;
        ModuleInfo.bHasValidation = false;
        ModuleInfo.IntegrationScore = 0.0f;
        ModuleInfo.ValidationErrors.Add(FString::Printf(TEXT("Failed to load class %s"), *ClassPath));
        UE_LOG(LogTemp, Error, TEXT("Module %s: FAILED to load"), *ModuleName);
    }
    
    ModuleData.Add(ModuleInfo);
}

void UBuild_FinalCycleIntegrationReport::ValidateWorldState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddCriticalIssue(TEXT("World is null - cannot validate world state"));
        return;
    }
    
    // Count all actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    Metrics.TotalActors = AllActors.Num();
    
    // Count dinosaur actors
    int32 DinoCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("Dino"), ESearchCase::IgnoreCase))
        {
            DinoCount++;
        }
    }
    Metrics.DinosaurCount = DinoCount;
    
    UE_LOG(LogTemp, Log, TEXT("World State: %d total actors, %d dinosaurs"), Metrics.TotalActors, Metrics.DinosaurCount);
}

void UBuild_FinalCycleIntegrationReport::CheckModuleDependencies()
{
    // Core modules that should be loaded
    TArray<FString> CoreModules = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("VFX_NiagaraLibrary"),
        TEXT("QA_CompilationValidator"),
        TEXT("BuildIntegrationManager")
    };
    
    int32 LoadedCount = 0;
    for (const FString& ModuleName : CoreModules)
    {
        ValidateModuleIntegration(ModuleName);
        
        // Check if this module was loaded successfully
        for (const FBuild_ModuleIntegrationData& Data : ModuleData)
        {
            if (Data.ModuleName == ModuleName && Data.bIsLoaded)
            {
                LoadedCount++;
                break;
            }
        }
    }
    
    Metrics.LoadedModules = LoadedCount;
    Metrics.OverallIntegrationScore = (float)LoadedCount / (float)CoreModules.Num() * 100.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Module Dependencies: %d/%d loaded (%.1f%%)"), 
           LoadedCount, CoreModules.Num(), Metrics.OverallIntegrationScore);
}

void UBuild_FinalCycleIntegrationReport::CalculateIntegrationMetrics()
{
    // Calculate memory usage estimate (rough approximation)
    Metrics.MemoryUsageEstimate = (float)Metrics.TotalActors * 0.5f; // 0.5MB per actor estimate
    
    UE_LOG(LogTemp, Log, TEXT("Integration Metrics: %.1f%% score, %.1fMB estimated memory"), 
           Metrics.OverallIntegrationScore, Metrics.MemoryUsageEstimate);
}

void UBuild_FinalCycleIntegrationReport::CheckSystemLimits()
{
    // Check actor limits (8000 max)
    Metrics.bWithinActorLimits = (Metrics.TotalActors <= 8000);
    if (!Metrics.bWithinActorLimits)
    {
        AddCriticalIssue(FString::Printf(TEXT("Actor count %d exceeds limit of 8000"), Metrics.TotalActors));
    }
    
    // Check dinosaur limits (150 max)
    Metrics.bWithinDinosaurLimits = (Metrics.DinosaurCount <= 150);
    if (!Metrics.bWithinDinosaurLimits)
    {
        AddCriticalIssue(FString::Printf(TEXT("Dinosaur count %d exceeds limit of 150"), Metrics.DinosaurCount));
    }
    
    UE_LOG(LogTemp, Log, TEXT("System Limits: Actors %s, Dinosaurs %s"), 
           Metrics.bWithinActorLimits ? TEXT("OK") : TEXT("EXCEEDED"),
           Metrics.bWithinDinosaurLimits ? TEXT("OK") : TEXT("EXCEEDED"));
}

void UBuild_FinalCycleIntegrationReport::AnalyzePerformanceMetrics()
{
    // Performance analysis based on current metrics
    if (Metrics.MemoryUsageEstimate > 4000.0f) // 4GB threshold
    {
        AddCriticalIssue(TEXT("Estimated memory usage exceeds 4GB - performance risk"));
    }
    
    if (Metrics.OverallIntegrationScore < 80.0f)
    {
        AddCriticalIssue(TEXT("Integration score below 80% - missing critical modules"));
    }
}

void UBuild_FinalCycleIntegrationReport::GenerateRecommendations()
{
    if (Metrics.TotalActors > 6000)
    {
        AddRecommendation(TEXT("Consider implementing LOD system for high actor count"));
    }
    
    if (Metrics.DinosaurCount > 100)
    {
        AddRecommendation(TEXT("Implement dinosaur culling system for performance"));
    }
    
    if (Metrics.OverallIntegrationScore < 100.0f)
    {
        AddRecommendation(TEXT("Fix module loading issues before final build"));
    }
    
    AddRecommendation(TEXT("Run full compilation test before handoff to Studio Director"));
}

void UBuild_FinalCycleIntegrationReport::AddCriticalIssue(const FString& Issue)
{
    CriticalIssues.Add(Issue);
    UE_LOG(LogTemp, Error, TEXT("CRITICAL ISSUE: %s"), *Issue);
}

void UBuild_FinalCycleIntegrationReport::AddRecommendation(const FString& Recommendation)
{
    Recommendations.Add(Recommendation);
    UE_LOG(LogTemp, Warning, TEXT("RECOMMENDATION: %s"), *Recommendation);
}

bool UBuild_FinalCycleIntegrationReport::IsIntegrationSuccessful() const
{
    return (Metrics.Status == EBuild_IntegrationStatus::Success) && bReadyForHandoff;
}

FString UBuild_FinalCycleIntegrationReport::GetReportSummary() const
{
    FString Summary = FString::Printf(
        TEXT("Integration Report Summary for %s:\n")
        TEXT("Status: %s\n")
        TEXT("Integration Score: %.1f%%\n")
        TEXT("Actors: %d/%d\n")
        TEXT("Dinosaurs: %d/%d\n")
        TEXT("Loaded Modules: %d\n")
        TEXT("Critical Issues: %d\n")
        TEXT("Ready for Handoff: %s"),
        *CycleID,
        *UEnum::GetValueAsString(Metrics.Status),
        Metrics.OverallIntegrationScore,
        Metrics.TotalActors, 8000,
        Metrics.DinosaurCount, 150,
        Metrics.LoadedModules,
        CriticalIssues.Num(),
        bReadyForHandoff ? TEXT("YES") : TEXT("NO")
    );
    
    return Summary;
}

void UBuild_FinalCycleIntegrationReport::ExportReportToFile(const FString& FilePath)
{
    FString ReportContent = GetReportSummary();
    
    // Add detailed module information
    ReportContent += TEXT("\n\nModule Details:\n");
    for (const FBuild_ModuleIntegrationData& Module : ModuleData)
    {
        ReportContent += FString::Printf(TEXT("- %s: %s (Score: %.1f%%)\n"), 
                                       *Module.ModuleName, 
                                       Module.bIsLoaded ? TEXT("LOADED") : TEXT("FAILED"),
                                       Module.IntegrationScore);
    }
    
    // Add critical issues
    if (CriticalIssues.Num() > 0)
    {
        ReportContent += TEXT("\nCritical Issues:\n");
        for (const FString& Issue : CriticalIssues)
        {
            ReportContent += FString::Printf(TEXT("- %s\n"), *Issue);
        }
    }
    
    // Add recommendations
    if (Recommendations.Num() > 0)
    {
        ReportContent += TEXT("\nRecommendations:\n");
        for (const FString& Rec : Recommendations)
        {
            ReportContent += FString::Printf(TEXT("- %s\n"), *Rec);
        }
    }
    
    // Write to file
    if (!FFileHelper::SaveStringToFile(ReportContent, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to export report to file: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Integration report exported to: %s"), *FilePath);
    }
}