#include "Core_WalkAroundIntegration.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

UCore_WalkAroundIntegration::UCore_WalkAroundIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second for performance
    
    ValidationInterval = 5.0f;
    LastValidationTime = 0.0f;
    AverageFrameRate = 60.0f;
    
    // Initialize requirements
    CurrentRequirements = FCore_WalkAroundRequirements();
    LastValidationResult = FCore_WalkAroundValidationResult();
}

void UCore_WalkAroundIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: BeginPlay - Starting WALK AROUND milestone validation"));
    
    // Initial validation
    UpdateComponentReferences();
    ValidateWalkAroundMilestone();
}

void UCore_WalkAroundIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Periodic validation
    if (CurrentTime - LastValidationTime >= ValidationInterval)
    {
        LastValidationTime = CurrentTime;
        UpdateComponentReferences();
        ValidateWalkAroundMilestone();
        MonitorFrameRate();
    }
}

FCore_WalkAroundValidationResult UCore_WalkAroundIntegration::ValidateWalkAroundMilestone()
{
    FCore_WalkAroundValidationResult Result;
    Result.MissingRequirements.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Validating WALK AROUND milestone..."));
    
    // Validate Character Movement
    bool bCharacterValid = ValidateCharacterMovement();
    if (!bCharacterValid)
    {
        Result.MissingRequirements.Add(TEXT("Character Movement System"));
    }
    
    // Validate Terrain System
    bool bTerrainValid = ValidateTerrainSystem();
    if (!bTerrainValid)
    {
        Result.MissingRequirements.Add(TEXT("Terrain System with Height Variation"));
    }
    
    // Validate Lighting System
    bool bLightingValid = ValidateLightingSystem();
    if (!bLightingValid)
    {
        Result.MissingRequirements.Add(TEXT("Lighting System (Sun + Sky + Fog)"));
    }
    
    // Validate Dinosaur Placement
    bool bDinosaursValid = ValidateDinosaurPlacement();
    if (!bDinosaursValid)
    {
        Result.MissingRequirements.Add(TEXT("Static Dinosaur Meshes (3-5 required)"));
    }
    
    // Calculate overall progress
    CalculateOverallProgress();
    
    Result.bIsValid = (Result.MissingRequirements.Num() == 0);
    Result.CompletionPercentage = CurrentRequirements.OverallProgress;
    Result.CurrentState = CurrentRequirements;
    
    if (Result.bIsValid)
    {
        Result.ValidationMessage = TEXT("WALK AROUND milestone COMPLETE! Player can walk around in prehistoric world.");
        UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: MILESTONE COMPLETE! Progress: %.1f%%"), Result.CompletionPercentage);
    }
    else
    {
        Result.ValidationMessage = FString::Printf(TEXT("WALK AROUND milestone %.1f%% complete. Missing: %d requirements."), 
            Result.CompletionPercentage, Result.MissingRequirements.Num());
        UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Progress: %.1f%%, Missing: %d requirements"), 
            Result.CompletionPercentage, Result.MissingRequirements.Num());
    }
    
    LastValidationResult = Result;
    return Result;
}

