#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Sound/SoundCue.h"
#include "Materials/Material.h"
#include "Particles/ParticleSystem.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bValidationInProgress = false;
    
    // Initialize critical systems list
    CriticalSystems.Add(TEXT("Character"));
    CriticalSystems.Add(TEXT("WorldGeneration"));
    CriticalSystems.Add(TEXT("Physics"));
    CriticalSystems.Add(TEXT("Audio"));
    CriticalSystems.Add(TEXT("Lighting"));
    CriticalSystems.Add(TEXT("VFX"));
    CriticalSystems.Add(TEXT("AssetPipeline"));
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Subsystem initialized"));
    
    // Start initial validation after a short delay
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, this, &UBuild_FinalIntegrationOrchestrator::StartIntegrationValidation, 2.0f, false);
    }
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Subsystem deinitialized"));
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::StartIntegrationValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integration validation already in progress"));
        return;
    }

    bValidationInProgress = true;
    LastReport = FBuild_IntegrationReport();
    LastReport.ReportTimestamp = FDateTime::Now();

    UE_LOG(LogTemp, Warning, TEXT("Starting comprehensive integration validation..."));

    // Validate all critical systems
    ValidateCharacterSystem();
    ValidateWorldGeneration();
    ValidatePhysicsSystem();
    ValidateAudioSystem();
    ValidateLightingSystem();
    ValidateVFXSystem();
    ValidateAssetPipeline();

    // Compile final report
    CompileValidationReport();
    
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Integration validation completed. Status: %s"), 
           LastReport.OverallStatus == EBuild_IntegrationStatus::Completed ? TEXT("SUCCESS") : TEXT("FAILED"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystem()
{
    FString SystemName = TEXT("Character");
    bool bValid = true;
    FString ErrorMsg = TEXT("");

    try
    {
        // Check if TranspersonalCharacter class exists
        UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
        if (!CharacterClass)
        {
            bValid = false;
            ErrorMsg = TEXT("TranspersonalCharacter class not found");
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Character system validation: TranspersonalCharacter class found"));
        }

        // Check for character instances in world
        if (UWorld* World = GetWorld())
        {
            int32 CharacterCount = 0;
            for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
            {
                if (ActorItr->GetClass()->GetName().Contains(TEXT("Character")))
                {
                    CharacterCount++;
                }
            }
            UE_LOG(LogTemp, Log, TEXT("Character system validation: Found %d character instances"), CharacterCount);
        }
    }
    catch (...)
    {
        bValid = false;
        ErrorMsg = TEXT("Exception during character system validation");
    }

    LastReport.SystemResults.Add(CreateValidationResult(SystemName, bValid, ErrorMsg));
}

void UBuild_FinalIntegrationOrchestrator::ValidateWorldGeneration()
{
    FString SystemName = TEXT("WorldGeneration");
    bool bValid = true;
    FString ErrorMsg = TEXT("");

    try
    {
        if (UWorld* World = GetWorld())
        {
            // Check for landscape actors
            int32 LandscapeCount = 0;
            for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
            {
                LandscapeCount++;
            }

            // Check for static mesh actors (terrain features)
            int32 StaticMeshCount = 0;
            for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                StaticMeshCount++;
            }

            UE_LOG(LogTemp, Log, TEXT("World generation validation: %d landscapes, %d static meshes"), 
                   LandscapeCount, StaticMeshCount);

            if (StaticMeshCount < 10)
            {
                bValid = false;
                ErrorMsg = FString::Printf(TEXT("Insufficient world content: only %d static meshes"), StaticMeshCount);
            }
        }
        else
        {
            bValid = false;
            ErrorMsg = TEXT("No world available for validation");
        }
    }
    catch (...)
    {
        bValid = false;
        ErrorMsg = TEXT("Exception during world generation validation");
    }

    LastReport.SystemResults.Add(CreateValidationResult(SystemName, bValid, ErrorMsg));
}

void UBuild_FinalIntegrationOrchestrator::ValidatePhysicsSystem()
{
    FString SystemName = TEXT("Physics");
    bool bValid = true;
    FString ErrorMsg = TEXT("");

    try
    {
        if (UWorld* World = GetWorld())
        {
            // Check physics world
            if (World->GetPhysicsScene())
            {
                UE_LOG(LogTemp, Log, TEXT("Physics system validation: Physics scene active"));
            }
            else
            {
                bValid = false;
                ErrorMsg = TEXT("Physics scene not active");
            }

            // Count actors with collision
            int32 CollisionActorCount = 0;
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                if (ActorItr->GetRootComponent() && ActorItr->GetRootComponent()->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                {
                    CollisionActorCount++;
                }
            }

            UE_LOG(LogTemp, Log, TEXT("Physics system validation: %d actors with collision"), CollisionActorCount);
        }
    }
    catch (...)
    {
        bValid = false;
        ErrorMsg = TEXT("Exception during physics system validation");
    }

    LastReport.SystemResults.Add(CreateValidationResult(SystemName, bValid, ErrorMsg));
}

