#include "QA_CriticalIncidentReport.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UQA_CriticalIncidentReport::UQA_CriticalIncidentReport()
{
    NextIncidentID = 1;
}

void UQA_CriticalIncidentReport::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("QA Critical Incident Report System Initialized"));
    
    // Report the current UE5 bridge failure
    FQA_IncidentReport BridgeFailure;
    BridgeFailure.Title = TEXT("UE5 Bridge Timeout Failure");
    BridgeFailure.Description = TEXT("Agent #17 VFX failures cascaded to Agent #18. UE5 bridge not responding to Python commands within 60s timeout.");
    BridgeFailure.Severity = EQA_IncidentSeverity::Blocker;
    BridgeFailure.Category = EQA_IncidentCategory::Bridge;
    BridgeFailure.AgentID = TEXT("Agent_18_QA");
    BridgeFailure.CycleID = TEXT("PROD_CYCLE_AUTO_20260511_001");
    BridgeFailure.AffectedSystems.Add(TEXT("UE5_Remote_Control"));
    BridgeFailure.AffectedSystems.Add(TEXT("VFX_Systems"));
    BridgeFailure.AffectedSystems.Add(TEXT("QA_Validation"));
    BridgeFailure.ReproductionSteps = TEXT("1. Agent #17 executes VFX commands 2. Multiple ue5_execute timeouts 3. Bridge becomes unresponsive");
    BridgeFailure.WorkaroundSolution = TEXT("Skip ue5_execute commands, use github_file_write only, restart bridge next cycle");
    
    ReportIncident(BridgeFailure);
}

void UQA_CriticalIncidentReport::Deinitialize()
{
    // Export final incident report before shutdown
    GenerateIncidentReport();
    Super::Deinitialize();
}

FString UQA_CriticalIncidentReport::ReportIncident(const FQA_IncidentReport& Incident)
{
    FQA_IncidentReport NewIncident = Incident;
    NewIncident.IncidentID = GenerateIncidentID();
    NewIncident.Timestamp = FDateTime::Now();
    
    IncidentDatabase.Add(NewIncident);
    LogIncidentToConsole(NewIncident);
    UpdateSystemBlockStatus();
    
    return NewIncident.IncidentID;
}

void UQA_CriticalIncidentReport::ResolveIncident(const FString& IncidentID, const FString& Resolution)
{
    for (FQA_IncidentReport& Incident : IncidentDatabase)
    {
        if (Incident.IncidentID == IncidentID)
        {
            Incident.bResolved = true;
            Incident.WorkaroundSolution = Resolution;
            UE_LOG(LogTemp, Warning, TEXT("Incident %s resolved: %s"), *IncidentID, *Resolution);
            break;
        }
    }
    UpdateSystemBlockStatus();
}

TArray<FQA_IncidentReport> UQA_CriticalIncidentReport::GetActiveIncidents()
{
    TArray<FQA_IncidentReport> ActiveIncidents;
    for (const FQA_IncidentReport& Incident : IncidentDatabase)
    {
        if (!Incident.bResolved)
        {
            ActiveIncidents.Add(Incident);
        }
    }
    return ActiveIncidents;
}

TArray<FQA_IncidentReport> UQA_CriticalIncidentReport::GetIncidentsByCategory(EQA_IncidentCategory Category)
{
    TArray<FQA_IncidentReport> CategoryIncidents;
    for (const FQA_IncidentReport& Incident : IncidentDatabase)
    {
        if (Incident.Category == Category)
        {
            CategoryIncidents.Add(Incident);
        }
    }
    return CategoryIncidents;
}

TArray<FQA_IncidentReport> UQA_CriticalIncidentReport::GetIncidentsBySeverity(EQA_IncidentSeverity Severity)
{
    TArray<FQA_IncidentReport> SeverityIncidents;
    for (const FQA_IncidentReport& Incident : IncidentDatabase)
    {
        if (Incident.Severity == Severity)
        {
            SeverityIncidents.Add(Incident);
        }
    }
    return SeverityIncidents;
}

void UQA_CriticalIncidentReport::ReportBridgeFailure(const FString& AgentID, const FString& CycleID, const FString& ErrorDetails)
{
    FQA_IncidentReport BridgeIncident;
    BridgeIncident.Title = TEXT("UE5 Bridge Communication Failure");
    BridgeIncident.Description = ErrorDetails;
    BridgeIncident.Severity = EQA_IncidentSeverity::Blocker;
    BridgeIncident.Category = EQA_IncidentCategory::Bridge;
    BridgeIncident.AgentID = AgentID;
    BridgeIncident.CycleID = CycleID;
    BridgeIncident.AffectedSystems.Add(TEXT("UE5_Remote_Control"));
    
    ReportIncident(BridgeIncident);
}

void UQA_CriticalIncidentReport::ReportVFXSystemFailure(const FString& SystemName, const FString& ErrorDetails)
{
    FQA_IncidentReport VFXIncident;
    VFXIncident.Title = FString::Printf(TEXT("VFX System Failure: %s"), *SystemName);
    VFXIncident.Description = ErrorDetails;
    VFXIncident.Severity = EQA_IncidentSeverity::High;
    VFXIncident.Category = EQA_IncidentCategory::VFX;
    VFXIncident.AffectedSystems.Add(SystemName);
    
    ReportIncident(VFXIncident);
}

