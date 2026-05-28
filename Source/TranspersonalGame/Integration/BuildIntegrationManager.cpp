#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/Character/TranspersonalCharacter.h"
#include "TranspersonalGame/Core/TranspersonalGameState.h"
#include "TranspersonalGame/WorldGeneration/PCGWorldGenerator.h"
#include "TranspersonalGame/VFX/VFX_ImpactManager.h"
#include "TranspersonalGame/AI/DinosaurTRex.h"
#include "TranspersonalGame/AI/DinosaurCombatAIController.h"
#include "TranspersonalGame/Environment/FoliageManager.h"
#include "TranspersonalGame/AI/CrowdSimulationManager.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bAllSystemsReady = false;
    LastValidationTime = 0.0f;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing..."));
    
    // Register core systems
    RegisterSystem(TEXT("Character"), ATranspersonalCharacter::StaticClass());
    RegisterSystem(TEXT("GameState"), ATranspersonalGameState::StaticClass());
    RegisterSystem(TEXT("WorldGeneration"), APCGWorldGenerator::StaticClass());
    RegisterSystem(TEXT("VFX"), UVFX_ImpactManager::StaticClass());
    RegisterSystem(TEXT("DinosaurAI"), ADinosaurTRex::StaticClass());
    RegisterSystem(TEXT("CombatAI"), ADinosaurCombatAIController::StaticClass());
    RegisterSystem(TEXT("Foliage"), UFoliageManager::StaticClass());
    RegisterSystem(TEXT("CrowdSim"), UCrowdSimulationManager::StaticClass());
    
    // Initial validation
    ValidateAllSystems();
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Shutting down..."));
    SystemRegistry.Empty();
    Super::Deinitialize();
}

void UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating all systems..."));
    
    LastValidationTime = FPlatformTime::Seconds();
    
    ValidateCharacterSystem();
    ValidateWorldGeneration();
    ValidateVFXSystem();
    ValidateDinosaurAI();
    ValidateCrowdSimulation();
    ValidateFoliageSystem();
    
    // Check if all systems are ready
    bAllSystemsReady = true;
    for (const auto& SystemPair : SystemRegistry)
    {
        if (SystemPair.Value.Status != EBuild_SystemStatus::Ready)
        {
            bAllSystemsReady = false;
            break;
        }
    }
    
    LogSystemStatus();
}

void UBuildIntegrationManager::ValidateCharacterSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemStatus(TEXT("Character"), EBuild_SystemStatus::Error, TEXT("No world context"));
        return;
    }
    
    // Check for TranspersonalCharacter in world
    int32 CharacterCount = 0;
    for (TActorIterator<ATranspersonalCharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        CharacterCount++;
    }
    
    if (CharacterCount > 0)
    {
        UpdateSystemStatus(TEXT("Character"), EBuild_SystemStatus::Ready);
        SystemRegistry[TEXT("Character")].ActorCount = CharacterCount;
    }
    else
    {
        UpdateSystemStatus(TEXT("Character"), EBuild_SystemStatus::Loading, TEXT("No character actors found"));
    }
}

void UBuildIntegrationManager::ValidateWorldGeneration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemStatus(TEXT("WorldGeneration"), EBuild_SystemStatus::Error, TEXT("No world context"));
        return;
    }
    
    // Check for PCG World Generator
    int32 GeneratorCount = 0;
    for (TActorIterator<APCGWorldGenerator> ActorItr(World); ActorItr; ++ActorItr)
    {
        GeneratorCount++;
    }
    
    if (GeneratorCount > 0)
    {
        UpdateSystemStatus(TEXT("WorldGeneration"), EBuild_SystemStatus::Ready);
        SystemRegistry[TEXT("WorldGeneration")].ActorCount = GeneratorCount;
    }
    else
    {
        UpdateSystemStatus(TEXT("WorldGeneration"), EBuild_SystemStatus::Loading, TEXT("No world generators found"));
    }
}

void UBuildIntegrationManager::ValidateVFXSystem()
{
    // Check if VFX_ImpactManager class exists
    UClass* VFXClass = UVFX_ImpactManager::StaticClass();
    if (VFXClass)
    {
        UpdateSystemStatus(TEXT("VFX"), EBuild_SystemStatus::Ready);
    }
    else
    {
        UpdateSystemStatus(TEXT("VFX"), EBuild_SystemStatus::Error, TEXT("VFX_ImpactManager class not found"));
    }
}

void UBuildIntegrationManager::ValidateDinosaurAI()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UpdateSystemStatus(TEXT("DinosaurAI"), EBuild_SystemStatus::Error, TEXT("No world context"));
        return;
    }
    
    // Check for dinosaur actors
    int32 DinosaurCount = 0;
    for (TActorIterator<ADinosaurTRex> ActorItr(World); ActorItr; ++ActorItr)
    {
        DinosaurCount++;
    }
    
    if (DinosaurCount > 0)
    {
        UpdateSystemStatus(TEXT("DinosaurAI"), EBuild_SystemStatus::Ready);
        SystemRegistry[TEXT("DinosaurAI")].ActorCount = DinosaurCount;
    }
    else
    {
        UpdateSystemStatus(TEXT("DinosaurAI"), EBuild_SystemStatus::Loading, TEXT("No dinosaur actors found"));
    }
}

