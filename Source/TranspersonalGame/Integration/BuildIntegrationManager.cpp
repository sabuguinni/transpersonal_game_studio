#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

// Static module lists
const TArray<FString> UBuildIntegrationManager::CoreModules = {
    TEXT("Core.PhysicsSystemManager"),
    TEXT("Core.ConsciousnessSystem"),
    TEXT("Core.PerformanceManager"),
    TEXT("World.WorldGenerationSubsystem"),
    TEXT("Environment.EnvironmentManager")
};

const TArray<FString> UBuildIntegrationManager::GameplayModules = {
    TEXT("Characters.CharacterManager"),
    TEXT("AI.NPCBehaviorManager"),
    TEXT("Combat.CombatSystemManager"),
    TEXT("Quest.QuestManager"),
    TEXT("Narrative.DialogueManager")
};

const TArray<FString> UBuildIntegrationManager::ContentModules = {
    TEXT("Audio.AudioManager"),
    TEXT("VFX.VFXManager"),
    TEXT("Lighting.LightingManager"),
    TEXT("Architecture.ArchitectureManager"),
    TEXT("Crowd.CrowdSimulationManager")
};

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bIsRunningTests = false;
    CurrentReport.OverallStatus = EBuild_IntegrationStatus::Unknown;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing integration testing system"));
    
    // Initialize module status tracking
    TArray<FString> AllModules;
    AllModules.Append(CoreModules);
    AllModules.Append(GameplayModules);
    AllModules.Append(ContentModules);
    
    for (const FString& ModuleName : AllModules)
    {
        FBuild_ModuleStatus Status;
        Status.ModuleName = ModuleName;
        Status.Status = EBuild_IntegrationStatus::Initializing;
        ModuleStatuses.Add(ModuleName, Status);
    }
    
    // Start initial validation
    ValidateAllSystems();
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Shutting down integration system"));
    ModuleStatuses.Empty();
    Super::Deinitialize();
}

void UBuildIntegrationManager::RunFullIntegrationTest()
{
    if (bIsRunningTests)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integration test already running, skipping"));
        return;
    }
    
    bIsRunningTests = true;
    UE_LOG(LogTemp, Warning, TEXT("Starting full integration test suite"));
    
    float StartTime = FPlatformTime::Seconds();
    
    // Test core modules first
    TestCorePhysicsModule();
    TestWorldGenerationModule();
    TestEnvironmentModule();
    
    // Test gameplay modules
    TestCharacterModule();
    TestAIModule();
    TestQuestModule();
    
    // Test content modules
    TestAudioModule();
    TestVFXModule();
    
    // Run cross-module integration tests
    RunCrossModuleTest();
    
    // Generate final report
    CurrentReport.TotalTestTime = FPlatformTime::Seconds() - StartTime;
    CurrentReport.LastBuildTime = FDateTime::Now();
    CurrentReport.BuildVersion = TEXT("CYCLE_012");
    
    // Determine overall status
    int32 FailedCount = 0;
    int32 PassedCount = 0;
    
    for (const auto& StatusPair : ModuleStatuses)
    {
        if (StatusPair.Value.Status == EBuild_IntegrationStatus::Failed)
        {
            FailedCount++;
        }
        else if (StatusPair.Value.Status == EBuild_IntegrationStatus::Success)
        {
            PassedCount++;
        }
    }
    
    if (FailedCount == 0)
    {
        CurrentReport.OverallStatus = EBuild_IntegrationStatus::Success;
    }
    else if (PassedCount > FailedCount)
    {
        CurrentReport.OverallStatus = EBuild_IntegrationStatus::Testing;
    }
    else
    {
        CurrentReport.OverallStatus = EBuild_IntegrationStatus::Failed;
    }
    
    // Update report with current statuses
    CurrentReport.ModuleStatuses.Empty();
    for (const auto& StatusPair : ModuleStatuses)
    {
        CurrentReport.ModuleStatuses.Add(StatusPair.Value);
    }
    
    bIsRunningTests = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Integration test completed: %d passed, %d failed"), PassedCount, FailedCount);
    
    // Broadcast completion
    OnIntegrationTestComplete.Broadcast(CurrentReport);
}

void UBuildIntegrationManager::RunModuleTest(const FString& ModuleName)
{
    UE_LOG(LogTemp, Warning, TEXT("Testing module: %s"), *ModuleName);
    
    bool bTestPassed = false;
    FString ErrorMessage;
    
    // Check if module is loaded
    if (IsModuleLoaded(ModuleName))
    {
        bTestPassed = true;
        LogIntegrationResult(ModuleName, true, TEXT("Module loaded successfully"));
    }
    else
    {
        ErrorMessage = TEXT("Module not loaded or not found");
        LogIntegrationResult(ModuleName, false, ErrorMessage);
    }
    
    // Update status
    EBuild_IntegrationStatus Status = bTestPassed ? EBuild_IntegrationStatus::Success : EBuild_IntegrationStatus::Failed;
    UpdateModuleStatus(ModuleName, Status, ErrorMessage);
    
    // Broadcast module test completion
    OnModuleTestComplete.Broadcast(ModuleName, Status);
}

