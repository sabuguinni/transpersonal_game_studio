#include "BuildIntegrationReport.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "UObject/UObjectGlobals.h"

UBuildIntegrationReport::UBuildIntegrationReport()
{
    IntegrationData = FBuild_IntegrationData();
}

void UBuildIntegrationReport::ValidateAllSystems()
{
    ClearReport();
    
    // Validate core systems
    ValidateCharacterSystem();
    ValidateWorldGeneration();
    ValidateDinosaurAI();
    ValidateVFXSystem();
    ValidateFoliageSystem();
    ValidateCrowdSimulation();
    ValidateAudioSystem();
    
    // Update overall status
    UpdateOverallStatus();
    
    // Update timestamp
    IntegrationData.LastValidation = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("Build Integration Validation Complete - Overall Status: %d"), (int32)IntegrationData.OverallStatus);
}

FBuild_IntegrationData UBuildIntegrationReport::GetIntegrationData() const
{
    return IntegrationData;
}

EBuild_IntegrationStatus UBuildIntegrationReport::GetSystemStatus(const FString& SystemName) const
{
    for (const FBuild_SystemStatus& Status : IntegrationData.SystemStatuses)
    {
        if (Status.SystemName == SystemName)
        {
            return Status.Status;
        }
    }
    return EBuild_IntegrationStatus::Unknown;
}

void UBuildIntegrationReport::AddSystemStatus(const FString& SystemName, EBuild_IntegrationStatus Status, int32 ActorCount, const FString& ErrorMessage)
{
    FBuild_SystemStatus NewStatus;
    NewStatus.SystemName = SystemName;
    NewStatus.Status = Status;
    NewStatus.ActorCount = ActorCount;
    NewStatus.LastError = ErrorMessage;
    
    IntegrationData.SystemStatuses.Add(NewStatus);
    
    UE_LOG(LogTemp, Log, TEXT("System Status Added: %s - Status: %d - Actors: %d"), *SystemName, (int32)Status, ActorCount);
}

bool UBuildIntegrationReport::IsSystemOperational(const FString& SystemName) const
{
    EBuild_IntegrationStatus Status = GetSystemStatus(SystemName);
    return Status == EBuild_IntegrationStatus::Operational || Status == EBuild_IntegrationStatus::Warning;
}

TArray<FString> UBuildIntegrationReport::GetFailedSystems() const
{
    TArray<FString> FailedSystems;
    
    for (const FBuild_SystemStatus& Status : IntegrationData.SystemStatuses)
    {
        if (Status.Status == EBuild_IntegrationStatus::Failed || Status.Status == EBuild_IntegrationStatus::Critical)
        {
            FailedSystems.Add(Status.SystemName);
        }
    }
    
    return FailedSystems;
}

void UBuildIntegrationReport::ClearReport()
{
    IntegrationData.SystemStatuses.Empty();
    IntegrationData.TotalActors = 0;
    IntegrationData.LoadedClasses = 0;
    IntegrationData.FailedClasses = 0;
    IntegrationData.OverallStatus = EBuild_IntegrationStatus::Unknown;
}

FString UBuildIntegrationReport::GenerateReportSummary() const
{
    FString Summary = FString::Printf(TEXT("=== BUILD INTEGRATION REPORT ===\n"));
    Summary += FString::Printf(TEXT("Validation Time: %s\n"), *IntegrationData.LastValidation.ToString());
    Summary += FString::Printf(TEXT("Overall Status: %d\n"), (int32)IntegrationData.OverallStatus);
    Summary += FString::Printf(TEXT("Total Actors: %d\n"), IntegrationData.TotalActors);
    Summary += FString::Printf(TEXT("Loaded Classes: %d\n"), IntegrationData.LoadedClasses);
    Summary += FString::Printf(TEXT("Failed Classes: %d\n"), IntegrationData.FailedClasses);
    Summary += FString::Printf(TEXT("\nSYSTEM STATUS:\n"));
    
    for (const FBuild_SystemStatus& Status : IntegrationData.SystemStatuses)
    {
        Summary += FString::Printf(TEXT("  %s: %d (Actors: %d)\n"), 
            *Status.SystemName, (int32)Status.Status, Status.ActorCount);
        
        if (!Status.LastError.IsEmpty())
        {
            Summary += FString::Printf(TEXT("    Error: %s\n"), *Status.LastError);
        }
    }
    
    return Summary;
}