void UBuildIntegrationManager::ValidateCrowdSimulation()
{
    // Check if CrowdSimulationManager class exists
    UClass* CrowdClass = UCrowdSimulationManager::StaticClass();
    if (CrowdClass)
    {
        UpdateSystemStatus(TEXT("CrowdSim"), EBuild_SystemStatus::Ready);
    }
    else
    {
        UpdateSystemStatus(TEXT("CrowdSim"), EBuild_SystemStatus::Error, TEXT("CrowdSimulationManager class not found"));
    }
}

void UBuildIntegrationManager::ValidateFoliageSystem()
{
    // Check if FoliageManager class exists
    UClass* FoliageClass = UFoliageManager::StaticClass();
    if (FoliageClass)
    {
        UpdateSystemStatus(TEXT("Foliage"), EBuild_SystemStatus::Ready);
    }
    else
    {
        UpdateSystemStatus(TEXT("Foliage"), EBuild_SystemStatus::Error, TEXT("FoliageManager class not found"));
    }
}

void UBuildIntegrationManager::RegisterSystem(const FString& SystemName, UClass* SystemClass)
{
    FBuild_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Status = SystemClass ? EBuild_SystemStatus::Loading : EBuild_SystemStatus::Error;
    NewSystem.ErrorMessage = SystemClass ? TEXT("") : TEXT("Class not found");
    NewSystem.ActorCount = 0;
    NewSystem.LastUpdateTime = FPlatformTime::Seconds();
    
    SystemRegistry.Add(SystemName, NewSystem);
}

void UBuildIntegrationManager::UpdateSystemStatus(const FString& SystemName, EBuild_SystemStatus NewStatus, const FString& ErrorMsg)
{
    if (SystemRegistry.Contains(SystemName))
    {
        SystemRegistry[SystemName].Status = NewStatus;
        SystemRegistry[SystemName].ErrorMessage = ErrorMsg;
        SystemRegistry[SystemName].LastUpdateTime = FPlatformTime::Seconds();
    }
}

bool UBuildIntegrationManager::IsSystemReady(const FString& SystemName) const
{
    if (SystemRegistry.Contains(SystemName))
    {
        return SystemRegistry[SystemName].Status == EBuild_SystemStatus::Ready;
    }
    return false;
}

TArray<FBuild_SystemInfo> UBuildIntegrationManager::GetSystemStatus() const
{
    TArray<FBuild_SystemInfo> StatusArray;
    for (const auto& SystemPair : SystemRegistry)
    {
        StatusArray.Add(SystemPair.Value);
    }
    return StatusArray;
}

int32 UBuildIntegrationManager::GetTotalActorCount() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 TotalCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalCount++;
    }
    
    return TotalCount;
}

void UBuildIntegrationManager::SaveMapSafely()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Cannot save - no world context"));
        return;
    }
    
    FString MapPath = TEXT("/Game/Maps/MinPlayableMap");
    bool bSaveResult = false;
    
    // Use the safe save method
    if (GEngine && GEngine->GetEditorSubsystem<UEditorLoadingAndSavingUtils>())
    {
        bSaveResult = UEditorLoadingAndSavingUtils::SaveMap(World, MapPath);
    }
    
    if (bSaveResult)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Map saved successfully to %s"), *MapPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Failed to save map to %s"), *MapPath);
    }
}

bool UBuildIntegrationManager::CompileAndValidateModule(const FString& ModuleName)
{
    // This would trigger module recompilation in a real scenario
    // For now, just validate that the module classes are loadable
    
    if (ModuleName == TEXT("TranspersonalGame"))
    {
        ValidateAllSystems();
        return bAllSystemsReady;
    }
    
    return false;
}

void UBuildIntegrationManager::LogSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("All Systems Ready: %s"), bAllSystemsReady ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), GetTotalActorCount());
    
    for (const auto& SystemPair : SystemRegistry)
    {
        const FBuild_SystemInfo& Info = SystemPair.Value;
        FString StatusStr;
        switch (Info.Status)
        {
            case EBuild_SystemStatus::Ready: StatusStr = TEXT("READY"); break;
            case EBuild_SystemStatus::Loading: StatusStr = TEXT("LOADING"); break;
            case EBuild_SystemStatus::Error: StatusStr = TEXT("ERROR"); break;
            case EBuild_SystemStatus::Disabled: StatusStr = TEXT("DISABLED"); break;
            default: StatusStr = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("System [%s]: %s (Actors: %d) %s"), 
               *Info.SystemName, *StatusStr, Info.ActorCount, 
               Info.ErrorMessage.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("- %s"), *Info.ErrorMessage));
    }
    UE_LOG(LogTemp, Warning, TEXT("=== END STATUS ==="));
}