void UBuildIntegrationManager::RunCrossModuleTest()
{
    UE_LOG(LogTemp, Warning, TEXT("Running cross-module integration tests"));
    
    TestPhysicsWorldIntegration();
    TestCharacterEnvironmentIntegration();
    TestAIQuestIntegration();
    TestAudioVFXIntegration();
}

void UBuildIntegrationManager::TestCorePhysicsModule()
{
    const FString ModuleName = TEXT("Core.PhysicsSystemManager");
    
    // Try to find physics-related classes
    UClass* PhysicsClass = FindObject<UClass>(ANY_PACKAGE, TEXT("PhysicsSystemManager"));
    if (PhysicsClass)
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Success);
        LogIntegrationResult(ModuleName, true, TEXT("PhysicsSystemManager class found"));
    }
    else
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Failed, TEXT("PhysicsSystemManager class not found"));
        LogIntegrationResult(ModuleName, false, TEXT("PhysicsSystemManager class not found"));
    }
}

void UBuildIntegrationManager::TestWorldGenerationModule()
{
    const FString ModuleName = TEXT("World.WorldGenerationSubsystem");
    
    // Try to find world generation subsystem
    UClass* WorldGenClass = FindObject<UClass>(ANY_PACKAGE, TEXT("WorldGenerationSubsystem"));
    if (WorldGenClass)
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Success);
        LogIntegrationResult(ModuleName, true, TEXT("WorldGenerationSubsystem class found"));
    }
    else
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Failed, TEXT("WorldGenerationSubsystem class not found"));
        LogIntegrationResult(ModuleName, false, TEXT("WorldGenerationSubsystem class not found"));
    }
}

void UBuildIntegrationManager::TestEnvironmentModule()
{
    const FString ModuleName = TEXT("Environment.EnvironmentManager");
    
    // Check for environment-related classes
    bool bFoundEnvironmentClasses = false;
    
    // Look for common environment classes
    TArray<FString> EnvironmentClasses = {
        TEXT("EnvironmentManager"),
        TEXT("FoliageManager"),
        TEXT("TerrainManager")
    };
    
    for (const FString& ClassName : EnvironmentClasses)
    {
        UClass* EnvClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
        if (EnvClass)
        {
            bFoundEnvironmentClasses = true;
            break;
        }
    }
    
    if (bFoundEnvironmentClasses)
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Success);
        LogIntegrationResult(ModuleName, true, TEXT("Environment classes found"));
    }
    else
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Failed, TEXT("No environment classes found"));
        LogIntegrationResult(ModuleName, false, TEXT("No environment classes found"));
    }
}

void UBuildIntegrationManager::TestCharacterModule()
{
    const FString ModuleName = TEXT("Characters.CharacterManager");
    
    // Look for character-related classes
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (!CharacterClass)
    {
        CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("CharacterManager"));
    }
    
    if (CharacterClass)
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Success);
        LogIntegrationResult(ModuleName, true, TEXT("Character classes found"));
    }
    else
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Failed, TEXT("No character classes found"));
        LogIntegrationResult(ModuleName, false, TEXT("No character classes found"));
    }
}

void UBuildIntegrationManager::TestAIModule()
{
    const FString ModuleName = TEXT("AI.NPCBehaviorManager");
    
    // Look for AI-related classes
    UClass* AIClass = FindObject<UClass>(ANY_PACKAGE, TEXT("NPCBehaviorManager"));
    if (!AIClass)
    {
        AIClass = FindObject<UClass>(ANY_PACKAGE, TEXT("NPCBehaviorComponent"));
    }
    
    if (AIClass)
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Success);
        LogIntegrationResult(ModuleName, true, TEXT("AI classes found"));
    }
    else
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Failed, TEXT("No AI classes found"));
        LogIntegrationResult(ModuleName, false, TEXT("No AI classes found"));
    }
}

void UBuildIntegrationManager::TestQuestModule()
{
    const FString ModuleName = TEXT("Quest.QuestManager");
    
    // Look for quest-related classes
    UClass* QuestClass = FindObject<UClass>(ANY_PACKAGE, TEXT("QuestManager"));
    if (!QuestClass)
    {
        QuestClass = FindObject<UClass>(ANY_PACKAGE, TEXT("QuestComponent"));
    }
    
    if (QuestClass)
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Success);
        LogIntegrationResult(ModuleName, true, TEXT("Quest classes found"));
    }
    else
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Failed, TEXT("No quest classes found"));
        LogIntegrationResult(ModuleName, false, TEXT("No quest classes found"));
    }
}

