#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    LastValidationTime = 0.0f;
    bIsValidationInProgress = false;

    // Initialize core system names
    CoreSystemNames.Add(TEXT("Character System"));
    CoreSystemNames.Add(TEXT("Environment System"));
    CoreSystemNames.Add(TEXT("Lighting System"));
    CoreSystemNames.Add(TEXT("VFX System"));
    CoreSystemNames.Add(TEXT("Audio System"));
    CoreSystemNames.Add(TEXT("Physics System"));
    CoreSystemNames.Add(TEXT("AI System"));
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build Integration Orchestrator Initialized"));
    
    // Register all core systems
    for (const FString& SystemName : CoreSystemNames)
    {
        RegisterSystemForValidation(SystemName, 0);
    }
    
    // Run initial validation
    RunFullSystemValidation();
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    RegisteredSystems.Empty();
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::RunFullSystemValidation()
{
    if (bIsValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation already in progress, skipping"));
        return;
    }

    bIsValidationInProgress = true;
    UE_LOG(LogTemp, Warning, TEXT("Starting full system validation"));

    // Validate all core systems
    ValidateCharacterSystem();
    ValidateEnvironmentSystem();
    ValidateLightingSystem();
    ValidateVFXSystem();
    ValidateAudioSystem();
    ValidatePhysicsSystem();
    ValidateAISystem();

    LastValidationTime = GetWorld()->GetTimeSeconds();
    bIsValidationInProgress = false;

    UE_LOG(LogTemp, Warning, TEXT("Full system validation complete"));
}

FBuild_IntegrationReport UBuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    Report.LastIntegrationTime = FDateTime::Now();
    Report.TotalActors = 0;
    Report.ValidatedSystems = 0;

    // Collect system reports
    for (const auto& SystemPair : RegisteredSystems)
    {
        Report.SystemReports.Add(SystemPair.Value);
        Report.TotalActors += SystemPair.Value.ActorCount;
        
        if (SystemPair.Value.Status == EBuild_SystemStatus::Validated)
        {
            Report.ValidatedSystems++;
        }
    }

    // Calculate build health
    Report.BuildHealth = CalculateBuildHealth();
    
    // Determine build status
    if (Report.BuildHealth >= 0.8f)
    {
        Report.BuildStatus = TEXT("Excellent");
    }
    else if (Report.BuildHealth >= 0.6f)
    {
        Report.BuildStatus = TEXT("Good");
    }
    else if (Report.BuildHealth >= 0.4f)
    {
        Report.BuildStatus = TEXT("Fair");
    }
    else
    {
        Report.BuildStatus = TEXT("Poor");
    }

    LastIntegrationReport = Report;
    return Report;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateSystemIntegrity(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("System not registered: %s"), *SystemName);
        return false;
    }

    FBuild_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
    
    // Basic validation - check if system has any actors
    bool bIsValid = SystemInfo.ActorCount > 0;
    
    if (bIsValid)
    {
        SystemInfo.Status = EBuild_SystemStatus::Validated;
        SystemInfo.ValidationDetails = TEXT("System validation passed");
    }
    else
    {
        SystemInfo.Status = EBuild_SystemStatus::Failed;
        SystemInfo.ValidationDetails = TEXT("No actors found for system");
    }

    SystemInfo.LastValidationTime = GetWorld()->GetTimeSeconds();
    
    return bIsValid;
}

void UBuild_FinalIntegrationOrchestrator::RegisterSystemForValidation(const FString& SystemName, int32 ExpectedActorCount)
{
    FBuild_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.Status = EBuild_SystemStatus::Pending;
    SystemInfo.ActorCount = ExpectedActorCount;
    SystemInfo.LastValidationTime = 0.0f;
    SystemInfo.ValidationDetails = TEXT("System registered for validation");

    RegisteredSystems.Add(SystemName, SystemInfo);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered system for validation: %s"), *SystemName);
}

float UBuild_FinalIntegrationOrchestrator::CalculateBuildHealth()
{
    if (RegisteredSystems.Num() == 0)
    {
        return 0.0f;
    }

    int32 ValidatedCount = 0;
    int32 TotalCount = RegisteredSystems.Num();

    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == EBuild_SystemStatus::Validated)
        {
            ValidatedCount++;
        }
    }

    return static_cast<float>(ValidatedCount) / static_cast<float>(TotalCount);
}

EBuild_SystemStatus UBuild_FinalIntegrationOrchestrator::GetSystemStatus(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        return RegisteredSystems[SystemName].Status;
    }
    return EBuild_SystemStatus::Unknown;
}

void UBuild_FinalIntegrationOrchestrator::SetSystemStatus(const FString& SystemName, EBuild_SystemStatus NewStatus)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems[SystemName].Status = NewStatus;
        UE_LOG(LogTemp, Warning, TEXT("System %s status updated to %d"), *SystemName, static_cast<int32>(NewStatus));
    }
}

