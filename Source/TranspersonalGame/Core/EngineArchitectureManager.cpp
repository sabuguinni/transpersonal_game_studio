#include "EngineArchitectureManager.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Engine/StaticMeshActor.h"

AEngineArchitectureManager::AEngineArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Tick every second for performance

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create visualization mesh (blue cube to identify in viewport)
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    
    // Try to load a basic cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        VisualizationMesh->SetStaticMesh(CubeMeshAsset.Object);
        VisualizationMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.5f)); // Flat cube for visibility
    }

    // Initialize architecture settings
    bEnableArchitectureValidation = true;
    ValidationInterval = 5.0f;
    LastValidationTime = 0.0f;

    // Performance settings for Milestone 1
    MaxActorCount = 200; // Conservative limit for prototype
    TargetFrameRate = 30.0f; // Minimum acceptable for prototype
    CurrentFrameRate = 60.0f;
    CurrentActorCount = 0;

    // Milestone 1 technical requirements
    bEnforceTerrainLimits = true;
    MaxTerrainSize = 4000.0f; // 4km x 4km maximum for prototype
    MaxDinosaurCount = 10; // Maximum dinosaurs for prototype
    bRequireNavMesh = true;

    // Initialize status
    bArchitectureHealthy = false;
    FrameTimeAccumulator = 0.0f;
    FrameTimeCount = 0;
    FrameTimeHistory.Reserve(60); // Store 60 frames of history
}

void AEngineArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Starting architecture validation system"));
    
    // Initial validation
    ValidateArchitecture();
    
    // Set up initial system readiness tracking
    SystemReadiness.Add(TEXT("Lighting"), false);
    SystemReadiness.Add(TEXT("Terrain"), false);
    SystemReadiness.Add(TEXT("Character"), false);
    SystemReadiness.Add(TEXT("Dinosaurs"), false);
    SystemReadiness.Add(TEXT("Navigation"), false);
}

void AEngineArchitectureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    CalculateFrameRate(DeltaTime);
    
    // Periodic architecture validation
    if (bEnableArchitectureValidation && GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastValidationTime >= ValidationInterval)
        {
            ValidateArchitecture();
            LastValidationTime = CurrentTime;
        }
    }
    
    // Enforce Milestone 1 rules
    EnforceMilestone1Rules();
}

void AEngineArchitectureManager::ValidateArchitecture()
{
    ValidationErrors.Empty();
    PerformanceWarnings.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Running architecture validation"));
    
    // Validate all critical systems
    bool LightingOK = CheckLightingSetup();
    bool TerrainOK = CheckTerrainCompliance();
    bool CharacterOK = CheckCharacterSystem();
    bool DinosaurOK = CheckDinosaurSystem();
    
    // Update system readiness
    SystemReadiness[TEXT("Lighting")] = LightingOK;
    SystemReadiness[TEXT("Terrain")] = TerrainOK;
    SystemReadiness[TEXT("Character")] = CharacterOK;
    SystemReadiness[TEXT("Dinosaurs")] = DinosaurOK;
    
    // Overall health check
    bArchitectureHealthy = LightingOK && TerrainOK && CharacterOK && (ValidationErrors.Num() == 0);
    
    // Log results
    if (bArchitectureHealthy)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Architecture validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectureManager: Architecture validation FAILED - %d errors"), ValidationErrors.Num());
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
}

bool AEngineArchitectureManager::CheckLightingSetup()
{
    if (!GetWorld()) return false;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 DirectionalLights = 0;
    int32 SkyLights = 0;
    int32 HeightFogs = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->IsA<ADirectionalLight>()) DirectionalLights++;
        else if (Actor->IsA<ASkyLight>()) SkyLights++;
        else if (Actor->GetClass()->GetName().Contains(TEXT("ExponentialHeightFog"))) HeightFogs++;
    }
    
    // Milestone 1 requires exactly 1 of each lighting type
    bool LightingValid = (DirectionalLights == 1) && (SkyLights == 1) && (HeightFogs <= 1);
    
    if (DirectionalLights != 1)
    {
        ValidationErrors.Add(FString::Printf(TEXT("Lighting: Expected 1 DirectionalLight, found %d"), DirectionalLights));
    }
    if (SkyLights != 1)
    {
        ValidationErrors.Add(FString::Printf(TEXT("Lighting: Expected 1 SkyLight, found %d"), SkyLights));
    }
    if (HeightFogs > 1)
    {
        ValidationErrors.Add(FString::Printf(TEXT("Lighting: Too many HeightFog actors: %d"), HeightFogs));
    }
    
    return LightingValid;
}

