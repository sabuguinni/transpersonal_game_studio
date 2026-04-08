#include "IntegrationManager.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY(LogIntegration);

UIntegrationManager::UIntegrationManager()
{
    MaxConcurrentCycles = 3;
    ValidationTimeoutSeconds = 300.0f;
    bAutoTriggerBuilds = true;
}

void UIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogIntegration, Log, TEXT("Integration Manager initialized"));
    
    InitializeAgentDependencies();
    
    // Load existing build history
    FString HistoryPath = FPaths::ProjectDir() / TEXT("Integration/BuildHistory.json");
    if (FPaths::FileExists(HistoryPath))
    {
        FString HistoryJson;
        if (FFileHelper::LoadFileToString(HistoryJson, *HistoryPath))
        {
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HistoryJson);
            
            if (FJsonSerializer::Deserialize(Reader, JsonObject))
            {
                const TArray<TSharedPtr<FJsonValue>>* HistoryArray;
                if (JsonObject->TryGetArrayField(TEXT("builds"), HistoryArray))
                {
                    for (const auto& BuildValue : *HistoryArray)
                    {
                        FString BuildID = BuildValue->AsString();
                        BuildHistory.Add(BuildID);
                    }
                }
            }
        }
    }
}

void UIntegrationManager::Deinitialize()
{
    // Save build history
    FString HistoryPath = FPaths::ProjectDir() / TEXT("Integration/BuildHistory.json");
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    TArray<TSharedPtr<FJsonValue>> HistoryArray;
    
    for (const FString& BuildID : BuildHistory)
    {
        HistoryArray.Add(MakeShareable(new FJsonValueString(BuildID)));
    }
    
    JsonObject->SetArrayField(TEXT("builds"), HistoryArray);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    FFileHelper::SaveStringToFile(OutputString, *HistoryPath);
    
    UE_LOG(LogIntegration, Log, TEXT("Integration Manager deinitialized"));
    Super::Deinitialize();
}

void UIntegrationManager::InitializeAgentDependencies()
{
    // Define the dependency chain according to the studio workflow
    AgentDependencies.Empty();
    
    // Engine Architect (02) - No dependencies, foundation for all
    AgentDependencies.Add(EAgentType::EngineArchitect, TArray<EAgentType>());
    
    // Core Systems (03) - Depends on Engine Architect
    AgentDependencies.Add(EAgentType::CoreSystems, {EAgentType::EngineArchitect});
    
    // Performance (04) - Depends on Core Systems
    AgentDependencies.Add(EAgentType::Performance, {EAgentType::CoreSystems});
    
    // World Generator (05) - Depends on Performance
    AgentDependencies.Add(EAgentType::WorldGenerator, {EAgentType::Performance});
    
    // Environment (06) - Depends on World Generator
    AgentDependencies.Add(EAgentType::Environment, {EAgentType::WorldGenerator});
    
    // Architecture (07) - Depends on Environment
    AgentDependencies.Add(EAgentType::Architecture, {EAgentType::Environment});
    
    // Lighting (08) - Depends on Architecture
    AgentDependencies.Add(EAgentType::Lighting, {EAgentType::Architecture});
    
    // Character (09) - Depends on Lighting
    AgentDependencies.Add(EAgentType::Character, {EAgentType::Lighting});
    
    // Animation (10) - Depends on Character
    AgentDependencies.Add(EAgentType::Animation, {EAgentType::Character});
    
    // NPC Behavior (11) - Depends on Animation
    AgentDependencies.Add(EAgentType::NPCBehavior, {EAgentType::Animation});
    
    // Combat (12) - Depends on NPC Behavior
    AgentDependencies.Add(EAgentType::Combat, {EAgentType::NPCBehavior});
    
    // Crowd (13) - Depends on Combat
    AgentDependencies.Add(EAgentType::Crowd, {EAgentType::Combat});
    
    // Narrative (15) - Special case, can work in parallel with technical systems
    AgentDependencies.Add(EAgentType::Narrative, {EAgentType::EngineArchitect});
    
    // Quest (14) - Depends on both Crowd and Narrative
    AgentDependencies.Add(EAgentType::Quest, {EAgentType::Crowd, EAgentType::Narrative});
    
    // Audio (16) - Depends on Quest
    AgentDependencies.Add(EAgentType::Audio, {EAgentType::Quest});
    
    // VFX (17) - Depends on Audio
    AgentDependencies.Add(EAgentType::VFX, {EAgentType::Audio});
    
    // QA (18) - Depends on all others
    AgentDependencies.Add(EAgentType::QA, {
        EAgentType::EngineArchitect, EAgentType::CoreSystems, EAgentType::Performance,
        EAgentType::WorldGenerator, EAgentType::Environment, EAgentType::Architecture,
        EAgentType::Lighting, EAgentType::Character, EAgentType::Animation,
        EAgentType::NPCBehavior, EAgentType::Combat, EAgentType::Crowd,
        EAgentType::Quest, EAgentType::Narrative, EAgentType::Audio, EAgentType::VFX
    });
}

