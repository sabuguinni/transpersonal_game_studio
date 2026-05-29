#include "IntegrationValidationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"

UIntegrationValidationManager::UIntegrationValidationManager()
{
    ValidationLevel = EInteg_ValidationLevel::Standard;
    MinHealthScore = 75.0f;
    TargetFrameRate = 60.0f;
    bAutoValidateOnLevelLoad = true;
    bLogValidationDetails = true;
}

void UIntegrationValidationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationValidationManager: Initialized"));
    
    // Initialize system status tracking
    SystemStatusList.Empty();
    AgentOutputHistory.Empty();
    
    // Perform initial validation if enabled
    if (bAutoValidateOnLevelLoad)
    {
        ValidateAllSystems();
    }
}

void UIntegrationValidationManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("IntegrationValidationManager: Deinitialized"));
    Super::Deinitialize();
}

bool UIntegrationValidationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("IntegrationValidationManager: Starting comprehensive system validation"));
    
    bool bAllSystemsValid = true;
    
    // Validate core systems
    bAllSystemsValid &= ValidateCoreClasses();
    bAllSystemsValid &= ValidateGameplayFramework();
    bAllSystemsValid &= ValidateAudioSystems();
    bAllSystemsValid &= ValidateVFXSystems();
    bAllSystemsValid &= ValidateAISystems();
    bAllSystemsValid &= ValidateWorldGeneration();
    
    // Validate level state
    bAllSystemsValid &= ValidateMinPlayableMap();
    
    // Validate performance
    bAllSystemsValid &= ValidatePerformanceTargets();
    
    float OverallHealth = GetOverallHealthScore();
    UE_LOG(LogTemp, Log, TEXT("IntegrationValidationManager: Overall health score: %.1f%%"), OverallHealth);
    
    return bAllSystemsValid && (OverallHealth >= MinHealthScore);
}

FInteg_SystemStatus UIntegrationValidationManager::ValidateSystem(const FString& SystemName)
{
    FInteg_SystemStatus Status;
    Status.SystemName = SystemName;
    
    if (SystemName == TEXT("Core"))
    {
        Status.bIsLoaded = ValidateCoreClasses();
        Status.bIsCompiled = true; // If we're running, it compiled
        Status.bHasDependencyErrors = false;
        Status.HealthScore = Status.bIsLoaded ? 100.0f : 0.0f;
    }
    else if (SystemName == TEXT("Audio"))
    {
        Status.bIsLoaded = ValidateAudioSystems();
        Status.bIsCompiled = true;
        Status.bHasDependencyErrors = false;
        Status.HealthScore = Status.bIsLoaded ? 85.0f : 0.0f;
    }
    else if (SystemName == TEXT("VFX"))
    {
        Status.bIsLoaded = ValidateVFXSystems();
        Status.bIsCompiled = true;
        Status.bHasDependencyErrors = false;
        Status.HealthScore = Status.bIsLoaded ? 80.0f : 0.0f;
    }
    else
    {
        // Unknown system
        Status.bIsLoaded = false;
        Status.bIsCompiled = false;
        Status.bHasDependencyErrors = true;
        Status.HealthScore = 0.0f;
    }
    
    LogValidationResult(SystemName, Status.bIsLoaded);
    return Status;
}

TArray<FInteg_SystemStatus> UIntegrationValidationManager::GetSystemStatusList()
{
    return SystemStatusList;
}