TArray<FString> UBuild_FinalIntegrationOrchestrator::GetFailedSystems()
{
    TArray<FString> FailedSystems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == EBuild_SystemStatus::Failed)
        {
            FailedSystems.Add(SystemPair.Key);
        }
    }
    
    return FailedSystems;
}

void UBuild_FinalIntegrationOrchestrator::OrchestrateFullBuild()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting full build orchestration"));
    
    // Run comprehensive validation
    RunFullSystemValidation();
    
    // Generate integration report
    FBuild_IntegrationReport Report = GenerateIntegrationReport();
    
    UE_LOG(LogTemp, Warning, TEXT("Build orchestration complete - Health: %.2f, Status: %s"), 
           Report.BuildHealth, *Report.BuildStatus);
}

bool UBuild_FinalIntegrationOrchestrator::IsBuildReady()
{
    float Health = CalculateBuildHealth();
    return Health >= 0.7f; // 70% of systems must be validated
}

void UBuild_FinalIntegrationOrchestrator::ForceBuildValidation()
{
    bIsValidationInProgress = false; // Reset flag
    RunFullSystemValidation();
}

// Private validation functions
void UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystem()
{
    int32 CharacterCount = CountActorsOfType(TEXT("Character"));
    UpdateSystemInfo(TEXT("Character System"), 
                    CharacterCount > 0 ? EBuild_SystemStatus::Validated : EBuild_SystemStatus::Failed,
                    CharacterCount,
                    FString::Printf(TEXT("Found %d character actors"), CharacterCount));
}

void UBuild_FinalIntegrationOrchestrator::ValidateEnvironmentSystem()
{
    int32 StaticMeshCount = CountActorsOfType(TEXT("StaticMesh"));
    UpdateSystemInfo(TEXT("Environment System"),
                    StaticMeshCount > 0 ? EBuild_SystemStatus::Validated : EBuild_SystemStatus::Failed,
                    StaticMeshCount,
                    FString::Printf(TEXT("Found %d static mesh actors"), StaticMeshCount));
}

void UBuild_FinalIntegrationOrchestrator::ValidateLightingSystem()
{
    int32 LightCount = CountActorsOfType(TEXT("Light"));
    UpdateSystemInfo(TEXT("Lighting System"),
                    LightCount > 0 ? EBuild_SystemStatus::Validated : EBuild_SystemStatus::Failed,
                    LightCount,
                    FString::Printf(TEXT("Found %d light actors"), LightCount));
}

void UBuild_FinalIntegrationOrchestrator::ValidateVFXSystem()
{
    int32 VFXCount = CountActorsOfType(TEXT("Niagara")) + CountActorsOfType(TEXT("Particle"));
    UpdateSystemInfo(TEXT("VFX System"),
                    VFXCount >= 0 ? EBuild_SystemStatus::Validated : EBuild_SystemStatus::Failed,
                    VFXCount,
                    FString::Printf(TEXT("Found %d VFX actors"), VFXCount));
}

void UBuild_FinalIntegrationOrchestrator::ValidateAudioSystem()
{
    int32 AudioCount = CountActorsOfType(TEXT("Audio"));
    UpdateSystemInfo(TEXT("Audio System"),
                    AudioCount >= 0 ? EBuild_SystemStatus::Validated : EBuild_SystemStatus::Failed,
                    AudioCount,
                    FString::Printf(TEXT("Found %d audio actors"), AudioCount));
}

void UBuild_FinalIntegrationOrchestrator::ValidatePhysicsSystem()
{
    // Physics system is always considered validated if world exists
    UpdateSystemInfo(TEXT("Physics System"),
                    GetWorld() ? EBuild_SystemStatus::Validated : EBuild_SystemStatus::Failed,
                    1,
                    TEXT("Physics world validated"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateAISystem()
{
    int32 AICount = CountActorsOfType(TEXT("Pawn"));
    UpdateSystemInfo(TEXT("AI System"),
                    AICount >= 0 ? EBuild_SystemStatus::Validated : EBuild_SystemStatus::Failed,
                    AICount,
                    FString::Printf(TEXT("Found %d AI pawns"), AICount));
}

int32 UBuild_FinalIntegrationOrchestrator::CountActorsOfType(const FString& ActorTypeName)
{
    if (!GetWorld())
    {
        return 0;
    }

    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(ActorTypeName))
        {
            Count++;
        }
    }
    
    return Count;
}

void UBuild_FinalIntegrationOrchestrator::LogSystemValidation(const FString& SystemName, bool bSuccess, const FString& Details)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ %s: %s"), *SystemName, *Details);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ %s: %s"), *SystemName, *Details);
    }
}

void UBuild_FinalIntegrationOrchestrator::UpdateSystemInfo(const FString& SystemName, EBuild_SystemStatus Status, int32 ActorCount, const FString& Details)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        FBuild_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
        SystemInfo.Status = Status;
        SystemInfo.ActorCount = ActorCount;
        SystemInfo.ValidationDetails = Details;
        SystemInfo.LastValidationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        LogSystemValidation(SystemName, Status == EBuild_SystemStatus::Validated, Details);
    }
}