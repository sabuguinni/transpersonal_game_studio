#include "Eng_MovementSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

// Static instance for singleton
UEng_MovementArchitectureManager* UEng_MovementArchitectureManager::Instance = nullptr;

// UEng_MovementSystem Implementation
UEng_MovementSystem::UEng_MovementSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default movement settings
    MovementSettings.WalkSpeed = 200.0f;
    MovementSettings.RunSpeed = 400.0f;
    MovementSettings.SprintSpeed = 600.0f;
    MovementSettings.CrouchSpeed = 100.0f;
    MovementSettings.JumpHeight = 200.0f;
    MovementSettings.StaminaDrainRate = 10.0f;
    MovementSettings.TerrainSpeedMultiplier = 1.0f;

    // Initialize movement state
    MovementState.CurrentMovementType = EEng_MovementType::Walking;
    MovementState.CurrentTerrain = EEng_TerrainType::Flat;
    MovementState.CurrentSpeed = 0.0f;
    MovementState.bIsOnGround = true;
    MovementState.bCanSprint = true;
    MovementState.StaminaLevel = 100.0f;
}

void UEng_MovementSystem::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner character and movement component
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        if (MovementComponent)
        {
            // Apply initial movement settings
            MovementComponent->MaxWalkSpeed = MovementSettings.WalkSpeed;
            MovementComponent->JumpZVelocity = MovementSettings.JumpHeight;
            bInitialized = true;

            UE_LOG(LogTemp, Log, TEXT("MovementSystem initialized for %s"), *OwnerCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MovementSystem: Owner has no CharacterMovementComponent"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MovementSystem: Owner is not a Character"));
    }

    // Register with architecture manager
    if (UEng_MovementArchitectureManager* Manager = UEng_MovementArchitectureManager::GetInstance())
    {
        Manager->RegisterMovementSystem(this);
    }
}

void UEng_MovementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bInitialized || !MovementComponent)
    {
        return;
    }

    // Update movement state
    MovementState.CurrentSpeed = MovementComponent->Velocity.Size();
    MovementState.bIsOnGround = MovementComponent->IsMovingOnGround();

    // Check terrain under character
    CheckTerrainUnderfoot();

    // Update movement speed based on current settings
    UpdateMovementSpeed();

    // Handle stamina regeneration
    if (MovementState.CurrentMovementType != EEng_MovementType::Sprinting)
    {
        RegenerateStamina(DeltaTime);
    }

    // Consume stamina during sprint
    if (MovementState.CurrentMovementType == EEng_MovementType::Sprinting && MovementState.CurrentSpeed > 0.1f)
    {
        ConsumeStamina(MovementSettings.StaminaDrainRate * DeltaTime);
    }
}

void UEng_MovementSystem::SetMovementType(EEng_MovementType NewMovementType)
{
    if (!CanPerformMovement(NewMovementType))
    {
        return;
    }

    MovementState.CurrentMovementType = NewMovementType;
    UpdateMovementSpeed();

    UE_LOG(LogTemp, Log, TEXT("Movement type changed to: %d"), (int32)NewMovementType);
}

void UEng_MovementSystem::UpdateTerrainType(EEng_TerrainType NewTerrain)
{
    MovementState.CurrentTerrain = NewTerrain;
    MovementSettings.TerrainSpeedMultiplier = CalculateTerrainSpeedModifier();
    UpdateMovementSpeed();
}

float UEng_MovementSystem::GetCurrentSpeedLimit() const
{
    float BaseSpeed = MovementSettings.WalkSpeed;

    switch (MovementState.CurrentMovementType)
    {
        case EEng_MovementType::Walking:
            BaseSpeed = MovementSettings.WalkSpeed;
            break;
        case EEng_MovementType::Running:
            BaseSpeed = MovementSettings.RunSpeed;
            break;
        case EEng_MovementType::Sprinting:
            BaseSpeed = MovementSettings.SprintSpeed;
            break;
        case EEng_MovementType::Crouching:
            BaseSpeed = MovementSettings.CrouchSpeed;
            break;
        default:
            BaseSpeed = MovementSettings.WalkSpeed;
            break;
    }

    return BaseSpeed * MovementSettings.TerrainSpeedMultiplier;
}

bool UEng_MovementSystem::CanPerformMovement(EEng_MovementType MovementType) const
{
    switch (MovementType)
    {
        case EEng_MovementType::Sprinting:
            return MovementState.bCanSprint && MovementState.StaminaLevel > 10.0f;
        case EEng_MovementType::Swimming:
            return MovementState.CurrentTerrain == EEng_TerrainType::Water;
        default:
            return true;
    }
}

