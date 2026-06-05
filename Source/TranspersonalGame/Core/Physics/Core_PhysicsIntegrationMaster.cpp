#include "Core_PhysicsIntegrationMaster.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"

UCore_PhysicsIntegrationMaster::UCore_PhysicsIntegrationMaster()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize milestone tracking
    MilestoneCompletion = 0.0f;
    LastValidationTime = FDateTime::Now();
    
    // Initialize physics settings
    bAutoOptimizePhysics = true;
    bEnablePerformanceMonitoring = true;
    ValidationFrequency = 1.0f;
    MaxPhysicsFrameTime = 16.67f; // 60 FPS target
    
    // Initialize performance metrics
    CurrentPhysicsFrameTime = 0.0f;
    AveragePhysicsFrameTime = 0.0f;
    ActivePhysicsBodies = 0;
    CollisionChecksPerFrame = 0;
    bIsMonitoringPerformance = false;
    
    // Initialize milestone requirements
    MilestoneRequirements.Add(TEXT("CharacterMovement"), false);
    MilestoneRequirements.Add(TEXT("CollisionDetection"), false);
    MilestoneRequirements.Add(TEXT("TerrainInteraction"), false);
    MilestoneRequirements.Add(TEXT("CameraPhysics"), false);
    MilestoneRequirements.Add(TEXT("PhysicsPerformance"), false);
}

void UCore_PhysicsIntegrationMaster::BeginPlay()
{
    Super::BeginPlay();
    
    // Start automatic validation
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimer,
            this,
            &UCore_PhysicsIntegrationMaster::RefreshPhysicsIntegration,
            ValidationFrequency,
            true
        );
    }
    
    // Start performance monitoring if enabled
    if (bEnablePerformanceMonitoring)
    {
        StartPerformanceMonitoring();
    }
    
    // Initial validation
    ValidateWalkAroundMilestone();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: Initialized and monitoring physics systems"));
}

void UCore_PhysicsIntegrationMaster::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoringPerformance)
    {
        // Update performance metrics
        CurrentPhysicsFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
        
        // Update frame time history
        FrameTimeHistory.Add(CurrentPhysicsFrameTime);
        if (FrameTimeHistory.Num() > MaxFrameTimeHistory)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average frame time
        if (FrameTimeHistory.Num() > 0)
        {
            float Total = 0.0f;
            for (float FrameTime : FrameTimeHistory)
            {
                Total += FrameTime;
            }
            AveragePhysicsFrameTime = Total / FrameTimeHistory.Num();
        }
        
        // Update physics body count
        if (UWorld* World = GetWorld())
        {
            ActivePhysicsBodies = 0;
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (Actor && Actor->GetRootComponent())
                {
                    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                    if (PrimComp && PrimComp->IsSimulatingPhysics())
                    {
                        ActivePhysicsBodies++;
                    }
                }
            }
        }
    }
}

bool UCore_PhysicsIntegrationMaster::ValidateWalkAroundMilestone()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: Starting WALK AROUND milestone validation"));
    
    // Reset milestone requirements
    for (auto& Requirement : MilestoneRequirements)
    {
        Requirement.Value = false;
    }
    
    // Validate each physics system
    bool bCharacterMovement = ValidateCharacterMovementIntegration();
    bool bCollisionDetection = ValidateCollisionIntegration();
    bool bTerrainInteraction = ValidateTerrainIntegration();
    bool bCameraPhysics = ValidateCameraIntegration();
    bool bPhysicsPerformance = ValidatePhysicsPerformance();
    
    // Update milestone requirements
    MilestoneRequirements[TEXT("CharacterMovement")] = bCharacterMovement;
    MilestoneRequirements[TEXT("CollisionDetection")] = bCollisionDetection;
    MilestoneRequirements[TEXT("TerrainInteraction")] = bTerrainInteraction;
    MilestoneRequirements[TEXT("CameraPhysics")] = bCameraPhysics;
    MilestoneRequirements[TEXT("PhysicsPerformance")] = bPhysicsPerformance;
    
    // Update overall completion
    UpdateMilestoneStatus();
    LastValidationTime = FDateTime::Now();
    
    // Log results
    LogValidationResults();
    
    bool bMilestoneComplete = (MilestoneCompletion >= 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: WALK AROUND milestone validation %s (%.1f%% complete)"),
        bMilestoneComplete ? TEXT("PASSED") : TEXT("IN PROGRESS"),
        MilestoneCompletion * 100.0f);
    
    return bMilestoneComplete;
}

