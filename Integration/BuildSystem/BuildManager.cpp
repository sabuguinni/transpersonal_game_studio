#include "BuildManager.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HAL/PlatformProcess.h"

DEFINE_LOG_CATEGORY(LogBuildManager);

const TMap<FString, FString> UBuildManager::DefaultAgentNames = {
    {TEXT("02"), TEXT("Engine Architect")},
    {TEXT("03"), TEXT("Core Systems Programmer")},
    {TEXT("04"), TEXT("Performance Optimizer")},
    {TEXT("05"), TEXT("Procedural World Generator")},
    {TEXT("06"), TEXT("Environment Artist")},
    {TEXT("07"), TEXT("Architecture & Interior Agent")},
    {TEXT("08"), TEXT("Lighting & Atmosphere Agent")},
    {TEXT("09"), TEXT("Character Artist Agent")},
    {TEXT("10"), TEXT("Animation Agent")},
    {TEXT("11"), TEXT("NPC Behavior Agent")},
    {TEXT("12"), TEXT("Combat & Enemy AI Agent")},
    {TEXT("13"), TEXT("Crowd & Traffic Simulation")},
    {TEXT("14"), TEXT("Quest & Mission Designer")},
    {TEXT("15"), TEXT("Narrative & Dialogue Agent")},
    {TEXT("16"), TEXT("Audio Agent")},
    {TEXT("17"), TEXT("VFX Agent")},
    {TEXT("18"), TEXT("QA & Testing Agent")}
};

UBuildManager::UBuildManager()
{
    bQABlocked = false;
    MaxBuildHistory = 10;
    AgentNames = DefaultAgentNames;
}

void UBuildManager::Initialize(const FString& ProjectRoot)
{
    ProjectRootPath = ProjectRoot;
    LoadAgentConfiguration();
    LoadBuildHistory();
    
    UE_LOG(LogBuildManager, Log, TEXT("BuildManager initialized with project root: %s"), *ProjectRootPath);
}

bool UBuildManager::StartIntegrationCycle(const FString& CycleId, EBuildConfiguration Configuration)
{
    UE_LOG(LogBuildManager, Log, TEXT("Starting integration cycle: %s"), *CycleId);
    
    // Initialize new build
    CurrentBuild = FBuildInfo();
    CurrentBuild.BuildId = CycleId;
    CurrentBuild.Configuration = Configuration;
    CurrentBuild.Status = EBuildStatus::InProgress;
    CurrentBuild.StartTime = FDateTime::Now();
    
    // Clear previous validation results
    AgentValidationResults.Empty();
    bQABlocked = false;
    
    // Validate all agent outputs
    bool bAllValid = true;
    for (const auto& AgentPair : AgentNames)
    {
        FString AgentId = AgentPair.Key;
        FString AgentName = AgentPair.Value;
        FString OutputPath = FPaths::Combine(ProjectRootPath, TEXT("AgentOutputs"), FString::Printf(TEXT("Agent_%s"), *AgentId));
        
        FAgentValidationResult ValidationResult = ValidateAgentOutput(AgentId, OutputPath);
        AgentValidationResults.Add(ValidationResult);
        
        if (!ValidationResult.bIsValid)
        {
            bAllValid = false;
            CurrentBuild.AgentStatuses.Add(AgentId, EAgentStatus::Failed);
            
            // QA failures block everything
            if (AgentId == TEXT("18"))
            {
                bQABlocked = true;
                UE_LOG(LogBuildManager, Error, TEXT("QA Agent validation failed - build blocked"));
                CurrentBuild.Status = EBuildStatus::Failed;
                CurrentBuild.EndTime = FDateTime::Now();
                return false;
            }
        }
        else
        {
            CurrentBuild.AgentStatuses.Add(AgentId, EAgentStatus::Validated);
        }
    }
    
    if (!bAllValid)
    {
        UE_LOG(LogBuildManager, Error, TEXT("Integration cycle failed - agent validation errors"));
        CurrentBuild.Status = EBuildStatus::Failed;
        CurrentBuild.EndTime = FDateTime::Now();
        return false;
    }
    
    // Build the project
    if (!BuildProject(Configuration))
    {
        UE_LOG(LogBuildManager, Error, TEXT("Integration cycle failed - build errors"));
        CurrentBuild.Status = EBuildStatus::Failed;
        CurrentBuild.EndTime = FDateTime::Now();
        return false;
    }
    
    // Create package
    FString PackagePath = CreateBuildPackage(Configuration);
    if (PackagePath.IsEmpty())
    {
        UE_LOG(LogBuildManager, Error, TEXT("Integration cycle failed - packaging errors"));
        CurrentBuild.Status = EBuildStatus::Failed;
        CurrentBuild.EndTime = FDateTime::Now();
        return false;
    }
    
    CurrentBuild.PackagePath = PackagePath;
    CurrentBuild.Status = EBuildStatus::Success;
    CurrentBuild.EndTime = FDateTime::Now();
    
    // Add to history
    BuildHistory.Add(CurrentBuild);
    if (BuildHistory.Num() > MaxBuildHistory)
    {
        BuildHistory.RemoveAt(0);
    }
    
    SaveBuildHistory();
    
    UE_LOG(LogBuildManager, Log, TEXT("Integration cycle completed successfully: %s"), *CycleId);
    return true;
}