void UBuildIntegrationReport::ValidateCharacterSystem()
{
    bool bClassLoaded = ValidateClassLoading(TEXT("TranspersonalCharacter"));
    int32 ActorCount = CountActorsOfType(TEXT("TranspersonalCharacter"));
    
    EBuild_IntegrationStatus Status = bClassLoaded ? EBuild_IntegrationStatus::Operational : EBuild_IntegrationStatus::Failed;
    FString ErrorMsg = bClassLoaded ? TEXT("") : TEXT("TranspersonalCharacter class not loaded");
    
    AddSystemStatus(TEXT("Character System"), Status, ActorCount, ErrorMsg);
    
    if (bClassLoaded)
    {
        IntegrationData.LoadedClasses++;
    }
    else
    {
        IntegrationData.FailedClasses++;
    }
}

void UBuildIntegrationReport::ValidateWorldGeneration()
{
    bool bClassLoaded = ValidateClassLoading(TEXT("PCGWorldGenerator"));
    int32 ActorCount = CountActorsOfType(TEXT("PCGWorldGenerator"));
    
    EBuild_IntegrationStatus Status = bClassLoaded ? EBuild_IntegrationStatus::Operational : EBuild_IntegrationStatus::Failed;
    FString ErrorMsg = bClassLoaded ? TEXT("") : TEXT("PCGWorldGenerator class not loaded");
    
    AddSystemStatus(TEXT("World Generation"), Status, ActorCount, ErrorMsg);
    
    if (bClassLoaded)
    {
        IntegrationData.LoadedClasses++;
    }
    else
    {
        IntegrationData.FailedClasses++;
    }
}

void UBuildIntegrationReport::ValidateDinosaurAI()
{
    bool bTRexLoaded = ValidateClassLoading(TEXT("DinosaurTRex"));
    bool bAILoaded = ValidateClassLoading(TEXT("DinosaurCombatAIController"));
    
    int32 DinosaurCount = CountActorsOfType(TEXT("DinosaurTRex"));
    DinosaurCount += CountActorsOfType(TEXT("DinosaurRaptor"));
    DinosaurCount += CountActorsOfType(TEXT("DinosaurBrachiosaurus"));
    
    EBuild_IntegrationStatus Status = (bTRexLoaded && bAILoaded) ? EBuild_IntegrationStatus::Operational : EBuild_IntegrationStatus::Failed;
    FString ErrorMsg = TEXT("");
    
    if (!bTRexLoaded)
    {
        ErrorMsg += TEXT("DinosaurTRex not loaded; ");
    }
    if (!bAILoaded)
    {
        ErrorMsg += TEXT("DinosaurCombatAIController not loaded; ");
    }
    
    AddSystemStatus(TEXT("Dinosaur AI"), Status, DinosaurCount, ErrorMsg);
    
    if (bTRexLoaded) IntegrationData.LoadedClasses++;
    else IntegrationData.FailedClasses++;
    
    if (bAILoaded) IntegrationData.LoadedClasses++;
    else IntegrationData.FailedClasses++;
}

void UBuildIntegrationReport::ValidateVFXSystem()
{
    int32 VFXCount = CountActorsOfType(TEXT("VFX"));
    VFXCount += CountActorsOfType(TEXT("Effect"));
    VFXCount += CountActorsOfType(TEXT("Particle"));
    
    // VFX system is optional, so we mark as operational if no critical errors
    EBuild_IntegrationStatus Status = EBuild_IntegrationStatus::Operational;
    
    AddSystemStatus(TEXT("VFX System"), Status, VFXCount, TEXT(""));
}

