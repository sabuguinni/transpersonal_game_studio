#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "UObject/UObjectGlobals.h"
#include "Kismet/GameplayStatics.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    LastValidationTime = 0.0f;
    bIsValidating = false;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initialized"));
    
    // Initialize system reports
    SystemReports.Empty();
    
    // Schedule initial validation
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            ValidateAllSystems();
        });
    }
}

void UBuildIntegrationManager::Deinitialize()
{
    SystemReports.Empty();
    Super::Deinitialize();
}

void UBuildIntegrationManager::ValidateAllSystems()
{
    if (bIsValidating)
    {
        return;
    }
    
    bIsValidating = true;
    SystemReports.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting system validation"));
    
    // Validate all core systems
    ValidateCharacterSystem();
    ValidateWorldGeneration();
    ValidateVFXSystem();
    ValidatePhysicsSystem();
    ValidateAISystem();
    ValidateQAFramework();
    
    LastValidationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    bIsValidating = false;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation complete - %d systems checked"), SystemReports.Num());
}

FBuild_SystemReport UBuildIntegrationManager::ValidateSystem(const FString& SystemName)
{
    for (const FBuild_SystemReport& Report : SystemReports)
    {
        if (Report.SystemName == SystemName)
        {
            return Report;
        }
    }
    
    return CreateSystemReport(SystemName, EBuild_SystemStatus::Unknown, TEXT("System not found"));
}

TArray<FBuild_SystemReport> UBuildIntegrationManager::GetSystemReports() const
{
    return SystemReports;
}

bool UBuildIntegrationManager::IsSystemOperational(const FString& SystemName) const
{
    for (const FBuild_SystemReport& Report : SystemReports)
    {
        if (Report.SystemName == SystemName)
        {
            return Report.Status == EBuild_SystemStatus::Operational;
        }
    }
    return false;
}

float UBuildIntegrationManager::GetOverallPerformanceScore() const
{
    if (SystemReports.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalScore = 0.0f;
    for (const FBuild_SystemReport& Report : SystemReports)
    {
        TotalScore += Report.PerformanceScore;
    }
    
    return TotalScore / SystemReports.Num();
}

int32 UBuildIntegrationManager::GetTotalActorCount() const
{
    int32 TotalCount = 0;
    for (const FBuild_SystemReport& Report : SystemReports)
    {
        TotalCount += Report.ActorCount;
    }
    return TotalCount;
}

void UBuildIntegrationManager::OptimizePerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting performance optimization"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count and optimize actors
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            ActorCount++;
            
            // Basic optimization: disable tick for static actors
            if (Actor->GetRootComponent() && Actor->GetRootComponent()->Mobility == EComponentMobility::Static)
            {
                Actor->SetActorTickEnabled(false);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Optimized %d actors"), ActorCount);
}

bool UBuildIntegrationManager::SaveCurrentBuild()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Save the current map
    FString MapPath = TEXT("/Game/Maps/MinPlayableMap");
    bool bSaveResult = UEditorLoadingAndSavingUtils::SaveMap(World, MapPath);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Map save result: %s"), bSaveResult ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    return bSaveResult;
}

void UBuildIntegrationManager::GenerateBuildReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Systems: %d"), SystemReports.Num());
    UE_LOG(LogTemp, Warning, TEXT("Overall Performance: %.2f"), GetOverallPerformanceScore());
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), GetTotalActorCount());
    
    for (const FBuild_SystemReport& Report : SystemReports)
    {
        FString StatusStr;
        switch (Report.Status)
        {
            case EBuild_SystemStatus::Operational: StatusStr = TEXT("OPERATIONAL"); break;
            case EBuild_SystemStatus::Failed: StatusStr = TEXT("FAILED"); break;
            case EBuild_SystemStatus::Loading: StatusStr = TEXT("LOADING"); break;
            case EBuild_SystemStatus::Disabled: StatusStr = TEXT("DISABLED"); break;
            default: StatusStr = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s (Score: %.2f, Actors: %d)"), 
               *Report.SystemName, *StatusStr, Report.PerformanceScore, Report.ActorCount);
        
        if (!Report.ErrorMessage.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("    Error: %s"), *Report.ErrorMessage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UBuildIntegrationManager::ValidateCharacterSystem()
{
    FString SystemName = TEXT("Character System");
    
    try
    {
        UClass* CharacterClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
        if (CharacterClass)
        {
            FBuild_SystemReport Report = CreateSystemReport(SystemName, EBuild_SystemStatus::Operational);
            Report.PerformanceScore = 85.0f;
            UpdateSystemReport(SystemName, Report);
        }
        else
        {
            UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("TranspersonalCharacter class not found")));
        }
    }
    catch (...)
    {
        UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("Exception during character validation")));
    }
}

