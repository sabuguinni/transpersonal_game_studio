#include "Core_DinosaurPhysics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UCore_DinosaurPhysics::UCore_DinosaurPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    DinosaurSpecies = ECore_DinosaurSpecies::TRex;
    CurrentMovementState = ECore_DinosaurMovementState::Idle;
    CurrentVelocity = FVector::ZeroVector;
    CurrentSpeed = 0.0f;
    
    bUseCustomPhysics = true;
    bEnableGroundAdaptation = true;
    bEnableFootIK = true;
    FootIKRange = 50.0f;
    
    CurrentHealth = 100.0f;
    MaxHealth = 100.0f;
    bIsAlive = true;
    
    bIsCharging = false;
    ChargingStartTime = 0.0f;
    LastGroundNormal = FVector::UpVector;
    
    // Initialize species data
    InitializeSpeciesData();
}

void UCore_DinosaurPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache component references
    if (AActor* Owner = GetOwner())
    {
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            MovementComponent = Character->GetCharacterMovement();
            MeshComponent = Character->GetMesh();
        }
        else if (APawn* Pawn = Cast<APawn>(Owner))
        {
            MovementComponent = Pawn->FindComponentByClass<UCharacterMovementComponent>();
            MeshComponent = Pawn->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    // Apply initial physics configuration
    ApplyPhysicsConfiguration();
    
    // Initialize foot IK targets
    FootIKTargets.SetNum(4); // Assuming quadruped
    for (int32 i = 0; i < FootIKTargets.Num(); i++)
    {
        FootIKTargets[i] = FVector::ZeroVector;
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Initialized for species %s"), 
           *UEnum::GetValueAsString(DinosaurSpecies));
}

void UCore_DinosaurPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsAlive || !GetOwner())
    {
        return;
    }
    
    // Update movement physics
    UpdateMovementPhysics(DeltaTime);
    
    // Update ground adaptation
    if (bEnableGroundAdaptation)
    {
        UpdateGroundAdaptation();
    }
    
    // Update foot IK
    if (bEnableFootIK && MeshComponent)
    {
        UpdateFootIK();
    }
    
    // Update current velocity and speed
    if (MovementComponent)
    {
        CurrentVelocity = MovementComponent->Velocity;
        CurrentSpeed = CurrentVelocity.Size();
    }
    
    // Auto-update movement state based on speed
    if (CurrentSpeed < 10.0f && CurrentMovementState != ECore_DinosaurMovementState::Idle)
    {
        if (!bIsCharging)
        {
            SetMovementState(ECore_DinosaurMovementState::Idle);
        }
    }
    else if (CurrentSpeed > SpeciesData.PhysicsConfig.MaxWalkSpeed * 0.8f && CurrentMovementState == ECore_DinosaurMovementState::Walking)
    {
        SetMovementState(ECore_DinosaurMovementState::Running);
    }
}

void UCore_DinosaurPhysics::SetMovementState(ECore_DinosaurMovementState NewState)
{
    if (CurrentMovementState == NewState)
    {
        return;
    }
    
    ECore_DinosaurMovementState PreviousState = CurrentMovementState;
    CurrentMovementState = NewState;
    
    // Apply movement state changes
    if (MovementComponent)
    {
        switch (NewState)
        {
            case ECore_DinosaurMovementState::Idle:
                MovementComponent->MaxWalkSpeed = 0.0f;
                bIsCharging = false;
                break;
                
            case ECore_DinosaurMovementState::Walking:
                MovementComponent->MaxWalkSpeed = SpeciesData.PhysicsConfig.MaxWalkSpeed;
                bIsCharging = false;
                break;
                
            case ECore_DinosaurMovementState::Running:
                MovementComponent->MaxWalkSpeed = SpeciesData.PhysicsConfig.MaxRunSpeed;
                bIsCharging = false;
                break;
                
            case ECore_DinosaurMovementState::Charging:
                MovementComponent->MaxWalkSpeed = SpeciesData.PhysicsConfig.MaxChargeSpeed;
                bIsCharging = true;
                ChargingStartTime = GetWorld()->GetTimeSeconds();
                break;
                
            case ECore_DinosaurMovementState::Dead:
                MovementComponent->MaxWalkSpeed = 0.0f;
                bIsCharging = false;
                break;
                
            default:
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Movement state changed from %s to %s"), 
           *UEnum::GetValueAsString(PreviousState),
           *UEnum::GetValueAsString(NewState));
}

void UCore_DinosaurPhysics::SetSpecies(ECore_DinosaurSpecies NewSpecies)
{
    if (DinosaurSpecies == NewSpecies)
    {
        return;
    }
    
    DinosaurSpecies = NewSpecies;
    SpeciesData = GetDefaultSpeciesData(NewSpecies);
    
    // Reapply physics configuration
    ApplyPhysicsConfiguration();
    
    // Reset health to species default
    MaxHealth = SpeciesData.BaseHealth;
    CurrentHealth = MaxHealth;
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Species changed to %s"), 
           *UEnum::GetValueAsString(DinosaurSpecies));
}

