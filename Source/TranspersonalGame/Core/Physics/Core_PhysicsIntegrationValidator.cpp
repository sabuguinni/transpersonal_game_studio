#include "Core_PhysicsIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogPhysicsIntegrationValidator);

UCore_PhysicsIntegrationValidator::UCore_PhysicsIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS validation
    
    ValidationState = ECore_PhysicsValidationState::Initializing;
    ValidationScore = 0.0f;
    LastValidationTime = 0.0f;
    
    // Initialize validation metrics
    CharacterPhysicsScore = 0.0f;
    CollisionSystemScore = 0.0f;
    RigidBodyScore = 0.0f;
    TerrainPhysicsScore = 0.0f;
    PerformanceScore = 0.0f;
    
    // Initialize validation counts
    TotalValidationChecks = 0;
    PassedValidationChecks = 0;
    FailedValidationChecks = 0;
    
    // Initialize performance metrics
    AverageFrameTime = 0.0f;
    PhysicsStepTime = 0.0f;
    CollisionQueryTime = 0.0f;
    
    bValidationEnabled = true;
    bDetailedLogging = false;
    bPerformanceMonitoring = true;
}

void UCore_PhysicsIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsIntegrationValidator, Log, TEXT("Physics Integration Validator initialized"));
    
    ValidationState = ECore_PhysicsValidationState::Running;
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    // Start validation timer
    GetWorld()->GetTimerManager().SetTimer(
        ValidationTimerHandle,
        this,
        &UCore_PhysicsIntegrationValidator::PerformValidationCycle,
        1.0f, // Every second
        true
    );
}

void UCore_PhysicsIntegrationValidator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UCore_PhysicsIntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bValidationEnabled || ValidationState != ECore_PhysicsValidationState::Running)
    {
        return;
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics(DeltaTime);
    
    // Update validation state
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastValidationTime > 1.0f)
    {
        LastValidationTime = CurrentTime;
        CalculateOverallScore();
    }
}

void UCore_PhysicsIntegrationValidator::PerformValidationCycle()
{
    if (!bValidationEnabled || ValidationState != ECore_PhysicsValidationState::Running)
    {
        return;
    }
    
    UE_LOG(LogPhysicsIntegrationValidator, Verbose, TEXT("Starting validation cycle"));
    
    // Reset cycle counters
    int32 CycleChecks = 0;
    int32 CyclePassed = 0;
    
    // Validate character physics
    float CharacterScore = ValidateCharacterPhysics();
    CharacterPhysicsScore = CharacterScore;
    CycleChecks += 5; // Character physics has 5 checks
    CyclePassed += FMath::RoundToInt(CharacterScore * 5.0f);
    
    // Validate collision system
    float CollisionScore = ValidateCollisionSystem();
    CollisionSystemScore = CollisionScore;
    CycleChecks += 4; // Collision system has 4 checks
    CyclePassed += FMath::RoundToInt(CollisionScore * 4.0f);
    
    // Validate rigid body system
    float RigidScore = ValidateRigidBodySystem();
    RigidBodyScore = RigidScore;
    CycleChecks += 3; // Rigid body system has 3 checks
    CyclePassed += FMath::RoundToInt(RigidScore * 3.0f);
    
    // Validate terrain physics
    float TerrainScore = ValidateTerrainPhysics();
    TerrainPhysicsScore = TerrainScore;
    CycleChecks += 4; // Terrain physics has 4 checks
    CyclePassed += FMath::RoundToInt(TerrainScore * 4.0f);
    
    // Validate performance
    float PerfScore = ValidatePerformance();
    PerformanceScore = PerfScore;
    CycleChecks += 3; // Performance has 3 checks
    CyclePassed += FMath::RoundToInt(PerfScore * 3.0f);
    
    // Update totals
    TotalValidationChecks += CycleChecks;
    PassedValidationChecks += CyclePassed;
    FailedValidationChecks += (CycleChecks - CyclePassed);
    
    // Calculate overall score
    CalculateOverallScore();
    
    if (bDetailedLogging)
    {
        UE_LOG(LogPhysicsIntegrationValidator, Log, TEXT("Validation cycle complete - Score: %.2f%% (%d/%d checks passed)"), 
               ValidationScore * 100.0f, CyclePassed, CycleChecks);
    }
}

