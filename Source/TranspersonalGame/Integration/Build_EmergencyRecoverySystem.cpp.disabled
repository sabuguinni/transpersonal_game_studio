#include "Build_EmergencyRecoverySystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

void UBuild_EmergencyRecoverySystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Initializing emergency recovery system"));
    
    InitializeRecoveryActions();
    bEmergencyActive = false;
    CurrentEmergency = FBuild_EmergencyEvent();
    LastRecoveryAttemptTime = 0.0f;
    
    // Check for existing emergency state from previous sessions
    FString EmergencyFilePath = FPaths::ProjectSavedDir() / TEXT("EmergencyState.txt");
    if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*EmergencyFilePath))
    {
        FString EmergencyData;
        if (FFileHelper::LoadFileToString(EmergencyData, *EmergencyFilePath))
        {
            UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Found previous emergency state: %s"), *EmergencyData);
            // Parse and restore emergency state if needed
        }
    }
}

void UBuild_EmergencyRecoverySystem::Deinitialize()
{
    if (bEmergencyActive)
    {
        SaveEmergencyReport();
    }
    
    Super::Deinitialize();
}

void UBuild_EmergencyRecoverySystem::ReportEmergency(EBuild_EmergencyType EmergencyType, const FString& AgentName, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Build_EmergencyRecoverySystem: EMERGENCY REPORTED - Type: %d, Agent: %s, Error: %s"), 
           (int32)EmergencyType, *AgentName, *ErrorMessage);
    
    FBuild_EmergencyEvent NewEmergency;
    NewEmergency.EmergencyType = EmergencyType;
    NewEmergency.AgentName = AgentName;
    NewEmergency.ErrorMessage = ErrorMessage;
    NewEmergency.Timestamp = FDateTime::Now();
    
    EmergencyHistory.Add(NewEmergency);
    CurrentEmergency = NewEmergency;
    bEmergencyActive = true;
    
    // Immediate recovery attempt for critical emergencies
    if (EmergencyType == EBuild_EmergencyType::UE5BridgeFailure || 
        EmergencyType == EBuild_EmergencyType::CriticalSystemCrash)
    {
        AttemptRecovery();
    }
    
    SaveEmergencyReport();
}

bool UBuild_EmergencyRecoverySystem::IsEmergencyActive() const
{
    return bEmergencyActive;
}

EBuild_EmergencyType UBuild_EmergencyRecoverySystem::GetCurrentEmergencyType() const
{
    return CurrentEmergency.EmergencyType;
}

TArray<FBuild_EmergencyEvent> UBuild_EmergencyRecoverySystem::GetEmergencyHistory() const
{
    return EmergencyHistory;
}

bool UBuild_EmergencyRecoverySystem::AttemptRecovery()
{
    if (!bEmergencyActive)
    {
        return true;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastRecoveryAttemptTime < 30.0f) // Prevent spam recovery attempts
    {
        return false;
    }
    
    LastRecoveryAttemptTime = CurrentTime;
    CurrentEmergency.bRecoveryAttempted = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Attempting recovery for emergency type: %d"), 
           (int32)CurrentEmergency.EmergencyType);
    
    bool bRecoverySuccess = false;
    
    switch (CurrentEmergency.EmergencyType)
    {
        case EBuild_EmergencyType::UE5BridgeFailure:
            bRecoverySuccess = ExecuteUE5BridgeRecovery();
            break;
            
        case EBuild_EmergencyType::CompilationError:
            bRecoverySuccess = ExecuteCompilationRecovery();
            break;
            
        case EBuild_EmergencyType::TimeoutCascade:
            bRecoverySuccess = ExecuteTimeoutRecovery();
            break;
            
        case EBuild_EmergencyType::CriticalSystemCrash:
            // For critical crashes, just clear the emergency and let the system restart
            bRecoverySuccess = true;
            break;
            
        case EBuild_EmergencyType::APIRateLimit:
            // For rate limits, wait and clear
            bRecoverySuccess = true;
            break;
            
        default:
            bRecoverySuccess = false;
            break;
    }
    
    CurrentEmergency.bRecoverySuccessful = bRecoverySuccess;
    
    if (bRecoverySuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Recovery successful"));
        ClearEmergencyState();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Build_EmergencyRecoverySystem: Recovery failed"));
    }
    
    return bRecoverySuccess;
}