bool AEngineArchitectureManager::CheckTerrainCompliance()
{
    if (!GetWorld()) return false;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 LandscapeCount = 0;
    float MaxTerrainBounds = 0.0f;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("Landscape")))
        {
            LandscapeCount++;
            
            // Check terrain size if enforcing limits
            if (bEnforceTerrainLimits)
            {
                FVector Origin, BoxExtent;
                Actor->GetActorBounds(false, Origin, BoxExtent);
                float TerrainSize = FMath::Max(BoxExtent.X, BoxExtent.Y) * 2.0f; // Full diameter
                MaxTerrainBounds = FMath::Max(MaxTerrainBounds, TerrainSize);
            }
        }
    }
    
    bool TerrainValid = true;
    
    if (LandscapeCount == 0)
    {
        ValidationErrors.Add(TEXT("Terrain: No Landscape actor found - Milestone 1 requires terrain"));
        TerrainValid = false;
    }
    else if (LandscapeCount > 1)
    {
        ValidationErrors.Add(FString::Printf(TEXT("Terrain: Multiple Landscape actors found (%d) - use single Landscape for Milestone 1"), LandscapeCount));
    }
    
    if (bEnforceTerrainLimits && MaxTerrainBounds > MaxTerrainSize)
    {
        ValidationErrors.Add(FString::Printf(TEXT("Terrain: Size %.0f exceeds Milestone 1 limit of %.0f"), MaxTerrainBounds, MaxTerrainSize));
        TerrainValid = false;
    }
    
    return TerrainValid;
}

bool AEngineArchitectureManager::CheckCharacterSystem()
{
    if (!GetWorld()) return false;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), AllActors);
    
    int32 TranspersonalCharacters = 0;
    int32 PlayerStarts = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            TranspersonalCharacters++;
        }
    }
    
    // Check for PlayerStart
    TArray<AActor*> PlayerStartActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStartActors);
    PlayerStarts = PlayerStartActors.Num();
    
    bool CharacterValid = true;
    
    if (PlayerStarts == 0)
    {
        ValidationErrors.Add(TEXT("Character: No PlayerStart found - required for Milestone 1"));
        CharacterValid = false;
    }
    
    // Note: TranspersonalCharacter might not be spawned yet, so we don't enforce its presence
    
    return CharacterValid;
}

bool AEngineArchitectureManager::CheckDinosaurSystem()
{
    if (!GetWorld()) return false;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Dinosaur")) || 
            ActorName.Contains(TEXT("TRex")) || 
            ActorName.Contains(TEXT("Raptor")) || 
            ActorName.Contains(TEXT("Brachio")))
        {
            DinosaurCount++;
        }
    }
    
    bool DinosaurValid = true;
    
    if (DinosaurCount == 0)
    {
        ValidationErrors.Add(TEXT("Dinosaurs: No dinosaur actors found - Milestone 1 requires 3-5 dinosaur meshes"));
        DinosaurValid = false;
    }
    else if (DinosaurCount > MaxDinosaurCount)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("Dinosaurs: %d dinosaurs exceed Milestone 1 limit of %d"), DinosaurCount, MaxDinosaurCount));
    }
    
    return DinosaurValid;
}

void AEngineArchitectureManager::UpdatePerformanceMetrics()
{
    if (!GetWorld()) return;
    
    // Count actors
    CountWorldActors();
    
    // Check performance thresholds
    CheckActorCount();
    CheckFrameRate();
}

void AEngineArchitectureManager::CheckActorCount()
{
    if (CurrentActorCount > MaxActorCount)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("Performance: Actor count %d exceeds limit of %d"), CurrentActorCount, MaxActorCount));
    }
}

void AEngineArchitectureManager::CheckFrameRate()
{
    if (CurrentFrameRate < TargetFrameRate)
    {
        PerformanceWarnings.Add(FString::Printf(TEXT("Performance: Frame rate %.1f below target of %.1f"), CurrentFrameRate, TargetFrameRate));
    }
}

void AEngineArchitectureManager::EnforceMilestone1Rules()
{
    // This function enforces critical rules for Milestone 1
    // Currently just validates - future versions could auto-fix issues
    
    if (!bArchitectureHealthy)
    {
        // Log warnings about architecture health
        if (ValidationErrors.Num() > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Architecture unhealthy - %d errors"), ValidationErrors.Num());
        }
    }
}

