#include "Eng_MovementArchitecture.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

UEng_MovementArchitecture::UEng_MovementArchitecture()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize biome modifiers
    InitializeBiomeModifiers();
}

void UEng_MovementArchitecture::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache owner character and movement component
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        CachedMovementComponent = OwnerCharacter->GetCharacterMovement();
        if (CachedMovementComponent)
        {
            SetMovementParameters(CachedMovementComponent);
            UE_LOG(LogTemp, Log, TEXT("MovementArchitecture: Initialized for character %s"), *OwnerCharacter->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MovementArchitecture: No CharacterMovementComponent found on %s"), *OwnerCharacter->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MovementArchitecture: Owner is not a Character"));
    }
}

void UEng_MovementArchitecture::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update movement modifiers periodically
    if (GetWorld()->GetTimeSeconds() - LastSpeedUpdate > SpeedUpdateInterval)
    {
        ApplyMovementModifiers();
        LastSpeedUpdate = GetWorld()->GetTimeSeconds();
    }
}

void UEng_MovementArchitecture::SetMovementParameters(UCharacterMovementComponent* MovementComp)
{
    if (!MovementComp)
    {
        UE_LOG(LogTemp, Error, TEXT("MovementArchitecture: Null MovementComponent passed"));
        return;
    }

    // Set base movement parameters
    MovementComp->MaxWalkSpeed = BaseWalkSpeed;
    MovementComp->JumpZVelocity = BaseJumpVelocity;
    MovementComp->AirControl = 0.2f;
    MovementComp->GroundFriction = 8.0f;
    MovementComp->MaxAcceleration = 2048.0f;
    MovementComp->BrakingDecelerationWalking = 2048.0f;
    
    // Enable advanced movement features
    MovementComp->bCanWalkOffLedges = true;
    MovementComp->bCanWalkOffLedgesWhenCrouching = true;
    MovementComp->SetWalkableFloorAngle(45.0f);
    
    UE_LOG(LogTemp, Log, TEXT("MovementArchitecture: Base parameters set - WalkSpeed: %f, JumpVelocity: %f"), 
           BaseWalkSpeed, BaseJumpVelocity);
}

void UEng_MovementArchitecture::UpdateMovementForTerrain(const FHitResult& GroundHit)
{
    if (!CachedMovementComponent)
        return;

    float TerrainModifier = GetTerrainSpeedModifier(GroundHit);
    CurrentSpeedMultiplier = TerrainModifier;
    
    // Apply terrain-based speed modification
    float ModifiedSpeed = BaseWalkSpeed * TerrainModifier;
    if (bIsRunning && bCanRun)
    {
        ModifiedSpeed = BaseRunSpeed * TerrainModifier;
    }
    
    CachedMovementComponent->MaxWalkSpeed = ModifiedSpeed;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("MovementArchitecture: Terrain modifier %f applied, speed: %f"), 
           TerrainModifier, ModifiedSpeed);
}

void UEng_MovementArchitecture::UpdateMovementForBiome(EEng_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    
    if (BiomeSpeedModifiers.Contains(NewBiome))
    {
        float BiomeModifier = BiomeSpeedModifiers[NewBiome];
        CurrentSpeedMultiplier *= BiomeModifier;
        
        UE_LOG(LogTemp, Log, TEXT("MovementArchitecture: Entered biome %d with speed modifier %f"), 
               (int32)NewBiome, BiomeModifier);
    }
}

void UEng_MovementArchitecture::UpdateMovementForStamina(float CurrentStamina, float MaxStamina)
{
    if (!CachedMovementComponent)
        return;

    float StaminaPercent = (MaxStamina > 0) ? (CurrentStamina / MaxStamina) : 0.0f;
    
    // Check if can run based on stamina
    bCanRun = CurrentStamina >= MinStaminaToRun;
    
    // Reduce speed when stamina is very low
    if (StaminaPercent < 0.2f)
    {
        CurrentSpeedMultiplier *= 0.7f; // 30% speed reduction when exhausted
        UE_LOG(LogTemp, Log, TEXT("MovementArchitecture: Low stamina speed reduction applied"));
    }
    
    // Force walk if no stamina for running
    if (!bCanRun && bIsRunning)
    {
        bIsRunning = false;
        CachedMovementComponent->MaxWalkSpeed = BaseWalkSpeed * CurrentSpeedMultiplier;
        UE_LOG(LogTemp, Log, TEXT("MovementArchitecture: Forced to walk due to low stamina"));
    }
}

void UEng_MovementArchitecture::UpdateMovementForFear(float FearLevel)
{
    if (!CachedMovementComponent)
        return;

    // Fear provides speed boost but increases stamina drain
    if (FearLevel > 50.0f)
    {
        float FearBoost = 1.0f + (FearLevel / 100.0f) * (FearSpeedBonus - 1.0f);
        CurrentSpeedMultiplier *= FearBoost;
        
        UE_LOG(LogTemp, Log, TEXT("MovementArchitecture: Fear speed boost %f applied (Fear: %f)"), 
               FearBoost, FearLevel);
    }
}