void UCore_DinosaurPhysics::ApplyMovementInput(FVector InputDirection, float InputMagnitude)
{
    if (!bIsAlive || !GetOwner() || InputMagnitude <= 0.0f)
    {
        return;
    }
    
    // Normalize input direction
    InputDirection.Normalize();
    
    // Determine target movement state based on input magnitude
    ECore_DinosaurMovementState TargetState = ECore_DinosaurMovementState::Walking;
    if (InputMagnitude > 0.8f)
    {
        TargetState = ECore_DinosaurMovementState::Running;
    }
    
    // Apply movement state
    SetMovementState(TargetState);
    
    // Apply movement input to character
    if (APawn* Pawn = Cast<APawn>(GetOwner()))
    {
        Pawn->AddMovementInput(InputDirection, InputMagnitude);
    }
}

void UCore_DinosaurPhysics::StartCharging()
{
    if (!bIsAlive)
    {
        return;
    }
    
    SetMovementState(ECore_DinosaurMovementState::Charging);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Started charging"));
}

void UCore_DinosaurPhysics::StopCharging()
{
    if (CurrentMovementState == ECore_DinosaurMovementState::Charging)
    {
        SetMovementState(ECore_DinosaurMovementState::Running);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Stopped charging"));
}

void UCore_DinosaurPhysics::TakeDamage(float DamageAmount)
{
    if (!bIsAlive || DamageAmount <= 0.0f)
    {
        return;
    }
    
    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
    
    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Took %.1f damage, health: %.1f/%.1f"), 
           DamageAmount, CurrentHealth, MaxHealth);
}

void UCore_DinosaurPhysics::Die()
{
    if (!bIsAlive)
    {
        return;
    }
    
    bIsAlive = false;
    CurrentHealth = 0.0f;
    SetMovementState(ECore_DinosaurMovementState::Dead);
    
    // Disable collision
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorEnableCollision(false);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Dinosaur died"));
}