bool UIntegrationManager::StartIntegrationCycle(const FString& CycleID)
{
    if (ActiveCycles.Contains(CycleID))
    {
        UE_LOG(LogIntegration, Warning, TEXT("Integration cycle %s already exists"), *CycleID);
        return false;
    }
    
    if (ActiveCycles.Num() >= MaxConcurrentCycles)
    {
        UE_LOG(LogIntegration, Error, TEXT("Maximum concurrent cycles reached (%d)"), MaxConcurrentCycles);
        return false;
    }
    
    FIntegrationCycle NewCycle;
    NewCycle.CycleID = CycleID;
    NewCycle.StartTime = FDateTime::Now();
    NewCycle.OverallStatus = EIntegrationStatus::InProgress;
    
    // Initialize agent outputs
    for (const auto& AgentPair : AgentDependencies)
    {
        FAgentOutput AgentOutput;
        AgentOutput.AgentType = AgentPair.Key;
        AgentOutput.Status = EIntegrationStatus::Pending;
        
        // Set agent names
        switch (AgentPair.Key)
        {
            case EAgentType::EngineArchitect: AgentOutput.AgentName = TEXT("Engine Architect"); break;
            case EAgentType::CoreSystems: AgentOutput.AgentName = TEXT("Core Systems Programmer"); break;
            case EAgentType::Performance: AgentOutput.AgentName = TEXT("Performance Optimizer"); break;
            case EAgentType::WorldGenerator: AgentOutput.AgentName = TEXT("Procedural World Generator"); break;
            case EAgentType::Environment: AgentOutput.AgentName = TEXT("Environment Artist"); break;
            case EAgentType::Architecture: AgentOutput.AgentName = TEXT("Architecture & Interior Agent"); break;
            case EAgentType::Lighting: AgentOutput.AgentName = TEXT("Lighting & Atmosphere Agent"); break;
            case EAgentType::Character: AgentOutput.AgentName = TEXT("Character Artist Agent"); break;
            case EAgentType::Animation: AgentOutput.AgentName = TEXT("Animation Agent"); break;
            case EAgentType::NPCBehavior: AgentOutput.AgentName = TEXT("NPC Behavior Agent"); break;
            case EAgentType::Combat: AgentOutput.AgentName = TEXT("Combat & Enemy AI Agent"); break;
            case EAgentType::Crowd: AgentOutput.AgentName = TEXT("Crowd & Traffic Simulation"); break;
            case EAgentType::Quest: AgentOutput.AgentName = TEXT("Quest & Mission Designer"); break;
            case EAgentType::Narrative: AgentOutput.AgentName = TEXT("Narrative & Dialogue Agent"); break;
            case EAgentType::Audio: AgentOutput.AgentName = TEXT("Audio Agent"); break;
            case EAgentType::VFX: AgentOutput.AgentName = TEXT("VFX Agent"); break;
            case EAgentType::QA: AgentOutput.AgentName = TEXT("QA & Testing Agent"); break;
        }
        
        NewCycle.AgentOutputs.Add(AgentOutput);
    }
    
    ActiveCycles.Add(CycleID, NewCycle);
    
    UE_LOG(LogIntegration, Log, TEXT("Started integration cycle: %s"), *CycleID);
    OnIntegrationStatusChanged.Broadcast(CycleID, EIntegrationStatus::InProgress);
    
    return true;
}

