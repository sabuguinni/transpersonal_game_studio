#include "Core_MilestonePhysics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UCore_MilestonePhysics::UCore_MilestonePhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz validation
    
    ValidationInterval = 1.0f;
    TargetFPS = 60.0f;
    MinTerrainHeightVariation = 100.0f;
    MinDinosaurMeshes = 3;
    LastValidationTime = 0.0f;
}

void UCore_MilestonePhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache world references for efficient validation
    CacheWorldReferences();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_MilestonePhysics: Component initialized for WALK AROUND milestone validation"));
}

void UCore_MilestonePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update validation data at specified interval
    if (CurrentTime - LastValidationTime >= ValidationInterval)
    {
        UpdateMovementValidation();
        UpdateTerrainPhysicsData();
        UpdatePhysicsPerformance();
        
        LastValidationTime = CurrentTime;
        
        // Log milestone progress for debugging
        float Progress = GetMilestonePhysicsProgress();
        UE_LOG(LogTemp, Log, TEXT("Milestone Physics Progress: %.1f%%"), Progress * 100.0f);
    }
}

bool UCore_MilestonePhysics::ValidateCharacterMovement(ACharacter* Character)
{
    if (!Character)
    {
        if (!FindCharacterInWorld())
        {
            return false;
        }
        Character = CachedCharacter.Get();
    }
    
    if (!Character)
    {
        return false;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
    {
        return false;
    }
    
    // Check movement component configuration
    bool bMovementValid = true;
    
    // Validate movement speeds
    if (MovementComp->MaxWalkSpeed < 100.0f)
    {
        bMovementValid = false;
        UE_LOG(LogTemp, Warning, TEXT("Character MaxWalkSpeed too low: %.1f"), MovementComp->MaxWalkSpeed);
    }
    
    // Check camera components
    UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>();
    USpringArmComponent* SpringArm = Character->FindComponentByClass<USpringArmComponent>();
    
    if (!Camera || !SpringArm)
    {
        bMovementValid = false;
        UE_LOG(LogTemp, Warning, TEXT("Character missing Camera or SpringArm component"));
    }
    
    // Check input bindings (basic validation)
    UInputComponent* InputComp = Character->InputComponent;
    if (!InputComp)
    {
        bMovementValid = false;
        UE_LOG(LogTemp, Warning, TEXT("Character missing InputComponent"));
    }
    
    return bMovementValid;
}

bool UCore_MilestonePhysics::ValidateTerrainPhysics(const FVector& CharacterLocation)
{
    if (!FindLandscapeInWorld())
    {
        return false;
    }
    
    ALandscape* Landscape = CachedLandscape.Get();
    if (!Landscape)
    {
        return false;
    }
    
    // Calculate terrain height variation around character
    float HeightVariation = CalculateTerrainHeightVariation(CharacterLocation, 1000.0f);
    
    // Update terrain physics data
    TerrainPhysicsData.bLandscapeDetected = true;
    TerrainPhysicsData.TerrainHeightVariation = HeightVariation;
    
    // Check if terrain has sufficient height variation for milestone
    bool bTerrainValid = HeightVariation >= MinTerrainHeightVariation;
    
    if (!bTerrainValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain height variation insufficient: %.1f (min: %.1f)"), 
               HeightVariation, MinTerrainHeightVariation);
    }
    
    return bTerrainValid;
}

bool UCore_MilestonePhysics::ValidateDinosaurCollision()
{
    if (!FindDinosaurMeshes())
    {
        return false;
    }
    
    int32 ValidMeshCount = 0;
    
    for (const TWeakObjectPtr<AStaticMeshActor>& MeshPtr : CachedDinosaurMeshes)
    {
        AStaticMeshActor* MeshActor = MeshPtr.Get();
        if (MeshActor && IsValidDinosaurMesh(MeshActor))
        {
            ValidMeshCount++;
        }
    }
    
    bool bCollisionValid = ValidMeshCount >= MinDinosaurMeshes;
    
    if (!bCollisionValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient valid dinosaur meshes: %d (min: %d)"), 
               ValidMeshCount, MinDinosaurMeshes);
    }
    
    return bCollisionValid;
}