void UCore_DinosaurPhysics::Respawn()
{
    bIsAlive = true;
    CurrentHealth = MaxHealth;
    SetMovementState(ECore_DinosaurMovementState::Idle);
    
    // Re-enable collision
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorEnableCollision(true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPhysics: Dinosaur respawned"));
}

float UCore_DinosaurPhysics::GetHealthPercentage() const
{
    if (MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    
    return CurrentHealth / MaxHealth;
}

bool UCore_DinosaurPhysics::IsMoving() const
{
    return CurrentSpeed > 10.0f;
}

bool UCore_DinosaurPhysics::IsCharging() const
{
    return bIsCharging && CurrentMovementState == ECore_DinosaurMovementState::Charging;
}

void UCore_DinosaurPhysics::ValidatePhysicsSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("=== DinosaurPhysics Validation ==="));
    UE_LOG(LogTemp, Warning, TEXT("Species: %s"), *UEnum::GetValueAsString(DinosaurSpecies));
    UE_LOG(LogTemp, Warning, TEXT("Movement State: %s"), *UEnum::GetValueAsString(CurrentMovementState));
    UE_LOG(LogTemp, Warning, TEXT("Health: %.1f/%.1f"), CurrentHealth, MaxHealth);
    UE_LOG(LogTemp, Warning, TEXT("Is Alive: %s"), bIsAlive ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Movement Component: %s"), MovementComponent ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("Mesh Component: %s"), MeshComponent ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("=== End Validation ==="));
}

void UCore_DinosaurPhysics::TestMovementStates()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Testing Movement States ==="));
    
    // Test each movement state
    TArray<ECore_DinosaurMovementState> TestStates = {
        ECore_DinosaurMovementState::Idle,
        ECore_DinosaurMovementState::Walking,
        ECore_DinosaurMovementState::Running,
        ECore_DinosaurMovementState::Charging
    };
    
    for (ECore_DinosaurMovementState State : TestStates)
    {
        SetMovementState(State);
        UE_LOG(LogTemp, Warning, TEXT("Set state to: %s"), *UEnum::GetValueAsString(State));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== End Movement State Test ==="));
}

void UCore_DinosaurPhysics::TestSpeciesConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Testing Species Configuration ==="));
    
    // Test each species
    TArray<ECore_DinosaurSpecies> TestSpecies = {
        ECore_DinosaurSpecies::TRex,
        ECore_DinosaurSpecies::Raptor,
        ECore_DinosaurSpecies::Brachiosaurus,
        ECore_DinosaurSpecies::Triceratops,
        ECore_DinosaurSpecies::Stegosaurus
    };
    
    for (ECore_DinosaurSpecies Species : TestSpecies)
    {
        SetSpecies(Species);
        UE_LOG(LogTemp, Warning, TEXT("Species: %s, Health: %.1f, Speed: %.1f"), 
               *UEnum::GetValueAsString(Species),
               SpeciesData.BaseHealth,
               SpeciesData.PhysicsConfig.MaxWalkSpeed);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== End Species Configuration Test ==="));
}

void UCore_DinosaurPhysics::InitializeSpeciesData()
{
    SpeciesData = GetDefaultSpeciesData(DinosaurSpecies);
}

void UCore_DinosaurPhysics::UpdateMovementPhysics(float DeltaTime)
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Apply species-specific physics
    MovementComponent->Mass = SpeciesData.PhysicsConfig.Mass;
    MovementComponent->MaxStepHeight = SpeciesData.PhysicsConfig.StepHeight;
    MovementComponent->GroundFriction = SpeciesData.PhysicsConfig.GroundFriction;
    MovementComponent->BrakingDecelerationWalking = SpeciesData.PhysicsConfig.BrakingDeceleration;
    
    // Handle charging physics
    if (bIsCharging && GetWorld())
    {
        float ChargingDuration = GetWorld()->GetTimeSeconds() - ChargingStartTime;
        
        // Auto-stop charging after 3 seconds
        if (ChargingDuration > 3.0f)
        {
            StopCharging();
        }
    }
}

void UCore_DinosaurPhysics::UpdateGroundAdaptation()
{
    if (!GetOwner())
    {
        return;
    }
    
    // Perform ground trace
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = StartLocation - FVector(0, 0, 200.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        LastGroundNormal = HitResult.Normal;
        
        // Adapt actor rotation to ground normal
        FRotator TargetRotation = UKismetMathLibrary::MakeRotFromZX(LastGroundNormal, GetOwner()->GetActorForwardVector());
        FRotator CurrentRotation = GetOwner()->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
        
        GetOwner()->SetActorRotation(NewRotation);
    }
}

void UCore_DinosaurPhysics::UpdateFootIK()
{
    // Simplified foot IK - trace down from foot bone positions
    if (!MeshComponent)
    {
        return;
    }
    
    TArray<FName> FootBones = {
        TEXT("foot_l"),
        TEXT("foot_r"),
        TEXT("foot_front_l"),
        TEXT("foot_front_r")
    };
    
    for (int32 i = 0; i < FootBones.Num() && i < FootIKTargets.Num(); i++)
    {
        if (MeshComponent->GetBoneIndex(FootBones[i]) != INDEX_NONE)
        {
            FVector FootLocation = MeshComponent->GetBoneLocation(FootBones[i]);
            FVector TraceStart = FootLocation + FVector(0, 0, FootIKRange);
            FVector TraceEnd = FootLocation - FVector(0, 0, FootIKRange);
            
            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(GetOwner());
            
            if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
            {
                FootIKTargets[i] = HitResult.Location;
            }
            else
            {
                FootIKTargets[i] = FootLocation;
            }
        }
    }
}