void UBuild_FinalIntegrationOrchestrator::ValidateAudioSystem()
{
    FString SystemName = TEXT("Audio");
    bool bValid = true;
    FString ErrorMsg = TEXT("");

    try
    {
        // Check for audio engine
        if (GEngine && GEngine->GetAudioDeviceManager())
        {
            UE_LOG(LogTemp, Log, TEXT("Audio system validation: Audio device manager available"));
        }
        else
        {
            bValid = false;
            ErrorMsg = TEXT("Audio device manager not available");
        }

        // Check for sound cues in content
        int32 SoundCueCount = 0;
        for (TObjectIterator<USoundCue> SoundItr; SoundItr; ++SoundItr)
        {
            SoundCueCount++;
        }

        UE_LOG(LogTemp, Log, TEXT("Audio system validation: %d sound cues loaded"), SoundCueCount);
    }
    catch (...)
    {
        bValid = false;
        ErrorMsg = TEXT("Exception during audio system validation");
    }

    LastReport.SystemResults.Add(CreateValidationResult(SystemName, bValid, ErrorMsg));
}

void UBuild_FinalIntegrationOrchestrator::ValidateLightingSystem()
{
    FString SystemName = TEXT("Lighting");
    bool bValid = true;
    FString ErrorMsg = TEXT("");

    try
    {
        if (UWorld* World = GetWorld())
        {
            // Check for directional light
            int32 DirectionalLightCount = 0;
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                if (ActorItr->GetClass()->GetName().Contains(TEXT("DirectionalLight")))
                {
                    DirectionalLightCount++;
                }
            }

            UE_LOG(LogTemp, Log, TEXT("Lighting system validation: %d directional lights"), DirectionalLightCount);

            if (DirectionalLightCount == 0)
            {
                bValid = false;
                ErrorMsg = TEXT("No directional lights found in scene");
            }
        }
    }
    catch (...)
    {
        bValid = false;
        ErrorMsg = TEXT("Exception during lighting system validation");
    }

    LastReport.SystemResults.Add(CreateValidationResult(SystemName, bValid, ErrorMsg));
}

void UBuild_FinalIntegrationOrchestrator::ValidateVFXSystem()
{
    FString SystemName = TEXT("VFX");
    bool bValid = true;
    FString ErrorMsg = TEXT("");

    try
    {
        // Check for particle systems
        int32 ParticleSystemCount = 0;
        for (TObjectIterator<UParticleSystem> ParticleItr; ParticleItr; ++ParticleItr)
        {
            ParticleSystemCount++;
        }

        UE_LOG(LogTemp, Log, TEXT("VFX system validation: %d particle systems loaded"), ParticleSystemCount);

        // VFX system is optional, so we don't fail if none found
        if (ParticleSystemCount == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("No particle systems found, but VFX is optional"));
        }
    }
    catch (...)
    {
        bValid = false;
        ErrorMsg = TEXT("Exception during VFX system validation");
    }

    LastReport.SystemResults.Add(CreateValidationResult(SystemName, bValid, ErrorMsg));
}

void UBuild_FinalIntegrationOrchestrator::ValidateAssetPipeline()
{
    FString SystemName = TEXT("AssetPipeline");
    bool bValid = true;
    FString ErrorMsg = TEXT("");

    try
    {
        // Check for materials
        int32 MaterialCount = 0;
        for (TObjectIterator<UMaterial> MaterialItr; MaterialItr; ++MaterialItr)
        {
            MaterialCount++;
        }

        UE_LOG(LogTemp, Log, TEXT("Asset pipeline validation: %d materials loaded"), MaterialCount);

        if (MaterialCount < 5)
        {
            bValid = false;
            ErrorMsg = FString::Printf(TEXT("Insufficient materials: only %d found"), MaterialCount);
        }
    }
    catch (...)
    {
        bValid = false;
        ErrorMsg = TEXT("Exception during asset pipeline validation");
    }

    LastReport.SystemResults.Add(CreateValidationResult(SystemName, bValid, ErrorMsg));
}

