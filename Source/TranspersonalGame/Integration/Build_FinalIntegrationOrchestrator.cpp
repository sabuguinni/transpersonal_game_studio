#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

ABuild_FinalIntegrationOrchestrator::ABuild_FinalIntegrationOrchestrator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create visual component
    OrchestratorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrchestratorMesh"));
    RootComponent = OrchestratorMesh;
    
    // Try to load a basic cube mesh for visibility
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        OrchestratorMesh->SetStaticMesh(CubeMesh.Object);
        OrchestratorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
    }
    
    // Initialize integration status
    bCharacterSystemActive = false;
    bPhysicsSystemActive = false;
    bVFXSystemActive = false;
    bQASystemActive = false;
    DinosaurAssetsLoaded = 0;
    TotalActorsInWorld = 0;
    IntegrationScore = 0.0f;
    
    // Set validation interval to 30 seconds
    ValidationInterval = 30.0f;
    LastValidationTime = 0.0f;
}

void ABuild_FinalIntegrationOrchestrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting integration monitoring"));
    
    // Perform initial validation
    ValidateAllSystems();
    UpdateIntegrationScore();
    LogIntegrationStatus();
}

void ABuild_FinalIntegrationOrchestrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Perform periodic validation
    LastValidationTime += DeltaTime;
    if (LastValidationTime >= ValidationInterval)
    {
        PerformPeriodicValidation();
        LastValidationTime = 0.0f;
    }
}

void ABuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Build_FinalIntegrationOrchestrator: Validating all systems"));
    
    bCharacterSystemActive = ValidateCharacterSystem();
    bPhysicsSystemActive = ValidatePhysicsSystem();
    bVFXSystemActive = ValidateVFXSystem();
    bQASystemActive = ValidateQASystem();
    DinosaurAssetsLoaded = ValidateDinosaurAssets();
    TotalActorsInWorld = GetTotalActorCount();
    
    UpdateIntegrationScore();
}

bool ABuild_FinalIntegrationOrchestrator::ValidateCharacterSystem()
{
    // Try to find TranspersonalCharacter class
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (CharacterClass)
    {
        UE_LOG(LogTemp, Log, TEXT("Character System: ACTIVE - TranspersonalCharacter class found"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Character System: INACTIVE - TranspersonalCharacter class not found"));
    return false;
}

bool ABuild_FinalIntegrationOrchestrator::ValidatePhysicsSystem()
{
    // Try to find Physics_RagdollComponent class
    UClass* PhysicsClass = FindObject<UClass>(ANY_PACKAGE, TEXT("Physics_RagdollComponent"));
    if (PhysicsClass)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics System: ACTIVE - Physics_RagdollComponent class found"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Physics System: INACTIVE - Physics_RagdollComponent class not found"));
    return false;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateVFXSystem()
{
    // Try to find VFX_ImpactManager class
    UClass* VFXClass = FindObject<UClass>(ANY_PACKAGE, TEXT("VFX_ImpactManager"));
    if (VFXClass)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX System: ACTIVE - VFX_ImpactManager class found"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX System: INACTIVE - VFX_ImpactManager class not found"));
    return false;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateQASystem()
{
    // Try to find QA_ValidationFramework class
    UClass* QAClass = FindObject<UClass>(ANY_PACKAGE, TEXT("QA_ValidationFramework"));
    if (QAClass)
    {
        UE_LOG(LogTemp, Log, TEXT("QA System: ACTIVE - QA_ValidationFramework class found"));
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA System: INACTIVE - QA_ValidationFramework class not found"));
    return false;
}

int32 ABuild_FinalIntegrationOrchestrator::ValidateDinosaurAssets()
{
    int32 LoadedCount = 0;
    
    // List of critical dinosaur assets to check
    TArray<FString> DinosaurPaths = {
        TEXT("/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin"),
        TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin"),
        TEXT("/Game/Dinosaur_Pack/Brachiosaurus/Mesh/SKM_Brachiosaurus")
    };
    
    for (const FString& AssetPath : DinosaurPaths)
    {
        UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
        if (Asset)
        {
            LoadedCount++;
            UE_LOG(LogTemp, Log, TEXT("Dinosaur Asset FOUND: %s"), *AssetPath);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Dinosaur Asset MISSING: %s"), *AssetPath);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Assets: %d/3 loaded"), LoadedCount);
    return LoadedCount;
}

void ABuild_FinalIntegrationOrchestrator::UpdateIntegrationScore()
{
    float Score = 0.0f;
    
    // Each system contributes to the score
    if (bCharacterSystemActive) Score += 1.0f;
    if (bPhysicsSystemActive) Score += 1.0f;
    if (bVFXSystemActive) Score += 1.0f;
    if (bQASystemActive) Score += 1.0f;
    
    // Dinosaur assets contribute
    Score += DinosaurAssetsLoaded;
    
    // World population bonus
    if (TotalActorsInWorld > 100) Score += 1.0f;
    
    IntegrationScore = Score;
    
    UE_LOG(LogTemp, Log, TEXT("Integration Score Updated: %.1f/8.0"), IntegrationScore);
}

void ABuild_FinalIntegrationOrchestrator::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Character System: %s"), bCharacterSystemActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    UE_LOG(LogTemp, Warning, TEXT("Physics System: %s"), bPhysicsSystemActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    UE_LOG(LogTemp, Warning, TEXT("VFX System: %s"), bVFXSystemActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    UE_LOG(LogTemp, Warning, TEXT("QA System: %s"), bQASystemActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Assets: %d/3 loaded"), DinosaurAssetsLoaded);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorsInWorld);
    UE_LOG(LogTemp, Warning, TEXT("Integration Score: %.1f/8.0"), IntegrationScore);
    
    if (IntegrationScore >= 6.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("INTEGRATION STATUS: EXCELLENT"));
    }
    else if (IntegrationScore >= 4.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("INTEGRATION STATUS: GOOD"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("INTEGRATION STATUS: NEEDS ATTENTION"));
    }
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION REPORT ==="));
}