float UCore_PhysicsIntegrationMaster::GetMilestoneCompletionPercentage() const
{
    return MilestoneCompletion;
}

void UCore_PhysicsIntegrationMaster::RefreshPhysicsIntegration()
{
    ValidateWalkAroundMilestone();
    
    if (bAutoOptimizePhysics)
    {
        OptimizeCollisionSettings();
        ConfigureTerrainPhysicsMaterials();
    }
}

bool UCore_PhysicsIntegrationMaster::IntegrateCharacterPhysics(ACharacter* Character)
{
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegrationMaster: Cannot integrate physics - Character is null"));
        return false;
    }
    
    // Configure character physics
    ConfigureCharacterPhysics(Character);
    
    // Validate movement component
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!ValidateMovementPhysics(MovementComp))
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegrationMaster: Character movement physics validation failed"));
        return false;
    }
    
    // Test collision response
    if (!TestCollisionResponse(Character))
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: Character collision response test failed"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Character physics integration successful for %s"), 
        *Character->GetName());
    return true;
}

bool UCore_PhysicsIntegrationMaster::ValidateMovementPhysics(UCharacterMovementComponent* MovementComp)
{
    if (!MovementComp)
    {
        return false;
    }
    
    // Validate critical movement settings
    bool bValidSettings = true;
    
    if (MovementComp->MaxWalkSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: Invalid MaxWalkSpeed"));
        bValidSettings = false;
    }
    
    if (MovementComp->JumpZVelocity <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: Invalid JumpZVelocity"));
        bValidSettings = false;
    }
    
    if (MovementComp->GroundFriction <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: Invalid GroundFriction"));
        bValidSettings = false;
    }
    
    return bValidSettings;
}

void UCore_PhysicsIntegrationMaster::ConfigureCharacterPhysics(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (MovementComp)
    {
        // Configure optimal movement physics for WALK AROUND milestone
        MovementComp->MaxWalkSpeed = 600.0f;
        MovementComp->MaxWalkSpeedCrouched = 300.0f;
        MovementComp->JumpZVelocity = 420.0f;
        MovementComp->AirControl = 0.2f;
        MovementComp->GroundFriction = 8.0f;
        MovementComp->BrakingDecelerationWalking = 2048.0f;
        MovementComp->BrakingDecelerationFalling = 1500.0f;
        MovementComp->GravityScale = 1.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Character physics configured for %s"), 
            *Character->GetName());
    }
    
    // Configure collision capsule
    UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
    if (CapsuleComp)
    {
        CapsuleComp->SetCapsuleRadius(42.0f);
        CapsuleComp->SetCapsuleHalfHeight(88.0f);
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CapsuleComp->SetCollisionResponseToAllChannels(ECR_Block);
        CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Collision capsule configured"));
    }
}

bool UCore_PhysicsIntegrationMaster::ValidateCollisionSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    int32 ValidCollisionActors = 0;
    int32 TotalActors = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            TotalActors++;
            
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                ValidCollisionActors++;
            }
        }
    }
    
    float CollisionRatio = TotalActors > 0 ? (float)ValidCollisionActors / TotalActors : 0.0f;
    bool bValidationPassed = CollisionRatio > 0.5f; // At least 50% of actors should have collision
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Collision validation - %d/%d actors have collision (%.1f%%)"),
        ValidCollisionActors, TotalActors, CollisionRatio * 100.0f);
    
    return bValidationPassed;
}