bool UIntegrationManager::ValidateAgentOutput(EAgentType AgentType, const FString& OutputPath)
{
    UE_LOG(LogIntegration, Log, TEXT("Validating output for agent type %d at path: %s"), 
           (int32)AgentType, *OutputPath);
    
    TArray<FString> ValidationErrors;
    bool bValidationPassed = true;
    
    // Check if output path exists
    if (!FPaths::DirectoryExists(OutputPath))
    {
        ValidationErrors.Add(FString::Printf(TEXT("Output directory does not exist: %s"), *OutputPath));
        bValidationPassed = false;
    }
    else
    {
        // Agent-specific validation
        switch (AgentType)
        {
            case EAgentType::EngineArchitecture:
                bValidationPassed = ValidateArchitectureFiles(OutputPath);
                break;
                
            case EAgentType::CoreSystems:
                bValidationPassed = ValidateCoreSystemFiles(OutputPath);
                break;
                
            case EAgentType::QA:
                bValidationPassed = ValidateQAReport(OutputPath);
                break;
                
            default:
                // Generic asset validation
                bValidationPassed = ValidateAssetIntegrity(OutputPath);
                break;
        }
    }
    
    // Update all active cycles with this agent's status
    for (auto& CyclePair : ActiveCycles)
    {
        FIntegrationCycle& Cycle = CyclePair.Value;
        for (FAgentOutput& AgentOutput : Cycle.AgentOutputs)
        {
            if (AgentOutput.AgentType == AgentType)
            {
                AgentOutput.Status = bValidationPassed ? EIntegrationStatus::Success : EIntegrationStatus::Failed;
                AgentOutput.ValidationErrors = ValidationErrors;
                AgentOutput.LastUpdated = FDateTime::Now();
                AgentOutput.OutputPath = OutputPath;
                break;
            }
        }
    }
    
    OnAgentValidationComplete.Broadcast(AgentType, bValidationPassed, ValidationErrors);
    
    return bValidationPassed;
}

bool UIntegrationManager::ValidateArchitectureFiles(const FString& Path) const
{
    TArray<FString> RequiredFiles = {
        TEXT("TechnicalArchitecture.md"),
        TEXT("SystemDependencies.json"),
        TEXT("PerformanceTargets.json"),
        TEXT("ModuleStructure.json")
    };
    
    for (const FString& File : RequiredFiles)
    {
        FString FilePath = FPaths::Combine(Path, File);
        if (!FPaths::FileExists(FilePath))
        {
            UE_LOG(LogIntegration, Error, TEXT("Missing required architecture file: %s"), *File);
            return false;
        }
    }
    
    return true;
}

bool UIntegrationManager::ValidateCoreSystemFiles(const FString& Path) const
{
    TArray<FString> CoreSystems = {
        TEXT("PhysicsSystem.cpp"),
        TEXT("CollisionSystem.cpp"),
        TEXT("RagdollSystem.cpp"),
        TEXT("DestructionSystem.cpp")
    };
    
    FString SourcePath = FPaths::Combine(Path, TEXT("Source/TranspersonalGame/Core"));
    
    for (const FString& System : CoreSystems)
    {
        FString SystemPath = FPaths::Combine(SourcePath, System);
        if (!FPaths::FileExists(SystemPath))
        {
            UE_LOG(LogIntegration, Warning, TEXT("Core system file not found: %s"), *System);
        }
    }
    
    return true;
}

bool UIntegrationManager::ValidateQAReport(const FString& Path) const
{
    FString QAReportPath = FPaths::Combine(Path, TEXT("QAReport.json"));
    
    if (!FPaths::FileExists(QAReportPath))
    {
        UE_LOG(LogIntegration, Error, TEXT("QA report not found - build blocked"));
        return false;
    }
    
    FString QAJson;
    if (!FFileHelper::LoadFileToString(QAJson, *QAReportPath))
    {
        UE_LOG(LogIntegration, Error, TEXT("Failed to load QA report"));
        return false;
    }
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(QAJson);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        UE_LOG(LogIntegration, Error, TEXT("Failed to parse QA report JSON"));
        return false;
    }
    
    // Check for critical failures
    int32 CriticalFailures = JsonObject->GetIntegerField(TEXT("critical_failures"));
    if (CriticalFailures > 0)
    {
        UE_LOG(LogIntegration, Error, TEXT("QA found %d critical failures - build blocked"), CriticalFailures);
        return false;
    }
    
    // Check test coverage
    double TestCoverage = JsonObject->GetNumberField(TEXT("test_coverage"));
    if (TestCoverage < 80.0)
    {
        UE_LOG(LogIntegration, Warning, TEXT("Test coverage below threshold: %.1f%%"), TestCoverage);
    }
    
    return true;
}

