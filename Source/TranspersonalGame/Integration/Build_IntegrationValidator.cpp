#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/Light.h"
#include "Landscape/Landscape.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/ObjectLibrary.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    CriticalSystemNames.Add(TEXT("TranspersonalCharacter"));
    CriticalSystemNames.Add(TEXT("TranspersonalGameState"));
    CriticalSystemNames.Add(TEXT("PCGWorldGenerator"));
    CriticalSystemNames.Add(TEXT("FoliageManager"));
    CriticalSystemNames.Add(TEXT("CrowdSimulationManager"));
    CriticalSystemNames.Add(TEXT("ProceduralWorldManager"));
    CriticalSystemNames.Add(TEXT("BuildIntegrationManager"));
}

FBuild_IntegrationReport UBuild_IntegrationValidator::ValidateAllSystems()
{
    FBuild_IntegrationReport Report;
    
    // Validate each critical system
    for (const FString& SystemName : CriticalSystemNames)
    {
        FString ClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassName);
        
        FBuild_SystemReport SystemReport = ValidateSystem(SystemName, SystemClass);
        Report.SystemReports.Add(SystemReport);
        
        if (SystemReport.Status == EBuild_SystemStatus::Functional)
        {
            Report.FunctionalSystemCount++;
        }
    }
    
    // Calculate overall metrics
    Report.TotalActorCount = GetActorCount();
    Report.OverallHealthScore = (float)Report.FunctionalSystemCount / (float)CriticalSystemNames.Num() * 100.0f;
    Report.bIsGameReady = (Report.OverallHealthScore >= 70.0f) && (Report.TotalActorCount > 0);
    
    LastReport = Report;
    return Report;
}

FBuild_SystemReport UBuild_IntegrationValidator::ValidateSystem(const FString& SystemName, UClass* SystemClass)
{
    FBuild_SystemReport Report;
    Report.SystemName = SystemName;
    
    if (!SystemClass)
    {
        Report.Status = EBuild_SystemStatus::Failed;
        Report.ErrorMessage = TEXT("Class not found or failed to load");
        Report.PerformanceScore = 0.0f;
        return Report;
    }
    
    // Test class instantiation
    if (!SystemClass->GetDefaultObject())
    {
        Report.Status = EBuild_SystemStatus::Failed;
        Report.ErrorMessage = TEXT("Default object construction failed");
        Report.PerformanceScore = 0.0f;
        return Report;
    }
    
    // Measure performance
    Report.PerformanceScore = MeasureSystemPerformance(SystemClass);
    
    // Check dependencies
    if (!CheckSystemDependencies(SystemName))
    {
        Report.Status = EBuild_SystemStatus::Degraded;
        Report.ErrorMessage = TEXT("Missing dependencies");
    }
    else
    {
        Report.Status = EBuild_SystemStatus::Functional;
        Report.ErrorMessage = TEXT("System operational");
    }
    
    return Report;
}

bool UBuild_IntegrationValidator::ValidateModuleCompilation()
{
    // Check if critical classes can be loaded
    for (const FString& SystemName : CriticalSystemNames)
    {
        if (!ValidateClassLoading(SystemName))
        {
            return false;
        }
    }
    return true;
}

int32 UBuild_IntegrationValidator::GetActorCount()
{
    UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
    if (!World)
    {
        // Try to get editor world
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
        return World->GetActorCount();
    }
    
    return 0;
}

float UBuild_IntegrationValidator::CalculatePerformanceScore()
{
    float Score = 100.0f;
    
    int32 ActorCount = GetActorCount();
    
    // Penalize high actor counts
    if (ActorCount > 8000)
    {
        Score -= 30.0f;
    }
    else if (ActorCount > 6000)
    {
        Score -= 15.0f;
    }
    
    // Check system functionality
    int32 FunctionalSystems = 0;
    for (const FString& SystemName : CriticalSystemNames)
    {
        if (ValidateClassLoading(SystemName))
        {
            FunctionalSystems++;
        }
    }
    
    float SystemScore = (float)FunctionalSystems / (float)CriticalSystemNames.Num() * 50.0f;
    Score = FMath::Min(Score, 50.0f + SystemScore);
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

bool UBuild_IntegrationValidator::ValidateCriticalSystems()
{
    int32 FunctionalCount = 0;
    
    for (const FString& SystemName : CriticalSystemNames)
    {
        if (ValidateClassLoading(SystemName))
        {
            FunctionalCount++;
        }
    }
    
    // Require at least 70% of systems to be functional
    float FunctionalPercentage = (float)FunctionalCount / (float)CriticalSystemNames.Num();
    return FunctionalPercentage >= 0.7f;
}

bool UBuild_IntegrationValidator::ValidateClassLoading(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *FullClassName);
    
    if (!LoadedClass)
    {
        return false;
    }
    
    // Try to get default object
    UObject* DefaultObject = LoadedClass->GetDefaultObject();
    return DefaultObject != nullptr;
}

float UBuild_IntegrationValidator::MeasureSystemPerformance(UClass* SystemClass)
{
    if (!SystemClass)
    {
        return 0.0f;
    }
    
    // Basic performance metric based on class complexity
    float Score = 100.0f;
    
    // Check number of properties
    int32 PropertyCount = 0;
    for (TFieldIterator<FProperty> PropIt(SystemClass); PropIt; ++PropIt)
    {
        PropertyCount++;
    }
    
    // Penalize overly complex classes
    if (PropertyCount > 50)
    {
        Score -= 20.0f;
    }
    else if (PropertyCount > 30)
    {
        Score -= 10.0f;
    }
    
    // Check if default object constructs quickly
    double StartTime = FPlatformTime::Seconds();
    UObject* DefaultObject = SystemClass->GetDefaultObject();
    double ConstructionTime = FPlatformTime::Seconds() - StartTime;
    
    if (ConstructionTime > 0.1)
    {
        Score -= 30.0f;
    }
    else if (ConstructionTime > 0.05)
    {
        Score -= 15.0f;
    }
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

bool UBuild_IntegrationValidator::CheckSystemDependencies(const FString& SystemName)
{
    // Basic dependency validation
    // In a full implementation, this would check module dependencies,
    // required components, etc.
    
    if (SystemName == TEXT("TranspersonalCharacter"))
    {
        // Character depends on movement component
        UClass* MovementClass = LoadClass<UObject>(nullptr, TEXT("/Script/Engine.CharacterMovementComponent"));
        return MovementClass != nullptr;
    }
    
    if (SystemName == TEXT("FoliageManager"))
    {
        // Foliage depends on landscape
        UClass* LandscapeClass = LoadClass<UObject>(nullptr, TEXT("/Script/Landscape.Landscape"));
        return LandscapeClass != nullptr;
    }
    
    if (SystemName == TEXT("CrowdSimulationManager"))
    {
        // Crowd simulation depends on AI
        UClass* AIClass = LoadClass<UObject>(nullptr, TEXT("/Script/AIModule.AIController"));
        return AIClass != nullptr;
    }
    
    // Default to true for other systems
    return true;
}