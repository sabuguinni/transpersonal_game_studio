#include "Core_MovementIntegration.h"
#include "Core_TerrainPhysics.h"
#include "Core_CollisionSystem.h"
#include "Core_PhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalCharacter.h"

ACore_MovementIntegration::ACore_MovementIntegration()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize physics system references
    TerrainPhysics = nullptr;
    CollisionSystem = nullptr;
    PhysicsManager = nullptr;
    
    // Set default movement settings
    TerrainAdaptationSpeed = 5.0f;
    SlopeLimit = 45.0f;
    UnstableTerrainSpeedMultiplier = 0.6f;
    bEnableRealisticPhysics = true;
}

void ACore_MovementIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    // Find and connect to physics systems
    IntegrateWithPhysicsSystems();
    
    // Auto-register existing characters in the world
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> FoundCharacters;
        UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), FoundCharacters);
        
        for (AActor* Actor : FoundCharacters)
        {
            if (ACharacter* Character = Cast<ACharacter>(Actor))
            {
                RegisterCharacter(Character);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Core_MovementIntegration: BeginPlay - Registered %d characters"), TrackedCharacters.Num());
}

void ACore_MovementIntegration::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bEnableRealisticPhysics)
        return;
    
    // Update movement for all tracked characters
    for (ACharacter* Character : TrackedCharacters)
    {
        if (IsValid(Character))
        {
            UpdateCharacterMovement(Character, DeltaTime);
        }
    }
}

void ACore_MovementIntegration::RegisterCharacter(ACharacter* Character)
{
    if (IsValid(Character) && !TrackedCharacters.Contains(Character))
    {
        TrackedCharacters.Add(Character);
        UE_LOG(LogTemp, Log, TEXT("Core_MovementIntegration: Registered character %s"), *Character->GetName());
    }
}

void ACore_MovementIntegration::UnregisterCharacter(ACharacter* Character)
{
    if (TrackedCharacters.Contains(Character))
    {
        TrackedCharacters.Remove(Character);
        UE_LOG(LogTemp, Log, TEXT("Core_MovementIntegration: Unregistered character %s"), *Character->GetName());
    }
}

bool ACore_MovementIntegration::IsTerrainStable(const FVector& Location, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
        return true;
    
    // Perform sphere trace to check terrain stability
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 200);
    
    bool bHit = World->SweepSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );
    
    if (bHit)
    {
        // Check slope angle
        float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.Normal, FVector::UpVector)));
        return SlopeAngle <= SlopeLimit;
    }
    
    return false; // No ground found - unstable
}

float ACore_MovementIntegration::GetTerrainSlope(const FVector& Location)
{
    FVector TerrainNormal = GetTerrainNormal(Location);
    return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(TerrainNormal, FVector::UpVector)));
}

FVector ACore_MovementIntegration::GetTerrainNormal(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
        return FVector::UpVector;
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 200);
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    return bHit ? HitResult.Normal : FVector::UpVector;
}

void ACore_MovementIntegration::ApplyTerrainMovementModifier(ACharacter* Character, float DeltaTime)
{
    if (!IsValid(Character))
        return;
    
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
        return;
    
    FVector CharacterLocation = Character->GetActorLocation();
    
    // Check terrain stability
    bool bStableTerrain = IsTerrainStable(CharacterLocation);
    float TerrainSlope = GetTerrainSlope(CharacterLocation);
    
    // Apply movement modifiers based on terrain
    float SpeedMultiplier = 1.0f;
    
    if (!bStableTerrain)
    {
        SpeedMultiplier *= UnstableTerrainSpeedMultiplier;
        HandleUnstableTerrain(Character, CharacterLocation);
    }
    
    // Slope-based speed modification
    if (TerrainSlope > 15.0f)
    {
        float SlopeMultiplier = FMath::Lerp(1.0f, 0.5f, (TerrainSlope - 15.0f) / 30.0f);
        SpeedMultiplier *= SlopeMultiplier;
    }
    
    // Apply speed modification
    float BaseMaxWalkSpeed = 600.0f; // Default UE5 character speed
    MovementComp->MaxWalkSpeed = BaseMaxWalkSpeed * SpeedMultiplier;
}

