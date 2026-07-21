#include "Build_QualityAssurance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/DateTime.h"

UBuild_QualityAssurance::UBuild_QualityAssurance()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize required systems
    RequiredSystems.Add(TEXT("TranspersonalCharacter"));
    RequiredSystems.Add(TEXT("TranspersonalGameState"));
    RequiredSystems.Add(TEXT("PCGWorldGenerator"));
    RequiredSystems.Add(TEXT("FoliageManager"));
    RequiredSystems.Add(TEXT("CrowdSimulationManager"));
    RequiredSystems.Add(TEXT("ProceduralWorldManager"));
    
    MinimumQualityThreshold = 75.0f;
    MaxActorCount = 8000;
}

FBuild_QualityReport UBuild_QualityAssurance::GenerateQualityReport()
{
    FBuild_QualityReport Report;
    
    // Set timestamp
    Report.BuildTimestamp = FDateTime::Now().ToString();
    
    // Get system statuses
    Report.SystemStatuses = GetSystemStatuses();
    Report.SystemsTotal = RequiredSystems.Num();
    Report.SystemsReady = 0;
    
    // Count ready systems
    for (const FBuild_SystemStatus& Status : Report.SystemStatuses)
    {
        if (Status.bIsLoaded && Status.bIsCompiled && Status.bIsFunctional)
        {
            Report.SystemsReady++;
        }
    }
    
    // Get actor count
    UWorld* World = GetWorld();
    if (World)
    {
        Report.TotalActors = World->GetCurrentLevel()->Actors.Num();
    }
    
    // Calculate quality score
    Report.QualityScore = CalculateSystemScore();
    Report.QualityLevel = CalculateQualityLevel(Report.QualityScore);
    
    // Cache report
    LastReport = Report;
    
    // Log report
    LogQualityReport(Report);
    
    return Report;
}

bool UBuild_QualityAssurance::ValidateSystemIntegration()
{
    bool bAllSystemsValid = true;
    
    for (const FString& SystemName : RequiredSystems)
    {
        if (!ValidateSystem(SystemName))
        {
            bAllSystemsValid = false;
            UE_LOG(LogTemp, Warning, TEXT("System validation failed: %s"), *SystemName);
        }
    }
    
    return bAllSystemsValid;
}

bool UBuild_QualityAssurance::CheckPerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    int32 ActorCount = World->GetCurrentLevel()->Actors.Num();
    
    if (ActorCount > MaxActorCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor count exceeds maximum: %d > %d"), ActorCount, MaxActorCount);
        return false;
    }
    
    return true;
}

TArray<FBuild_SystemStatus> UBuild_QualityAssurance::GetSystemStatuses()
{
    TArray<FBuild_SystemStatus> Statuses;
    
    for (const FString& SystemName : RequiredSystems)
    {
        FBuild_SystemStatus Status;
        Status.SystemName = SystemName;
        
        // Try to load the class
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (SystemClass)
        {
            Status.bIsLoaded = true;
            Status.bIsCompiled = true;
            
            // Check if we can create a default object
            try
            {
                UObject* DefaultObject = SystemClass->GetDefaultObject();
                if (DefaultObject)
                {
                    Status.bIsFunctional = true;
                }
                else
                {
                    Status.bIsFunctional = false;
                    Status.ErrorMessage = TEXT("Failed to get default object");
                }
            }
            catch (...)
            {
                Status.bIsFunctional = false;
                Status.ErrorMessage = TEXT("Exception during default object creation");
            }
        }
        else
        {
            Status.bIsLoaded = false;
            Status.bIsCompiled = false;
            Status.bIsFunctional = false;
            Status.ErrorMessage = TEXT("Class not found");
        }
        
        Statuses.Add(Status);
    }
    
    return Statuses;
}

EBuild_QualityLevel UBuild_QualityAssurance::CalculateQualityLevel(float QualityScore)
{
    if (QualityScore >= 95.0f)
    {
        return EBuild_QualityLevel::Excellent;
    }
    else if (QualityScore >= 80.0f)
    {
        return EBuild_QualityLevel::Good;
    }
    else if (QualityScore >= 60.0f)
    {
        return EBuild_QualityLevel::Fair;
    }
    else if (QualityScore >= 30.0f)
    {
        return EBuild_QualityLevel::Poor;
    }
    else
    {
        return EBuild_QualityLevel::Critical;
    }
}

bool UBuild_QualityAssurance::ValidateSystem(const FString& SystemName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
    UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (!SystemClass)
    {
        return false;
    }
    
    // Try to get default object
    UObject* DefaultObject = SystemClass->GetDefaultObject();
    return DefaultObject != nullptr;
}

float UBuild_QualityAssurance::CalculateSystemScore()
{
    TArray<FBuild_SystemStatus> Statuses = GetSystemStatuses();
    
    if (Statuses.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalScore = 0.0f;
    
    for (const FBuild_SystemStatus& Status : Statuses)
    {
        float SystemScore = 0.0f;
        
        if (Status.bIsLoaded)
        {
            SystemScore += 33.33f;
        }
        
        if (Status.bIsCompiled)
        {
            SystemScore += 33.33f;
        }
        
        if (Status.bIsFunctional)
        {
            SystemScore += 33.34f;
        }
        
        TotalScore += SystemScore;
    }
    
    return TotalScore / Statuses.Num();
}

void UBuild_QualityAssurance::LogQualityReport(const FBuild_QualityReport& Report)
{
    UE_LOG(LogTemp, Log, TEXT("=== BUILD QUALITY REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Timestamp: %s"), *Report.BuildTimestamp);
    UE_LOG(LogTemp, Log, TEXT("Quality Level: %d"), (int32)Report.QualityLevel);
    UE_LOG(LogTemp, Log, TEXT("Quality Score: %.1f%%"), Report.QualityScore);
    UE_LOG(LogTemp, Log, TEXT("Systems Ready: %d/%d"), Report.SystemsReady, Report.SystemsTotal);
    UE_LOG(LogTemp, Log, TEXT("Total Actors: %d"), Report.TotalActors);
    
    for (const FBuild_SystemStatus& Status : Report.SystemStatuses)
    {
        FString StatusText = Status.bIsFunctional ? TEXT("OK") : TEXT("FAIL");
        UE_LOG(LogTemp, Log, TEXT("System %s: %s"), *Status.SystemName, *StatusText);
        
        if (!Status.ErrorMessage.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("  Error: %s"), *Status.ErrorMessage);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}