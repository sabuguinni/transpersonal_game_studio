#include "Char_PrimitiveHumanCharacter.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

AChar_PrimitiveHumanCharacter::AChar_PrimitiveHumanCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MaxAcceleration = 1024.0f;

    // Configure capsule collision
    GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);

    // Don't rotate when controller rotates
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Create additional mesh components for equipment
    SpearComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpearComponent"));
    SpearComponent->SetupAttachment(GetMesh(), TEXT("hand_r"));
    SpearComponent->SetVisibility(false);

    StoneToolComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneToolComponent"));
    StoneToolComponent->SetupAttachment(GetMesh(), TEXT("hand_l"));
    StoneToolComponent->SetVisibility(false);

    // Initialize customization data with defaults
    CustomizationData.SkinTone = 0.6f; // Slightly tanned
    CustomizationData.BodyBuild = 0.7f; // Muscular for survival
    CustomizationData.WeatheringLevel = 0.4f; // Moderately weathered
    CustomizationData.ScarPattern = 0;
    CustomizationData.ClothingVariant = 0;

    // Initialize survival stats
    SurvivalStats.Hunger = 80.0f;
    SurvivalStats.Thirst = 85.0f;
    SurvivalStats.Warmth = 70.0f;
    SurvivalStats.Stamina = 100.0f;
    SurvivalStats.Fear = 10.0f; // Slight baseline fear in dangerous world
    SurvivalStats.Exhaustion = 0.0f;

    // Set default survival decay rates
    HungerDecayRate = 0.8f; // Slower hunger decay
    ThirstDecayRate = 1.2f; // Faster thirst decay
    StaminaRegenRate = 15.0f; // Good stamina regen
    FearDecayRate = 8.0f; // Fear decays relatively quickly when safe
}

void AChar_PrimitiveHumanCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Create dynamic material instances
    if (GetMesh() && BaseSkinMaterial)
    {
        BodyMaterialInstance = UMaterialInstanceDynamic::Create(BaseSkinMaterial, this);
        GetMesh()->SetMaterial(0, BodyMaterialInstance);
    }

    if (GetMesh() && BaseClothingMaterial)
    {
        ClothingMaterialInstance = UMaterialInstanceDynamic::Create(BaseClothingMaterial, this);
        GetMesh()->SetMaterial(1, ClothingMaterialInstance);
    }

    // Apply initial customization
    ApplyCustomization(CustomizationData);

    // Set initial equipment visibility
    UpdateEquipmentVisibility();
}

void AChar_PrimitiveHumanCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update survival stats over time
    UpdateSurvivalStats(DeltaTime);

    // Apply survival effects to character
    ApplySurvivalEffects();
}

void AChar_PrimitiveHumanCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Basic movement and actions will be handled by the game mode
    // This character can be controlled by AI or player input
}

void AChar_PrimitiveHumanCharacter::ApplyCustomization(const FChar_HumanCustomization& NewCustomization)
{
    CustomizationData = NewCustomization;
    
    // Update materials with new customization
    UpdateBodyMaterial();
    UpdateClothingMaterial();
    UpdateEquipmentVisibility();

    // Apply body build to mesh scale
    if (GetMesh())
    {
        float ScaleMultiplier = 0.9f + (CustomizationData.BodyBuild * 0.2f); // 0.9 to 1.1 scale
        GetMesh()->SetRelativeScale3D(FVector(ScaleMultiplier, ScaleMultiplier, ScaleMultiplier));
    }
}

void AChar_PrimitiveHumanCharacter::SetSkinTone(float NewSkinTone)
{
    CustomizationData.SkinTone = FMath::Clamp(NewSkinTone, 0.0f, 1.0f);
    UpdateBodyMaterial();
}

void AChar_PrimitiveHumanCharacter::SetBodyBuild(float NewBodyBuild)
{
    CustomizationData.BodyBuild = FMath::Clamp(NewBodyBuild, 0.0f, 1.0f);
    
    // Update mesh scale
    if (GetMesh())
    {
        float ScaleMultiplier = 0.9f + (CustomizationData.BodyBuild * 0.2f);
        GetMesh()->SetRelativeScale3D(FVector(ScaleMultiplier, ScaleMultiplier, ScaleMultiplier));
    }
}

void AChar_PrimitiveHumanCharacter::SetWeatheringLevel(float NewWeathering)
{
    CustomizationData.WeatheringLevel = FMath::Clamp(NewWeathering, 0.0f, 1.0f);
    UpdateBodyMaterial();
    UpdateClothingMaterial();
}

void AChar_PrimitiveHumanCharacter::SetClothingVariant(int32 VariantIndex)
{
    CustomizationData.ClothingVariant = FMath::Max(0, VariantIndex);
    UpdateClothingMaterial();
}