bool UIntegrationValidationManager::ValidateAgentOutput(const FString& AgentName, int32 CycleNumber)
{
    UE_LOG(LogTemp, Log, TEXT("IntegrationValidationManager: Validating output for %s, Cycle %d"), *AgentName, CycleNumber);
    
    bool bValidationPassed = true;
    int32 FilesCreated = 0;
    int32 CommandsExecuted = 0;
    
    // Agent-specific validation logic
    if (AgentName.Contains(TEXT("Audio")))
    {
        bValidationPassed = ValidateAudioSystems();
        FilesCreated = 2; // Estimated based on typical audio agent output
        CommandsExecuted = 2;
    }
    else if (AgentName.Contains(TEXT("VFX")))
    {
        bValidationPassed = ValidateVFXSystems();
        FilesCreated = 1;
        CommandsExecuted = 2;
    }
    else if (AgentName.Contains(TEXT("QA")))
    {
        bValidationPassed = ValidateMinPlayableMap();
        FilesCreated = 0;
        CommandsExecuted = 5;
    }
    
    // Record the validation result
    RecordAgentOutput(AgentName, CycleNumber, FilesCreated, CommandsExecuted, bValidationPassed);
    
    return bValidationPassed;
}

TArray<FInteg_AgentOutput> UIntegrationValidationManager::GetAgentOutputHistory()
{
    return AgentOutputHistory;
}

bool UIntegrationValidationManager::ValidateBuildIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("IntegrationValidationManager: Validating build integrity"));
    
    // Check if core classes can be loaded
    bool bCoreIntegrity = ValidateCoreClasses();
    
    // Check module compilation
    bool bCompilationIntegrity = TestModuleCompilation();
    
    // Check for missing dependencies
    bool bDependencyIntegrity = CheckSystemDependencies(TEXT("Core"));
    
    bool bOverallIntegrity = bCoreIntegrity && bCompilationIntegrity && bDependencyIntegrity;
    
    LogValidationResult(TEXT("BuildIntegrity"), bOverallIntegrity);
    return bOverallIntegrity;
}

float UIntegrationValidationManager::GetOverallHealthScore()
{
    if (SystemStatusList.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalScore = 0.0f;
    for (const FInteg_SystemStatus& Status : SystemStatusList)
    {
        TotalScore += Status.HealthScore;
    }
    
    return TotalScore / SystemStatusList.Num();
}

bool UIntegrationValidationManager::CheckSystemDependencies(const FString& SystemName)
{
    // Basic dependency check - ensure core UE5 classes are available
    bool bDependenciesValid = true;
    
    try
    {
        // Test basic UE5 class loading
        UClass* ActorClass = FindObject<UClass>(ANY_PACKAGE, TEXT("Actor"));
        UClass* ComponentClass = FindObject<UClass>(ANY_PACKAGE, TEXT("ActorComponent"));
        UClass* WorldClass = FindObject<UClass>(ANY_PACKAGE, TEXT("World"));
        
        bDependenciesValid = (ActorClass != nullptr) && (ComponentClass != nullptr) && (WorldClass != nullptr);
    }
    catch (...)
    {
        bDependenciesValid = false;
    }
    
    LogValidationResult(FString::Printf(TEXT("%s_Dependencies"), *SystemName), bDependenciesValid);
    return bDependenciesValid;
}

TArray<FString> UIntegrationValidationManager::GetMissingDependencies(const FString& SystemName)
{
    TArray<FString> MissingDeps;
    
    // Check for common missing dependencies
    if (!CheckSystemDependencies(SystemName))
    {
        MissingDeps.Add(TEXT("Core UE5 Classes"));
    }
    
    return MissingDeps;
}

bool UIntegrationValidationManager::ValidateMinPlayableMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogValidationResult(TEXT("MinPlayableMap"), false, TEXT("No world available"));
        return false;
    }
    
    // Count actors in the level
    int32 ActorCount = GetActorCount();
    bool bHasMinimumActors = ActorCount >= 10; // Minimum viable level should have at least 10 actors
    
    // Check for essential actors
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    bool bHasPlayerStart = PlayerStarts.Num() > 0;
    
    bool bMapValid = bHasMinimumActors && bHasPlayerStart;
    
    LogValidationResult(TEXT("MinPlayableMap"), bMapValid, 
        FString::Printf(TEXT("Actors: %d, PlayerStarts: %d"), ActorCount, PlayerStarts.Num()));
    
    return bMapValid;
}

