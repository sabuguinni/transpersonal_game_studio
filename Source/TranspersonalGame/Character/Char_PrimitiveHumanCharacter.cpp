#include "Char_PrimitiveHumanCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

AChar_PrimitiveHumanCharacter::AChar_PrimitiveHumanCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set default character size and collision
    GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    // Don't rotate when the controller rotates
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Create clothing and tool mesh components
    AnimalHideMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnimalHideMesh"));
    AnimalHideMesh->SetupAttachment(GetMesh());
    AnimalHideMesh->SetVisibility(false);

    BoneJewelryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoneJewelryMesh"));
    BoneJewelryMesh->SetupAttachment(GetMesh());
    BoneJewelryMesh->SetVisibility(false);

    StoneAxeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneAxeMesh"));
    StoneAxeMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
    StoneAxeMesh->SetVisibility(false);

    SpearMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpearMesh"));
    SpearMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
    SpearMesh->SetVisibility(false);

    // Initialize default customization
    CharacterCustomization.SkinTone = ESkinTone::Medium;
    CharacterCustomization.HairColor = EHairColor::Brown;
    CharacterCustomization.BodyBuild = EBodyBuild::Athletic;
    CharacterCustomization.bWearingAnimalHide = true;
    CharacterCustomization.bWearingBoneJewelry = false;
    CharacterCustomization.bCarryingStoneAxe = false;
    CharacterCustomization.bCarryingSpear = false;
    CharacterCustomization.bHasFacePaint = false;
    CharacterCustomization.WeatheringLevel = 0.5f;

    // Initialize survival stats
    SurvivalStats.Health = 100.0f;
    SurvivalStats.MaxHealth = 100.0f;
    SurvivalStats.Hunger = 100.0f;
    SurvivalStats.Thirst = 100.0f;
    SurvivalStats.Stamina = 100.0f;
    SurvivalStats.MaxStamina = 100.0f;
    SurvivalStats.Temperature = 37.0f;
    SurvivalStats.Fear = 0.0f;
}

void AChar_PrimitiveHumanCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial customization
    ApplyCharacterCustomization(CharacterCustomization);
    UpdateMeshVisibility();
}

void AChar_PrimitiveHumanCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update survival stats over time
    UpdateSurvivalStats(DeltaTime);
}

void AChar_PrimitiveHumanCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Input bindings will be handled by the player controller
}

void AChar_PrimitiveHumanCharacter::ApplyCharacterCustomization(const FChar_CharacterCustomization& NewCustomization)
{
    CharacterCustomization = NewCustomization;
    UpdateMaterialParameters();
    UpdateMeshVisibility();
}

void AChar_PrimitiveHumanCharacter::SetSkinTone(ESkinTone NewSkinTone)
{
    CharacterCustomization.SkinTone = NewSkinTone;
    UpdateMaterialParameters();
}

void AChar_PrimitiveHumanCharacter::SetHairColor(EHairColor NewHairColor)
{
    CharacterCustomization.HairColor = NewHairColor;
    UpdateMaterialParameters();
}

void AChar_PrimitiveHumanCharacter::SetBodyBuild(EBodyBuild NewBodyBuild)
{
    CharacterCustomization.BodyBuild = NewBodyBuild;
    
    // Adjust character scale based on body build
    FVector NewScale = FVector(1.0f);
    switch (NewBodyBuild)
    {
    case EBodyBuild::Slim:
        NewScale = FVector(0.9f, 0.9f, 1.0f);
        break;
    case EBodyBuild::Athletic:
        NewScale = FVector(1.0f, 1.0f, 1.0f);
        break;
    case EBodyBuild::Muscular:
        NewScale = FVector(1.1f, 1.1f, 1.0f);
        break;
    case EBodyBuild::Heavy:
        NewScale = FVector(1.2f, 1.2f, 1.0f);
        break;
    }
    
    GetMesh()->SetWorldScale3D(NewScale);
}

void AChar_PrimitiveHumanCharacter::ToggleAnimalHide(bool bVisible)
{
    CharacterCustomization.bWearingAnimalHide = bVisible;
    if (AnimalHideMesh)
    {
        AnimalHideMesh->SetVisibility(bVisible);
    }
}

void AChar_PrimitiveHumanCharacter::ToggleBoneJewelry(bool bVisible)
{
    CharacterCustomization.bWearingBoneJewelry = bVisible;
    if (BoneJewelryMesh)
    {
        BoneJewelryMesh->SetVisibility(bVisible);
    }
}

