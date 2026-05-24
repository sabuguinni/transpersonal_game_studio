#include "Eng_MovementArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

AEng_MovementArchitecture::AEng_MovementArchitecture()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default values for movement architecture
    BaseWalkSpeed = 400.0f;
    BaseRunSpeed = 800.0f;
    JumpHeight = 300.0f;
    MaxStamina = 100.0f;
    
    MaxWalkableAngle = 45.0f;
    TerrainAdaptationSpeed = 5.0f;
    
    GravityScale = 1.0f;
    AirControl = 0.2f;
    
    CharacterRadius = 34.0f;
    CharacterHeight = 180.0f;
    
    bArchitectureValid = false;
    LastValidationTime = 0.0f;
}

void AEng_MovementArchitecture::BeginPlay()
{
    Super::BeginPlay();
    
    // Validate architecture on start
    ValidateArchitecture();
    
    // Apply settings to all characters in the world
    ApplyMovementSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Movement Architecture initialized"));
}

void AEng_MovementArchitecture::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Periodic validation every 5 seconds
    if (GetWorld()->GetTimeSeconds() - LastValidationTime > 5.0f)
    {
        ValidateMovementSettings();
        LastValidationTime = GetWorld()->GetTimeSeconds();
    }
}

bool AEng_MovementArchitecture::ValidateMovementSettings()
{
    bool bValid = true;
    
    // Validate speed settings
    if (BaseWalkSpeed <= 0.0f || BaseWalkSpeed > 1000.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid BaseWalkSpeed: %f"), BaseWalkSpeed);
        bValid = false;
    }
    
    if (BaseRunSpeed <= BaseWalkSpeed || BaseRunSpeed > 2000.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid BaseRunSpeed: %f"), BaseRunSpeed);
        bValid = false;
    }
    
    // Validate jump settings
    if (JumpHeight <= 0.0f || JumpHeight > 1000.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid JumpHeight: %f"), JumpHeight);
        bValid = false;
    }
    
    // Validate terrain settings
    ValidateTerrainSettings();
    ValidatePhysicsSettings();
    ValidateCollisionSettings();
    
    bArchitectureValid = bValid;
    return bValid;
}

void AEng_MovementArchitecture::ApplyMovementSettings()
{
    if (!ValidateMovementSettings())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot apply invalid movement settings"));
        return;
    }
    
    // Find all characters in the world and apply settings
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                // Apply movement settings
                MovementComp->MaxWalkSpeed = BaseWalkSpeed;
                MovementComp->JumpZVelocity = JumpHeight;
                MovementComp->AirControl = AirControl;
                MovementComp->GravityScale = GravityScale;
                
                // Apply collision settings
                if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
                {
                    Capsule->SetCapsuleRadius(CharacterRadius);
                    Capsule->SetCapsuleHalfHeight(CharacterHeight);
                }
                
                UE_LOG(LogTemp, Log, TEXT("Applied movement settings to character: %s"), *Character->GetName());
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Movement architecture settings applied to %d characters"), FoundActors.Num());
}

void AEng_MovementArchitecture::ResetToDefaults()
{
    BaseWalkSpeed = 400.0f;
    BaseRunSpeed = 800.0f;
    JumpHeight = 300.0f;
    MaxStamina = 100.0f;
    
    MaxWalkableAngle = 45.0f;
    TerrainAdaptationSpeed = 5.0f;
    
    GravityScale = 1.0f;
    AirControl = 0.2f;
    
    CharacterRadius = 34.0f;
    CharacterHeight = 180.0f;
    
    ApplyMovementSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Movement architecture reset to defaults"));
}

void AEng_MovementArchitecture::ValidateArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MOVEMENT ARCHITECTURE VALIDATION ==="));
    
    bool bValid = ValidateMovementSettings();
    
    if (bValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ Movement architecture validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Movement architecture validation FAILED"));
    }
    
    GenerateArchitectureReport();
}