bool UIntegrationManager::ValidateAssetIntegrity(const FString& Path) const
{
    // Basic asset validation - check for common file types and structure
    TArray<FString> AssetExtensions = {TEXT(".uasset"), TEXT(".umap"), TEXT(".cpp"), TEXT(".h")};
    
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFilesRecursive(FoundFiles, *Path, TEXT("*.*"), true, false);
    
    bool bHasValidAssets = false;
    for (const FString& File : FoundFiles)
    {
        for (const FString& Extension : AssetExtensions)
        {
            if (File.EndsWith(Extension))
            {
                bHasValidAssets = true;
                break;
            }
        }
        if (bHasValidAssets) break;
    }
    
    return bHasValidAssets;
}

EIntegrationStatus UIntegrationManager::GetCycleStatus(const FString& CycleID) const
{
    if (const FIntegrationCycle* Cycle = ActiveCycles.Find(CycleID))
    {
        return Cycle->OverallStatus;
    }
    
    return EIntegrationStatus::Failed;
}

TArray<FString> UIntegrationManager::GetValidationErrors(const FString& CycleID) const
{
    TArray<FString> AllErrors;
    
    if (const FIntegrationCycle* Cycle = ActiveCycles.Find(CycleID))
    {
        for (const FAgentOutput& AgentOutput : Cycle->AgentOutputs)
        {
            if (AgentOutput.Status == EIntegrationStatus::Failed)
            {
                for (const FString& Error : AgentOutput.ValidationErrors)
                {
                    AllErrors.Add(FString::Printf(TEXT("[%s] %s"), *AgentOutput.AgentName, *Error));
                }
            }
        }
    }
    
    return AllErrors;
}

TArray<EAgentType> UIntegrationManager::GetAgentDependencies(EAgentType AgentType) const
{
    if (const TArray<EAgentType>* Dependencies = AgentDependencies.Find(AgentType))
    {
        return *Dependencies;
    }
    
    return TArray<EAgentType>();
}

bool UIntegrationManager::CanAgentProceed(EAgentType AgentType, const FString& CycleID) const
{
    const FIntegrationCycle* Cycle = ActiveCycles.Find(CycleID);
    if (!Cycle)
    {
        return false;
    }
    
    // Check if all dependencies are satisfied
    TArray<EAgentType> Dependencies = GetAgentDependencies(AgentType);
    
    for (EAgentType Dependency : Dependencies)
    {
        bool bDependencyMet = false;
        for (const FAgentOutput& AgentOutput : Cycle->AgentOutputs)
        {
            if (AgentOutput.AgentType == Dependency && AgentOutput.Status == EIntegrationStatus::Success)
            {
                bDependencyMet = true;
                break;
            }
        }
        
        if (!bDependencyMet)
        {
            return false;
        }
    }
    
    return true;
}

bool UIntegrationManager::TriggerBuild(const FString& Configuration)
{
    UE_LOG(LogIntegration, Log, TEXT("Triggering build with configuration: %s"), *Configuration);
    
    // This would integrate with UE5's build system
    // For now, we'll simulate the build process
    
    FString BuildCommand = FString::Printf(TEXT("BuildGame.bat %s"), *Configuration);
    FString BuildPath = FPaths::ProjectDir() / TEXT("Build");
    
    // Add to build history
    FString BuildID = FString::Printf(TEXT("%s_%s"), *Configuration, 
                                     *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    BuildHistory.Add(BuildID);
    
    // Maintain history size
    MaintainBuildHistory();
    
    LogIntegrationEvent(FString::Printf(TEXT("Build triggered: %s"), *BuildID));
    
    return true;
}

bool UIntegrationManager::MaintainBuildHistory(int32 MaxBuilds)
{
    while (BuildHistory.Num() > MaxBuilds)
    {
        BuildHistory.RemoveAt(0);
    }
    
    return true;
}

void UIntegrationManager::LogIntegrationEvent(const FString& Message, bool bIsError) const
{
    if (bIsError)
    {
        UE_LOG(LogIntegration, Error, TEXT("%s"), *Message);
    }
    else
    {
        UE_LOG(LogIntegration, Log, TEXT("%s"), *Message);
    }
}