void UBuildIntegrationReport::ValidateFoliageSystem()
{
    bool bClassLoaded = ValidateClassLoading(TEXT("FoliageManager"));
    int32 ActorCount = CountActorsOfType(TEXT("FoliageManager"));
    
    EBuild_IntegrationStatus Status = bClassLoaded ? EBuild_IntegrationStatus::Operational : EBuild_IntegrationStatus::Failed;
    FString ErrorMsg = bClassLoaded ? TEXT("") : TEXT("FoliageManager class not loaded");
    
    AddSystemStatus(TEXT("Foliage System"), Status, ActorCount, ErrorMsg);
    
    if (bClassLoaded)
    {
        IntegrationData.LoadedClasses++;
    }
    else
    {
        IntegrationData.FailedClasses++;
    }
}

void UBuildIntegrationReport::ValidateCrowdSimulation()
{
    bool bClassLoaded = ValidateClassLoading(TEXT("CrowdSimulationManager"));
    int32 ActorCount = CountActorsOfType(TEXT("CrowdSimulationManager"));
    
    EBuild_IntegrationStatus Status = bClassLoaded ? EBuild_IntegrationStatus::Operational : EBuild_IntegrationStatus::Failed;
    FString ErrorMsg = bClassLoaded ? TEXT("") : TEXT("CrowdSimulationManager class not loaded");
    
    AddSystemStatus(TEXT("Crowd Simulation"), Status, ActorCount, ErrorMsg);
    
    if (bClassLoaded)
    {
        IntegrationData.LoadedClasses++;
    }
    else
    {
        IntegrationData.FailedClasses++;
    }
}

void UBuildIntegrationReport::ValidateAudioSystem()
{
    int32 AudioCount = CountActorsOfType(TEXT("Audio"));
    AudioCount += CountActorsOfType(TEXT("Sound"));
    AudioCount += CountActorsOfType(TEXT("Music"));
    
    // Audio system validation - check for basic audio actors
    EBuild_IntegrationStatus Status = EBuild_IntegrationStatus::Operational;
    
    AddSystemStatus(TEXT("Audio System"), Status, AudioCount, TEXT(""));
}

void UBuildIntegrationReport::UpdateOverallStatus()
{
    int32 OperationalCount = 0;
    int32 WarningCount = 0;
    int32 CriticalCount = 0;
    int32 FailedCount = 0;
    
    for (const FBuild_SystemStatus& Status : IntegrationData.SystemStatuses)
    {
        switch (Status.Status)
        {
            case EBuild_IntegrationStatus::Operational:
                OperationalCount++;
                break;
            case EBuild_IntegrationStatus::Warning:
                WarningCount++;
                break;
            case EBuild_IntegrationStatus::Critical:
                CriticalCount++;
                break;
            case EBuild_IntegrationStatus::Failed:
                FailedCount++;
                break;
        }
    }
    
    // Determine overall status
    if (FailedCount > 0 || CriticalCount > 2)
    {
        IntegrationData.OverallStatus = EBuild_IntegrationStatus::Failed;
    }
    else if (CriticalCount > 0 || WarningCount > 3)
    {
        IntegrationData.OverallStatus = EBuild_IntegrationStatus::Critical;
    }
    else if (WarningCount > 0)
    {
        IntegrationData.OverallStatus = EBuild_IntegrationStatus::Warning;
    }
    else
    {
        IntegrationData.OverallStatus = EBuild_IntegrationStatus::Operational;
    }
}

bool UBuildIntegrationReport::ValidateClassLoading(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *FullClassName);
    
    return LoadedClass != nullptr;
}

int32 UBuildIntegrationReport::CountActorsOfType(const FString& ActorType)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(ActorType))
        {
            Count++;
        }
    }
    
    IntegrationData.TotalActors += Count;
    return Count;
}