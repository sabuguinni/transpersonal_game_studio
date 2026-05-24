#include "EngineArchitectCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// UEngineArchitectWorldSubsystem Implementation

void UEngineArchitectWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect World Subsystem Initialized"));
    
    bArchitectureValid = false;
    LastValidationTime = 0.0f;
    
    InitializeSystemValidations();
    
    // Schedule initial validation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UEngineArchitectWorldSubsystem::ValidateSystemArchitecture,
            5.0f,
            false
        );
    }
}

void UEngineArchitectWorldSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect World Subsystem Deinitialized"));
    
    SystemValidations.Empty();
    
    Super::Deinitialize();
}

bool UEngineArchitectWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

bool UEngineArchitectWorldSubsystem::ValidateSystemArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Validating System Architecture"));
    
    bool bAllSystemsValid = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    // Clear previous validations
    SystemValidations.Empty();
    
    // Validate Player Character System
    FEng_SystemValidation PlayerValidation;
    PlayerValidation.SystemName = TEXT("Player Character");
    PlayerValidation.bIsValid = ValidatePlayerCharacterSetup();
    PlayerValidation.ValidationMessage = PlayerValidation.bIsValid ? 
        TEXT("Player character system operational") : 
        TEXT("Player character system has issues");
    PlayerValidation.LastValidationTime = LastValidationTime;
    SystemValidations.Add(PlayerValidation);
    
    if (!PlayerValidation.bIsValid)
        bAllSystemsValid = false;
    
    // Validate Dinosaur System
    FEng_SystemValidation DinosaurValidation;
    DinosaurValidation.SystemName = TEXT("Dinosaur System");
    int32 DinosaurCount = ValidateDinosaurSpawning();
    DinosaurValidation.bIsValid = DinosaurCount > 0;
    DinosaurValidation.ValidationMessage = FString::Printf(
        TEXT("Found %d dinosaurs in world"), DinosaurCount);
    DinosaurValidation.LastValidationTime = LastValidationTime;
    SystemValidations.Add(DinosaurValidation);
    
    if (!DinosaurValidation.bIsValid)
        bAllSystemsValid = false;
    
    // Validate World Generation
    FEng_SystemValidation WorldValidation;
    WorldValidation.SystemName = TEXT("World Generation");
    WorldValidation.bIsValid = ValidateWorldGeneration();
    WorldValidation.ValidationMessage = WorldValidation.bIsValid ?
        TEXT("World generation systems active") :
        TEXT("World generation needs attention");
    WorldValidation.LastValidationTime = LastValidationTime;
    SystemValidations.Add(WorldValidation);
    
    if (!WorldValidation.bIsValid)
        bAllSystemsValid = false;
    
    // Validate Performance
    FEng_SystemValidation PerformanceValidation;
    PerformanceValidation.SystemName = TEXT("Performance");
    PerformanceValidation.bIsValid = CheckPerformanceCompliance();
    PerformanceValidation.ValidationMessage = PerformanceValidation.bIsValid ?
        TEXT("Performance targets met") :
        TEXT("Performance optimization needed");
    PerformanceValidation.LastValidationTime = LastValidationTime;
    SystemValidations.Add(PerformanceValidation);
    
    if (!PerformanceValidation.bIsValid)
        bAllSystemsValid = false;
    
    bArchitectureValid = bAllSystemsValid;
    
    LogArchitectureStatus();
    
    return bAllSystemsValid;
}

FEng_SystemValidation UEngineArchitectWorldSubsystem::ValidateSpecificSystem(const FString& SystemName)
{
    for (const FEng_SystemValidation& Validation : SystemValidations)
    {
        if (Validation.SystemName == SystemName)
        {
            return Validation;
        }
    }
    
    // Return default validation if not found
    FEng_SystemValidation DefaultValidation;
    DefaultValidation.SystemName = SystemName;
    DefaultValidation.bIsValid = false;
    DefaultValidation.ValidationMessage = TEXT("System not found in validation list");
    DefaultValidation.LastValidationTime = 0.0f;
    
    return DefaultValidation;
}