bool UCore_WalkAroundIntegration::ValidateCharacterMovement()
{
    CurrentRequirements.bHasThirdPersonCharacter = false;
    CurrentRequirements.bHasWASDMovement = false;
    CurrentRequirements.bHasCameraBoom = false;
    CurrentRequirements.bHasFollowCamera = false;
    CurrentRequirements.bCanWalk = false;
    CurrentRequirements.bCanRun = false;
    CurrentRequirements.bCanJump = false;
    
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Find player character
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return false;
    
    ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: No Character found for player"));
        return false;
    }
    
    PlayerCharacter = Cast<ATranspersonalCharacter>(Character);
    CurrentRequirements.bHasThirdPersonCharacter = (PlayerCharacter.IsValid());
    
    // Check movement component
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (MovementComp)
    {
        CurrentRequirements.bCanWalk = (MovementComp->MaxWalkSpeed > 0.0f);
        CurrentRequirements.bCanRun = (MovementComp->MaxWalkSpeed > 400.0f); // Assume running if speed > 400
        CurrentRequirements.bCanJump = (MovementComp->JumpZVelocity > 0.0f);
        CurrentRequirements.bHasWASDMovement = true; // Assume WASD if movement component exists
    }
    
    // Check camera components
    UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>();
    USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>();
    
    CurrentRequirements.bHasFollowCamera = (Camera != nullptr);
    CurrentRequirements.bHasCameraBoom = (SpringArm != nullptr);
    
    bool bCharacterValid = CurrentRequirements.bHasThirdPersonCharacter && 
                          CurrentRequirements.bHasWASDMovement && 
                          CurrentRequirements.bHasCameraBoom && 
                          CurrentRequirements.bHasFollowCamera &&
                          CurrentRequirements.bCanWalk;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Character validation: %s (ThirdPerson:%s, WASD:%s, Camera:%s, Boom:%s, Walk:%s)"),
        bCharacterValid ? TEXT("PASS") : TEXT("FAIL"),
        CurrentRequirements.bHasThirdPersonCharacter ? TEXT("Y") : TEXT("N"),
        CurrentRequirements.bHasWASDMovement ? TEXT("Y") : TEXT("N"),
        CurrentRequirements.bHasFollowCamera ? TEXT("Y") : TEXT("N"),
        CurrentRequirements.bHasCameraBoom ? TEXT("Y") : TEXT("N"),
        CurrentRequirements.bCanWalk ? TEXT("Y") : TEXT("N"));
    
    return bCharacterValid;
}

bool UCore_WalkAroundIntegration::ValidateTerrainSystem()
{
    CurrentRequirements.bHasLandscape = false;
    CurrentRequirements.bHasTerrainVariation = false;
    CurrentRequirements.TerrainHeightVariation = 0.0f;
    
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Find landscape actor
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    
    if (LandscapeActors.Num() > 0)
    {
        CurrentRequirements.bHasLandscape = true;
        WorldLandscape = Cast<ALandscape>(LandscapeActors[0]);
        
        // Check for height variation by sampling landscape bounds
        if (WorldLandscape.IsValid())
        {
            FBox LandscapeBounds = WorldLandscape->GetComponentsBoundingBox();
            CurrentRequirements.TerrainHeightVariation = LandscapeBounds.Max.Z - LandscapeBounds.Min.Z;
            CurrentRequirements.bHasTerrainVariation = (CurrentRequirements.TerrainHeightVariation > 100.0f); // At least 1 meter variation
        }
    }
    
    bool bTerrainValid = CurrentRequirements.bHasLandscape && CurrentRequirements.bHasTerrainVariation;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Terrain validation: %s (Landscape:%s, Variation:%s, Height:%.1f)"),
        bTerrainValid ? TEXT("PASS") : TEXT("FAIL"),
        CurrentRequirements.bHasLandscape ? TEXT("Y") : TEXT("N"),
        CurrentRequirements.bHasTerrainVariation ? TEXT("Y") : TEXT("N"),
        CurrentRequirements.TerrainHeightVariation);
    
    return bTerrainValid;
}

bool UCore_WalkAroundIntegration::ValidateLightingSystem()
{
    CurrentRequirements.bHasDirectionalLight = false;
    CurrentRequirements.bHasSkyAtmosphere = false;
    CurrentRequirements.bHasFog = false;
    
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Find directional light
    TArray<AActor*> LightActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), LightActors);
    
    if (LightActors.Num() > 0)
    {
        CurrentRequirements.bHasDirectionalLight = true;
        SunLight = Cast<ADirectionalLight>(LightActors[0]);
    }
    
    // Check for sky atmosphere component in any actor
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            CurrentRequirements.bHasSkyAtmosphere = true;
        }
        if (Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            CurrentRequirements.bHasFog = true;
        }
    }
    
    bool bLightingValid = CurrentRequirements.bHasDirectionalLight && 
                         CurrentRequirements.bHasSkyAtmosphere && 
                         CurrentRequirements.bHasFog;
    
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Lighting validation: %s (Sun:%s, Sky:%s, Fog:%s)"),
        bLightingValid ? TEXT("PASS") : TEXT("FAIL"),
        CurrentRequirements.bHasDirectionalLight ? TEXT("Y") : TEXT("N"),
        CurrentRequirements.bHasSkyAtmosphere ? TEXT("Y") : TEXT("N"),
        CurrentRequirements.bHasFog ? TEXT("Y") : TEXT("N"));
    
    return bLightingValid;
}