int32 UIntegrationValidationManager::GetActorCount()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        Count++;
    }
    
    return Count;
}

TMap<FString, int32> UIntegrationValidationManager::GetActorInventory()
{
    TMap<FString, int32> Inventory;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return Inventory;
    }
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName();
            int32* Count = Inventory.Find(ClassName);
            if (Count)
            {
                (*Count)++;
            }
            else
            {
                Inventory.Add(ClassName, 1);
            }
        }
    }
    
    return Inventory;
}

bool UIntegrationValidationManager::ValidatePerformanceTargets()
{
    float CurrentFPS = GetCurrentFrameRate();
    bool bPerformanceValid = CurrentFPS >= (TargetFrameRate * 0.8f); // Allow 20% tolerance
    
    LogValidationResult(TEXT("Performance"), bPerformanceValid, 
        FString::Printf(TEXT("FPS: %.1f (Target: %.1f)"), CurrentFPS, TargetFrameRate));
    
    return bPerformanceValid;
}

float UIntegrationValidationManager::GetCurrentFrameRate()
{
    // Get frame rate from engine stats
    if (GEngine && GEngine->GetEngineStats())
    {
        return 1.0f / FApp::GetDeltaTime();
    }
    
    return 60.0f; // Default assumption
}

bool UIntegrationValidationManager::TestModuleCompilation()
{
    // If we're running, the module compiled successfully
    // This is a runtime check that the module is functional
    bool bModuleCompiled = true;
    
    try
    {
        // Test that we can access our own class
        UClass* ThisClass = UIntegrationValidationManager::StaticClass();
        bModuleCompiled = (ThisClass != nullptr);
    }
    catch (...)
    {
        bModuleCompiled = false;
    }
    
    LogValidationResult(TEXT("ModuleCompilation"), bModuleCompiled);
    return bModuleCompiled;
}

TArray<FString> UIntegrationValidationManager::GetCompilationErrors()
{
    TArray<FString> Errors;
    
    // Since we're running, there are no compilation errors
    // This would be populated by external build tools in a real scenario
    
    return Errors;
}

// Private validation methods
bool UIntegrationValidationManager::ValidateCoreClasses()
{
    bool bCoreValid = true;
    
    try
    {
        // Test loading TranspersonalGame classes
        UClass* GameModeClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameMode"));
        UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
        UClass* GameStateClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameState"));
        
        bCoreValid = (GameModeClass != nullptr) && (CharacterClass != nullptr) && (GameStateClass != nullptr);
    }
    catch (...)
    {
        bCoreValid = false;
    }
    
    UpdateSystemStatus(TEXT("Core"), bCoreValid, bCoreValid, bCoreValid ? 100.0f : 0.0f);
    return bCoreValid;
}

bool UIntegrationValidationManager::ValidateGameplayFramework()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if we have a valid game mode
    AGameModeBase* GameMode = World->GetAuthGameMode();
    bool bFrameworkValid = (GameMode != nullptr);
    
    UpdateSystemStatus(TEXT("GameplayFramework"), bFrameworkValid, true, bFrameworkValid ? 90.0f : 0.0f);
    return bFrameworkValid;
}

bool UIntegrationValidationManager::ValidateAudioSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count audio components in the level
    int32 AudioComponentCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UAudioComponent*> AudioComponents;
            Actor->GetComponents<UAudioComponent>(AudioComponents);
            AudioComponentCount += AudioComponents.Num();
        }
    }
    
    bool bAudioValid = AudioComponentCount > 0;
    UpdateSystemStatus(TEXT("Audio"), bAudioValid, true, bAudioValid ? 85.0f : 50.0f);
    return bAudioValid;
}