float UEng_MovementArchitecture::CalculateMovementSpeed(bool bWantsToRun, float StaminaPercent, float FearLevel)
{
    float BaseSpeed = bWantsToRun && bCanRun ? BaseRunSpeed : BaseWalkSpeed;
    float FinalSpeed = BaseSpeed * CurrentSpeedMultiplier;
    
    // Apply fear bonus
    if (FearLevel > 50.0f)
    {
        float FearBoost = 1.0f + (FearLevel / 100.0f) * (FearSpeedBonus - 1.0f);
        FinalSpeed *= FearBoost;
    }
    
    // Apply stamina penalty for very low stamina
    if (StaminaPercent < 0.2f)
    {
        FinalSpeed *= 0.7f;
    }
    
    return FinalSpeed;
}

bool UEng_MovementArchitecture::CanPerformAction(const FString& ActionName, float StaminaCost)
{
    // For now, simple stamina check
    // This will be expanded when survival stats are integrated
    return bCanRun; // Placeholder logic
}

void UEng_MovementArchitecture::ValidateMovementSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MOVEMENT ARCHITECTURE VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Base Walk Speed: %f"), BaseWalkSpeed);
    UE_LOG(LogTemp, Warning, TEXT("Base Run Speed: %f"), BaseRunSpeed);
    UE_LOG(LogTemp, Warning, TEXT("Jump Velocity: %f"), BaseJumpVelocity);
    UE_LOG(LogTemp, Warning, TEXT("Stamina Drain Rate: %f"), StaminaDrainRate);
    UE_LOG(LogTemp, Warning, TEXT("Stamina Recovery Rate: %f"), StaminaRecoveryRate);
    UE_LOG(LogTemp, Warning, TEXT("Current Speed Multiplier: %f"), CurrentSpeedMultiplier);
    UE_LOG(LogTemp, Warning, TEXT("Can Run: %s"), bCanRun ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Is Running: %s"), bIsRunning ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Current Biome: %d"), (int32)CurrentBiome);
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION ==="));
}

void UEng_MovementArchitecture::LogMovementState()
{
    if (OwnerCharacter && CachedMovementComponent)
    {
        FVector Velocity = CachedMovementComponent->Velocity;
        float Speed = Velocity.Size();
        
        UE_LOG(LogTemp, Log, TEXT("MovementArchitecture State for %s:"), *OwnerCharacter->GetName());
        UE_LOG(LogTemp, Log, TEXT("  Current Speed: %f"), Speed);
        UE_LOG(LogTemp, Log, TEXT("  Max Walk Speed: %f"), CachedMovementComponent->MaxWalkSpeed);
        UE_LOG(LogTemp, Log, TEXT("  Is Falling: %s"), CachedMovementComponent->IsFalling() ? TEXT("Yes") : TEXT("No"));
        UE_LOG(LogTemp, Log, TEXT("  Is On Ground: %s"), CachedMovementComponent->IsMovingOnGround() ? TEXT("Yes") : TEXT("No"));
        UE_LOG(LogTemp, Log, TEXT("  Speed Multiplier: %f"), CurrentSpeedMultiplier);
    }
}

void UEng_MovementArchitecture::InitializeBiomeModifiers()
{
    // Set movement speed modifiers for different biomes
    BiomeSpeedModifiers.Add(EEng_BiomeType::Forest, 0.9f);      // Slightly slower due to undergrowth
    BiomeSpeedModifiers.Add(EEng_BiomeType::Swamp, 0.6f);       // Much slower due to mud and water
    BiomeSpeedModifiers.Add(EEng_BiomeType::Savanna, 1.1f);     // Faster on open plains
    BiomeSpeedModifiers.Add(EEng_BiomeType::Desert, 0.8f);      // Slower due to sand and heat
    BiomeSpeedModifiers.Add(EEng_BiomeType::SnowyMountain, 0.7f); // Slower due to snow and altitude
    
    UE_LOG(LogTemp, Log, TEXT("MovementArchitecture: Biome modifiers initialized"));
}

float UEng_MovementArchitecture::GetTerrainSpeedModifier(const FHitResult& GroundHit)
{
    if (!GroundHit.bBlockingHit)
        return 1.0f;

    // Check surface material/physics material for terrain type
    // For now, use simple slope-based calculation
    FVector Normal = GroundHit.Normal;
    float SlopeAngle = FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)) * 180.0f / PI;
    
    if (SlopeAngle > 30.0f)
    {
        return SlopeSpeedMultiplier; // Steep slope penalty
    }
    
    // TODO: Add material-based detection (mud, water, etc.)
    // This will be expanded when terrain materials are implemented
    
    return 1.0f; // Normal terrain
}

void UEng_MovementArchitecture::ApplyMovementModifiers()
{
    if (!CachedMovementComponent || !OwnerCharacter)
        return;

    // Get current ground information
    FHitResult GroundHit;
    FVector Start = OwnerCharacter->GetActorLocation();
    FVector End = Start - FVector(0, 0, 200.0f); // Trace down 2 meters
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    if (GetWorld()->LineTraceSingleByChannel(GroundHit, Start, End, ECC_WorldStatic, QueryParams))
    {
        UpdateMovementForTerrain(GroundHit);
    }
    
    // Reset speed multiplier for next frame
    CurrentSpeedMultiplier = 1.0f;
}