FAgentValidationResult UBuildManager::ValidateAgentOutput(const FString& AgentId, const FString& OutputPath)
{
    FAgentValidationResult Result;
    Result.AgentId = AgentId;
    Result.AgentName = AgentNames.FindRef(AgentId);
    Result.ValidationTime = FDateTime::Now();
    
    TArray<FString> Errors;
    TArray<FString> Warnings;
    
    // Check if output path exists
    if (!FPaths::DirectoryExists(OutputPath))
    {
        Errors.Add(FString::Printf(TEXT("Output directory not found: %s"), *OutputPath));
        Result.bIsValid = false;
        Result.Errors = Errors;
        return Result;
    }
    
    // Agent-specific validation
    bool bValid = true;
    
    if (AgentId == TEXT("02")) // Engine Architect
    {
        bValid = ValidateArchitectureFiles(OutputPath, Errors);
    }
    else if (AgentId == TEXT("03")) // Core Systems Programmer
    {
        bValid = ValidateCoreSystemsOutput(OutputPath, Errors);
    }
    else if (AgentId == TEXT("04")) // Performance Optimizer
    {
        bValid = ValidatePerformanceOutput(OutputPath, Errors);
    }
    else if (AgentId == TEXT("18")) // QA & Testing Agent
    {
        bValid = ValidateQAResults(OutputPath, Errors);
    }
    
    Result.bIsValid = bValid && Errors.Num() == 0;
    Result.Errors = Errors;
    Result.Warnings = Warnings;
    
    if (Result.bIsValid)
    {
        UE_LOG(LogBuildManager, Log, TEXT("Agent %s (%s) validation passed"), *AgentId, *Result.AgentName);
    }
    else
    {
        UE_LOG(LogBuildManager, Warning, TEXT("Agent %s (%s) validation failed with %d errors"), 
               *AgentId, *Result.AgentName, Errors.Num());
    }
    
    return Result;
}

bool UBuildManager::ValidateArchitectureFiles(const FString& OutputPath, TArray<FString>& OutErrors)
{
    TArray<FString> RequiredFiles = {
        TEXT("TechnicalArchitecture.md"),
        TEXT("SystemDependencies.json"),
        TEXT("PerformanceTargets.json"),
        TEXT("ModuleStructure.json")
    };
    
    bool bValid = true;
    for (const FString& RequiredFile : RequiredFiles)
    {
        FString FilePath = FPaths::Combine(OutputPath, RequiredFile);
        if (!FPaths::FileExists(FilePath))
        {
            OutErrors.Add(FString::Printf(TEXT("Missing required architecture file: %s"), *RequiredFile));
            bValid = false;
        }
    }
    
    return bValid;
}