void ACore_MovementIntegration::HandleUnstableTerrain(ACharacter* Character, const FVector& Location)
{
    if (!IsValid(Character))
        return;
    
    // Add slight random movement to simulate unstable footing
    FVector RandomOffset = FVector(
        FMath::RandRange(-50.0f, 50.0f),
        FMath::RandRange(-50.0f, 50.0f),
        0.0f
    );
    
    Character->AddActorWorldOffset(RandomOffset * GetWorld()->GetDeltaSeconds());
    
    // Log unstable terrain event
    UE_LOG(LogTemp, Warning, TEXT("Character %s on unstable terrain at %s"), 
           *Character->GetName(), *Location.ToString());
}

void ACore_MovementIntegration::UpdateCharacterMovement(ACharacter* Character, float DeltaTime)
{
    if (!IsValid(Character))
        return;
    
    // Apply terrain-based movement modifications
    ApplyTerrainMovementModifier(Character, DeltaTime);
    
    // Calculate terrain influence
    CalculateTerrainInfluence(Character, DeltaTime);
    
    // Apply physics corrections
    ApplyPhysicsCorrections(Character, DeltaTime);
    
    // Log movement state for debugging
    LogMovementState(Character);
}

void ACore_MovementIntegration::IntegrateWithPhysicsSystems()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find physics system actors in the world
    TArray<AActor*> FoundActors;
    
    // Find TerrainPhysics
    UGameplayStatics::GetAllActorsOfClass(World, UCore_TerrainPhysics::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        TerrainPhysics = Cast<UCore_TerrainPhysics>(FoundActors[0]);
    }
    
    // Find CollisionSystem
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, UCore_CollisionSystem::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        CollisionSystem = Cast<UCore_CollisionSystem>(FoundActors[0]);
    }
    
    // Find PhysicsManager
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, UCore_PhysicsManager::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PhysicsManager = Cast<UCore_PhysicsManager>(FoundActors[0]);
    }
    
    ValidatePhysicsIntegration();
}

void ACore_MovementIntegration::ValidatePhysicsIntegration()
{
    bool bValidation = ValidateSystemReferences();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_MovementIntegration: Physics integration validation %s"), 
           bValidation ? TEXT("PASSED") : TEXT("FAILED"));
    
    if (!bValidation)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_MovementIntegration: Missing physics system references"));
        UE_LOG(LogTemp, Error, TEXT("TerrainPhysics: %s"), TerrainPhysics ? TEXT("FOUND") : TEXT("MISSING"));
        UE_LOG(LogTemp, Error, TEXT("CollisionSystem: %s"), CollisionSystem ? TEXT("FOUND") : TEXT("MISSING"));
        UE_LOG(LogTemp, Error, TEXT("PhysicsManager: %s"), PhysicsManager ? TEXT("FOUND") : TEXT("MISSING"));
    }
}

void ACore_MovementIntegration::TestMovementIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TESTING MOVEMENT INTEGRATION ==="));
    
    // Test terrain stability at current location
    FVector TestLocation = GetActorLocation();
    bool bStable = IsTerrainStable(TestLocation);
    float Slope = GetTerrainSlope(TestLocation);
    FVector Normal = GetTerrainNormal(TestLocation);
    
    UE_LOG(LogTemp, Warning, TEXT("Test Location: %s"), *TestLocation.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Terrain Stable: %s"), bStable ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Terrain Slope: %.2f degrees"), Slope);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Normal: %s"), *Normal.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Tracked Characters: %d"), TrackedCharacters.Num());
    
    // Validate physics integration
    ValidatePhysicsIntegration();
}