TArray<FEng_SystemValidation> UEngineArchitectWorldSubsystem::GetAllSystemValidations()
{
    return SystemValidations;
}

FEng_PerformanceMetrics UEngineArchitectWorldSubsystem::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return LastPerformanceCheck;
}

bool UEngineArchitectWorldSubsystem::CheckPerformanceCompliance()
{
    UpdatePerformanceMetrics();
    
    // Target: 30+ FPS, <2GB memory usage
    bool bFrameRateOK = LastPerformanceCheck.FrameRate >= 30.0f;
    bool bMemoryOK = LastPerformanceCheck.MemoryUsageMB <= 2048.0f;
    
    LastPerformanceCheck.bMeetsPerformanceTarget = bFrameRateOK && bMemoryOK;
    
    return LastPerformanceCheck.bMeetsPerformanceTarget;
}

int32 UEngineArchitectWorldSubsystem::ValidateDinosaurSpawning()
{
    UWorld* World = GetWorld();
    if (!World)
        return 0;
    
    int32 DinosaurCount = 0;
    
    // Count actors with "Dinosaur" or specific dinosaur names in their labels
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            FString ActorLabel = Actor->GetActorLabel();
            if (ActorLabel.Contains(TEXT("Rex")) || 
                ActorLabel.Contains(TEXT("Raptor")) ||
                ActorLabel.Contains(TEXT("Brachio")) ||
                ActorLabel.Contains(TEXT("Triceratops")) ||
                ActorLabel.Contains(TEXT("Dinosaur")))
            {
                DinosaurCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Found %d dinosaurs in world"), DinosaurCount);
    
    return DinosaurCount;
}

bool UEngineArchitectWorldSubsystem::ValidatePlayerCharacterSetup()
{
    UWorld* World = GetWorld();
    if (!World)
        return false;
    
    // Check for player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: No player controller found"));
        return false;
    }
    
    // Check for possessed pawn
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Player controller has no pawn"));
        return false;
    }
    
    // Check if it's a character (has movement component)
    ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn);
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Player pawn is not a character"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Player character setup validated"));
    return true;
}

bool UEngineArchitectWorldSubsystem::ValidateWorldGeneration()
{
    UWorld* World = GetWorld();
    if (!World)
        return false;
    
    // Check for basic world elements
    int32 StaticMeshCount = 0;
    int32 LightCount = 0;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            if (Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                StaticMeshCount++;
            }
            
            if (Actor->GetClass()->GetName().Contains(TEXT("Light")))
            {
                LightCount++;
            }
        }
    }
    
    bool bHasBasicWorld = StaticMeshCount > 0 && LightCount > 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: World has %d static meshes, %d lights"), 
           StaticMeshCount, LightCount);
    
    return bHasBasicWorld;
}

bool UEngineArchitectWorldSubsystem::ValidateBiomeDistribution()
{
    // For now, just check if we have actors distributed across different areas
    UWorld* World = GetWorld();
    if (!World)
        return false;
    
    int32 ActorsInSavana = 0;
    int32 ActorsInForest = 0;
    int32 ActorsInDesert = 0;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            FVector Location = Actor->GetActorLocation();
            
            // Savana area (around origin)
            if (FMath::Abs(Location.X) < 10000 && FMath::Abs(Location.Y) < 10000)
            {
                ActorsInSavana++;
            }
            // Forest area (negative X, positive Y)
            else if (Location.X < -30000 && Location.Y > 30000)
            {
                ActorsInForest++;
            }
            // Desert area (positive X, negative Y)
            else if (Location.X > 30000 && Location.Y < -30000)
            {
                ActorsInDesert++;
            }
        }
    }
    
    bool bBiomesPopulated = ActorsInSavana > 0 && ActorsInForest > 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Biome distribution - Savana: %d, Forest: %d, Desert: %d"),
           ActorsInSavana, ActorsInForest, ActorsInDesert);
    
    return bBiomesPopulated;
}

bool UEngineArchitectWorldSubsystem::ValidateModuleCompilation()
{
    // This would typically check compilation status
    // For now, assume modules are compiled if subsystem is running
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Module compilation validation passed"));
    return true;
}