bool UCore_MilestonePhysics::ValidatePhysicsPerformance()
{
    // Get current FPS
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    PhysicsPerformance.CurrentFPS = CurrentFPS;
    
    // Check if performance target is met
    bool bPerformanceValid = CurrentFPS >= (TargetFPS * 0.9f); // 90% of target
    PhysicsPerformance.bPerformanceTargetMet = bPerformanceValid;
    
    if (!bPerformanceValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics performance below target: %.1f FPS (target: %.1f)"), 
               CurrentFPS, TargetFPS);
    }
    
    return bPerformanceValid;
}

float UCore_MilestonePhysics::GetMilestonePhysicsProgress()
{
    float TotalScore = 0.0f;
    float MaxScore = 4.0f; // 4 validation categories
    
    // Character movement validation (25%)
    if (ValidateCharacterMovement(CachedCharacter.Get()))
    {
        TotalScore += 1.0f;
    }
    
    // Terrain physics validation (25%)
    FVector CharacterLocation = FVector::ZeroVector;
    if (CachedCharacter.IsValid())
    {
        CharacterLocation = CachedCharacter->GetActorLocation();
    }
    
    if (ValidateTerrainPhysics(CharacterLocation))
    {
        TotalScore += 1.0f;
    }
    
    // Dinosaur collision validation (25%)
    if (ValidateDinosaurCollision())
    {
        TotalScore += 1.0f;
    }
    
    // Physics performance validation (25%)
    if (ValidatePhysicsPerformance())
    {
        TotalScore += 1.0f;
    }
    
    return TotalScore / MaxScore;
}

bool UCore_MilestonePhysics::CheckMilestone1Requirements()
{
    float Progress = GetMilestonePhysicsProgress();
    return Progress >= 1.0f; // 100% completion required
}

