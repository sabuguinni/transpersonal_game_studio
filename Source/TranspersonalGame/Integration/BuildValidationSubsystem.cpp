#include "BuildValidationSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/Core/TranspersonalGameState.h"
#include "TranspersonalGame/Character/TranspersonalCharacter.h"
#include "TranspersonalGame/AI/DinosaurTRex.h"
#include "TranspersonalGame/AI/DinosaurCombatAIController.h"
#include "TranspersonalGame/World/PCGWorldGenerator.h"
#include "TranspersonalGame/Environment/FoliageManager.h"
#include "TranspersonalGame/Crowd/CrowdSimulationManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildValidation, Log, All);

UBuildValidationSubsystem::UBuildValidationSubsystem()
{
    ValidationInterval = 30.0f;
}

void UBuildValidationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogBuildValidation, Log, TEXT("BuildValidationSubsystem: Initializing build validation"));
    
    // Start periodic validation timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildValidationSubsystem::PerformPeriodicValidation,
            ValidationInterval,
            true
        );
    }
    
    // Perform initial validation
    ForceValidation();
}

void UBuildValidationSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::Deinitialize();
}

FBuild_ValidationMetrics UBuildValidationSubsystem::ValidateBuildHealth()
{
    FBuild_ValidationMetrics Metrics;
    Metrics.LastValidationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogBuildValidation, Log, TEXT("Starting comprehensive build validation"));
    
    // Clear previous errors
    Metrics.ValidationErrors.Empty();
    ModuleStatuses.Empty();
    
    // Validate all critical systems
    ValidateCoreGameSystems(Metrics);
    ValidateCharacterSystems(Metrics);
    ValidateAISystems(Metrics);
    ValidateWorldSystems(Metrics);
    ValidateWorldActors(Metrics);
    
    // Calculate overall health
    Metrics.bBuildHealthy = (Metrics.FailedModules == 0) && (Metrics.ValidationErrors.Num() == 0);
    
    // Store latest metrics
    LatestMetrics = Metrics;
    
    UE_LOG(LogBuildValidation, Log, TEXT("Build validation complete: %s"), 
           Metrics.bBuildHealthy ? TEXT("HEALTHY") : TEXT("ISSUES DETECTED"));
    
    return Metrics;
}

FBuild_ModuleStatus UBuildValidationSubsystem::ValidateModule(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    
    double StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Try to load the class
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            Status.bIsLoaded = true;
            Status.bHasErrors = false;
            UE_LOG(LogBuildValidation, Log, TEXT("Module %s: LOADED"), *ModuleName);
        }
        else
        {
            Status.bIsLoaded = false;
            Status.bHasErrors = true;
            Status.ErrorMessage = TEXT("Class not found");
            UE_LOG(LogBuildValidation, Warning, TEXT("Module %s: NOT FOUND"), *ModuleName);
        }
    }
    catch (const std::exception& e)
    {
        Status.bIsLoaded = false;
        Status.bHasErrors = true;
        Status.ErrorMessage = FString::Printf(TEXT("Exception: %s"), ANSI_TO_TCHAR(e.what()));
        UE_LOG(LogBuildValidation, Error, TEXT("Module %s: EXCEPTION - %s"), *ModuleName, *Status.ErrorMessage);
    }
    
    Status.LoadTime = FPlatformTime::Seconds() - StartTime;
    return Status;
}

void UBuildValidationSubsystem::ForceValidation()
{
    ValidateBuildHealth();
}

void UBuildValidationSubsystem::ValidateCoreGameSystems(FBuild_ValidationMetrics& Metrics)
{
    // Validate TranspersonalGameState
    FBuild_ModuleStatus GameStateStatus = ValidateModule(TEXT("TranspersonalGameState"));
    ModuleStatuses.Add(GameStateStatus);
    
    if (!GameStateStatus.bIsLoaded)
    {
        Metrics.FailedModules++;
        Metrics.ValidationErrors.Add(TEXT("TranspersonalGameState not loaded"));
    }
    else
    {
        Metrics.CriticalSystemsOnline++;
        
        // Check if actually active in world
        if (UWorld* World = GetWorld())
        {
            AGameStateBase* GameState = World->GetGameState();
            if (GameState && GameState->IsA<ATranspersonalGameState>())
            {
                UE_LOG(LogBuildValidation, Log, TEXT("TranspersonalGameState is active in world"));
            }
            else
            {
                Metrics.ValidationErrors.Add(TEXT("TranspersonalGameState not active in world"));
            }
        }
    }
    
    Metrics.TotalModulesLoaded++;
}

