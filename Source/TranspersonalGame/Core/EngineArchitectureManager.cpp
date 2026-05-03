#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AEngineArchitectureManager::AEngineArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create visualization component
    ArchitectureVisualization = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArchitectureVisualization"));
    ArchitectureVisualization->SetupAttachment(RootComponent);

    // Create status display
    StatusDisplay = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusDisplay"));
    StatusDisplay->SetupAttachment(RootComponent);
    StatusDisplay->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    StatusDisplay->SetWorldSize(50.0f);
    StatusDisplay->SetText(FText::FromString("ENGINE ARCHITECT - INITIALIZING"));

    // Set default values
    bPerformArchitectureValidation = true;
    ValidationInterval = 5.0f;
    bArchitectureValid = false;
    TotalActorsInLevel = 0;
    DuplicateActorsFound = 0;
    CompilationErrors = 0;
    ActiveCppClasses = 0;
    CurrentFrameRate = 0.0f;
    MemoryUsageMB = 0.0f;
    DrawCalls = 0;
    
    // Milestone 1 validation
    bCharacterMovementValid = false;
    bCameraSystemValid = false;
    bTerrainValid = false;
    bLightingValid = false;
    bDinosaurActorsValid = false;
    DinosaurCount = 0;
    
    LastValidationTime = 0.0f;
    CurrentStatusText = "INITIALIZING";

    // Try to load a basic cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        ArchitectureVisualization->SetStaticMesh(CubeMeshAsset.Object);
        ArchitectureVisualization->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.1f));
    }
}

void AEngineArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: BeginPlay - Starting architecture validation"));
    
    // Perform initial validation
    ValidateArchitecture();
    RefreshModuleList();
    CheckMilestone1Requirements();
    
    CurrentStatusText = "ARCHITECTURE MANAGER ACTIVE";
    UpdateStatusDisplay();
}

void AEngineArchitectureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bPerformArchitectureValidation)
    {
        LastValidationTime += DeltaTime;
        
        if (LastValidationTime >= ValidationInterval)
        {
            ValidateArchitecture();
            UpdatePerformanceMetrics();
            CheckMilestone1Requirements();
            UpdateStatusDisplay();
            LastValidationTime = 0.0f;
        }
    }
}

void AEngineArchitectureManager::ValidateArchitecture()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Validating architecture..."));
    
    ValidateActorCounts();
    ValidateSystemIntegrity();
    
    // Overall validation result
    bArchitectureValid = (DuplicateActorsFound == 0 && CompilationErrors == 0);
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture validation complete. Valid: %s"), 
           bArchitectureValid ? TEXT("TRUE") : TEXT("FALSE"));
}

void AEngineArchitectureManager::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorsInLevel = AllActors.Num();

    // Count duplicate lighting actors
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    TArray<AActor*> FogActors;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            FogActors.Add(Actor);
        }
    }

    // Calculate duplicates (should be max 1 of each lighting type)
    DuplicateActorsFound = 0;
    if (DirectionalLights.Num() > 1) DuplicateActorsFound += DirectionalLights.Num() - 1;
    if (FogActors.Num() > 1) DuplicateActorsFound += FogActors.Num() - 1;

    UE_LOG(LogTemp, Log, TEXT("Actor validation: Total=%d, DirectionalLights=%d, FogActors=%d, Duplicates=%d"),
           TotalActorsInLevel, DirectionalLights.Num(), FogActors.Num(), DuplicateActorsFound);
}

void AEngineArchitectureManager::ValidateSystemIntegrity()
{
    // Reset error count
    CompilationErrors = 0;
    
    // Try to load core classes to verify compilation
    LoadedModules.Empty();
    FailedModules.Empty();
    
    // Test TranspersonalGame module classes
    TArray<FString> CoreClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.ProductionDirector")
    };
    
    for (const FString& ClassName : CoreClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            LoadedModules.Add(ClassName);
            ActiveCppClasses++;
        }
        else
        {
            FailedModules.Add(ClassName);
            CompilationErrors++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("System integrity: Loaded=%d, Failed=%d, Errors=%d"),
           LoadedModules.Num(), FailedModules.Num(), CompilationErrors);
}

void AEngineArchitectureManager::CheckMilestone1Requirements()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Check character movement system
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    bCharacterMovementValid = Characters.Num() > 0;

    // Check terrain (landscape or static mesh terrain)
    TArray<AActor*> TerrainActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), TerrainActors);
    bTerrainValid = TerrainActors.Num() > 0;

    // Check lighting system
    TArray<AActor*> LightActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), LightActors);
    bLightingValid = LightActors.Num() > 0;

    // Check camera system (implied by character)
    bCameraSystemValid = bCharacterMovementValid;

    // Count dinosaur actors (placeholder - will be refined by other agents)
    DinosaurCount = 0;
    for (AActor* Actor : TerrainActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            DinosaurCount++;
        }
    }
    bDinosaurActorsValid = DinosaurCount >= 3; // Minimum 3 dinosaurs for Milestone 1

    UE_LOG(LogTemp, Log, TEXT("Milestone 1 check: Character=%s, Camera=%s, Terrain=%s, Lighting=%s, Dinosaurs=%d"),
           bCharacterMovementValid ? TEXT("OK") : TEXT("FAIL"),
           bCameraSystemValid ? TEXT("OK") : TEXT("FAIL"),
           bTerrainValid ? TEXT("OK") : TEXT("FAIL"),
           bLightingValid ? TEXT("OK") : TEXT("FAIL"),
           DinosaurCount);
}