void UBuild_FinalIntegrationOrchestrator::CompileValidationReport()
{
    LastReport.TotalActors = GetActiveActorCount();
    LastReport.MemoryUsagePercent = GetMemoryUsagePercent();

    // Determine overall status
    bool bAllSystemsValid = true;
    for (const FBuild_SystemValidationResult& Result : LastReport.SystemResults)
    {
        if (!Result.bIsValid)
        {
            bAllSystemsValid = false;
            break;
        }
    }

    LastReport.OverallStatus = bAllSystemsValid ? EBuild_IntegrationStatus::Completed : EBuild_IntegrationStatus::Failed;

    UE_LOG(LogTemp, Warning, TEXT("Integration Report Compiled:"));
    UE_LOG(LogTemp, Warning, TEXT("- Total Actors: %d"), LastReport.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("- Memory Usage: %.1f%%"), LastReport.MemoryUsagePercent);
    UE_LOG(LogTemp, Warning, TEXT("- Systems Validated: %d"), LastReport.SystemResults.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Overall Status: %s"), 
           LastReport.OverallStatus == EBuild_IntegrationStatus::Completed ? TEXT("SUCCESS") : TEXT("FAILED"));
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::CreateValidationResult(const FString& SystemName, bool bValid, const FString& Error)
{
    FBuild_SystemValidationResult Result;
    Result.SystemName = SystemName;
    Result.bIsValid = bValid;
    Result.ErrorMessage = Error;
    Result.ValidationTime = FPlatformTime::Seconds();
    return Result;
}

FBuild_IntegrationReport UBuild_FinalIntegrationOrchestrator::GetLastIntegrationReport() const
{
    return LastReport;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateSystemIntegrity(const FString& SystemName)
{
    for (const FBuild_SystemValidationResult& Result : LastReport.SystemResults)
    {
        if (Result.SystemName == SystemName)
        {
            return Result.bIsValid;
        }
    }
    return false;
}

void UBuild_FinalIntegrationOrchestrator::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("Forcing garbage collection..."));
    GEngine->ForceGarbageCollection(true);
}

int32 UBuild_FinalIntegrationOrchestrator::GetActiveActorCount() const
{
    int32 Count = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            Count++;
        }
    }
    return Count;
}

float UBuild_FinalIntegrationOrchestrator::GetMemoryUsagePercent() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    if (MemStats.TotalPhysical > 0)
    {
        return (float)(MemStats.UsedPhysical) / (float)(MemStats.TotalPhysical) * 100.0f;
    }
    return 0.0f;
}

// ABuild_IntegrationTestActor Implementation

ABuild_IntegrationTestActor::ABuild_IntegrationTestActor()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    bTestsCompleted = false;
}

void ABuild_IntegrationTestActor::RunIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("Running integration tests..."));
    
    TestResults.Empty();
    bTestsCompleted = false;

    bool bAllTestsPassed = true;

    // Test 1: Actor Spawning
    if (TestActorSpawning())
    {
        TestResults.Add(TEXT("Actor Spawning: PASS"));
    }
    else
    {
        TestResults.Add(TEXT("Actor Spawning: FAIL"));
        bAllTestsPassed = false;
    }

    // Test 2: Component Registration
    if (TestComponentRegistration())
    {
        TestResults.Add(TEXT("Component Registration: PASS"));
    }
    else
    {
        TestResults.Add(TEXT("Component Registration: FAIL"));
        bAllTestsPassed = false;
    }

    // Test 3: Cross-System Communication
    if (TestCrossSystemCommunication())
    {
        TestResults.Add(TEXT("Cross-System Communication: PASS"));
    }
    else
    {
        TestResults.Add(TEXT("Cross-System Communication: FAIL"));
        bAllTestsPassed = false;
    }

    bTestsCompleted = true;

    UE_LOG(LogTemp, Warning, TEXT("Integration tests completed. Overall result: %s"), 
           bAllTestsPassed ? TEXT("PASS") : TEXT("FAIL"));

    for (const FString& Result : TestResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("Test Result: %s"), *Result);
    }
}

bool ABuild_IntegrationTestActor::TestActorSpawning()
{
    try
    {
        if (UWorld* World = GetWorld())
        {
            // Try to spawn a basic static mesh actor
            FVector SpawnLocation = GetActorLocation() + FVector(100, 0, 0);
            AActor* SpawnedActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
            
            if (SpawnedActor)
            {
                UE_LOG(LogTemp, Log, TEXT("Successfully spawned test actor"));
                SpawnedActor->Destroy(); // Clean up
                return true;
            }
        }
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception during actor spawning test"));
    }

    return false;
}

bool ABuild_IntegrationTestActor::TestComponentRegistration()
{
    try
    {
        // Check if our root component is properly registered
        if (RootComponent && RootComponent->IsRegistered())
        {
            UE_LOG(LogTemp, Log, TEXT("Component registration test passed"));
            return true;
        }
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception during component registration test"));
    }

    return false;
}

bool ABuild_IntegrationTestActor::TestCrossSystemCommunication()
{
    try
    {
        // Test subsystem access
        if (UWorld* World = GetWorld())
        {
            UBuild_FinalIntegrationOrchestrator* Orchestrator = World->GetSubsystem<UBuild_FinalIntegrationOrchestrator>();
            if (Orchestrator)
            {
                int32 ActorCount = Orchestrator->GetActiveActorCount();
                UE_LOG(LogTemp, Log, TEXT("Cross-system communication test: Got actor count %d"), ActorCount);
                return true;
            }
        }
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Exception during cross-system communication test"));
    }

    return false;
}