void ABuild_FinalIntegrationOrchestrator::NotifySystemChange(const FString& SystemName, bool bIsActive)
{
    UE_LOG(LogTemp, Log, TEXT("System Change Notification: %s is now %s"), 
           *SystemName, bIsActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    
    // Trigger re-validation
    ValidateAllSystems();
}

FString ABuild_FinalIntegrationOrchestrator::GetIntegrationReport()
{
    FString Report = FString::Printf(TEXT("Integration Score: %.1f/8.0\n"), IntegrationScore);
    Report += FString::Printf(TEXT("Character: %s\n"), bCharacterSystemActive ? TEXT("OK") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("Physics: %s\n"), bPhysicsSystemActive ? TEXT("OK") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("VFX: %s\n"), bVFXSystemActive ? TEXT("OK") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("QA: %s\n"), bQASystemActive ? TEXT("OK") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("Dinosaurs: %d/3\n"), DinosaurAssetsLoaded);
    Report += FString::Printf(TEXT("Actors: %d"), TotalActorsInWorld);
    
    return Report;
}

bool ABuild_FinalIntegrationOrchestrator::ValidateBuildIntegrity()
{
    // Check if all critical systems are functional
    bool bBuildValid = bCharacterSystemActive && bPhysicsSystemActive && 
                      (DinosaurAssetsLoaded >= 2) && (TotalActorsInWorld > 50);
    
    UE_LOG(LogTemp, Warning, TEXT("Build Integrity Check: %s"), 
           bBuildValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bBuildValid;
}

void ABuild_FinalIntegrationOrchestrator::TriggerBuildValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Triggering comprehensive build validation..."));
    
    ValidateAllSystems();
    bool bValid = ValidateBuildIntegrity();
    
    if (bValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("BUILD VALIDATION: PASSED - Ready for deployment"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BUILD VALIDATION: FAILED - Critical systems missing"));
    }
}

void ABuild_FinalIntegrationOrchestrator::MonitorPerformanceMetrics()
{
    // Basic performance monitoring
    float MemUsage = GetMemoryUsage();
    int32 ActorCount = GetTotalActorCount();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Metrics - Actors: %d, Memory: %.2f MB"), 
           ActorCount, MemUsage);
    
    // Performance warnings
    if (ActorCount > 10000)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: High actor count (%d)"), ActorCount);
    }
    
    if (MemUsage > 2048.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: High memory usage (%.2f MB)"), MemUsage);
    }
}

int32 ABuild_FinalIntegrationOrchestrator::GetTotalActorCount()
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    return AllActors.Num();
}

float ABuild_FinalIntegrationOrchestrator::GetMemoryUsage()
{
    // Basic memory usage estimation
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

void ABuild_FinalIntegrationOrchestrator::PerformPeriodicValidation()
{
    UE_LOG(LogTemp, Log, TEXT("Performing periodic validation..."));
    
    ValidateAllSystems();
    MonitorPerformanceMetrics();
    UpdateSystemStatus();
    BroadcastIntegrationStatus();
}

void ABuild_FinalIntegrationOrchestrator::UpdateSystemStatus()
{
    // Update internal status tracking
    TotalActorsInWorld = GetTotalActorCount();
    UpdateIntegrationScore();
}

void ABuild_FinalIntegrationOrchestrator::BroadcastIntegrationStatus()
{
    // Broadcast status to other systems if needed
    if (IntegrationScore < 4.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Integration score below threshold (%.1f/8.0)"), IntegrationScore);
    }
}