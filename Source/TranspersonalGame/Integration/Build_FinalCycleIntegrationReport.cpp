#include "Build_FinalCycleIntegrationReport.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UBuild_FinalCycleIntegrationReport::UBuild_FinalCycleIntegrationReport()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    CycleID = TEXT("UNKNOWN");
    AgentName = TEXT("Integration_Build_Agent_19");
    QAStatus = EBuild_QAStatus::Unknown;
    VFXStatus = EBuild_VFXStatus::Unknown;
    CoreClassesLoaded = 0;
    TotalCoreClasses = 5;
    ContentAssets = 0;
    CurrentMap = TEXT("UNKNOWN");
    ActorsInWorld = 0;
    BuildStatus = EBuild_BuildStatus::Unknown;
    bReportGenerated = false;
}

void UBuild_FinalCycleIntegrationReport::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-generate report on begin play
    GenerateIntegrationReport();
}

void UBuild_FinalCycleIntegrationReport::GenerateIntegrationReport()
{
    if (bReportGenerated)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integration report already generated"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Generating final cycle integration report..."));
    
    // Collect world data
    if (UWorld* World = GetWorld())
    {
        CurrentMap = World->GetName();
        ActorsInWorld = World->GetCurrentLevel()->Actors.Num();
    }
    
    // Set cycle information
    CycleID = TEXT("PROD_CYCLE_AUTO_20260605_003");
    
    // Determine build status based on loaded classes
    if (CoreClassesLoaded >= 4)
    {
        BuildStatus = EBuild_BuildStatus::Validated;
    }
    else if (CoreClassesLoaded >= 2)
    {
        BuildStatus = EBuild_BuildStatus::Partial;
    }
    else
    {
        BuildStatus = EBuild_BuildStatus::Failed;
    }
    
    // Generate JSON report
    GenerateJSONReport();
    
    bReportGenerated = true;
    
    UE_LOG(LogTemp, Log, TEXT("Integration report generated successfully"));
    UE_LOG(LogTemp, Log, TEXT("Cycle: %s"), *CycleID);
    UE_LOG(LogTemp, Log, TEXT("Build Status: %s"), *GetBuildStatusString());
    UE_LOG(LogTemp, Log, TEXT("Core Classes: %d/%d"), CoreClassesLoaded, TotalCoreClasses);
    UE_LOG(LogTemp, Log, TEXT("World Actors: %d"), ActorsInWorld);
}