void AEngineArchitectureManager::UpdatePerformanceMetrics()
{
    // Get basic performance metrics
    CurrentFrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
    
    // Memory usage (basic estimation)
    MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Draw calls (placeholder - would need render thread access for real value)
    DrawCalls = TotalActorsInLevel * 2; // Rough estimation
}

void AEngineArchitectureManager::UpdateStatusDisplay()
{
    FString StatusText = FString::Printf(TEXT("ENGINE ARCHITECT\n"
                                              "Architecture: %s\n"
                                              "Actors: %d\n"
                                              "Duplicates: %d\n"
                                              "Modules: %d/%d\n"
                                              "FPS: %.1f\n"
                                              "Memory: %.1f MB\n"
                                              "Milestone 1: %s"),
                                         bArchitectureValid ? TEXT("VALID") : TEXT("INVALID"),
                                         TotalActorsInLevel,
                                         DuplicateActorsFound,
                                         LoadedModules.Num(),
                                         LoadedModules.Num() + FailedModules.Num(),
                                         CurrentFrameRate,
                                         MemoryUsageMB,
                                         (bCharacterMovementValid && bCameraSystemValid && bTerrainValid && bLightingValid) ? TEXT("READY") : TEXT("PENDING"));

    StatusDisplay->SetText(FText::FromString(StatusText));
}

void AEngineArchitectureManager::CleanupDuplicateActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Cleaning up duplicate actors..."));

    // Clean up duplicate directional lights (keep only first one)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    for (int32 i = 1; i < DirectionalLights.Num(); i++)
    {
        if (DirectionalLights[i])
        {
            DirectionalLights[i]->Destroy();
            UE_LOG(LogTemp, Log, TEXT("Destroyed duplicate DirectionalLight: %s"), *DirectionalLights[i]->GetName());
        }
    }

    // Clean up duplicate fog actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    TArray<AActor*> FogActors;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            FogActors.Add(Actor);
        }
    }
    
    for (int32 i = 1; i < FogActors.Num(); i++)
    {
        if (FogActors[i])
        {
            FogActors[i]->Destroy();
            UE_LOG(LogTemp, Log, TEXT("Destroyed duplicate Fog actor: %s"), *FogActors[i]->GetName());
        }
    }

    // Revalidate after cleanup
    ValidateArchitecture();
}

void AEngineArchitectureManager::ValidateModuleIntegrity()
{
    RefreshModuleList();
    ValidateSystemIntegrity();
}

bool AEngineArchitectureManager::IsModuleLoaded(const FString& ModuleName)
{
    return LoadedModules.Contains(ModuleName);
}

void AEngineArchitectureManager::RefreshModuleList()
{
    ValidateSystemIntegrity();
}

FString AEngineArchitectureManager::GetArchitectureReport()
{
    return FString::Printf(TEXT("ARCHITECTURE REPORT\n"
                               "==================\n"
                               "Overall Status: %s\n"
                               "Total Actors: %d\n"
                               "Duplicate Actors: %d\n"
                               "Compilation Errors: %d\n"
                               "Loaded Modules: %d\n"
                               "Failed Modules: %d\n"
                               "Active C++ Classes: %d\n"
                               "Current FPS: %.1f\n"
                               "Memory Usage: %.1f MB\n"
                               "\nMILESTONE 1 STATUS\n"
                               "Character Movement: %s\n"
                               "Camera System: %s\n"
                               "Terrain: %s\n"
                               "Lighting: %s\n"
                               "Dinosaur Actors: %s (%d found)\n"),
                           bArchitectureValid ? TEXT("VALID") : TEXT("INVALID"),
                           TotalActorsInLevel,
                           DuplicateActorsFound,
                           CompilationErrors,
                           LoadedModules.Num(),
                           FailedModules.Num(),
                           ActiveCppClasses,
                           CurrentFrameRate,
                           MemoryUsageMB,
                           bCharacterMovementValid ? TEXT("OK") : TEXT("FAIL"),
                           bCameraSystemValid ? TEXT("OK") : TEXT("FAIL"),
                           bTerrainValid ? TEXT("OK") : TEXT("FAIL"),
                           bLightingValid ? TEXT("OK") : TEXT("FAIL"),
                           bDinosaurActorsValid ? TEXT("OK") : TEXT("FAIL"),
                           DinosaurCount);
}

void AEngineArchitectureManager::LogArchitectureStatus()
{
    FString Report = GetArchitectureReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void AEngineArchitectureManager::SetArchitectureVisualizationVisible(bool bVisible)
{
    if (ArchitectureVisualization)
    {
        ArchitectureVisualization->SetVisibility(bVisible);
    }
    if (StatusDisplay)
    {
        StatusDisplay->SetVisibility(bVisible);
    }
}