bool UBuildManager::ValidateCoreSystemsOutput(const FString& OutputPath, TArray<FString>& OutErrors)
{
    TArray<FString> CoreSystems = {
        TEXT("PhysicsSystem.cpp"),
        TEXT("PhysicsSystem.h"),
        TEXT("CollisionSystem.cpp"),
        TEXT("CollisionSystem.h"),
        TEXT("RagdollSystem.cpp"),
        TEXT("RagdollSystem.h")
    };
    
    bool bValid = true;
    FString SourcePath = FPaths::Combine(OutputPath, TEXT("Source"), TEXT("TranspersonalGame"), TEXT("Core"));
    
    for (const FString& SystemFile : CoreSystems)
    {
        FString FilePath = FPaths::Combine(SourcePath, SystemFile);
        if (!FPaths::FileExists(FilePath))
        {
            OutErrors.Add(FString::Printf(TEXT("Missing core system file: %s"), *SystemFile));
            bValid = false;
        }
    }
    
    return bValid;
}

bool UBuildManager::ValidatePerformanceOutput(const FString& OutputPath, TArray<FString>& OutErrors)
{
    FString PerformanceReportPath = FPaths::Combine(OutputPath, TEXT("PerformanceReport.json"));
    
    if (!FPaths::FileExists(PerformanceReportPath))
    {
        OutErrors.Add(TEXT("Performance report not found"));
        return false;
    }
    
    // Load and validate performance report
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *PerformanceReportPath))
    {
        OutErrors.Add(TEXT("Failed to load performance report"));
        return false;
    }
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        OutErrors.Add(TEXT("Invalid performance report JSON"));
        return false;
    }
    
    // Check performance targets
    double TargetFPS = JsonObject->GetNumberField(TEXT("target_fps"));
    double ActualFPS = JsonObject->GetNumberField(TEXT("actual_fps"));
    
    if (ActualFPS < TargetFPS * 0.9) // Allow 10% tolerance
    {
        OutErrors.Add(FString::Printf(TEXT("Performance target not met: %.1f FPS (target: %.1f FPS)"), 
                                     ActualFPS, TargetFPS));
        return false;
    }
    
    return true;
}

bool UBuildManager::ValidateQAResults(const FString& OutputPath, TArray<FString>& OutErrors)
{
    FString QAReportPath = FPaths::Combine(OutputPath, TEXT("QAReport.json"));
    
    if (!FPaths::FileExists(QAReportPath))
    {
        OutErrors.Add(TEXT("QA report not found - build blocked"));
        return false;
    }
    
    // Load QA report
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *QAReportPath))
    {
        OutErrors.Add(TEXT("Failed to load QA report"));
        return false;
    }
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        OutErrors.Add(TEXT("Invalid QA report JSON"));
        return false;
    }
    
    // Check for critical failures
    int32 CriticalFailures = JsonObject->GetIntegerField(TEXT("critical_failures"));
    if (CriticalFailures > 0)
    {
        OutErrors.Add(FString::Printf(TEXT("QA found %d critical failures - build blocked"), CriticalFailures));
        return false;
    }
    
    // Check test coverage
    double TestCoverage = JsonObject->GetNumberField(TEXT("test_coverage"));
    if (TestCoverage < 80.0)
    {
        OutErrors.Add(FString::Printf(TEXT("Test coverage below threshold: %.1f%% (minimum: 80%%)"), TestCoverage));
    }
    
    return true;
}