FString UCore_MilestonePhysics::GetPhysicsStatusReport()
{
    FString Report = TEXT("=== MILESTONE 1 PHYSICS STATUS ===\n");
    
    // Character movement status
    bool bCharacterValid = ValidateCharacterMovement(CachedCharacter.Get());
    Report += FString::Printf(TEXT("Character Movement: %s\n"), 
                             bCharacterValid ? TEXT("✓ PASS") : TEXT("✗ FAIL"));
    
    if (MovementValidation.bWASDMovementActive)
    {
        Report += FString::Printf(TEXT("  - WASD Movement: ✓ Active (Speed: %.1f)\n"), 
                                 MovementValidation.MovementSpeed);
    }
    else
    {
        Report += TEXT("  - WASD Movement: ✗ Inactive\n");
    }
    
    if (MovementValidation.bCameraControlActive)
    {
        Report += TEXT("  - Camera Control: ✓ Active\n");
    }
    else
    {
        Report += TEXT("  - Camera Control: ✗ Inactive\n");
    }
    
    if (MovementValidation.bJumpFunctional)
    {
        Report += TEXT("  - Jump Function: ✓ Working\n");
    }
    else
    {
        Report += TEXT("  - Jump Function: ✗ Not working\n");
    }
    
    // Terrain physics status
    FVector CharacterLocation = CachedCharacter.IsValid() ? CachedCharacter->GetActorLocation() : FVector::ZeroVector;
    bool bTerrainValid = ValidateTerrainPhysics(CharacterLocation);
    Report += FString::Printf(TEXT("Terrain Physics: %s\n"), 
                             bTerrainValid ? TEXT("✓ PASS") : TEXT("✗ FAIL"));
    
    Report += FString::Printf(TEXT("  - Height Variation: %.1f (min: %.1f)\n"), 
                             TerrainPhysicsData.TerrainHeightVariation, MinTerrainHeightVariation);
    
    if (TerrainPhysicsData.bCharacterGrounded)
    {
        Report += TEXT("  - Character Grounded: ✓ Yes\n");
    }
    else
    {
        Report += TEXT("  - Character Grounded: ✗ No\n");
    }
    
    // Dinosaur collision status
    bool bDinosaurValid = ValidateDinosaurCollision();
    Report += FString::Printf(TEXT("Dinosaur Collision: %s\n"), 
                             bDinosaurValid ? TEXT("✓ PASS") : TEXT("✗ FAIL"));
    
    Report += FString::Printf(TEXT("  - Valid Meshes: %d (min: %d)\n"), 
                             CachedDinosaurMeshes.Num(), MinDinosaurMeshes);
    
    // Physics performance status
    bool bPerformanceValid = ValidatePhysicsPerformance();
    Report += FString::Printf(TEXT("Physics Performance: %s\n"), 
                             bPerformanceValid ? TEXT("✓ PASS") : TEXT("✗ FAIL"));
    
    Report += FString::Printf(TEXT("  - Current FPS: %.1f (target: %.1f)\n"), 
                             PhysicsPerformance.CurrentFPS, TargetFPS);
    
    // Overall progress
    float Progress = GetMilestonePhysicsProgress();
    Report += FString::Printf(TEXT("\nOVERALL PROGRESS: %.1f%%\n"), Progress * 100.0f);
    
    if (Progress >= 1.0f)
    {
        Report += TEXT("STATUS: ✓ MILESTONE 1 PHYSICS REQUIREMENTS MET\n");
    }
    else
    {
        Report += TEXT("STATUS: ✗ MILESTONE 1 PHYSICS REQUIREMENTS NOT MET\n");
    }
    
    return Report;
}

void UCore_MilestonePhysics::UpdateMovementValidation()
{
    if (!CachedCharacter.IsValid())
    {
        FindCharacterInWorld();
    }
    
    ACharacter* Character = CachedCharacter.Get();
    if (!Character)
    {
        MovementValidation = FCore_MovementValidation(); // Reset to defaults
        return;
    }
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (MovementComp)
    {
        // Check if character is currently moving
        FVector Velocity = MovementComp->Velocity;
        MovementValidation.MovementSpeed = Velocity.Size();
        MovementValidation.bWASDMovementActive = MovementValidation.MovementSpeed > 10.0f;
        
        // Check if character is grounded
        TerrainPhysicsData.bCharacterGrounded = MovementComp->IsMovingOnGround();
        
        // Basic jump validation (check if jump is possible)
        MovementValidation.bJumpFunctional = MovementComp->CanAttemptJump();
    }
    
    // Check camera control
    UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>();
    if (Camera)
    {
        FRotator CameraRotation = Camera->GetComponentRotation();
        MovementValidation.CameraYaw = CameraRotation.Yaw;
        MovementValidation.CameraPitch = CameraRotation.Pitch;
        MovementValidation.bCameraControlActive = true;
    }
    else
    {
        MovementValidation.bCameraControlActive = false;
    }
}

void UCore_MilestonePhysics::UpdateTerrainPhysicsData()
{
    if (!CachedCharacter.IsValid())
    {
        return;
    }
    
    FVector CharacterLocation = CachedCharacter->GetActorLocation();
    
    // Update ground normal
    TerrainPhysicsData.GroundNormal = GetGroundNormal(CharacterLocation);
    
    // Calculate current slope
    float DotProduct = FVector::DotProduct(TerrainPhysicsData.GroundNormal, FVector::UpVector);
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
    TerrainPhysicsData.CurrentSlope = SlopeAngle;
}