void UEng_MovementSystem::ConsumeStamina(float Amount)
{
    MovementState.StaminaLevel = FMath::Clamp(MovementState.StaminaLevel - Amount, 0.0f, 100.0f);
    
    if (MovementState.StaminaLevel <= 0.0f)
    {
        MovementState.bCanSprint = false;
        if (MovementState.CurrentMovementType == EEng_MovementType::Sprinting)
        {
            SetMovementType(EEng_MovementType::Running);
        }
    }
}

void UEng_MovementSystem::RegenerateStamina(float DeltaTime)
{
    float RegenRate = 15.0f; // Base regeneration rate
    
    // Slower regen while moving
    if (MovementState.CurrentSpeed > 0.1f)
    {
        RegenRate *= 0.5f;
    }

    MovementState.StaminaLevel = FMath::Clamp(MovementState.StaminaLevel + (RegenRate * DeltaTime), 0.0f, 100.0f);
    
    if (MovementState.StaminaLevel > 25.0f)
    {
        MovementState.bCanSprint = true;
    }
}

void UEng_MovementSystem::ValidateMovementArchitecture()
{
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("MovementSystem: No owner character"));
        return;
    }

    if (!MovementComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("MovementSystem: No movement component"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("MovementSystem: Architecture validation PASSED for %s"), *OwnerCharacter->GetName());
}

void UEng_MovementSystem::UpdateMovementSpeed()
{
    if (!MovementComponent)
    {
        return;
    }

    float TargetSpeed = GetCurrentSpeedLimit();
    MovementComponent->MaxWalkSpeed = TargetSpeed;
}

void UEng_MovementSystem::CheckTerrainUnderfoot()
{
    if (!OwnerCharacter)
    {
        return;
    }

    // Simple terrain detection via line trace
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        // Basic terrain type detection based on surface material
        // This would be expanded with proper material detection
        if (HitResult.GetActor())
        {
            FString ActorName = HitResult.GetActor()->GetName();
            if (ActorName.Contains("Water"))
            {
                UpdateTerrainType(EEng_TerrainType::Water);
            }
            else if (ActorName.Contains("Sand"))
            {
                UpdateTerrainType(EEng_TerrainType::Sand);
            }
            else if (ActorName.Contains("Rock"))
            {
                UpdateTerrainType(EEng_TerrainType::Rock);
            }
            else
            {
                UpdateTerrainType(EEng_TerrainType::Flat);
            }
        }
    }
}

float UEng_MovementSystem::CalculateTerrainSpeedModifier() const
{
    switch (MovementState.CurrentTerrain)
    {
        case EEng_TerrainType::Flat:
            return 1.0f;
        case EEng_TerrainType::Slope:
            return 0.8f;
        case EEng_TerrainType::Rough:
            return 0.7f;
        case EEng_TerrainType::Mud:
            return 0.5f;
        case EEng_TerrainType::Sand:
            return 0.6f;
        case EEng_TerrainType::Rock:
            return 0.9f;
        case EEng_TerrainType::Water:
            return 0.3f;
        default:
            return 1.0f;
    }
}

// UEng_MovementArchitectureManager Implementation
UEng_MovementArchitectureManager* UEng_MovementArchitectureManager::GetInstance()
{
    if (!Instance)
    {
        Instance = NewObject<UEng_MovementArchitectureManager>();
        Instance->AddToRoot(); // Prevent garbage collection
    }
    return Instance;
}

void UEng_MovementArchitectureManager::ValidateAllMovementSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Validating %d movement systems"), RegisteredSystems.Num());
    
    for (UEng_MovementSystem* System : RegisteredSystems)
    {
        if (IsValid(System))
        {
            System->ValidateMovementArchitecture();
        }
    }
}

void UEng_MovementArchitectureManager::GenerateMovementReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== MOVEMENT ARCHITECTURE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Movement Systems: %d"), RegisteredSystems.Num());
    
    int32 ValidSystems = 0;
    for (UEng_MovementSystem* System : RegisteredSystems)
    {
        if (IsValid(System))
        {
            ValidSystems++;
            UE_LOG(LogTemp, Log, TEXT("- %s: ACTIVE"), *System->GetOwner()->GetName());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Valid Systems: %d/%d"), ValidSystems, RegisteredSystems.Num());
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

void UEng_MovementArchitectureManager::RegisterMovementSystem(UEng_MovementSystem* MovementSystem)
{
    if (IsValid(MovementSystem))
    {
        RegisteredSystems.AddUnique(MovementSystem);
        UE_LOG(LogTemp, Log, TEXT("Registered movement system for %s"), *MovementSystem->GetOwner()->GetName());
    }
}

void UEng_MovementArchitectureManager::UnregisterMovementSystem(UEng_MovementSystem* MovementSystem)
{
    RegisteredSystems.Remove(MovementSystem);
}