void UQA_CriticalIncidentReport::ReportCompilationFailure(const FString& ModuleName, const FString& ErrorDetails)
{
    FQA_IncidentReport CompileIncident;
    CompileIncident.Title = FString::Printf(TEXT("Compilation Failure: %s"), *ModuleName);
    CompileIncident.Description = ErrorDetails;
    CompileIncident.Severity = EQA_IncidentSeverity::Blocker;
    CompileIncident.Category = EQA_IncidentCategory::Compilation;
    CompileIncident.AffectedSystems.Add(ModuleName);
    
    ReportIncident(CompileIncident);
}

bool UQA_CriticalIncidentReport::IsSystemBlocked(const FString& SystemName)
{
    bool* BlockStatus = SystemBlockStatus.Find(SystemName);
    return BlockStatus ? *BlockStatus : false;
}

void UQA_CriticalIncidentReport::GenerateIncidentReport()
{
    FString ReportContent = TEXT("=== QA CRITICAL INCIDENT REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Total Incidents: %d\n"), IncidentDatabase.Num());
    
    TArray<FQA_IncidentReport> ActiveIncidents = GetActiveIncidents();
    ReportContent += FString::Printf(TEXT("Active Incidents: %d\n\n"), ActiveIncidents.Num());
    
    // Group by severity
    for (int32 SevIndex = 0; SevIndex < 5; SevIndex++)
    {
        EQA_IncidentSeverity Severity = static_cast<EQA_IncidentSeverity>(SevIndex);
        TArray<FQA_IncidentReport> SeverityIncidents = GetIncidentsBySeverity(Severity);
        
        if (SeverityIncidents.Num() > 0)
        {
            FString SeverityName;
            switch (Severity)
            {
                case EQA_IncidentSeverity::Low: SeverityName = TEXT("LOW"); break;
                case EQA_IncidentSeverity::Medium: SeverityName = TEXT("MEDIUM"); break;
                case EQA_IncidentSeverity::High: SeverityName = TEXT("HIGH"); break;
                case EQA_IncidentSeverity::Critical: SeverityName = TEXT("CRITICAL"); break;
                case EQA_IncidentSeverity::Blocker: SeverityName = TEXT("BLOCKER"); break;
            }
            
            ReportContent += FString::Printf(TEXT("=== %s SEVERITY (%d incidents) ===\n"), *SeverityName, SeverityIncidents.Num());
            
            for (const FQA_IncidentReport& Incident : SeverityIncidents)
            {
                ReportContent += FString::Printf(TEXT("ID: %s\n"), *Incident.IncidentID);
                ReportContent += FString::Printf(TEXT("Title: %s\n"), *Incident.Title);
                ReportContent += FString::Printf(TEXT("Agent: %s\n"), *Incident.AgentID);
                ReportContent += FString::Printf(TEXT("Cycle: %s\n"), *Incident.CycleID);
                ReportContent += FString::Printf(TEXT("Status: %s\n"), Incident.bResolved ? TEXT("RESOLVED") : TEXT("ACTIVE"));
                ReportContent += FString::Printf(TEXT("Description: %s\n"), *Incident.Description);
                if (!Incident.WorkaroundSolution.IsEmpty())
                {
                    ReportContent += FString::Printf(TEXT("Solution: %s\n"), *Incident.WorkaroundSolution);
                }
                ReportContent += TEXT("\n");
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *ReportContent);
}

void UQA_CriticalIncidentReport::ExportIncidentsToFile(const FString& FilePath)
{
    GenerateIncidentReport();
    // File export would be implemented here if needed
}

FString UQA_CriticalIncidentReport::GenerateIncidentID()
{
    FString ID = FString::Printf(TEXT("INC_%04d_%s"), NextIncidentID++, *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    return ID;
}

void UQA_CriticalIncidentReport::LogIncidentToConsole(const FQA_IncidentReport& Incident)
{
    FString SeverityStr;
    switch (Incident.Severity)
    {
        case EQA_IncidentSeverity::Low: SeverityStr = TEXT("LOW"); break;
        case EQA_IncidentSeverity::Medium: SeverityStr = TEXT("MEDIUM"); break;
        case EQA_IncidentSeverity::High: SeverityStr = TEXT("HIGH"); break;
        case EQA_IncidentSeverity::Critical: SeverityStr = TEXT("CRITICAL"); break;
        case EQA_IncidentSeverity::Blocker: SeverityStr = TEXT("BLOCKER"); break;
    }
    
    UE_LOG(LogTemp, Error, TEXT("QA INCIDENT [%s] %s: %s - %s"), 
           *SeverityStr, *Incident.IncidentID, *Incident.Title, *Incident.Description);
}

void UQA_CriticalIncidentReport::UpdateSystemBlockStatus()
{
    SystemBlockStatus.Empty();
    
    for (const FQA_IncidentReport& Incident : IncidentDatabase)
    {
        if (!Incident.bResolved && Incident.Severity == EQA_IncidentSeverity::Blocker)
        {
            for (const FString& System : Incident.AffectedSystems)
            {
                SystemBlockStatus.Add(System, true);
            }
        }
    }
}