void UCore_MilestonePhysics::UpdatePhysicsPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update FPS
    PhysicsPerformance.CurrentFPS = 1.0f / World->GetDeltaSeconds();
    
    // Update physics tick time (approximate)
    PhysicsPerformance.PhysicsTickTime = World->GetDeltaSeconds();
    
    // Count active rigid bodies (approximate)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 RigidBodyCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                RigidBodyCount++;
            }
        }
    }
    
    PhysicsPerformance.ActiveRigidBodies = RigidBodyCount;
    PhysicsPerformance.bPerformanceTargetMet = PhysicsPerformance.CurrentFPS >= (TargetFPS * 0.9f);
}

void UCore_MilestonePhysics::CacheWorldReferences()
{
    FindCharacterInWorld();
    FindLandscapeInWorld();
    FindDinosaurMeshes();
}

bool UCore_MilestonePhysics::FindCharacterInWorld()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Look for TranspersonalCharacter first
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    for (AActor* Actor : Characters)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            CachedCharacter = Character;
            return true;
        }
    }
    
    // Fallback to any character
    if (Characters.Num() > 0)
    {
        CachedCharacter = Cast<ACharacter>(Characters[0]);
        return true;
    }
    
    return false;
}

bool UCore_MilestonePhysics::FindLandscapeInWorld()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Landscapes);
    
    if (Landscapes.Num() > 0)
    {
        CachedLandscape = Cast<ALandscape>(Landscapes[0]);
        return true;
    }
    
    return false;
}

bool UCore_MilestonePhysics::FindDinosaurMeshes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    CachedDinosaurMeshes.Empty();
    
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);
    
    for (AActor* Actor : StaticMeshActors)
    {
        AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor);
        if (MeshActor && IsValidDinosaurMesh(MeshActor))
        {
            CachedDinosaurMeshes.Add(MeshActor);
        }
    }
    
    return CachedDinosaurMeshes.Num() > 0;
}

float UCore_MilestonePhysics::CalculateTerrainHeightVariation(const FVector& Center, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    float MinHeight = FLT_MAX;
    float MaxHeight = -FLT_MAX;
    
    // Sample height at multiple points around the center
    int32 SampleCount = 16;
    for (int32 i = 0; i < SampleCount; i++)
    {
        float Angle = (2.0f * PI * i) / SampleCount;
        FVector SampleLocation = Center + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        
        // Perform line trace to find ground height
        FHitResult HitResult;
        FVector TraceStart = SampleLocation + FVector(0, 0, 1000.0f);
        FVector TraceEnd = SampleLocation - FVector(0, 0, 1000.0f);
        
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            float Height = HitResult.Location.Z;
            MinHeight = FMath::Min(MinHeight, Height);
            MaxHeight = FMath::Max(MaxHeight, Height);
        }
    }
    
    return (MinHeight != FLT_MAX && MaxHeight != -FLT_MAX) ? (MaxHeight - MinHeight) : 0.0f;
}

FVector UCore_MilestonePhysics::GetGroundNormal(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::UpVector;
    }
    
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0, 0, 100.0f);
    FVector TraceEnd = Location - FVector(0, 0, 200.0f);
    
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        return HitResult.Normal;
    }
    
    return FVector::UpVector;
}

bool UCore_MilestonePhysics::IsValidDinosaurMesh(AStaticMeshActor* MeshActor)
{
    if (!MeshActor)
    {
        return false;
    }
    
    FString ActorName = MeshActor->GetName();
    
    // Check if actor name suggests it's a dinosaur
    TArray<FString> DinosaurKeywords = {
        TEXT("Dinosaur"),
        TEXT("TRex"),
        TEXT("Raptor"),
        TEXT("Brachiosaurus"),
        TEXT("Triceratops"),
        TEXT("Stego"),
        TEXT("Dino")
    };
    
    for (const FString& Keyword : DinosaurKeywords)
    {
        if (ActorName.Contains(Keyword))
        {
            // Check if mesh has collision
            UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
            if (MeshComp && MeshComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                return true;
            }
        }
    }
    
    return false;
}