void ACore_MovementIntegration::SpawnTestCharacter()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Try to spawn a TranspersonalCharacter for testing
    UClass* CharacterClass = ATranspersonalCharacter::StaticClass();
    if (CharacterClass)
    {
        FVector SpawnLocation = GetActorLocation() + FVector(200, 0, 100);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        ATranspersonalCharacter* TestCharacter = World->SpawnActor<ATranspersonalCharacter>(
            CharacterClass, SpawnLocation, SpawnRotation);
        
        if (TestCharacter)
        {
            RegisterCharacter(TestCharacter);
            UE_LOG(LogTemp, Warning, TEXT("Spawned test character: %s"), *TestCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn test character"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TranspersonalCharacter class not found"));
    }
}

void ACore_MovementIntegration::CalculateTerrainInfluence(ACharacter* Character, float DeltaTime)
{
    if (!IsValid(Character))
        return;
    
    FVector CharacterLocation = Character->GetActorLocation();
    FVector TerrainNormal = GetTerrainNormal(CharacterLocation);
    
    // Apply terrain normal influence to character orientation
    FRotator CurrentRotation = Character->GetActorRotation();
    FRotator TargetRotation = FRotationMatrix::MakeFromZX(TerrainNormal, Character->GetActorForwardVector()).Rotator();
    
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, TerrainAdaptationSpeed);
    Character->SetActorRotation(NewRotation);
}

void ACore_MovementIntegration::ApplyPhysicsCorrections(ACharacter* Character, float DeltaTime)
{
    if (!IsValid(Character))
        return;
    
    // Apply gravity and physics corrections based on terrain
    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!MovementComp)
        return;
    
    // Adjust gravity scale based on terrain slope
    float TerrainSlope = GetTerrainSlope(Character->GetActorLocation());
    float GravityScale = FMath::Lerp(1.0f, 1.5f, TerrainSlope / 45.0f);
    MovementComp->GravityScale = GravityScale;
}

void ACore_MovementIntegration::HandleCollisionResponse(ACharacter* Character, const FHitResult& Hit)
{
    if (!IsValid(Character))
        return;
    
    // Handle collision with terrain and objects
    if (Hit.bBlockingHit)
    {
        FVector ImpactNormal = Hit.Normal;
        FVector CharacterVelocity = Character->GetVelocity();
        
        // Calculate reflection vector for bouncing off surfaces
        FVector ReflectedVelocity = CharacterVelocity - 2.0f * FVector::DotProduct(CharacterVelocity, ImpactNormal) * ImpactNormal;
        
        // Apply dampening
        ReflectedVelocity *= 0.3f;
        
        // Apply the reflected velocity
        Character->GetCharacterMovement()->Velocity = ReflectedVelocity;
        
        UE_LOG(LogTemp, Log, TEXT("Character %s collision response applied"), *Character->GetName());
    }
}

bool ACore_MovementIntegration::ValidateSystemReferences()
{
    int32 ValidSystems = 0;
    
    if (TerrainPhysics) ValidSystems++;
    if (CollisionSystem) ValidSystems++;
    if (PhysicsManager) ValidSystems++;
    
    // At least 1 system should be available for basic functionality
    return ValidSystems >= 1;
}

void ACore_MovementIntegration::LogMovementState(ACharacter* Character)
{
    if (!IsValid(Character))
        return;
    
    // Log detailed movement state for debugging (only occasionally to avoid spam)
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastLogTime > 2.0f) // Log every 2 seconds
    {
        FVector Location = Character->GetActorLocation();
        FVector Velocity = Character->GetVelocity();
        float Speed = Velocity.Size();
        bool bStable = IsTerrainStable(Location);
        float Slope = GetTerrainSlope(Location);
        
        UE_LOG(LogTemp, Log, TEXT("Character %s - Loc: %s, Speed: %.2f, Stable: %s, Slope: %.2f"), 
               *Character->GetName(), *Location.ToString(), Speed, 
               bStable ? TEXT("YES") : TEXT("NO"), Slope);
        
        LastLogTime = CurrentTime;
    }
}