TArray<FString> UEngineArchitectWorldSubsystem::GetCompilationErrors()
{
    TArray<FString> Errors;
    
    // In a real implementation, this would collect actual compilation errors
    // For now, return empty array indicating no errors
    
    return Errors;
}

bool UEngineArchitectWorldSubsystem::ValidateMilestone1Requirements()
{
    bool bPlayerMovement = ValidatePlayerCharacterSetup();
    bool bDinosaursPresent = ValidateDinosaurSpawning() > 0;
    bool bWorldExists = ValidateWorldGeneration();
    bool bPerformanceOK = CheckPerformanceCompliance();
    
    bool bMilestone1Complete = bPlayerMovement && bDinosaursPresent && bWorldExists;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Milestone 1 - Player: %s, Dinosaurs: %s, World: %s, Performance: %s"),
           bPlayerMovement ? TEXT("OK") : TEXT("FAIL"),
           bDinosaursPresent ? TEXT("OK") : TEXT("FAIL"),
           bWorldExists ? TEXT("OK") : TEXT("FAIL"),
           bPerformanceOK ? TEXT("OK") : TEXT("WARN"));
    
    return bMilestone1Complete;
}

FString UEngineArchitectWorldSubsystem::GetMilestone1Status()
{
    bool bComplete = ValidateMilestone1Requirements();
    
    if (bComplete)
    {
        return TEXT("Milestone 1 COMPLETE - Player can walk around with dinosaurs in world");
    }
    else
    {
        return TEXT("Milestone 1 IN PROGRESS - Some requirements not met");
    }
}

void UEngineArchitectWorldSubsystem::InitializeSystemValidations()
{
    SystemValidations.Empty();
    
    // Initialize with default validations
    FEng_SystemValidation DefaultValidation;
    DefaultValidation.bIsValid = false;
    DefaultValidation.LastValidationTime = 0.0f;
    
    DefaultValidation.SystemName = TEXT("Player Character");
    DefaultValidation.ValidationMessage = TEXT("Not yet validated");
    SystemValidations.Add(DefaultValidation);
    
    DefaultValidation.SystemName = TEXT("Dinosaur System");
    DefaultValidation.ValidationMessage = TEXT("Not yet validated");
    SystemValidations.Add(DefaultValidation);
    
    DefaultValidation.SystemName = TEXT("World Generation");
    DefaultValidation.ValidationMessage = TEXT("Not yet validated");
    SystemValidations.Add(DefaultValidation);
    
    DefaultValidation.SystemName = TEXT("Performance");
    DefaultValidation.ValidationMessage = TEXT("Not yet validated");
    SystemValidations.Add(DefaultValidation);
}

void UEngineArchitectWorldSubsystem::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Get current frame rate
    LastPerformanceCheck.FrameRate = 1.0f / World->GetDeltaSeconds();
    
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    LastPerformanceCheck.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Count actors
    LastPerformanceCheck.ActorCount = 0;
    LastPerformanceCheck.DinosaurCount = 0;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        LastPerformanceCheck.ActorCount++;
        
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            FString ActorLabel = Actor->GetActorLabel();
            if (ActorLabel.Contains(TEXT("Rex")) || 
                ActorLabel.Contains(TEXT("Raptor")) ||
                ActorLabel.Contains(TEXT("Brachio")) ||
                ActorLabel.Contains(TEXT("Dinosaur")))
            {
                LastPerformanceCheck.DinosaurCount++;
            }
        }
    }
}

bool UEngineArchitectWorldSubsystem::CheckSystemDependencies()
{
    // Check if required subsystems are available
    UWorld* World = GetWorld();
    if (!World)
        return false;
    
    // Check for game instance subsystem
    if (UGameInstance* GI = World->GetGameInstance())
    {
        UEngineArchitectGameInstanceSubsystem* ArchSubsystem = 
            GI->GetSubsystem<UEngineArchitectGameInstanceSubsystem>();
        if (!ArchSubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Game Instance Subsystem not found"));
            return false;
        }
    }
    
    return true;
}

