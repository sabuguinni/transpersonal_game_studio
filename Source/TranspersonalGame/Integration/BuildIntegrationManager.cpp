#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    LastValidationTime = 0.0f;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing integration system"));
    
    // Initialize build status
    CurrentBuildStatus = FInteg_BuildStatus();
    CurrentBuildStatus.bCompilationSuccessful = true;
    
    // Register core systems
    RegisterSystem(TEXT("Core"), this);
    RegisterSystem(TEXT("Integration"), this);
    
    // Perform initial validation
    ValidateAllSystems();
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Shutting down integration system"));
    
    RegisteredSystems.Empty();
    BuildSnapshots.Empty();
    
    Super::Deinitialize();
}

FInteg_BuildStatus UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting system validation"));
    
    CurrentBuildStatus = FInteg_BuildStatus();
    CurrentBuildStatus.SystemReports.Empty();
    
    // List of systems to validate
    TArray<TPair<FString, FString>> SystemsToValidate = {
        {TEXT("Core"), TEXT("/Script/TranspersonalGame.TranspersonalCharacter")},
        {TEXT("GameMode"), TEXT("/Script/TranspersonalGame.TranspersonalGameMode")},
        {TEXT("GameState"), TEXT("/Script/TranspersonalGame.TranspersonalGameState")},
        {TEXT("VFX"), TEXT("/Script/TranspersonalGame.VFXManager")},
        {TEXT("Audio"), TEXT("/Script/TranspersonalGame.AudioManager")},
        {TEXT("QA"), TEXT("/Script/TranspersonalGame.QATestManager")},
        {TEXT("Integration"), TEXT("/Script/TranspersonalGame.BuildIntegrationManager")}
    };
    
    for (const auto& SystemPair : SystemsToValidate)
    {
        FInteg_SystemReport Report;
        ValidateSystemClass(SystemPair.Key, SystemPair.Value, Report);
        CurrentBuildStatus.SystemReports.Add(Report);
        
        if (Report.Status == EInteg_SystemStatus::Functional)
        {
            CurrentBuildStatus.FunctionalSystems++;
        }
        else
        {
            CurrentBuildStatus.BrokenSystems++;
            CurrentBuildStatus.bCompilationSuccessful = false;
        }
    }
    
    CurrentBuildStatus.TotalSystems = SystemsToValidate.Num();
    CurrentBuildStatus.BuildTime = FPlatformTime::Seconds();
    LastValidationTime = CurrentBuildStatus.BuildTime;
    
    // Test cross-system dependencies
    TestCrossSystemDependencies();
    
    // Log results
    LogIntegrationResults(CurrentBuildStatus);
    
    return CurrentBuildStatus;
}

bool UBuildIntegrationManager::TestSystemIntegration(const FString& SystemName)
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Testing system integration for %s"), *SystemName);
    
    if (RegisteredSystems.Contains(SystemName))
    {
        TWeakObjectPtr<UObject> SystemPtr = RegisteredSystems[SystemName];
        if (SystemPtr.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("System %s is registered and valid"), *SystemName);
            return true;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("System %s integration test failed"), *SystemName);
    return false;
}

void UBuildIntegrationManager::RegisterSystem(const FString& SystemName, UObject* SystemObject)
{
    if (SystemObject && IsValid(SystemObject))
    {
        RegisteredSystems.Add(SystemName, SystemObject);
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Registered system %s"), *SystemName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Failed to register system %s - invalid object"), *SystemName);
    }
}

void UBuildIntegrationManager::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems.Remove(SystemName);
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Unregistered system %s"), *SystemName);
    }
}

bool UBuildIntegrationManager::TestVFXEnvironmentIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Testing VFX-Environment integration"));
    
    // Test if VFX system can interact with environment actors
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count environment actors that could have VFX
    int32 EnvironmentActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("Tree")))
        {
            EnvironmentActors++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d environment actors for VFX integration"), EnvironmentActors);
    return EnvironmentActors > 0;
}