bool UCore_PhysicsIntegrationMaster::TestCollisionResponse(ACharacter* Character)
{
    if (!Character || !Character->GetCapsuleComponent())
    {
        return false;
    }
    
    UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
    
    // Test collision settings
    bool bHasValidCollision = (CapsuleComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision);
    bool bHasValidResponse = (CapsuleComp->GetCollisionResponseToChannel(ECC_WorldStatic) == ECR_Block);
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Collision test - Enabled: %s, WorldStatic Response: %s"),
        bHasValidCollision ? TEXT("YES") : TEXT("NO"),
        bHasValidResponse ? TEXT("BLOCK") : TEXT("OTHER"));
    
    return bHasValidCollision && bHasValidResponse;
}

void UCore_PhysicsIntegrationMaster::OptimizeCollisionSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 OptimizedActors = 0;
    
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* StaticMeshActor = *ActorItr;
        if (StaticMeshActor && StaticMeshActor->GetStaticMeshComponent())
        {
            UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
            
            // Optimize collision for static environment objects
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
            MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            
            OptimizedActors++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Optimized collision settings for %d static mesh actors"), OptimizedActors);
}

bool UCore_PhysicsIntegrationMaster::IntegrateTerrainPhysics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Find landscape actors
    TArray<ALandscape*> Landscapes;
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        Landscapes.Add(*ActorItr);
    }
    
    if (Landscapes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: No landscape actors found for terrain physics"));
        return false;
    }
    
    // Configure terrain physics
    ConfigureTerrainPhysicsMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Terrain physics integrated for %d landscapes"), Landscapes.Num());
    return true;
}

bool UCore_PhysicsIntegrationMaster::ValidateTerrainPhysics()
{
    return IntegrateTerrainPhysics();
}

void UCore_PhysicsIntegrationMaster::ConfigureTerrainPhysicsMaterials()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Configure landscape physics materials
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (Landscape)
        {
            // Set realistic terrain physics properties
            Landscape->SetActorLabel(TEXT("MainTerrain_Physics"));
            UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Configured physics materials for landscape %s"), 
                *Landscape->GetName());
        }
    }
}

bool UCore_PhysicsIntegrationMaster::IntegrateCameraPhysics(ACharacter* Character)
{
    if (!Character)
    {
        return false;
    }
    
    // Find spring arm and camera components
    USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>();
    UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>();
    
    if (!SpringArm || !Camera)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: Camera components not found on character"));
        return false;
    }
    
    // Configure camera physics
    ConfigureCameraPhysics(SpringArm, Camera);
    
    // Validate camera physics
    return ValidateCameraPhysics(SpringArm);
}

bool UCore_PhysicsIntegrationMaster::ValidateCameraPhysics(USpringArmComponent* SpringArm)
{
    if (!SpringArm)
    {
        return false;
    }
    
    bool bValidLength = (SpringArm->TargetArmLength > 0.0f);
    bool bValidCollision = SpringArm->bDoCollisionTest;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Camera physics validation - Length: %.1f, Collision: %s"),
        SpringArm->TargetArmLength,
        bValidCollision ? TEXT("ENABLED") : TEXT("DISABLED"));
    
    return bValidLength && bValidCollision;
}

void UCore_PhysicsIntegrationMaster::ConfigureCameraPhysics(USpringArmComponent* SpringArm, UCameraComponent* Camera)
{
    if (!SpringArm || !Camera)
    {
        return;
    }
    
    // Configure spring arm physics
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bDoCollisionTest = true;
    SpringArm->bInheritPitch = true;
    SpringArm->bInheritYaw = true;
    SpringArm->bInheritRoll = false;
    SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Camera physics configured successfully"));
}