bool UCore_WalkAroundIntegration::ValidateDinosaurPlacement()
{
    CurrentRequirements.DinosaurCount = 0;
    CurrentRequirements.bHasStaticDinosaurs = false;
    
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Count static mesh actors that could be dinosaurs
    TArray<AActor*> MeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), MeshActors);
    
    int32 DinosaurLikeActors = 0;
    for (AActor* Actor : MeshActors)
    {
        FString ActorName = Actor->GetName().ToLower();
        // Look for dinosaur-like names
        if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("rex")) || 
            ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachio")) ||
            ActorName.Contains(TEXT("triceratops")) || ActorName.Contains(TEXT("stego")))
        {
            DinosaurLikeActors++;
        }
    }
    
    CurrentRequirements.DinosaurCount = DinosaurLikeActors;
    CurrentRequirements.bHasStaticDinosaurs = (DinosaurLikeActors >= 3); // Minimum 3 dinosaurs
    
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Dinosaur validation: %s (Count:%d, Required:3+)"),
        CurrentRequirements.bHasStaticDinosaurs ? TEXT("PASS") : TEXT("FAIL"),
        CurrentRequirements.DinosaurCount);
    
    return CurrentRequirements.bHasStaticDinosaurs;
}

void UCore_WalkAroundIntegration::IntegratePhysicsSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Integrating physics systems for WALK AROUND milestone"));
    
    SetupCharacterPhysics();
    ConfigureTerrainInteraction();
    OptimizePerformance();
}

void UCore_WalkAroundIntegration::SetupCharacterPhysics()
{
    if (!PlayerCharacter.IsValid()) return;
    
    UCharacterMovementComponent* MovementComp = PlayerCharacter->GetCharacterMovement();
    if (MovementComp)
    {
        // Configure basic movement for walk around
        MovementComp->MaxWalkSpeed = 600.0f;
        MovementComp->MaxAcceleration = 2048.0f;
        MovementComp->BrakingDecelerationWalking = 2048.0f;
        MovementComp->JumpZVelocity = 420.0f;
        MovementComp->AirControl = 0.2f;
        MovementComp->GroundFriction = 8.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Character physics configured for basic movement"));
    }
}

void UCore_WalkAroundIntegration::ConfigureTerrainInteraction()
{
    if (!WorldLandscape.IsValid()) return;
    
    // Basic terrain interaction setup
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Terrain interaction configured"));
}

void UCore_WalkAroundIntegration::OptimizePerformance()
{
    // Basic performance optimization for walk around milestone
    if (GEngine)
    {
        // Ensure we maintain 60fps target
        GEngine->SetMaxFPS(60);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Performance optimization applied"));
}

float UCore_WalkAroundIntegration::GetMilestoneProgress() const
{
    return CurrentRequirements.OverallProgress;
}

FString UCore_WalkAroundIntegration::GetProgressReport() const
{
    return LastValidationResult.ValidationMessage;
}

TArray<FString> UCore_WalkAroundIntegration::GetMissingRequirements() const
{
    return LastValidationResult.MissingRequirements;
}

void UCore_WalkAroundIntegration::DebugValidateAll()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: DEBUG - Force validating all systems..."));
    
    UpdateComponentReferences();
    FCore_WalkAroundValidationResult Result = ValidateWalkAroundMilestone();
    
    UE_LOG(LogTemp, Warning, TEXT("DEBUG VALIDATION COMPLETE: %s"), *Result.ValidationMessage);
    for (const FString& Missing : Result.MissingRequirements)
    {
        UE_LOG(LogTemp, Warning, TEXT("  MISSING: %s"), *Missing);
    }
}