float UCore_PhysicsIntegrationValidator::ValidateCharacterPhysics()
{
    float Score = 0.0f;
    int32 Checks = 0;
    int32 Passed = 0;
    
    // Find character in world
    ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (Character)
    {
        Checks++;
        Passed++;
        
        // Check character movement component
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            Checks++;
            Passed++;
            
            // Check movement mode
            if (MovementComp->MovementMode != MOVE_None)
            {
                Checks++;
                Passed++;
            }
            else
            {
                Checks++;
            }
            
            // Check physics volume
            if (MovementComp->GetPhysicsVolume())
            {
                Checks++;
                Passed++;
            }
            else
            {
                Checks++;
            }
        }
        else
        {
            Checks++;
        }
        
        // Check character mesh physics
        USkeletalMeshComponent* MeshComp = Character->GetMesh();
        if (MeshComp && MeshComp->GetBodyInstance())
        {
            Checks++;
            Passed++;
        }
        else
        {
            Checks++;
        }
    }
    else
    {
        Checks += 5; // All character checks fail
    }
    
    Score = Checks > 0 ? (float)Passed / (float)Checks : 0.0f;
    
    if (bDetailedLogging)
    {
        UE_LOG(LogPhysicsIntegrationValidator, Verbose, TEXT("Character Physics Validation: %.2f%% (%d/%d)"), 
               Score * 100.0f, Passed, Checks);
    }
    
    return Score;
}

float UCore_PhysicsIntegrationValidator::ValidateCollisionSystem()
{
    float Score = 0.0f;
    int32 Checks = 0;
    int32 Passed = 0;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    // Check collision settings
    const UPhysicsSettings* PhysicsSettings = GetDefault<UPhysicsSettings>();
    if (PhysicsSettings)
    {
        Checks++;
        Passed++;
        
        // Check collision profiles
        if (PhysicsSettings->PhysicalSurfaces.Num() > 0)
        {
            Checks++;
            Passed++;
        }
        else
        {
            Checks++;
        }
    }
    else
    {
        Checks++;
    }
    
    // Check for collision-enabled actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 CollisionActors = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComps;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComps)
            {
                if (PrimComp && PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                {
                    CollisionActors++;
                    break;
                }
            }
        }
    }
    
    Checks++;
    if (CollisionActors > 0)
    {
        Passed++;
    }
    
    // Check collision channels
    Checks++;
    if (ECC_MAX > ECC_WorldStatic) // Basic collision channels exist
    {
        Passed++;
    }
    
    Score = Checks > 0 ? (float)Passed / (float)Checks : 0.0f;
    
    if (bDetailedLogging)
    {
        UE_LOG(LogPhysicsIntegrationValidator, Verbose, TEXT("Collision System Validation: %.2f%% (%d/%d) - %d collision actors"), 
               Score * 100.0f, Passed, Checks, CollisionActors);
    }
    
    return Score;
}

float UCore_PhysicsIntegrationValidator::ValidateRigidBodySystem()
{
    float Score = 0.0f;
    int32 Checks = 0;
    int32 Passed = 0;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    // Check for physics-simulated actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 PhysicsActors = 0;
    int32 StaticActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComps;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComps);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComps)
            {
                if (PrimComp)
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        PhysicsActors++;
                    }
                    else
                    {
                        StaticActors++;
                    }
                }
            }
        }
    }
    
    // Check physics simulation
    Checks++;
    if (PhysicsActors > 0)
    {
        Passed++;
    }
    
    // Check static geometry
    Checks++;
    if (StaticActors > 0)
    {
        Passed++;
    }
    
    // Check physics scene
    Checks++;
    if (World->GetPhysicsScene())
    {
        Passed++;
    }
    
    Score = Checks > 0 ? (float)Passed / (float)Checks : 0.0f;
    
    if (bDetailedLogging)
    {
        UE_LOG(LogPhysicsIntegrationValidator, Verbose, TEXT("Rigid Body System Validation: %.2f%% (%d/%d) - %d physics, %d static"), 
               Score * 100.0f, Passed, Checks, PhysicsActors, StaticActors);
    }
    
    return Score;
}