void UBuild_FinalCycleIntegrationReport::GenerateJSONReport()
{
    // Create JSON object
    TSharedPtr<FJsonObject> ReportObject = MakeShareable(new FJsonObject);
    
    ReportObject->SetStringField(TEXT("cycle_id"), CycleID);
    ReportObject->SetStringField(TEXT("agent"), AgentName);
    ReportObject->SetStringField(TEXT("timestamp"), FDateTime::Now().ToIso8601());
    ReportObject->SetStringField(TEXT("qa_status"), GetQAStatusString());
    ReportObject->SetStringField(TEXT("vfx_status"), GetVFXStatusString());
    ReportObject->SetNumberField(TEXT("core_classes_loaded"), CoreClassesLoaded);
    ReportObject->SetNumberField(TEXT("total_core_classes"), TotalCoreClasses);
    ReportObject->SetNumberField(TEXT("content_assets"), ContentAssets);
    ReportObject->SetStringField(TEXT("current_map"), CurrentMap);
    ReportObject->SetNumberField(TEXT("actors_in_world"), ActorsInWorld);
    ReportObject->SetStringField(TEXT("build_status"), GetBuildStatusString());
    
    // Convert to JSON string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ReportObject.ToSharedRef(), Writer);
    
    // Save to file
    FString ProjectDir = FPaths::ProjectDir();
    FString ReportPath = FPaths::Combine(ProjectDir, TEXT("Intermediate"), TEXT("integration_report.json"));
    
    if (FFileHelper::SaveStringToFile(OutputString, *ReportPath))
    {
        UE_LOG(LogTemp, Log, TEXT("Integration report saved to: %s"), *ReportPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save integration report to: %s"), *ReportPath);
    }
}

void UBuild_FinalCycleIntegrationReport::SetQAStatus(EBuild_QAStatus NewStatus)
{
    QAStatus = NewStatus;
    UE_LOG(LogTemp, Log, TEXT("QA Status updated to: %s"), *GetQAStatusString());
}

void UBuild_FinalCycleIntegrationReport::SetVFXStatus(EBuild_VFXStatus NewStatus)
{
    VFXStatus = NewStatus;
    UE_LOG(LogTemp, Log, TEXT("VFX Status updated to: %s"), *GetVFXStatusString());
}

void UBuild_FinalCycleIntegrationReport::SetCoreClassesLoaded(int32 LoadedCount)
{
    CoreClassesLoaded = LoadedCount;
    UE_LOG(LogTemp, Log, TEXT("Core classes loaded: %d/%d"), CoreClassesLoaded, TotalCoreClasses);
}

void UBuild_FinalCycleIntegrationReport::SetContentAssets(int32 AssetCount)
{
    ContentAssets = AssetCount;
    UE_LOG(LogTemp, Log, TEXT("Content assets: %d"), ContentAssets);
}

FString UBuild_FinalCycleIntegrationReport::GetQAStatusString() const
{
    switch (QAStatus)
    {
        case EBuild_QAStatus::Passed: return TEXT("PASSED");
        case EBuild_QAStatus::Failed: return TEXT("FAILED");
        case EBuild_QAStatus::Warning: return TEXT("WARNING");
        case EBuild_QAStatus::InProgress: return TEXT("IN_PROGRESS");
        case EBuild_QAStatus::Unknown:
        default: return TEXT("UNKNOWN");
    }
}

FString UBuild_FinalCycleIntegrationReport::GetVFXStatusString() const
{
    switch (VFXStatus)
    {
        case EBuild_VFXStatus::Validated: return TEXT("VALIDATED");
        case EBuild_VFXStatus::Failed: return TEXT("FAILED");
        case EBuild_VFXStatus::Partial: return TEXT("PARTIAL");
        case EBuild_VFXStatus::Unknown:
        default: return TEXT("UNKNOWN");
    }
}

FString UBuild_FinalCycleIntegrationReport::GetBuildStatusString() const
{
    switch (BuildStatus)
    {
        case EBuild_BuildStatus::Validated: return TEXT("VALIDATED");
        case EBuild_BuildStatus::Partial: return TEXT("PARTIAL");
        case EBuild_BuildStatus::Failed: return TEXT("FAILED");
        case EBuild_BuildStatus::Unknown:
        default: return TEXT("UNKNOWN");
    }
}

bool UBuild_FinalCycleIntegrationReport::IsIntegrationSuccessful() const
{
    return BuildStatus == EBuild_BuildStatus::Validated && 
           QAStatus != EBuild_QAStatus::Failed &&
           CoreClassesLoaded >= 4;
}

void UBuild_FinalCycleIntegrationReport::LogIntegrationSummary() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL CYCLE INTEGRATION SUMMARY ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle ID: %s"), *CycleID);
    UE_LOG(LogTemp, Warning, TEXT("Build Status: %s"), *GetBuildStatusString());
    UE_LOG(LogTemp, Warning, TEXT("QA Status: %s"), *GetQAStatusString());
    UE_LOG(LogTemp, Warning, TEXT("VFX Status: %s"), *GetVFXStatusString());
    UE_LOG(LogTemp, Warning, TEXT("Core Classes: %d/%d loaded"), CoreClassesLoaded, TotalCoreClasses);
    UE_LOG(LogTemp, Warning, TEXT("Content Assets: %d"), ContentAssets);
    UE_LOG(LogTemp, Warning, TEXT("Current Map: %s"), *CurrentMap);
    UE_LOG(LogTemp, Warning, TEXT("World Actors: %d"), ActorsInWorld);
    UE_LOG(LogTemp, Warning, TEXT("Integration Successful: %s"), IsIntegrationSuccessful() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
}