void AChar_PrimitiveHumanCharacter::EquipSpear(int32 SpearIndex)
{
    if (SpearVariants.IsValidIndex(SpearIndex) && SpearComponent)
    {
        SpearComponent->SetStaticMesh(SpearVariants[SpearIndex]);
        SpearComponent->SetVisibility(true);
        bHasSpearEquipped = true;
        CurrentSpearIndex = SpearIndex;
    }
}

void AChar_PrimitiveHumanCharacter::EquipTool(int32 ToolIndex)
{
    if (ToolVariants.IsValidIndex(ToolIndex) && StoneToolComponent)
    {
        StoneToolComponent->SetStaticMesh(ToolVariants[ToolIndex]);
        StoneToolComponent->SetVisibility(true);
        bHasToolEquipped = true;
        CurrentToolIndex = ToolIndex;
    }
}

void AChar_PrimitiveHumanCharacter::UnequipWeapons()
{
    if (SpearComponent)
    {
        SpearComponent->SetVisibility(false);
        bHasSpearEquipped = false;
        CurrentSpearIndex = -1;
    }

    if (StoneToolComponent)
    {
        StoneToolComponent->SetVisibility(false);
        bHasToolEquipped = false;
        CurrentToolIndex = -1;
    }
}

void AChar_PrimitiveHumanCharacter::UpdateSurvivalStats(float DeltaTime)
{
    // Convert delta time to minutes for easier calculation
    float DeltaMinutes = DeltaTime / 60.0f;

    // Decay hunger and thirst over time
    SurvivalStats.Hunger = FMath::Max(0.0f, SurvivalStats.Hunger - (HungerDecayRate * DeltaMinutes));
    SurvivalStats.Thirst = FMath::Max(0.0f, SurvivalStats.Thirst - (ThirstDecayRate * DeltaMinutes));

    // Regenerate stamina when not exhausted
    if (SurvivalStats.Exhaustion < 50.0f)
    {
        SurvivalStats.Stamina = FMath::Min(100.0f, SurvivalStats.Stamina + (StaminaRegenRate * DeltaMinutes));
    }

    // Decay fear when safe (no immediate threats)
    if (SurvivalStats.Fear > 0.0f)
    {
        SurvivalStats.Fear = FMath::Max(0.0f, SurvivalStats.Fear - (FearDecayRate * DeltaMinutes));
    }

    // Increase exhaustion if hunger or thirst are critically low
    if (SurvivalStats.Hunger < 20.0f || SurvivalStats.Thirst < 20.0f)
    {
        SurvivalStats.Exhaustion = FMath::Min(100.0f, SurvivalStats.Exhaustion + (2.0f * DeltaMinutes));
    }
    else if (SurvivalStats.Exhaustion > 0.0f)
    {
        SurvivalStats.Exhaustion = FMath::Max(0.0f, SurvivalStats.Exhaustion - (1.0f * DeltaMinutes));
    }
}

void AChar_PrimitiveHumanCharacter::ModifyHunger(float Amount)
{
    SurvivalStats.Hunger = FMath::Clamp(SurvivalStats.Hunger + Amount, 0.0f, 100.0f);
}

void AChar_PrimitiveHumanCharacter::ModifyThirst(float Amount)
{
    SurvivalStats.Thirst = FMath::Clamp(SurvivalStats.Thirst + Amount, 0.0f, 100.0f);
}

void AChar_PrimitiveHumanCharacter::ModifyFear(float Amount)
{
    SurvivalStats.Fear = FMath::Clamp(SurvivalStats.Fear + Amount, 0.0f, 100.0f);
}

bool AChar_PrimitiveHumanCharacter::IsStarving() const
{
    return SurvivalStats.Hunger < 15.0f;
}

bool AChar_PrimitiveHumanCharacter::IsDehydrated() const
{
    return SurvivalStats.Thirst < 15.0f;
}

bool AChar_PrimitiveHumanCharacter::IsTerrified() const
{
    return SurvivalStats.Fear > 80.0f;
}

void AChar_PrimitiveHumanCharacter::PlaySurvivalAnimation(const FString& AnimationName)
{
    // Animation playback will be implemented when animation assets are available
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("Playing animation: %s"), *AnimationName));
    }
}

void AChar_PrimitiveHumanCharacter::SetMovementStyle(bool bIsSneaking)
{
    this->bIsSneaking = bIsSneaking;
    
    if (GetCharacterMovement())
    {
        if (bIsSneaking)
        {
            GetCharacterMovement()->MaxWalkSpeed = 150.0f; // Slow sneaking speed
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = 400.0f; // Normal walking speed
        }
    }
}

void AChar_PrimitiveHumanCharacter::InteractWithObject(AActor* TargetObject)
{
    if (TargetObject)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
                FString::Printf(TEXT("Interacting with: %s"), *TargetObject->GetName()));
        }
    }
}