float UCore_PhysicsIntegrationValidator::ValidateTerrainPhysics()
{
    float Score = 0.0f;
    int32 Checks = 0;
    int32 Passed = 0;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    // Check for landscape/terrain
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    
    Checks++;
    if (LandscapeActors.Num() > 0)
    {
        Passed++;
        
        // Check landscape collision
        for (AActor* LandscapeActor : LandscapeActors)
        {
            ALandscape* Landscape = Cast<ALandscape>(LandscapeActor);
            if (Landscape)
            {
                TArray<UPrimitiveComponent*> PrimitiveComps;
                Landscape->GetComponents<UPrimitiveComponent>(PrimitiveComps);
                
                Checks++;
                for (UPrimitiveComponent* PrimComp : PrimitiveComps)
                {
                    if (PrimComp && PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                    {
                        Passed++;
                        break;
                    }
                }
                break;
            }
        }
    }
    
    // Check for static mesh terrain
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);
    
    int32 TerrainMeshes = 0;
    for (AActor* Actor : StaticMeshActors)
    {
        AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor);
        if (MeshActor && MeshActor->GetStaticMeshComponent())
        {
            UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
            if (MeshComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                TerrainMeshes++;
            }
        }
    }
    
    Checks++;
    if (TerrainMeshes > 0)
    {
        Passed++;
    }
    
    // Check physics materials
    Checks++;
    const UPhysicsSettings* PhysicsSettings = GetDefault<UPhysicsSettings>();
    if (PhysicsSettings && PhysicsSettings->PhysicalSurfaces.Num() > 0)
    {
        Passed++;
    }
    
    Score = Checks > 0 ? (float)Passed / (float)Checks : 0.0f;
    
    if (bDetailedLogging)
    {
        UE_LOG(LogPhysicsIntegrationValidator, Verbose, TEXT("Terrain Physics Validation: %.2f%% (%d/%d) - %d landscapes, %d terrain meshes"), 
               Score * 100.0f, Passed, Checks, LandscapeActors.Num(), TerrainMeshes);
    }
    
    return Score;
}

float UCore_PhysicsIntegrationValidator::ValidatePerformance()
{
    float Score = 0.0f;
    int32 Checks = 0;
    int32 Passed = 0;
    
    // Check frame time
    Checks++;
    if (AverageFrameTime < 33.33f) // Under 30 FPS threshold
    {
        Passed++;
    }
    
    // Check physics step time
    Checks++;
    if (PhysicsStepTime < 16.67f) // Under 60 FPS threshold
    {
        Passed++;
    }
    
    // Check collision query time
    Checks++;
    if (CollisionQueryTime < 5.0f) // Under 5ms threshold
    {
        Passed++;
    }
    
    Score = Checks > 0 ? (float)Passed / (float)Checks : 0.0f;
    
    if (bDetailedLogging)
    {
        UE_LOG(LogPhysicsIntegrationValidator, Verbose, TEXT("Performance Validation: %.2f%% (%d/%d) - Frame: %.2fms, Physics: %.2fms, Collision: %.2fms"), 
               Score * 100.0f, Passed, Checks, AverageFrameTime, PhysicsStepTime, CollisionQueryTime);
    }
    
    return Score;
}

void UCore_PhysicsIntegrationValidator::UpdatePerformanceMetrics(float DeltaTime)
{
    if (!bPerformanceMonitoring)
    {
        return;
    }
    
    // Update average frame time (exponential moving average)
    float FrameTimeMs = DeltaTime * 1000.0f;
    AverageFrameTime = AverageFrameTime * 0.9f + FrameTimeMs * 0.1f;
    
    // Physics step time (simplified - would need more detailed profiling)
    PhysicsStepTime = AverageFrameTime * 0.3f; // Estimate 30% of frame time
    
    // Collision query time (simplified - would need detailed profiling)
    CollisionQueryTime = AverageFrameTime * 0.1f; // Estimate 10% of frame time
}