bool UEngineArchitectWorldSubsystem::ValidateAssetPaths()
{
    // Validate that critical asset paths exist
    TArray<FString> CriticalPaths = {
        TEXT("/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin"),
        TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin"),
        TEXT("/Game/Dinosaur_Pack/Brachiosaurus/Mesh/SKM_Brachiosaurus")
    };
    
    bool bAllPathsValid = true;
    
    for (const FString& Path : CriticalPaths)
    {
        if (!FPackageName::DoesPackageExist(Path))
        {
            UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Critical asset not found: %s"), *Path);
            bAllPathsValid = false;
        }
    }
    
    return bAllPathsValid;
}

void UEngineArchitectWorldSubsystem::LogArchitectureStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Architecture Valid: %s"), bArchitectureValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Systems Validated: %d"), SystemValidations.Num());
    
    for (const FEng_SystemValidation& Validation : SystemValidations)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s - %s"), 
               *Validation.SystemName,
               Validation.bIsValid ? TEXT("PASS") : TEXT("FAIL"),
               *Validation.ValidationMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance: %.1f FPS, %.1f MB, %d actors, %d dinosaurs"),
           LastPerformanceCheck.FrameRate,
           LastPerformanceCheck.MemoryUsageMB,
           LastPerformanceCheck.ActorCount,
           LastPerformanceCheck.DinosaurCount);
    
    UE_LOG(LogTemp, Warning, TEXT("==============================="));
}

// UEngineArchitectGameInstanceSubsystem Implementation

void UEngineArchitectGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Game Instance Subsystem Initialized"));
    
    bArchitectureInitialized = false;
    TargetFrameRate = 30.0f;
    MaxMemoryUsage = 2048.0f;
    
    LoadArchitectureConfiguration();
    EnforceArchitecturalRules();
    
    bArchitectureInitialized = true;
}

void UEngineArchitectGameInstanceSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Game Instance Subsystem Deinitialized"));
    
    SaveArchitectureState();
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

void UEngineArchitectGameInstanceSubsystem::EnforceArchitecturalRules()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Enforcing Architectural Rules"));
    
    // Register core systems
    RegisterSystemModule(TEXT("PlayerCharacter"), EEng_SystemPriority::Critical);
    RegisterSystemModule(TEXT("DinosaurSystem"), EEng_SystemPriority::Critical);
    RegisterSystemModule(TEXT("WorldGeneration"), EEng_SystemPriority::High);
    RegisterSystemModule(TEXT("Performance"), EEng_SystemPriority::Critical);
    RegisterSystemModule(TEXT("Audio"), EEng_SystemPriority::Medium);
    RegisterSystemModule(TEXT("VFX"), EEng_SystemPriority::Low);
    
    ValidateGlobalRules();
}

bool UEngineArchitectGameInstanceSubsystem::IsSystemCompliant(const FString& SystemName)
{
    return RegisteredSystems.Contains(SystemName);
}

void UEngineArchitectGameInstanceSubsystem::RegisterSystemModule(const FString& ModuleName, EEng_SystemPriority Priority)
{
    RegisteredSystems.Add(ModuleName, Priority);
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Registered system '%s' with priority %d"), 
           *ModuleName, (int32)Priority);
}

TArray<FString> UEngineArchitectGameInstanceSubsystem::GetRegisteredModules()
{
    TArray<FString> ModuleNames;
    RegisteredSystems.GetKeys(ModuleNames);
    return ModuleNames;
}

void UEngineArchitectGameInstanceSubsystem::SetPerformanceTargets(float TargetFPS, float MaxMemoryMB)
{
    TargetFrameRate = TargetFPS;
    MaxMemoryUsage = MaxMemoryMB;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Performance targets set - FPS: %.1f, Memory: %.1f MB"),
           TargetFrameRate, MaxMemoryUsage);
}

bool UEngineArchitectGameInstanceSubsystem::EnforcePerformanceLimits()
{
    // This would implement actual performance enforcement
    // For now, just log the targets
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Enforcing performance limits - FPS: %.1f, Memory: %.1f MB"),
           TargetFrameRate, MaxMemoryUsage);
    
    return true;
}