void UCore_DinosaurPhysics::ApplyPhysicsConfiguration()
{
    if (!MovementComponent)
    {
        return;
    }
    
    // Apply species configuration to movement component
    MovementComponent->MaxWalkSpeed = SpeciesData.PhysicsConfig.MaxWalkSpeed;
    MovementComponent->Mass = SpeciesData.PhysicsConfig.Mass;
    MovementComponent->MaxStepHeight = SpeciesData.PhysicsConfig.StepHeight;
    MovementComponent->GroundFriction = SpeciesData.PhysicsConfig.GroundFriction;
    MovementComponent->BrakingDecelerationWalking = SpeciesData.PhysicsConfig.BrakingDeceleration;
}

FCore_DinosaurSpeciesData UCore_DinosaurPhysics::GetDefaultSpeciesData(ECore_DinosaurSpecies Species)
{
    FCore_DinosaurSpeciesData Data;
    Data.Species = Species;
    
    switch (Species)
    {
        case ECore_DinosaurSpecies::TRex:
            Data.DisplayName = TEXT("Tyrannosaurus Rex");
            Data.BaseHealth = 500.0f;
            Data.BaseSize = 3.0f;
            Data.bIsCarnivore = true;
            Data.bIsPackHunter = false;
            Data.PhysicsConfig.MaxWalkSpeed = 400.0f;
            Data.PhysicsConfig.MaxRunSpeed = 800.0f;
            Data.PhysicsConfig.MaxChargeSpeed = 1200.0f;
            Data.PhysicsConfig.Mass = 5000.0f;
            Data.PhysicsConfig.AttackDamage = 100.0f;
            Data.PhysicsConfig.AttackRange = 300.0f;
            break;
            
        case ECore_DinosaurSpecies::Raptor:
            Data.DisplayName = TEXT("Velociraptor");
            Data.BaseHealth = 150.0f;
            Data.BaseSize = 1.0f;
            Data.bIsCarnivore = true;
            Data.bIsPackHunter = true;
            Data.PhysicsConfig.MaxWalkSpeed = 500.0f;
            Data.PhysicsConfig.MaxRunSpeed = 1000.0f;
            Data.PhysicsConfig.MaxChargeSpeed = 1400.0f;
            Data.PhysicsConfig.Mass = 800.0f;
            Data.PhysicsConfig.AttackDamage = 60.0f;
            Data.PhysicsConfig.AttackRange = 150.0f;
            break;
            
        case ECore_DinosaurSpecies::Brachiosaurus:
            Data.DisplayName = TEXT("Brachiosaurus");
            Data.BaseHealth = 1000.0f;
            Data.BaseSize = 5.0f;
            Data.bIsCarnivore = false;
            Data.bIsPackHunter = false;
            Data.PhysicsConfig.MaxWalkSpeed = 200.0f;
            Data.PhysicsConfig.MaxRunSpeed = 400.0f;
            Data.PhysicsConfig.MaxChargeSpeed = 600.0f;
            Data.PhysicsConfig.Mass = 15000.0f;
            Data.PhysicsConfig.AttackDamage = 80.0f;
            Data.PhysicsConfig.AttackRange = 400.0f;
            break;
            
        case ECore_DinosaurSpecies::Triceratops:
            Data.DisplayName = TEXT("Triceratops");
            Data.BaseHealth = 400.0f;
            Data.BaseSize = 2.5f;
            Data.bIsCarnivore = false;
            Data.bIsPackHunter = false;
            Data.PhysicsConfig.MaxWalkSpeed = 300.0f;
            Data.PhysicsConfig.MaxRunSpeed = 600.0f;
            Data.PhysicsConfig.MaxChargeSpeed = 1000.0f;
            Data.PhysicsConfig.Mass = 8000.0f;
            Data.PhysicsConfig.AttackDamage = 120.0f;
            Data.PhysicsConfig.AttackRange = 250.0f;
            break;
            
        case ECore_DinosaurSpecies::Stegosaurus:
            Data.DisplayName = TEXT("Stegosaurus");
            Data.BaseHealth = 350.0f;
            Data.BaseSize = 2.0f;
            Data.bIsCarnivore = false;
            Data.bIsPackHunter = false;
            Data.PhysicsConfig.MaxWalkSpeed = 250.0f;
            Data.PhysicsConfig.MaxRunSpeed = 500.0f;
            Data.PhysicsConfig.MaxChargeSpeed = 700.0f;
            Data.PhysicsConfig.Mass = 6000.0f;
            Data.PhysicsConfig.AttackDamage = 90.0f;
            Data.PhysicsConfig.AttackRange = 200.0f;
            break;
    }
    
    return Data;
}