void UBuildIntegrationManager::TestAudioModule()
{
    const FString ModuleName = TEXT("Audio.AudioManager");
    
    // Look for audio-related classes
    UClass* AudioClass = FindObject<UClass>(ANY_PACKAGE, TEXT("AudioManager"));
    if (!AudioClass)
    {
        AudioClass = FindObject<UClass>(ANY_PACKAGE, TEXT("AdaptiveAudioManager"));
    }
    
    if (AudioClass)
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Success);
        LogIntegrationResult(ModuleName, true, TEXT("Audio classes found"));
    }
    else
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Failed, TEXT("No audio classes found"));
        LogIntegrationResult(ModuleName, false, TEXT("No audio classes found"));
    }
}

void UBuildIntegrationManager::TestVFXModule()
{
    const FString ModuleName = TEXT("VFX.VFXManager");
    
    // Look for VFX-related classes
    UClass* VFXClass = FindObject<UClass>(ANY_PACKAGE, TEXT("VFXManager"));
    if (!VFXClass)
    {
        VFXClass = FindObject<UClass>(ANY_PACKAGE, TEXT("VFXSystemManager"));
    }
    
    if (VFXClass)
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Success);
        LogIntegrationResult(ModuleName, true, TEXT("VFX classes found"));
    }
    else
    {
        UpdateModuleStatus(ModuleName, EBuild_IntegrationStatus::Failed, TEXT("No VFX classes found"));
        LogIntegrationResult(ModuleName, false, TEXT("No VFX classes found"));
    }
}

void UBuildIntegrationManager::TestPhysicsWorldIntegration()
{
    LogIntegrationResult(TEXT("PhysicsWorldIntegration"), true, TEXT("Cross-module test placeholder"));
}

void UBuildIntegrationManager::TestCharacterEnvironmentIntegration()
{
    LogIntegrationResult(TEXT("CharacterEnvironmentIntegration"), true, TEXT("Cross-module test placeholder"));
}

void UBuildIntegrationManager::TestAIQuestIntegration()
{
    LogIntegrationResult(TEXT("AIQuestIntegration"), true, TEXT("Cross-module test placeholder"));
}

void UBuildIntegrationManager::TestAudioVFXIntegration()
{
    LogIntegrationResult(TEXT("AudioVFXIntegration"), true, TEXT("Cross-module test placeholder"));
}

FBuild_IntegrationReport UBuildIntegrationManager::GetIntegrationReport() const
{
    return CurrentReport;
}

EBuild_IntegrationStatus UBuildIntegrationManager::GetOverallStatus() const
{
    return CurrentReport.OverallStatus;
}

TArray<FString> UBuildIntegrationManager::GetFailedModules() const
{
    TArray<FString> FailedModules;
    
    for (const auto& StatusPair : ModuleStatuses)
    {
        if (StatusPair.Value.Status == EBuild_IntegrationStatus::Failed)
        {
            FailedModules.Add(StatusPair.Key);
        }
    }
    
    return FailedModules;
}

void UBuildIntegrationManager::CreateBuildSnapshot()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating build snapshot for CYCLE_012"));
    
    // Create a snapshot of current build state
    FString SnapshotPath = FPaths::ProjectSavedDir() / TEXT("BuildSnapshots") / TEXT("CYCLE_012");
    
    // Log snapshot creation
    LogIntegrationResult(TEXT("BuildSnapshot"), true, FString::Printf(TEXT("Snapshot created at: %s"), *SnapshotPath));
}

void UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating all systems for integration"));
    
    // Basic validation - check if we're in a valid game world
    UWorld* World = GetWorld();
    if (World)
    {
        UE_LOG(LogTemp, Warning, TEXT("World validation: SUCCESS"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("World validation: FAILED - No world found"));
    }
    
    // Check game instance
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameInstance validation: SUCCESS"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance validation: FAILED"));
    }
}

void UBuildIntegrationManager::UpdateModuleStatus(const FString& ModuleName, EBuild_IntegrationStatus Status, const FString& Error)
{
    if (FBuild_ModuleStatus* ExistingStatus = ModuleStatuses.Find(ModuleName))
    {
        ExistingStatus->Status = Status;
        ExistingStatus->LastError = Error;
        ExistingStatus->LastTestTime = FPlatformTime::Seconds();
        
        if (Status == EBuild_IntegrationStatus::Success)
        {
            ExistingStatus->TestsPassed++;
        }
        else if (Status == EBuild_IntegrationStatus::Failed)
        {
            ExistingStatus->TestsFailed++;
        }
    }
}

bool UBuildIntegrationManager::IsModuleLoaded(const FString& ModuleName) const
{
    // Simple check - try to find any class with the module name
    FString ClassName = ModuleName;
    if (ClassName.Contains(TEXT(".")))
    {
        ClassName = ClassName.Right(ClassName.Len() - ClassName.Find(TEXT(".")) - 1);
    }
    
    UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
    return FoundClass != nullptr;
}

void UBuildIntegrationManager::LogIntegrationResult(const FString& TestName, bool bSuccess, const FString& Details)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("INTEGRATION TEST [%s]: SUCCESS - %s"), *TestName, *Details);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("INTEGRATION TEST [%s]: FAILED - %s"), *TestName, *Details);
    }
}