void UBuildValidationSubsystem::ValidateCharacterSystems(FBuild_ValidationMetrics& Metrics)
{
    // Validate TranspersonalCharacter
    FBuild_ModuleStatus CharacterStatus = ValidateModule(TEXT("TranspersonalCharacter"));
    ModuleStatuses.Add(CharacterStatus);
    
    if (!CharacterStatus.bIsLoaded)
    {
        Metrics.FailedModules++;
        Metrics.ValidationErrors.Add(TEXT("TranspersonalCharacter not loaded"));
    }
    else
    {
        Metrics.CriticalSystemsOnline++;
        
        // Check for character instances in world
        if (UWorld* World = GetWorld())
        {
            TArray<AActor*> Characters;
            UGameplayStatics::GetAllActorsOfClass(World, ATranspersonalCharacter::StaticClass(), Characters);
            UE_LOG(LogBuildValidation, Log, TEXT("Found %d TranspersonalCharacter instances"), Characters.Num());
        }
    }
    
    Metrics.TotalModulesLoaded++;
}

void UBuildValidationSubsystem::ValidateAISystems(FBuild_ValidationMetrics& Metrics)
{
    // Validate DinosaurTRex
    FBuild_ModuleStatus TRexStatus = ValidateModule(TEXT("DinosaurTRex"));
    ModuleStatuses.Add(TRexStatus);
    
    if (!TRexStatus.bIsLoaded)
    {
        Metrics.FailedModules++;
        Metrics.ValidationErrors.Add(TEXT("DinosaurTRex not loaded"));
    }
    else
    {
        Metrics.CriticalSystemsOnline++;
    }
    
    // Validate DinosaurCombatAIController
    FBuild_ModuleStatus AIControllerStatus = ValidateModule(TEXT("DinosaurCombatAIController"));
    ModuleStatuses.Add(AIControllerStatus);
    
    if (!AIControllerStatus.bIsLoaded)
    {
        Metrics.FailedModules++;
        Metrics.ValidationErrors.Add(TEXT("DinosaurCombatAIController not loaded"));
    }
    else
    {
        Metrics.CriticalSystemsOnline++;
    }
    
    Metrics.TotalModulesLoaded += 2;
}

void UBuildValidationSubsystem::ValidateWorldSystems(FBuild_ValidationMetrics& Metrics)
{
    // Validate PCGWorldGenerator
    FBuild_ModuleStatus WorldGenStatus = ValidateModule(TEXT("PCGWorldGenerator"));
    ModuleStatuses.Add(WorldGenStatus);
    
    if (!WorldGenStatus.bIsLoaded)
    {
        Metrics.FailedModules++;
        Metrics.ValidationErrors.Add(TEXT("PCGWorldGenerator not loaded"));
    }
    else
    {
        Metrics.CriticalSystemsOnline++;
    }
    
    // Validate FoliageManager
    FBuild_ModuleStatus FoliageStatus = ValidateModule(TEXT("FoliageManager"));
    ModuleStatuses.Add(FoliageStatus);
    
    if (!FoliageStatus.bIsLoaded)
    {
        Metrics.FailedModules++;
        Metrics.ValidationErrors.Add(TEXT("FoliageManager not loaded"));
    }
    else
    {
        Metrics.CriticalSystemsOnline++;
    }
    
    // Validate CrowdSimulationManager
    FBuild_ModuleStatus CrowdStatus = ValidateModule(TEXT("CrowdSimulationManager"));
    ModuleStatuses.Add(CrowdStatus);
    
    if (!CrowdStatus.bIsLoaded)
    {
        Metrics.FailedModules++;
        Metrics.ValidationErrors.Add(TEXT("CrowdSimulationManager not loaded"));
    }
    else
    {
        Metrics.CriticalSystemsOnline++;
    }
    
    Metrics.TotalModulesLoaded += 3;
}

void UBuildValidationSubsystem::ValidateWorldActors(FBuild_ValidationMetrics& Metrics)
{
    if (!GetWorld())
    {
        Metrics.ValidationErrors.Add(TEXT("No valid world for actor validation"));
        return;
    }
    
    // Count all actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    Metrics.TotalActorsInWorld = AllActors.Num();
    
    UE_LOG(LogBuildValidation, Log, TEXT("World contains %d total actors"), AllActors.Num());
    
    // Check for minimum required actors
    if (AllActors.Num() < 10)
    {
        Metrics.ValidationErrors.Add(TEXT("World has too few actors - may be empty"));
    }
    
    // Count specific actor types
    int32 LightCount = 0;
    int32 MeshCount = 0;
    int32 DinosaurCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->IsA<ALight>())
        {
            LightCount++;
        }
        else if (Actor->IsA<AStaticMeshActor>())
        {
            MeshCount++;
        }
        else if (Actor->IsA<ADinosaurTRex>())
        {
            DinosaurCount++;
        }
    }
    
    UE_LOG(LogBuildValidation, Log, TEXT("Actor breakdown: %d lights, %d meshes, %d dinosaurs"), 
           LightCount, MeshCount, DinosaurCount);
    
    // Validate minimum world requirements
    if (LightCount == 0)
    {
        Metrics.ValidationErrors.Add(TEXT("No lighting found in world"));
    }
}

void UBuildValidationSubsystem::PerformPeriodicValidation()
{
    UE_LOG(LogBuildValidation, Log, TEXT("Performing periodic build validation"));
    ValidateBuildHealth();
}