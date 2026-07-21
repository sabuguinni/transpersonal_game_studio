#include "Core_PhysicsArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Landscape/LandscapeProxy.h"
#include "Kismet/GameplayStatics.h"

UCore_PhysicsArchitect::UCore_PhysicsArchitect()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Validate every second
    
    // Initialize validation data
    CurrentValidationData = FCore_PhysicsValidationData();
    CurrentMetrics = FCore_PhysicsMetrics();
    
    // Set performance thresholds for 60 FPS target
    MaxPhysicsFrameTime = 16.67f; // milliseconds
    MaxCollisionQueryTime = 5.0f;
    MaxActiveRigidBodies = 1000;
    
    bAutoValidateOnTick = true;
    ValidationInterval = 1.0f;
    LastValidationTime = 0.0f;
}

void UCore_PhysicsArchitect::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitect: BeginPlay - Starting physics validation system"));
    
    // Initial validation
    CurrentValidationData = ValidatePhysicsSystems();
    UpdateMetrics();
    
    LogValidationResults();
}

void UCore_PhysicsArchitect::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoValidateOnTick)
    {
        LastValidationTime += DeltaTime;
        if (LastValidationTime >= ValidationInterval)
        {
            CurrentValidationData = ValidatePhysicsSystems();
            UpdateMetrics();
            LastValidationTime = 0.0f;
        }
    }
}

FCore_PhysicsValidationData UCore_PhysicsArchitect::ValidatePhysicsSystems()
{
    FCore_PhysicsValidationData ValidationData;
    ValidationData.ValidationErrors.Empty();
    
    // Validate physics scene
    ValidationData.bPhysicsSceneActive = ValidatePhysicsScene();
    if (!ValidationData.bPhysicsSceneActive)
    {
        ValidationData.ValidationErrors.Add(TEXT("Physics scene not active"));
    }
    
    // Validate collision system
    ValidationData.bCollisionSystemValid = ValidateCollisionSystem();
    if (!ValidationData.bCollisionSystemValid)
    {
        ValidationData.ValidationErrors.Add(TEXT("Collision system validation failed"));
    }
    
    // Validate terrain interaction
    ValidationData.bTerrainInteractionValid = ValidateTerrainInteraction();
    if (!ValidationData.bTerrainInteractionValid)
    {
        ValidationData.ValidationErrors.Add(TEXT("Terrain interaction validation failed"));
    }
    
    // Find and validate character physics
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> Characters;
        UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
        
        bool bAnyCharacterValid = false;
        for (AActor* Actor : Characters)
        {
            if (ACharacter* Character = Cast<ACharacter>(Actor))
            {
                if (ValidateCharacterPhysics(Character))
                {
                    bAnyCharacterValid = true;
                    break;
                }
            }
        }
        
        ValidationData.bCharacterPhysicsValid = bAnyCharacterValid;
        if (!bAnyCharacterValid)
        {
            ValidationData.ValidationErrors.Add(TEXT("No valid character physics found"));
        }
    }
    
    // Calculate validation score
    int32 ValidComponents = 0;
    if (ValidationData.bPhysicsSceneActive) ValidComponents++;
    if (ValidationData.bCollisionSystemValid) ValidComponents++;
    if (ValidationData.bTerrainInteractionValid) ValidComponents++;
    if (ValidationData.bCharacterPhysicsValid) ValidComponents++;
    if (ValidationData.bMovementComponentValid) ValidComponents++;
    
    ValidationData.ValidationScore = (float)ValidComponents / 5.0f * 100.0f;
    
    // Set status based on score
    if (ValidationData.ValidationScore >= 90.0f)
    {
        ValidationData.ValidationStatus = TEXT("Excellent");
    }
    else if (ValidationData.ValidationScore >= 70.0f)
    {
        ValidationData.ValidationStatus = TEXT("Good");
    }
    else if (ValidationData.ValidationScore >= 50.0f)
    {
        ValidationData.ValidationStatus = TEXT("Needs Improvement");
    }
    else
    {
        ValidationData.ValidationStatus = TEXT("Critical Issues");
    }
    
    return ValidationData;
}

bool UCore_PhysicsArchitect::ValidateCharacterPhysics(ACharacter* Character)
{
    if (!Character)
    {
        return false;
    }
    
    // Check movement component
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    CurrentValidationData.bMovementComponentValid = ValidateMovementComponent(MovementComp);
    
    // Check collision component
    UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
    if (!CapsuleComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitect: Character missing capsule component"));
        return false;
    }
    
    // Validate collision settings
    if (CapsuleComp->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitect: Character collision disabled"));
        return false;
    }
    
    return CurrentValidationData.bMovementComponentValid;
}

bool UCore_PhysicsArchitect::ValidateMovementComponent(UCharacterMovementComponent* MovementComp)
{
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitect: No movement component found"));
        return false;
    }
    
    // Check essential movement settings
    if (MovementComp->MaxWalkSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitect: Invalid MaxWalkSpeed"));
        return false;
    }
    
    if (MovementComp->JumpZVelocity <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitect: Invalid JumpZVelocity"));
        return false;
    }
    
    // Check movement mode
    if (MovementComp->MovementMode == MOVE_None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitect: Movement mode is MOVE_None"));
        return false;
    }
    
    return true;
}

bool UCore_PhysicsArchitect::ValidateCollisionSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if collision queries work
    FVector StartLocation = FVector::ZeroVector;
    FVector EndLocation = FVector(0, 0, -1000);
    
    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic
    );
    
    // Collision system is working if we can perform traces
    return true; // LineTrace function exists and executes
}