bool UBuildIntegrationManager::TestAudioCharacterIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Testing Audio-Character integration"));
    
    // Test if audio system can interact with character actors
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Look for character actors
    int32 CharacterActors = 0;
    for (TActorIterator<APawn> PawnItr(World); PawnItr; ++PawnItr)
    {
        APawn* Pawn = *PawnItr;
        if (Pawn)
        {
            CharacterActors++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d character actors for audio integration"), CharacterActors);
    return CharacterActors > 0;
}

bool UBuildIntegrationManager::TestAICombatIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Testing AI-Combat integration"));
    
    // Test if AI and Combat systems can work together
    // This would test dinosaur AI with combat behaviors
    
    return TestSystemIntegration(TEXT("AI")) && TestSystemIntegration(TEXT("Combat"));
}

void UBuildIntegrationManager::CreateBuildSnapshot()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Creating build snapshot"));
    
    FInteg_BuildStatus Snapshot = CurrentBuildStatus;
    Snapshot.BuildTime = FPlatformTime::Seconds();
    
    BuildSnapshots.Add(Snapshot);
    
    // Keep only last 10 snapshots
    if (BuildSnapshots.Num() > 10)
    {
        BuildSnapshots.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build snapshot created. Total snapshots: %d"), BuildSnapshots.Num());
}

bool UBuildIntegrationManager::RestoreBuildSnapshot(int32 SnapshotIndex)
{
    if (BuildSnapshots.IsValidIndex(SnapshotIndex))
    {
        CurrentBuildStatus = BuildSnapshots[SnapshotIndex];
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Restored build snapshot %d"), SnapshotIndex);
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Invalid snapshot index %d"), SnapshotIndex);
    return false;
}

TArray<FString> UBuildIntegrationManager::GetRegisteredSystems() const
{
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    return SystemNames;
}

void UBuildIntegrationManager::ValidateSystemClass(const FString& SystemName, const FString& ClassPath, FInteg_SystemReport& OutReport)
{
    OutReport.SystemName = SystemName;
    OutReport.LastTestTime = FPlatformTime::Seconds();
    
    // Try to load the class
    UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (SystemClass)
    {
        OutReport.Status = EInteg_SystemStatus::Functional;
        OutReport.ErrorMessage = TEXT("System loaded successfully");
        UE_LOG(LogTemp, Warning, TEXT("System %s: FUNCTIONAL"), *SystemName);
    }
    else
    {
        OutReport.Status = EInteg_SystemStatus::Missing;
        OutReport.ErrorMessage = FString::Printf(TEXT("Class not found: %s"), *ClassPath);
        UE_LOG(LogTemp, Error, TEXT("System %s: MISSING - %s"), *SystemName, *ClassPath);
    }
}

void UBuildIntegrationManager::TestCrossSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Testing cross-system dependencies"));
    
    // Test VFX-Environment integration
    bool VFXEnvIntegration = TestVFXEnvironmentIntegration();
    
    // Test Audio-Character integration
    bool AudioCharIntegration = TestAudioCharacterIntegration();
    
    // Test AI-Combat integration
    bool AICombatIntegration = TestAICombatIntegration();
    
    UE_LOG(LogTemp, Warning, TEXT("Cross-system integration results:"));
    UE_LOG(LogTemp, Warning, TEXT("  VFX-Environment: %s"), VFXEnvIntegration ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("  Audio-Character: %s"), AudioCharIntegration ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("  AI-Combat: %s"), AICombatIntegration ? TEXT("PASS") : TEXT("FAIL"));
}

void UBuildIntegrationManager::LogIntegrationResults(const FInteg_BuildStatus& Status)
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION RESULTS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Compilation Successful: %s"), Status.bCompilationSuccessful ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Total Systems: %d"), Status.TotalSystems);
    UE_LOG(LogTemp, Warning, TEXT("Functional Systems: %d"), Status.FunctionalSystems);
    UE_LOG(LogTemp, Warning, TEXT("Broken Systems: %d"), Status.BrokenSystems);
    UE_LOG(LogTemp, Warning, TEXT("Build Time: %.2f seconds"), Status.BuildTime);
    
    for (const FInteg_SystemReport& Report : Status.SystemReports)
    {
        FString StatusStr;
        switch (Report.Status)
        {
            case EInteg_SystemStatus::Functional: StatusStr = TEXT("FUNCTIONAL"); break;
            case EInteg_SystemStatus::Broken: StatusStr = TEXT("BROKEN"); break;
            case EInteg_SystemStatus::Missing: StatusStr = TEXT("MISSING"); break;
            default: StatusStr = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s - %s"), *Report.SystemName, *StatusStr, *Report.ErrorMessage);
    }
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION RESULTS ==="));
}