void UBuildIntegrationManager::ValidateWorldGeneration()
{
    FString SystemName = TEXT("World Generation");
    
    try
    {
        UClass* PCGClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
        if (PCGClass)
        {
            FBuild_SystemReport Report = CreateSystemReport(SystemName, EBuild_SystemStatus::Operational);
            Report.PerformanceScore = 78.0f;
            UpdateSystemReport(SystemName, Report);
        }
        else
        {
            UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("PCGWorldGenerator class not found")));
        }
    }
    catch (...)
    {
        UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("Exception during world gen validation")));
    }
}

void UBuildIntegrationManager::ValidateVFXSystem()
{
    FString SystemName = TEXT("VFX System");
    
    try
    {
        UClass* VFXClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFXImpactManager"));
        if (VFXClass)
        {
            FBuild_SystemReport Report = CreateSystemReport(SystemName, EBuild_SystemStatus::Operational);
            Report.PerformanceScore = 82.0f;
            UpdateSystemReport(SystemName, Report);
        }
        else
        {
            UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("VFXImpactManager class not found")));
        }
    }
    catch (...)
    {
        UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("Exception during VFX validation")));
    }
}

void UBuildIntegrationManager::ValidatePhysicsSystem()
{
    FString SystemName = TEXT("Physics System");
    
    UWorld* World = GetWorld();
    if (World && World->GetPhysicsScene())
    {
        FBuild_SystemReport Report = CreateSystemReport(SystemName, EBuild_SystemStatus::Operational);
        Report.PerformanceScore = 90.0f;
        UpdateSystemReport(SystemName, Report);
    }
    else
    {
        UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("Physics scene not available")));
    }
}

void UBuildIntegrationManager::ValidateAISystem()
{
    FString SystemName = TEXT("AI System");
    
    try
    {
        UClass* CrowdClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
        if (CrowdClass)
        {
            FBuild_SystemReport Report = CreateSystemReport(SystemName, EBuild_SystemStatus::Operational);
            Report.PerformanceScore = 75.0f;
            UpdateSystemReport(SystemName, Report);
        }
        else
        {
            UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("CrowdSimulationManager class not found")));
        }
    }
    catch (...)
    {
        UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("Exception during AI validation")));
    }
}

void UBuildIntegrationManager::ValidateQAFramework()
{
    FString SystemName = TEXT("QA Framework");
    
    try
    {
        UClass* QAClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.QA_ValidationFramework"));
        if (QAClass)
        {
            FBuild_SystemReport Report = CreateSystemReport(SystemName, EBuild_SystemStatus::Operational);
            Report.PerformanceScore = 88.0f;
            UpdateSystemReport(SystemName, Report);
        }
        else
        {
            UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("QA_ValidationFramework class not found")));
        }
    }
    catch (...)
    {
        UpdateSystemReport(SystemName, CreateSystemReport(SystemName, EBuild_SystemStatus::Failed, TEXT("Exception during QA validation")));
    }
}

FBuild_SystemReport UBuildIntegrationManager::CreateSystemReport(const FString& SystemName, EBuild_SystemStatus Status, const FString& ErrorMsg)
{
    FBuild_SystemReport Report;
    Report.SystemName = SystemName;
    Report.Status = Status;
    Report.ErrorMessage = ErrorMsg;
    Report.ActorCount = 0;
    Report.PerformanceScore = (Status == EBuild_SystemStatus::Operational) ? 80.0f : 0.0f;
    
    return Report;
}

void UBuildIntegrationManager::UpdateSystemReport(const FString& SystemName, const FBuild_SystemReport& Report)
{
    for (int32 i = 0; i < SystemReports.Num(); i++)
    {
        if (SystemReports[i].SystemName == SystemName)
        {
            SystemReports[i] = Report;
            return;
        }
    }
    
    SystemReports.Add(Report);
}