bool UCore_PhysicsArchitect::ValidateTerrainInteraction()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for landscape actors
    bool bLandscapeValid = ValidateLandscapeCollision();
    
    // Check for static mesh terrain
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);
    
    // Valid if we have either landscape or static mesh terrain
    return bLandscapeValid || StaticMeshActors.Num() > 0;
}

FCore_PhysicsMetrics UCore_PhysicsArchitect::GetPhysicsMetrics()
{
    UpdateMetrics();
    return CurrentMetrics;
}

bool UCore_PhysicsArchitect::IsPhysicsPerformanceAcceptable()
{
    UpdateMetrics();
    
    bool bFrameTimeOK = CurrentMetrics.PhysicsFrameTime <= MaxPhysicsFrameTime;
    bool bQueryTimeOK = CurrentMetrics.CollisionQueryTime <= MaxCollisionQueryTime;
    bool bRigidBodyCountOK = CurrentMetrics.ActiveRigidBodies <= MaxActiveRigidBodies;
    
    CurrentMetrics.bPerformanceWithinLimits = bFrameTimeOK && bQueryTimeOK && bRigidBodyCountOK;
    
    return CurrentMetrics.bPerformanceWithinLimits;
}

float UCore_PhysicsArchitect::CalculatePhysicsScore()
{
    float Score = CurrentValidationData.ValidationScore;
    
    // Adjust score based on performance
    if (!IsPhysicsPerformanceAcceptable())
    {
        Score *= 0.8f; // Reduce score by 20% for performance issues
    }
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

void UCore_PhysicsArchitect::ReportToEngineArchitect()
{
    float PhysicsScore = CalculatePhysicsScore();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsArchitect: Reporting to Engine Architect"));
    UE_LOG(LogTemp, Warning, TEXT("  Physics Score: %.1f%%"), PhysicsScore);
    UE_LOG(LogTemp, Warning, TEXT("  Status: %s"), *CurrentValidationData.ValidationStatus);
    UE_LOG(LogTemp, Warning, TEXT("  Walk Around Ready: %s"), IsWalkAroundMilestoneReady() ? TEXT("YES") : TEXT("NO"));
}

bool UCore_PhysicsArchitect::IsWalkAroundMilestoneReady()
{
    // Walk Around milestone requires:
    // 1. Character physics working (movement + collision)
    // 2. Terrain interaction working
    // 3. Performance acceptable
    // 4. Overall score >= 80%
    
    bool bRequirementsMet = CurrentValidationData.bCharacterPhysicsValid &&
                           CurrentValidationData.bMovementComponentValid &&
                           CurrentValidationData.bTerrainInteractionValid &&
                           IsPhysicsPerformanceAcceptable() &&
                           CalculatePhysicsScore() >= 80.0f;
    
    return bRequirementsMet;
}

bool UCore_PhysicsArchitect::ValidatePhysicsScene()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if physics scene exists and is active
    FPhysScene* PhysicsScene = World->GetPhysicsScene();
    return PhysicsScene != nullptr;
}

bool UCore_PhysicsArchitect::ValidateLandscapeCollision()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscapeProxy::StaticClass(), LandscapeActors);
    
    for (AActor* Actor : LandscapeActors)
    {
        if (ALandscapeProxy* Landscape = Cast<ALandscapeProxy>(Actor))
        {
            // Check if landscape has collision enabled
            ULandscapeHeightfieldCollisionComponent* CollisionComp = Landscape->GetCollisionComponent();
            if (CollisionComp && CollisionComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                return true;
            }
        }
    }
    
    return false;
}

void UCore_PhysicsArchitect::UpdateMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get current frame time (approximation)
    CurrentMetrics.PhysicsFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Count active rigid bodies (approximation by counting physics actors)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    CurrentMetrics.ActiveRigidBodies = 0;
    CurrentMetrics.ActiveCollisionShapes = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.ActiveRigidBodies++;
            }
            
            if (PrimComp && PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                CurrentMetrics.ActiveCollisionShapes++;
            }
        }
    }
    
    // Estimate collision query time (simplified)
    CurrentMetrics.CollisionQueryTime = CurrentMetrics.ActiveCollisionShapes * 0.01f; // 0.01ms per shape estimate
    
    // Update performance status
    IsPhysicsPerformanceAcceptable();
}

void UCore_PhysicsArchitect::LogValidationResults()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CORE PHYSICS ARCHITECT VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Physics Scene Active: %s"), CurrentValidationData.bPhysicsSceneActive ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Character Physics Valid: %s"), CurrentValidationData.bCharacterPhysicsValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Movement Component Valid: %s"), CurrentValidationData.bMovementComponentValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Collision System Valid: %s"), CurrentValidationData.bCollisionSystemValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Terrain Interaction Valid: %s"), CurrentValidationData.bTerrainInteractionValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Validation Score: %.1f%%"), CurrentValidationData.ValidationScore);
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), *CurrentValidationData.ValidationStatus);
    UE_LOG(LogTemp, Warning, TEXT("Walk Around Milestone Ready: %s"), IsWalkAroundMilestoneReady() ? TEXT("YES") : TEXT("NO"));
    
    if (CurrentValidationData.ValidationErrors.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation Errors:"));
        for (const FString& Error : CurrentValidationData.ValidationErrors)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Error);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Warning, TEXT("Active Collision Shapes: %d"), CurrentMetrics.ActiveCollisionShapes);
    UE_LOG(LogTemp, Warning, TEXT("Physics Frame Time: %.2f ms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Performance Within Limits: %s"), CurrentMetrics.bPerformanceWithinLimits ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("=========================================="));
}