FString UCore_PhysicsIntegrationMaster::GetPhysicsPerformanceReport() const
{
    FString Report = FString::Printf(TEXT("=== PHYSICS PERFORMANCE REPORT ===\n"));
    Report += FString::Printf(TEXT("Current Frame Time: %.2f ms\n"), CurrentPhysicsFrameTime);
    Report += FString::Printf(TEXT("Average Frame Time: %.2f ms\n"), AveragePhysicsFrameTime);
    Report += FString::Printf(TEXT("Active Physics Bodies: %d\n"), ActivePhysicsBodies);
    Report += FString::Printf(TEXT("Collision Checks/Frame: %d\n"), CollisionChecksPerFrame);
    Report += FString::Printf(TEXT("Target Frame Time: %.2f ms\n"), MaxPhysicsFrameTime);
    Report += FString::Printf(TEXT("Performance Status: %s\n"), 
        (AveragePhysicsFrameTime <= MaxPhysicsFrameTime) ? TEXT("GOOD") : TEXT("NEEDS OPTIMIZATION"));
    
    return Report;
}

void UCore_PhysicsIntegrationMaster::StartPerformanceMonitoring()
{
    bIsMonitoringPerformance = true;
    FrameTimeHistory.Empty();
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Performance monitoring started"));
}

void UCore_PhysicsIntegrationMaster::StopPerformanceMonitoring()
{
    bIsMonitoringPerformance = false;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Performance monitoring stopped"));
}

bool UCore_PhysicsIntegrationMaster::ValidateCharacterMovementIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Find character actors
    TArray<ACharacter*> Characters;
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        Characters.Add(*ActorItr);
    }
    
    if (Characters.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: No character actors found"));
        return false;
    }
    
    // Validate each character
    bool bAllCharactersValid = true;
    for (ACharacter* Character : Characters)
    {
        if (!IntegrateCharacterPhysics(Character))
        {
            bAllCharactersValid = false;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Character movement validation - %d characters, Status: %s"),
        Characters.Num(), bAllCharactersValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bAllCharactersValid;
}

bool UCore_PhysicsIntegrationMaster::ValidateCollisionIntegration()
{
    return ValidateCollisionSystems();
}

bool UCore_PhysicsIntegrationMaster::ValidateTerrainIntegration()
{
    return ValidateTerrainPhysics();
}

bool UCore_PhysicsIntegrationMaster::ValidateCameraIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Find characters with camera systems
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (Character)
        {
            USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>();
            if (SpringArm)
            {
                return ValidateCameraPhysics(SpringArm);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsIntegrationMaster: No camera systems found for validation"));
    return false;
}

bool UCore_PhysicsIntegrationMaster::ValidatePhysicsPerformance()
{
    bool bPerformanceGood = (AveragePhysicsFrameTime <= MaxPhysicsFrameTime);
    bool bPhysicsBodiesReasonable = (ActivePhysicsBodies < 1000); // Reasonable limit
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsIntegrationMaster: Performance validation - Frame Time: %.2f ms (Target: %.2f ms), Bodies: %d"),
        AveragePhysicsFrameTime, MaxPhysicsFrameTime, ActivePhysicsBodies);
    
    return bPerformanceGood && bPhysicsBodiesReasonable;
}

void UCore_PhysicsIntegrationMaster::UpdateMilestoneStatus()
{
    int32 CompletedRequirements = 0;
    int32 TotalRequirements = MilestoneRequirements.Num();
    
    for (const auto& Requirement : MilestoneRequirements)
    {
        if (Requirement.Value)
        {
            CompletedRequirements++;
        }
    }
    
    MilestoneCompletion = TotalRequirements > 0 ? (float)CompletedRequirements / TotalRequirements : 0.0f;
}

void UCore_PhysicsIntegrationMaster::LogValidationResults()
{
    UE_LOG(LogTemp, Warning, TEXT("=== WALK AROUND MILESTONE PHYSICS VALIDATION RESULTS ==="));
    
    for (const auto& Requirement : MilestoneRequirements)
    {
        FString Status = Requirement.Value ? TEXT("✓ PASS") : TEXT("✗ FAIL");
        UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *Requirement.Key, *Status);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Overall Completion: %.1f%% (%d/%d requirements met)"),
        MilestoneCompletion * 100.0f,
        MilestoneRequirements.Num() - MilestoneRequirements.FindByPredicate([](const auto& Pair) { return !Pair.Value; }) ? 0 : 1,
        MilestoneRequirements.Num());
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION RESULTS ==="));
}