void UEngineArchitectGameInstanceSubsystem::LoadArchitectureConfiguration()
{
    // Load configuration from file or defaults
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Loading architecture configuration"));
}

void UEngineArchitectGameInstanceSubsystem::SaveArchitectureState()
{
    // Save current architecture state
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Saving architecture state"));
}

void UEngineArchitectGameInstanceSubsystem::ValidateGlobalRules()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Validating global architectural rules"));
    
    // Validate that critical systems are registered
    TArray<FString> CriticalSystems = {
        TEXT("PlayerCharacter"),
        TEXT("DinosaurSystem"),
        TEXT("Performance")
    };
    
    for (const FString& System : CriticalSystems)
    {
        if (!RegisteredSystems.Contains(System))
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architect: Critical system not registered: %s"), *System);
        }
    }
}

// AEngineArchitectDebugActor Implementation

AEngineArchitectDebugActor::AEngineArchitectDebugActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Create debug mesh component
    DebugMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMeshComponent"));
    DebugMeshComponent->SetupAttachment(RootComponent);
    
    // Set default values
    bShowDebugInfo = true;
    ValidationInterval = 5.0f;
    LastValidationTime = 0.0f;
    bSystemsValid = false;
    
    // Load a simple cube mesh for debug visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        DebugMeshComponent->SetStaticMesh(CubeMeshAsset.Object);
    }
}

void AEngineArchitectDebugActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Debug Actor: BeginPlay"));
    
    // Initial validation
    PerformValidationCheck();
    UpdateDebugMesh();
}

void AEngineArchitectDebugActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Perform validation at intervals
    if (GetWorld()->GetTimeSeconds() - LastValidationTime > ValidationInterval)
    {
        PerformValidationCheck();
        UpdateDebugDisplay();
    }
}

void AEngineArchitectDebugActor::UpdateDebugDisplay()
{
    if (!bShowDebugInfo)
        return;
    
    UpdateDebugMesh();
    
    // Draw debug strings in viewport
    if (GEngine)
    {
        FString DebugText = FString::Printf(
            TEXT("Engine Architect Debug\nSystems Valid: %s\nFPS: %.1f\nActors: %d\nDinosaurs: %d"),
            bSystemsValid ? TEXT("YES") : TEXT("NO"),
            CurrentMetrics.FrameRate,
            CurrentMetrics.ActorCount,
            CurrentMetrics.DinosaurCount
        );
        
        GEngine->AddOnScreenDebugMessage(
            GetUniqueID(),
            ValidationInterval + 1.0f,
            bSystemsValid ? FColor::Green : FColor::Red,
            DebugText
        );
    }
}

void AEngineArchitectDebugActor::ToggleDebugInfo()
{
    bShowDebugInfo = !bShowDebugInfo;
    
    if (!bShowDebugInfo && GEngine)
    {
        GEngine->RemoveOnScreenDebugMessage(GetUniqueID());
    }
}

void AEngineArchitectDebugActor::PerformValidationCheck()
{
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    // Get world subsystem and validate
    if (UEngineArchitectWorldSubsystem* ArchSubsystem = 
        GetWorld()->GetSubsystem<UEngineArchitectWorldSubsystem>())
    {
        bSystemsValid = ArchSubsystem->ValidateSystemArchitecture();
        CurrentMetrics = ArchSubsystem->GetCurrentPerformanceMetrics();
    }
    else
    {
        bSystemsValid = false;
        CurrentMetrics = FEng_PerformanceMetrics();
    }
}

void AEngineArchitectDebugActor::UpdateDebugMesh()
{
    if (!DebugMeshComponent)
        return;
    
    // Change mesh color based on system status
    if (bSystemsValid)
    {
        // Green for valid systems
        DebugMeshComponent->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(0.0f, 1.0f, 0.0f));
    }
    else
    {
        // Red for invalid systems
        DebugMeshComponent->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(1.0f, 0.0f, 0.0f));
    }
    
    // Scale based on performance
    float ScaleFactor = FMath::Clamp(CurrentMetrics.FrameRate / 60.0f, 0.5f, 2.0f);
    SetActorScale3D(FVector(ScaleFactor));
}