void AEng_MovementArchitecture::GenerateArchitectureReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MOVEMENT ARCHITECTURE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Base Walk Speed: %f"), BaseWalkSpeed);
    UE_LOG(LogTemp, Warning, TEXT("Base Run Speed: %f"), BaseRunSpeed);
    UE_LOG(LogTemp, Warning, TEXT("Jump Height: %f"), JumpHeight);
    UE_LOG(LogTemp, Warning, TEXT("Max Stamina: %f"), MaxStamina);
    UE_LOG(LogTemp, Warning, TEXT("Max Walkable Angle: %f"), MaxWalkableAngle);
    UE_LOG(LogTemp, Warning, TEXT("Character Radius: %f"), CharacterRadius);
    UE_LOG(LogTemp, Warning, TEXT("Character Height: %f"), CharacterHeight);
    UE_LOG(LogTemp, Warning, TEXT("Architecture Valid: %s"), bArchitectureValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("====================================="));
}

void AEng_MovementArchitecture::ValidateTerrainSettings()
{
    if (MaxWalkableAngle < 0.0f || MaxWalkableAngle > 90.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid MaxWalkableAngle: %f"), MaxWalkableAngle);
        MaxWalkableAngle = 45.0f;
    }
    
    if (TerrainAdaptationSpeed <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid TerrainAdaptationSpeed: %f"), TerrainAdaptationSpeed);
        TerrainAdaptationSpeed = 5.0f;
    }
}

void AEng_MovementArchitecture::ValidatePhysicsSettings()
{
    if (GravityScale <= 0.0f || GravityScale > 5.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid GravityScale: %f"), GravityScale);
        GravityScale = 1.0f;
    }
    
    if (AirControl < 0.0f || AirControl > 1.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid AirControl: %f"), AirControl);
        AirControl = 0.2f;
    }
}

void AEng_MovementArchitecture::ValidateCollisionSettings()
{
    if (CharacterRadius <= 0.0f || CharacterRadius > 100.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid CharacterRadius: %f"), CharacterRadius);
        CharacterRadius = 34.0f;
    }
    
    if (CharacterHeight <= 0.0f || CharacterHeight > 300.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid CharacterHeight: %f"), CharacterHeight);
        CharacterHeight = 180.0f;
    }
}

// Movement Architecture Component Implementation

UEng_MovementArchitectureComponent::UEng_MovementArchitectureComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    MovementArchitecture = nullptr;
    bUseArchitectureSettings = true;
    MovementMultiplier = 1.0f;
}

void UEng_MovementArchitectureComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find movement architecture in the world
    FindMovementArchitecture();
    
    // Apply architecture settings to this character
    if (bUseArchitectureSettings)
    {
        ApplyArchitectureToCharacter();
    }
}

void UEng_MovementArchitectureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Validate architecture connection
    ValidateArchitectureConnection();
}

void UEng_MovementArchitectureComponent::ApplyArchitectureToCharacter()
{
    if (!MovementArchitecture || !bUseArchitectureSettings)
    {
        return;
    }
    
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            // Apply architecture settings with multiplier
            MovementComp->MaxWalkSpeed = MovementArchitecture->BaseWalkSpeed * MovementMultiplier;
            MovementComp->JumpZVelocity = MovementArchitecture->JumpHeight * MovementMultiplier;
            MovementComp->AirControl = MovementArchitecture->AirControl;
            MovementComp->GravityScale = MovementArchitecture->GravityScale;
            
            UE_LOG(LogTemp, Log, TEXT("Applied movement architecture to character: %s"), *Character->GetName());
        }
    }
}

void UEng_MovementArchitectureComponent::UpdateMovementFromArchitecture()
{
    ApplyArchitectureToCharacter();
}

void UEng_MovementArchitectureComponent::FindMovementArchitecture()
{
    if (MovementArchitecture)
    {
        return; // Already found
    }
    
    // Search for movement architecture in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEng_MovementArchitecture::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        MovementArchitecture = Cast<AEng_MovementArchitecture>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("Found movement architecture: %s"), *MovementArchitecture->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No movement architecture found in world"));
    }
}

void UEng_MovementArchitectureComponent::ValidateArchitectureConnection()
{
    if (!MovementArchitecture)
    {
        FindMovementArchitecture();
    }
    
    if (MovementArchitecture && !MovementArchitecture->ValidateMovementSettings())
    {
        UE_LOG(LogTemp, Warning, TEXT("Movement architecture validation failed"));
    }
}