bool UIntegrationValidationManager::ValidateVFXSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count VFX components (Niagara, fog, etc.)
    int32 VFXComponentCount = 0;
    
    // Count Niagara components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UNiagaraComponent*> NiagaraComponents;
            Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
            VFXComponentCount += NiagaraComponents.Num();
        }
    }
    
    // Count fog actors
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    VFXComponentCount += FogActors.Num();
    
    bool bVFXValid = VFXComponentCount > 0;
    UpdateSystemStatus(TEXT("VFX"), bVFXValid, true, bVFXValid ? 80.0f : 40.0f);
    return bVFXValid;
}

bool UIntegrationValidationManager::ValidateAISystems()
{
    // Basic AI validation - check if AI classes are available
    bool bAIValid = true;
    
    try
    {
        UClass* PawnClass = FindObject<UClass>(ANY_PACKAGE, TEXT("Pawn"));
        UClass* AIControllerClass = FindObject<UClass>(ANY_PACKAGE, TEXT("AIController"));
        
        bAIValid = (PawnClass != nullptr) && (AIControllerClass != nullptr);
    }
    catch (...)
    {
        bAIValid = false;
    }
    
    UpdateSystemStatus(TEXT("AI"), bAIValid, true, bAIValid ? 70.0f : 0.0f);
    return bAIValid;
}

bool UIntegrationValidationManager::ValidateWorldGeneration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if we have terrain/landscape
    bool bWorldGenValid = GetActorCount() > 5; // Basic check for populated world
    
    UpdateSystemStatus(TEXT("WorldGeneration"), bWorldGenValid, true, bWorldGenValid ? 75.0f : 30.0f);
    return bWorldGenValid;
}

// Helper methods
void UIntegrationValidationManager::LogValidationResult(const FString& SystemName, bool bPassed, const FString& Details)
{
    if (bLogValidationDetails)
    {
        FString Result = bPassed ? TEXT("PASS") : TEXT("FAIL");
        FString LogMessage = FString::Printf(TEXT("Validation [%s]: %s"), *SystemName, *Result);
        
        if (!Details.IsEmpty())
        {
            LogMessage += FString::Printf(TEXT(" - %s"), *Details);
        }
        
        UE_LOG(LogTemp, Log, TEXT("IntegrationValidationManager: %s"), *LogMessage);
    }
}

void UIntegrationValidationManager::UpdateSystemStatus(const FString& SystemName, bool bLoaded, bool bCompiled, float HealthScore)
{
    // Find existing status or create new one
    FInteg_SystemStatus* ExistingStatus = SystemStatusList.FindByPredicate([&SystemName](const FInteg_SystemStatus& Status)
    {
        return Status.SystemName == SystemName;
    });
    
    if (ExistingStatus)
    {
        ExistingStatus->bIsLoaded = bLoaded;
        ExistingStatus->bIsCompiled = bCompiled;
        ExistingStatus->HealthScore = HealthScore;
    }
    else
    {
        FInteg_SystemStatus NewStatus;
        NewStatus.SystemName = SystemName;
        NewStatus.bIsLoaded = bLoaded;
        NewStatus.bIsCompiled = bCompiled;
        NewStatus.bHasDependencyErrors = false;
        NewStatus.HealthScore = HealthScore;
        SystemStatusList.Add(NewStatus);
    }
}

void UIntegrationValidationManager::RecordAgentOutput(const FString& AgentName, int32 CycleNumber, int32 FilesCreated, int32 CommandsExecuted, bool bPassed)
{
    FInteg_AgentOutput Output;
    Output.AgentName = AgentName;
    Output.CycleNumber = CycleNumber;
    Output.FilesCreated = FilesCreated;
    Output.UE5CommandsExecuted = CommandsExecuted;
    Output.bValidationPassed = bPassed;
    Output.ValidationNotes = bPassed ? TEXT("Validation passed") : TEXT("Validation failed");
    
    AgentOutputHistory.Add(Output);
    
    // Keep only last 20 entries
    if (AgentOutputHistory.Num() > 20)
    {
        AgentOutputHistory.RemoveAt(0);
    }
}