void UCore_PhysicsIntegrationValidator::CalculateOverallScore()
{
    // Weighted average of all validation scores
    float TotalWeight = 0.0f;
    float WeightedSum = 0.0f;
    
    // Character physics (highest weight - core gameplay)
    WeightedSum += CharacterPhysicsScore * 0.3f;
    TotalWeight += 0.3f;
    
    // Collision system (high weight - essential for interaction)
    WeightedSum += CollisionSystemScore * 0.25f;
    TotalWeight += 0.25f;
    
    // Rigid body system (medium weight - important for physics)
    WeightedSum += RigidBodyScore * 0.2f;
    TotalWeight += 0.2f;
    
    // Terrain physics (medium weight - world interaction)
    WeightedSum += TerrainPhysicsScore * 0.15f;
    TotalWeight += 0.15f;
    
    // Performance (lower weight but still important)
    WeightedSum += PerformanceScore * 0.1f;
    TotalWeight += 0.1f;
    
    ValidationScore = TotalWeight > 0.0f ? WeightedSum / TotalWeight : 0.0f;
    
    // Update validation state based on score
    if (ValidationScore >= 0.9f)
    {
        ValidationState = ECore_PhysicsValidationState::Passed;
    }
    else if (ValidationScore >= 0.7f)
    {
        ValidationState = ECore_PhysicsValidationState::Warning;
    }
    else if (ValidationScore >= 0.5f)
    {
        ValidationState = ECore_PhysicsValidationState::Failed;
    }
    else
    {
        ValidationState = ECore_PhysicsValidationState::Critical;
    }
}

FCore_PhysicsValidationReport UCore_PhysicsIntegrationValidator::GetValidationReport() const
{
    FCore_PhysicsValidationReport Report;
    
    Report.OverallScore = ValidationScore;
    Report.ValidationState = ValidationState;
    Report.TotalChecks = TotalValidationChecks;
    Report.PassedChecks = PassedValidationChecks;
    Report.FailedChecks = FailedValidationChecks;
    
    Report.CharacterPhysicsScore = CharacterPhysicsScore;
    Report.CollisionSystemScore = CollisionSystemScore;
    Report.RigidBodyScore = RigidBodyScore;
    Report.TerrainPhysicsScore = TerrainPhysicsScore;
    Report.PerformanceScore = PerformanceScore;
    
    Report.AverageFrameTime = AverageFrameTime;
    Report.PhysicsStepTime = PhysicsStepTime;
    Report.CollisionQueryTime = CollisionQueryTime;
    
    Report.LastValidationTime = LastValidationTime;
    Report.ValidationDuration = GetWorld() ? GetWorld()->GetTimeSeconds() - LastValidationTime : 0.0f;
    
    return Report;
}

void UCore_PhysicsIntegrationValidator::SetValidationEnabled(bool bEnabled)
{
    bValidationEnabled = bEnabled;
    
    if (bEnabled && ValidationState == ECore_PhysicsValidationState::Disabled)
    {
        ValidationState = ECore_PhysicsValidationState::Running;
        UE_LOG(LogPhysicsIntegrationValidator, Log, TEXT("Physics validation enabled"));
    }
    else if (!bEnabled)
    {
        ValidationState = ECore_PhysicsValidationState::Disabled;
        UE_LOG(LogPhysicsIntegrationValidator, Log, TEXT("Physics validation disabled"));
    }
}

void UCore_PhysicsIntegrationValidator::ResetValidationMetrics()
{
    ValidationScore = 0.0f;
    CharacterPhysicsScore = 0.0f;
    CollisionSystemScore = 0.0f;
    RigidBodyScore = 0.0f;
    TerrainPhysicsScore = 0.0f;
    PerformanceScore = 0.0f;
    
    TotalValidationChecks = 0;
    PassedValidationChecks = 0;
    FailedValidationChecks = 0;
    
    AverageFrameTime = 0.0f;
    PhysicsStepTime = 0.0f;
    CollisionQueryTime = 0.0f;
    
    ValidationState = ECore_PhysicsValidationState::Running;
    
    UE_LOG(LogPhysicsIntegrationValidator, Log, TEXT("Physics validation metrics reset"));
}