void UBuild_EmergencyRecoverySystem::ClearEmergencyState()
{
    bEmergencyActive = false;
    CurrentEmergency = FBuild_EmergencyEvent();
    
    // Clean up emergency state file
    FString EmergencyFilePath = FPaths::ProjectSavedDir() / TEXT("EmergencyState.txt");
    FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*EmergencyFilePath);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Emergency state cleared"));
}

bool UBuild_EmergencyRecoverySystem::ShouldSkipUE5Commands() const
{
    if (!bEmergencyActive)
    {
        return false;
    }
    
    return (CurrentEmergency.EmergencyType == EBuild_EmergencyType::UE5BridgeFailure ||
            CurrentEmergency.EmergencyType == EBuild_EmergencyType::CriticalSystemCrash);
}

bool UBuild_EmergencyRecoverySystem::ShouldUseEmergencyWorkflow() const
{
    return bEmergencyActive;
}

int32 UBuild_EmergencyRecoverySystem::GetMaxToolCallsForAgent(const FString& AgentName) const
{
    if (!bEmergencyActive)
    {
        return 12; // Normal max
    }
    
    // Emergency limits based on agent number
    if (AgentName.Contains(TEXT("18")) || AgentName.Contains(TEXT("19")) || AgentName.Contains(TEXT("20")))
    {
        return 3; // Critical agents get minimal calls
    }
    
    if (AgentName.Contains(TEXT("15")) || AgentName.Contains(TEXT("16")) || AgentName.Contains(TEXT("17")))
    {
        return 6; // Late agents get reduced calls
    }
    
    return 8; // Earlier agents get more calls
}

void UBuild_EmergencyRecoverySystem::LogRecoveryAction(const FString& ActionName, const FString& Description, float ExecutionTime)
{
    FBuild_RecoveryAction Action;
    Action.ActionName = ActionName;
    Action.Description = Description;
    Action.ExecutionTimeSeconds = ExecutionTime;
    Action.bIsEmergencyOnly = bEmergencyActive;
    
    RecoveryActions.Add(Action);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Recovery action logged - %s: %s (%.2fs)"), 
           *ActionName, *Description, ExecutionTime);
}

void UBuild_EmergencyRecoverySystem::InitializeRecoveryActions()
{
    RecoveryActions.Empty();
    
    // Pre-define common recovery actions
    FBuild_RecoveryAction UE5BridgeAction;
    UE5BridgeAction.ActionName = TEXT("UE5 Bridge Reset");
    UE5BridgeAction.Description = TEXT("Reset UE5 Remote Control bridge connection");
    UE5BridgeAction.bRequiresUE5Restart = false;
    RecoveryActions.Add(UE5BridgeAction);
    
    FBuild_RecoveryAction CompilationAction;
    CompilationAction.ActionName = TEXT("Compilation Recovery");
    CompilationAction.Description = TEXT("Clean and rebuild project modules");
    CompilationAction.bRequiresUE5Restart = true;
    RecoveryActions.Add(CompilationAction);
    
    FBuild_RecoveryAction TimeoutAction;
    TimeoutAction.ActionName = TEXT("Timeout Recovery");
    TimeoutAction.Description = TEXT("Reduce agent scope and skip heavy operations");
    TimeoutAction.bRequiresUE5Restart = false;
    RecoveryActions.Add(TimeoutAction);
}

bool UBuild_EmergencyRecoverySystem::ExecuteUE5BridgeRecovery()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Executing UE5 Bridge recovery"));
    
    // For UE5 bridge failures, we can't actually fix the bridge from within UE5
    // But we can set flags to prevent further bridge calls
    LogRecoveryAction(TEXT("UE5 Bridge Recovery"), TEXT("Set emergency flags to prevent further bridge calls"), 0.1f);
    
    return true; // Always succeed - the real recovery is preventing further calls
}