void AChar_PrimitiveHumanCharacter::ToggleStoneAxe(bool bVisible)
{
    CharacterCustomization.bCarryingStoneAxe = bVisible;
    if (StoneAxeMesh)
    {
        StoneAxeMesh->SetVisibility(bVisible);
    }
    
    // Hide spear if axe is shown (can't carry both)
    if (bVisible && CharacterCustomization.bCarryingSpear)
    {
        ToggleSpear(false);
    }
}

void AChar_PrimitiveHumanCharacter::ToggleSpear(bool bVisible)
{
    CharacterCustomization.bCarryingSpear = bVisible;
    if (SpearMesh)
    {
        SpearMesh->SetVisibility(bVisible);
    }
    
    // Hide axe if spear is shown (can't carry both)
    if (bVisible && CharacterCustomization.bCarryingStoneAxe)
    {
        ToggleStoneAxe(false);
    }
}

void AChar_PrimitiveHumanCharacter::SetFacePaint(bool bEnabled)
{
    CharacterCustomization.bHasFacePaint = bEnabled;
    UpdateMaterialParameters();
}

void AChar_PrimitiveHumanCharacter::SetWeatheringLevel(float Level)
{
    CharacterCustomization.WeatheringLevel = FMath::Clamp(Level, 0.0f, 1.0f);
    UpdateMaterialParameters();
}

void AChar_PrimitiveHumanCharacter::UpdateSurvivalStats(float DeltaTime)
{
    ProcessSurvivalDecay(DeltaTime);
    
    // Health regeneration when well-fed and hydrated
    if (SurvivalStats.Hunger > 50.0f && SurvivalStats.Thirst > 50.0f && SurvivalStats.Health < SurvivalStats.MaxHealth)
    {
        SurvivalStats.Health = FMath::Min(SurvivalStats.MaxHealth, SurvivalStats.Health + (5.0f * DeltaTime));
    }
    
    // Stamina regeneration when not moving
    if (GetVelocity().Size() < 10.0f && SurvivalStats.Stamina < SurvivalStats.MaxStamina)
    {
        SurvivalStats.Stamina = FMath::Min(SurvivalStats.MaxStamina, SurvivalStats.Stamina + (20.0f * DeltaTime));
    }
    
    // Fear decay over time
    if (SurvivalStats.Fear > 0.0f)
    {
        SurvivalStats.Fear = FMath::Max(0.0f, SurvivalStats.Fear - (10.0f * DeltaTime));
    }
    
    // Death check
    if (SurvivalStats.Health <= 0.0f)
    {
        // Handle character death
        UE_LOG(LogTemp, Warning, TEXT("Character has died!"));
    }
}

void AChar_PrimitiveHumanCharacter::TakeDamage(float DamageAmount)
{
    SurvivalStats.Health = FMath::Max(0.0f, SurvivalStats.Health - DamageAmount);
    
    // Add fear when taking damage
    AddFear(DamageAmount * 0.5f);
}

void AChar_PrimitiveHumanCharacter::RestoreHealth(float HealAmount)
{
    SurvivalStats.Health = FMath::Min(SurvivalStats.MaxHealth, SurvivalStats.Health + HealAmount);
}

void AChar_PrimitiveHumanCharacter::ConsumeFood(float FoodValue)
{
    SurvivalStats.Hunger = FMath::Min(100.0f, SurvivalStats.Hunger + FoodValue);
}

void AChar_PrimitiveHumanCharacter::DrinkWater(float WaterValue)
{
    SurvivalStats.Thirst = FMath::Min(100.0f, SurvivalStats.Thirst + WaterValue);
}

void AChar_PrimitiveHumanCharacter::UseStamina(float StaminaCost)
{
    SurvivalStats.Stamina = FMath::Max(0.0f, SurvivalStats.Stamina - StaminaCost);
}

void AChar_PrimitiveHumanCharacter::AddFear(float FearAmount)
{
    SurvivalStats.Fear = FMath::Min(100.0f, SurvivalStats.Fear + FearAmount);
}

void AChar_PrimitiveHumanCharacter::ReduceFear(float FearReduction)
{
    SurvivalStats.Fear = FMath::Max(0.0f, SurvivalStats.Fear - FearReduction);
}