void AChar_PrimitiveHumanCharacter::GatherResource(AActor* ResourceObject)
{
    if (ResourceObject)
    {
        bIsGathering = true;
        
        // Simulate resource gathering
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("Gathering from: %s"), *ResourceObject->GetName()));
        }
        
        // Stop gathering after a short delay
        GetWorld()->GetTimerManager().SetTimer(FTimerHandle(), [this]()
        {
            bIsGathering = false;
        }, 2.0f, false);
    }
}

void AChar_PrimitiveHumanCharacter::ThrowSpear(FVector TargetLocation)
{
    if (bHasSpearEquipped)
    {
        bIsInCombat = true;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("Throwing spear!"));
        }
        
        // Unequip spear after throwing
        UnequipWeapons();
        
        // Exit combat state after delay
        GetWorld()->GetTimerManager().SetTimer(FTimerHandle(), [this]()
        {
            bIsInCombat = false;
        }, 3.0f, false);
    }
}

void AChar_PrimitiveHumanCharacter::MeleeAttack()
{
    bIsInCombat = true;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
            TEXT("Melee attack!"));
    }
    
    // Reduce stamina from attack
    SurvivalStats.Stamina = FMath::Max(0.0f, SurvivalStats.Stamina - 15.0f);
    
    // Exit combat state after delay
    GetWorld()->GetTimerManager().SetTimer(FTimerHandle(), [this]()
    {
        bIsInCombat = false;
    }, 2.0f, false);
}

void AChar_PrimitiveHumanCharacter::DefensiveStance(bool bActivate)
{
    bIsInCombat = bActivate;
    
    if (GetCharacterMovement())
    {
        if (bActivate)
        {
            GetCharacterMovement()->MaxWalkSpeed = 200.0f; // Slower in defensive stance
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = 400.0f; // Normal speed
        }
    }
}

void AChar_PrimitiveHumanCharacter::UpdateBodyMaterial()
{
    if (BodyMaterialInstance)
    {
        // Set skin tone parameter
        BodyMaterialInstance->SetScalarParameterValue(TEXT("SkinTone"), CustomizationData.SkinTone);
        
        // Set weathering level
        BodyMaterialInstance->SetScalarParameterValue(TEXT("WeatheringLevel"), CustomizationData.WeatheringLevel);
        
        // Set scar pattern
        BodyMaterialInstance->SetScalarParameterValue(TEXT("ScarPattern"), float(CustomizationData.ScarPattern));
    }
}

void AChar_PrimitiveHumanCharacter::UpdateClothingMaterial()
{
    if (ClothingMaterialInstance)
    {
        // Set clothing variant
        ClothingMaterialInstance->SetScalarParameterValue(TEXT("ClothingVariant"), float(CustomizationData.ClothingVariant));
        
        // Set weathering level for clothing
        ClothingMaterialInstance->SetScalarParameterValue(TEXT("WeatheringLevel"), CustomizationData.WeatheringLevel);
    }
}

void AChar_PrimitiveHumanCharacter::UpdateEquipmentVisibility()
{
    // Update spear visibility
    if (SpearComponent)
    {
        SpearComponent->SetVisibility(bHasSpearEquipped);
    }
    
    // Update tool visibility
    if (StoneToolComponent)
    {
        StoneToolComponent->SetVisibility(bHasToolEquipped);
    }
}

void AChar_PrimitiveHumanCharacter::ApplySurvivalEffects()
{
    // Apply movement speed penalties based on survival stats
    if (GetCharacterMovement())
    {
        float SpeedMultiplier = 1.0f;
        
        // Reduce speed when hungry or thirsty
        if (IsStarving())
        {
            SpeedMultiplier *= 0.7f;
        }
        else if (SurvivalStats.Hunger < 30.0f)
        {
            SpeedMultiplier *= 0.85f;
        }
        
        if (IsDehydrated())
        {
            SpeedMultiplier *= 0.6f;
        }
        else if (SurvivalStats.Thirst < 30.0f)
        {
            SpeedMultiplier *= 0.8f;
        }
        
        // Reduce speed when exhausted
        if (SurvivalStats.Exhaustion > 70.0f)
        {
            SpeedMultiplier *= 0.5f;
        }
        else if (SurvivalStats.Exhaustion > 40.0f)
        {
            SpeedMultiplier *= 0.75f;
        }
        
        // Apply fear effects (shaking, reduced accuracy - visual only for now)
        if (IsTerrified())
        {
            SpeedMultiplier *= 1.2f; // Move faster when terrified
        }
        
        // Don't modify speed if already in special movement mode
        if (!bIsSneaking && !bIsInCombat)
        {
            float BaseSpeed = 400.0f;
            GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * SpeedMultiplier;
        }
    }
}