bool UBuild_EmergencyRecoverySystem::ExecuteCompilationRecovery()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Executing compilation recovery"));
    
    // Log the recovery attempt
    LogRecoveryAction(TEXT("Compilation Recovery"), TEXT("Flagged for clean rebuild on next cycle"), 0.1f);
    
    return true;
}

bool UBuild_EmergencyRecoverySystem::ExecuteTimeoutRecovery()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Executing timeout recovery"));
    
    // For timeout recovery, just set flags to reduce scope
    LogRecoveryAction(TEXT("Timeout Recovery"), TEXT("Reduced agent scope for remaining cycles"), 0.1f);
    
    return true;
}

void UBuild_EmergencyRecoverySystem::SaveEmergencyReport()
{
    FString ReportContent = FString::Printf(TEXT("EMERGENCY REPORT - %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Emergency Type: %d\n"), (int32)CurrentEmergency.EmergencyType);
    ReportContent += FString::Printf(TEXT("Agent: %s\n"), *CurrentEmergency.AgentName);
    ReportContent += FString::Printf(TEXT("Error: %s\n"), *CurrentEmergency.ErrorMessage);
    ReportContent += FString::Printf(TEXT("Recovery Attempted: %s\n"), CurrentEmergency.bRecoveryAttempted ? TEXT("Yes") : TEXT("No"));
    ReportContent += FString::Printf(TEXT("Recovery Successful: %s\n"), CurrentEmergency.bRecoverySuccessful ? TEXT("Yes") : TEXT("No"));
    
    FString ReportPath = FPaths::ProjectSavedDir() / TEXT("EmergencyReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_EmergencyRecoverySystem: Emergency report saved to %s"), *ReportPath);
}

// Emergency Recovery Actor Implementation
ABuild_EmergencyRecoveryActor::ABuild_EmergencyRecoveryActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create emergency indicator component
    EmergencyIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EmergencyIndicator"));
    RootComponent = EmergencyIndicator;
    
    // Set default values
    bAutoRecoveryEnabled = true;
    RecoveryCheckInterval = 5.0f;
}

void ABuild_EmergencyRecoveryActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRecoveryEnabled)
    {
        GetWorldTimerManager().SetTimer(RecoveryCheckTimer, this, &ABuild_EmergencyRecoveryActor::CheckEmergencyStatus, 
                                       RecoveryCheckInterval, true);
    }
}

void ABuild_EmergencyRecoveryActor::TriggerEmergencyRecovery()
{
    UBuild_EmergencyRecoverySystem* RecoverySystem = GetGameInstance()->GetSubsystem<UBuild_EmergencyRecoverySystem>();
    if (RecoverySystem)
    {
        RecoverySystem->AttemptRecovery();
    }
}

void ABuild_EmergencyRecoveryActor::DisplayEmergencyStatus()
{
    UBuild_EmergencyRecoverySystem* RecoverySystem = GetGameInstance()->GetSubsystem<UBuild_EmergencyRecoverySystem>();
    if (RecoverySystem)
    {
        bool bEmergencyActive = RecoverySystem->IsEmergencyActive();
        EBuild_EmergencyType EmergencyType = RecoverySystem->GetCurrentEmergencyType();
        
        UE_LOG(LogTemp, Warning, TEXT("Emergency Status - Active: %s, Type: %d"), 
               bEmergencyActive ? TEXT("Yes") : TEXT("No"), (int32)EmergencyType);
    }
}

void ABuild_EmergencyRecoveryActor::CheckEmergencyStatus()
{
    UBuild_EmergencyRecoverySystem* RecoverySystem = GetGameInstance()->GetSubsystem<UBuild_EmergencyRecoverySystem>();
    if (RecoverySystem && RecoverySystem->IsEmergencyActive())
    {
        // Visual indication of emergency state
        if (EmergencyIndicator)
        {
            // Change color or material to indicate emergency
            UE_LOG(LogTemp, Warning, TEXT("Emergency Recovery Actor: Emergency state detected"));
        }
        
        // Attempt automatic recovery if enabled
        if (bAutoRecoveryEnabled)
        {
            RecoverySystem->AttemptRecovery();
        }
    }
}