bool UBuildManager::BuildProject(EBuildConfiguration Configuration)
{
    UE_LOG(LogBuildManager, Log, TEXT("Starting build with configuration: %s"), 
           *UEnum::GetValueAsString(Configuration));
    
    FString ConfigString;
    switch (Configuration)
    {
        case EBuildConfiguration::Debug:
            ConfigString = TEXT("Debug");
            break;
        case EBuildConfiguration::DebugGame:
            ConfigString = TEXT("DebugGame");
            break;
        case EBuildConfiguration::Development:
            ConfigString = TEXT("Development");
            break;
        case EBuildConfiguration::Test:
            ConfigString = TEXT("Test");
            break;
        case EBuildConfiguration::Shipping:
            ConfigString = TEXT("Shipping");
            break;
    }
    
    // Build using UnrealBuildTool
    FString UBTPath = FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries"), TEXT("DotNET"), TEXT("UnrealBuildTool.exe"));
    FString Arguments = FString::Printf(TEXT("TranspersonalGame Win64 %s -Project=\"%s\" -WaitMutex -FromMsBuild"),
                                       *ConfigString,
                                       *FPaths::Combine(ProjectRootPath, TEXT("TranspersonalGame.uproject")));
    
    int32 ReturnCode = 0;
    FString StdOut;
    FString StdErr;
    
    bool bSuccess = FPlatformProcess::ExecProcess(*UBTPath, *Arguments, &ReturnCode, &StdOut, &StdErr);
    
    if (!bSuccess || ReturnCode != 0)
    {
        UE_LOG(LogBuildManager, Error, TEXT("Build failed with return code %d"), ReturnCode);
        UE_LOG(LogBuildManager, Error, TEXT("StdErr: %s"), *StdErr);
        return false;
    }
    
    UE_LOG(LogBuildManager, Log, TEXT("Build completed successfully"));
    return true;
}

FString UBuildManager::CreateBuildPackage(EBuildConfiguration Configuration)
{
    FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    FString ConfigString = UEnum::GetValueAsString(Configuration);
    FString PackageName = FString::Printf(TEXT("TranspersonalGame_%s_%s"), *ConfigString, *Timestamp);
    FString PackagePath = FPaths::Combine(ProjectRootPath, TEXT("Builds"), PackageName);
    
    UE_LOG(LogBuildManager, Log, TEXT("Creating build package: %s"), *PackageName);
    
    // Create package directory
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.CreateDirectoryTree(*PackagePath))
    {
        UE_LOG(LogBuildManager, Error, TEXT("Failed to create package directory: %s"), *PackagePath);
        return FString();
    }
    
    // Create build manifest
    CreateBuildManifest(PackagePath, CurrentBuild);
    
    UE_LOG(LogBuildManager, Log, TEXT("Build package created: %s"), *PackagePath);
    return PackagePath;
}

void UBuildManager::CreateBuildManifest(const FString& PackagePath, const FBuildInfo& BuildInfo)
{
    TSharedPtr<FJsonObject> ManifestObject = MakeShareable(new FJsonObject);
    
    ManifestObject->SetStringField(TEXT("build_id"), BuildInfo.BuildId);
    ManifestObject->SetStringField(TEXT("configuration"), UEnum::GetValueAsString(BuildInfo.Configuration));
    ManifestObject->SetStringField(TEXT("status"), UEnum::GetValueAsString(BuildInfo.Status));
    ManifestObject->SetStringField(TEXT("start_time"), BuildInfo.StartTime.ToString());
    ManifestObject->SetStringField(TEXT("end_time"), BuildInfo.EndTime.ToString());
    ManifestObject->SetStringField(TEXT("package_path"), BuildInfo.PackagePath);
    
    // Add agent statuses
    TSharedPtr<FJsonObject> AgentStatusObject = MakeShareable(new FJsonObject);
    for (const auto& StatusPair : BuildInfo.AgentStatuses)
    {
        AgentStatusObject->SetStringField(StatusPair.Key, UEnum::GetValueAsString(StatusPair.Value));
    }
    ManifestObject->SetObjectField(TEXT("agent_statuses"), AgentStatusObject);
    
    // Add validation errors
    TArray<TSharedPtr<FJsonValue>> ErrorArray;
    for (const FString& Error : BuildInfo.ValidationErrors)
    {
        ErrorArray.Add(MakeShareable(new FJsonValueString(Error)));
    }
    ManifestObject->SetArrayField(TEXT("validation_errors"), ErrorArray);
    
    // Write manifest to file
    FString ManifestPath = FPaths::Combine(PackagePath, TEXT("build_manifest.json"));
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ManifestObject.ToSharedRef(), Writer);
    
    FFileHelper::SaveStringToFile(OutputString, *ManifestPath);
}