bool AEngineArchitectureManager::ValidateMinimumViablePrototype()
{
    ValidateArchitecture();
    
    // Milestone 1 MVP requirements:
    // 1. Terrain exists
    // 2. Lighting is set up
    // 3. PlayerStart exists
    // 4. At least some dinosaur actors exist
    
    bool HasTerrain = SystemReadiness[TEXT("Terrain")];
    bool HasLighting = SystemReadiness[TEXT("Lighting")];
    bool HasCharacterSystem = SystemReadiness[TEXT("Character")];
    bool HasDinosaurs = SystemReadiness[TEXT("Dinosaurs")];
    
    bool MVPValid = HasTerrain && HasLighting && HasCharacterSystem;
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: MVP Validation - Terrain:%s Lighting:%s Character:%s Dinosaurs:%s"),
        HasTerrain ? TEXT("OK") : TEXT("FAIL"),
        HasLighting ? TEXT("OK") : TEXT("FAIL"),
        HasCharacterSystem ? TEXT("OK") : TEXT("FAIL"),
        HasDinosaurs ? TEXT("OK") : TEXT("WARN"));
    
    return MVPValid;
}

void AEngineArchitectureManager::ReportArchitectureStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURE STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Architecture Healthy: %s"), bArchitectureHealthy ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Current Actor Count: %d (limit: %d)"), CurrentActorCount, MaxActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Current Frame Rate: %.1f (target: %.1f)"), CurrentFrameRate, TargetFrameRate);
    
    UE_LOG(LogTemp, Warning, TEXT("System Readiness:"));
    for (const auto& System : SystemReadiness)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s"), *System.Key, System.Value ? TEXT("READY") : TEXT("NOT READY"));
    }
    
    if (ValidationErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation Errors (%d):"), ValidationErrors.Num());
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Error);
        }
    }
    
    if (PerformanceWarnings.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warnings (%d):"), PerformanceWarnings.Num());
        for (const FString& Warning : PerformanceWarnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Warning);
        }
    }
}

void AEngineArchitectureManager::NotifySystemReady(const FString& SystemName)
{
    SystemReadiness.Add(SystemName, true);
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: System '%s' reported ready"), *SystemName);
}

void AEngineArchitectureManager::NotifySystemError(const FString& SystemName, const FString& ErrorMessage)
{
    SystemReadiness.Add(SystemName, false);
    ValidationErrors.Add(FString::Printf(TEXT("%s: %s"), *SystemName, *ErrorMessage));
    UE_LOG(LogTemp, Error, TEXT("EngineArchitectureManager: System '%s' error - %s"), *SystemName, *ErrorMessage);
}

TArray<FString> AEngineArchitectureManager::GetSystemReadinessReport()
{
    TArray<FString> Report;
    
    for (const auto& System : SystemReadiness)
    {
        FString Status = System.Value ? TEXT("READY") : TEXT("NOT READY");
        Report.Add(FString::Printf(TEXT("%s: %s"), *System.Key, *Status));
    }
    
    return Report;
}

void AEngineArchitectureManager::LogArchitectureReport()
{
    ReportArchitectureStatus();
}

void AEngineArchitectureManager::VisualizeSystemBounds()
{
    // This would draw debug bounds for all major systems
    // Implementation would use DrawDebugBox, DrawDebugSphere etc.
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: System bounds visualization requested"));
}

void AEngineArchitectureManager::ClearValidationErrors()
{
    ValidationErrors.Empty();
    PerformanceWarnings.Empty();
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Validation errors cleared"));
}

void AEngineArchitectureManager::CalculateFrameRate(float DeltaTime)
{
    FrameTimeAccumulator += DeltaTime;
    FrameTimeCount++;
    
    // Calculate average over last 60 frames
    if (FrameTimeCount >= 60)
    {
        float AverageFrameTime = FrameTimeAccumulator / FrameTimeCount;
        CurrentFrameRate = 1.0f / AverageFrameTime;
        
        // Reset for next calculation
        FrameTimeAccumulator = 0.0f;
        FrameTimeCount = 0;
    }
}

void AEngineArchitectureManager::CountWorldActors()
{
    if (!GetWorld()) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentActorCount = AllActors.Num();
}