void AChar_PrimitiveHumanCharacter::UpdateMaterialParameters()
{
    if (!GetMesh() || !BaseSkinMaterial)
    {
        return;
    }
    
    // Create dynamic material instance
    UMaterialInstanceDynamic* DynamicMaterial = GetMesh()->CreateAndSetMaterialInstanceDynamic(0, BaseSkinMaterial);
    
    if (DynamicMaterial)
    {
        // Set skin tone parameters
        FLinearColor SkinColor = FLinearColor::White;
        switch (CharacterCustomization.SkinTone)
        {
        case ESkinTone::VeryLight:
            SkinColor = FLinearColor(1.0f, 0.9f, 0.8f);
            break;
        case ESkinTone::Light:
            SkinColor = FLinearColor(0.9f, 0.8f, 0.7f);
            break;
        case ESkinTone::Medium:
            SkinColor = FLinearColor(0.8f, 0.7f, 0.6f);
            break;
        case ESkinTone::Dark:
            SkinColor = FLinearColor(0.6f, 0.5f, 0.4f);
            break;
        case ESkinTone::VeryDark:
            SkinColor = FLinearColor(0.4f, 0.3f, 0.2f);
            break;
        }
        
        DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), SkinColor);
        DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), CharacterCustomization.WeatheringLevel);
        DynamicMaterial->SetScalarParameterValue(TEXT("FacePaint"), CharacterCustomization.bHasFacePaint ? 1.0f : 0.0f);
    }
    
    // Update hair material if available
    if (HairMaterial)
    {
        UMaterialInstanceDynamic* HairDynamicMaterial = GetMesh()->CreateAndSetMaterialInstanceDynamic(1, HairMaterial);
        
        if (HairDynamicMaterial)
        {
            FLinearColor HairColor = FLinearColor::Black;
            switch (CharacterCustomization.HairColor)
            {
            case EHairColor::Black:
                HairColor = FLinearColor(0.1f, 0.1f, 0.1f);
                break;
            case EHairColor::Brown:
                HairColor = FLinearColor(0.4f, 0.3f, 0.2f);
                break;
            case EHairColor::Blonde:
                HairColor = FLinearColor(0.8f, 0.7f, 0.5f);
                break;
            case EHairColor::Red:
                HairColor = FLinearColor(0.6f, 0.3f, 0.2f);
                break;
            case EHairColor::Gray:
                HairColor = FLinearColor(0.5f, 0.5f, 0.5f);
                break;
            case EHairColor::White:
                HairColor = FLinearColor(0.9f, 0.9f, 0.9f);
                break;
            }
            
            HairDynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), HairColor);
        }
    }
}

void AChar_PrimitiveHumanCharacter::UpdateMeshVisibility()
{
    if (AnimalHideMesh)
    {
        AnimalHideMesh->SetVisibility(CharacterCustomization.bWearingAnimalHide);
    }
    
    if (BoneJewelryMesh)
    {
        BoneJewelryMesh->SetVisibility(CharacterCustomization.bWearingBoneJewelry);
    }
    
    if (StoneAxeMesh)
    {
        StoneAxeMesh->SetVisibility(CharacterCustomization.bCarryingStoneAxe);
    }
    
    if (SpearMesh)
    {
        SpearMesh->SetVisibility(CharacterCustomization.bCarryingSpear);
    }
}

void AChar_PrimitiveHumanCharacter::ProcessSurvivalDecay(float DeltaTime)
{
    // Hunger decreases over time
    SurvivalStats.Hunger = FMath::Max(0.0f, SurvivalStats.Hunger - (2.0f * DeltaTime));
    
    // Thirst decreases faster than hunger
    SurvivalStats.Thirst = FMath::Max(0.0f, SurvivalStats.Thirst - (3.0f * DeltaTime));
    
    // Health decreases when hungry or thirsty
    if (SurvivalStats.Hunger < 20.0f || SurvivalStats.Thirst < 20.0f)
    {
        SurvivalStats.Health = FMath::Max(0.0f, SurvivalStats.Health - (5.0f * DeltaTime));
    }
    
    // Stamina decreases when moving
    if (GetVelocity().Size() > 10.0f)
    {
        float StaminaDrain = 10.0f * DeltaTime;
        
        // Running drains more stamina
        if (GetVelocity().Size() > 200.0f)
        {
            StaminaDrain *= 2.0f;
        }
        
        UseStamina(StaminaDrain);
    }
    
    // Temperature regulation based on environment
    // This would be expanded with environmental temperature system
    float TargetTemp = 37.0f; // Normal body temperature
    float TempDifference = TargetTemp - SurvivalStats.Temperature;
    SurvivalStats.Temperature += TempDifference * 0.1f * DeltaTime;
}