bool UBuildManager::RollbackToPreviousBuild()
{
    if (BuildHistory.Num() == 0)
    {
        UE_LOG(LogBuildManager, Error, TEXT("No previous builds available for rollback"));
        return false;
    }
    
    FBuildInfo LastGoodBuild = BuildHistory.Last();
    UE_LOG(LogBuildManager, Log, TEXT("Rolling back to build: %s"), *LastGoodBuild.BuildId);
    
    // Implementation would copy files from the last good build package
    // This is a simplified version
    CurrentBuild = LastGoodBuild;
    
    return true;
}

void UBuildManager::LoadAgentConfiguration()
{
    FString ConfigPath = FPaths::Combine(ProjectRootPath, TEXT("Integration"), TEXT("AgentConfig.json"));
    
    if (FPaths::FileExists(ConfigPath))
    {
        FString JsonString;
        if (FFileHelper::LoadFileToString(JsonString, *ConfigPath))
        {
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
            
            if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
            {
                TSharedPtr<FJsonObject> AgentsObject = JsonObject->GetObjectField(TEXT("agents"));
                if (AgentsObject.IsValid())
                {
                    AgentNames.Empty();
                    for (const auto& AgentPair : AgentsObject->Values)
                    {
                        AgentNames.Add(AgentPair.Key, AgentPair.Value->AsString());
                    }
                }
            }
        }
    }
}

void UBuildManager::SaveBuildHistory()
{
    FString HistoryPath = FPaths::Combine(ProjectRootPath, TEXT("Integration"), TEXT("BuildHistory.json"));
    
    TSharedPtr<FJsonObject> HistoryObject = MakeShareable(new FJsonObject);
    TArray<TSharedPtr<FJsonValue>> BuildArray;
    
    for (const FBuildInfo& Build : BuildHistory)
    {
        TSharedPtr<FJsonObject> BuildObject = MakeShareable(new FJsonObject);
        BuildObject->SetStringField(TEXT("build_id"), Build.BuildId);
        BuildObject->SetStringField(TEXT("configuration"), UEnum::GetValueAsString(Build.Configuration));
        BuildObject->SetStringField(TEXT("status"), UEnum::GetValueAsString(Build.Status));
        BuildObject->SetStringField(TEXT("start_time"), Build.StartTime.ToString());
        BuildObject->SetStringField(TEXT("end_time"), Build.EndTime.ToString());
        BuildObject->SetStringField(TEXT("package_path"), Build.PackagePath);
        
        BuildArray.Add(MakeShareable(new FJsonValueObject(BuildObject)));
    }
    
    HistoryObject->SetArrayField(TEXT("builds"), BuildArray);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(HistoryObject.ToSharedRef(), Writer);
    
    FFileHelper::SaveStringToFile(OutputString, *HistoryPath);
}

void UBuildManager::LoadBuildHistory()
{
    FString HistoryPath = FPaths::Combine(ProjectRootPath, TEXT("Integration"), TEXT("BuildHistory.json"));
    
    if (FPaths::FileExists(HistoryPath))
    {
        FString JsonString;
        if (FFileHelper::LoadFileToString(JsonString, *HistoryPath))
        {
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
            
            if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
            {
                const TArray<TSharedPtr<FJsonValue>>* BuildArray;
                if (JsonObject->TryGetArrayField(TEXT("builds"), BuildArray))
                {
                    BuildHistory.Empty();
                    for (const TSharedPtr<FJsonValue>& BuildValue : *BuildArray)
                    {
                        TSharedPtr<FJsonObject> BuildObject = BuildValue->AsObject();
                        if (BuildObject.IsValid())
                        {
                            FBuildInfo Build;
                            Build.BuildId = BuildObject->GetStringField(TEXT("build_id"));
                            // Parse other fields as needed
                            BuildHistory.Add(Build);
                        }
                    }
                }
            }
        }
    }
}