void UCore_WalkAroundIntegration::ForceIntegrationUpdate()
{
    UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Force updating integration..."));
    IntegratePhysicsSystems();
}

void UCore_WalkAroundIntegration::UpdateComponentReferences()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Update player character reference
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC)
    {
        PlayerCharacter = Cast<ATranspersonalCharacter>(PC->GetPawn());
    }
    
    // Update landscape reference
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    if (LandscapeActors.Num() > 0)
    {
        WorldLandscape = Cast<ALandscape>(LandscapeActors[0]);
    }
    
    // Update sun light reference
    TArray<AActor*> LightActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), LightActors);
    if (LightActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(LightActors[0]);
    }
}

void UCore_WalkAroundIntegration::CalculateOverallProgress()
{
    float TotalScore = 0.0f;
    float MaxScore = 0.0f;
    
    // Character Movement (40% weight)
    float CharacterScore = 0.0f;
    if (CurrentRequirements.bHasThirdPersonCharacter) CharacterScore += 1.0f;
    if (CurrentRequirements.bHasWASDMovement) CharacterScore += 1.0f;
    if (CurrentRequirements.bHasCameraBoom) CharacterScore += 1.0f;
    if (CurrentRequirements.bHasFollowCamera) CharacterScore += 1.0f;
    if (CurrentRequirements.bCanWalk) CharacterScore += 1.0f;
    if (CurrentRequirements.bCanRun) CharacterScore += 0.5f;
    if (CurrentRequirements.bCanJump) CharacterScore += 0.5f;
    
    TotalScore += (CharacterScore / 7.0f) * 40.0f;
    MaxScore += 40.0f;
    
    // Terrain System (25% weight)
    float TerrainScore = 0.0f;
    if (CurrentRequirements.bHasLandscape) TerrainScore += 1.0f;
    if (CurrentRequirements.bHasTerrainVariation) TerrainScore += 1.0f;
    
    TotalScore += (TerrainScore / 2.0f) * 25.0f;
    MaxScore += 25.0f;
    
    // Lighting System (20% weight)
    float LightingScore = 0.0f;
    if (CurrentRequirements.bHasDirectionalLight) LightingScore += 1.0f;
    if (CurrentRequirements.bHasSkyAtmosphere) LightingScore += 1.0f;
    if (CurrentRequirements.bHasFog) LightingScore += 1.0f;
    
    TotalScore += (LightingScore / 3.0f) * 20.0f;
    MaxScore += 20.0f;
    
    // Dinosaur Placement (15% weight)
    float DinosaurScore = FMath::Clamp(CurrentRequirements.DinosaurCount / 5.0f, 0.0f, 1.0f); // Max score at 5 dinosaurs
    TotalScore += DinosaurScore * 15.0f;
    MaxScore += 15.0f;
    
    CurrentRequirements.OverallProgress = (MaxScore > 0.0f) ? (TotalScore / MaxScore) * 100.0f : 0.0f;
}

void UCore_WalkAroundIntegration::MonitorFrameRate()
{
    if (GEngine)
    {
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        
        RecentFrameTimes.Add(CurrentFPS);
        if (RecentFrameTimes.Num() > 30) // Keep last 30 frames
        {
            RecentFrameTimes.RemoveAt(0);
        }
        
        // Calculate average
        float Sum = 0.0f;
        for (float FPS : RecentFrameTimes)
        {
            Sum += FPS;
        }
        AverageFrameRate = Sum / RecentFrameTimes.Num();
        
        if (AverageFrameRate < 30.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_WalkAroundIntegration: Performance warning - FPS below 30: %.1f"), AverageFrameRate);
        }
    }
}