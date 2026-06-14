#include "Char_PrimitiveHumanCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"

AChar_PrimitiveHumanCharacter::AChar_PrimitiveHumanCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set default character properties
    GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);
    
    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;

    // Initialize default appearance values
    SkinWeatheringLevel = 0.6f;
    SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    // Initialize clothing defaults
    TribalClothing.bWearingAnimalHide = true;
    TribalClothing.bWearingBoneAccessories = true;
    TribalClothing.HideColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    TribalClothing.WearLevel = 0.5f;

    // Initialize weapon defaults
    PrimitiveWeapons.bHasStoneSpear = true;
    PrimitiveWeapons.bHasStoneAxe = false;
    PrimitiveWeapons.bHasBoneKnife = false;
    PrimitiveWeapons.WeaponDurability = 1.0f;

    // Create weapon components
    CreateWeaponComponents();
}

void AChar_PrimitiveHumanCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateCharacterAppearance();
    UpdateWeaponVisibility();
}

void AChar_PrimitiveHumanCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AChar_PrimitiveHumanCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AChar_PrimitiveHumanCharacter::CreateWeaponComponents()
{
    // Create spear component
    SpearMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpearMesh"));
    SpearMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_r"));
    SpearMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    SpearMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    SpearMeshComponent->SetVisibility(false);

    // Create axe component
    AxeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AxeMesh"));
    AxeMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_r"));
    AxeMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    AxeMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    AxeMeshComponent->SetVisibility(false);

    // Create knife component
    KnifeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KnifeMesh"));
    KnifeMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_r"));
    KnifeMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    KnifeMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    KnifeMeshComponent->SetVisibility(false);
}

void AChar_PrimitiveHumanCharacter::SetTribalClothing(const FChar_TribalClothing& NewClothing)
{
    TribalClothing = NewClothing;
    UpdateCharacterAppearance();
}

void AChar_PrimitiveHumanCharacter::SetPrimitiveWeapons(const FChar_PrimitiveWeapons& NewWeapons)
{
    PrimitiveWeapons = NewWeapons;
    UpdateWeaponVisibility();
}

void AChar_PrimitiveHumanCharacter::SetSkinWeathering(float WeatheringLevel)
{
    SkinWeatheringLevel = FMath::Clamp(WeatheringLevel, 0.0f, 1.0f);
    UpdateCharacterAppearance();
}

void AChar_PrimitiveHumanCharacter::SetSkinTone(FLinearColor NewSkinTone)
{
    SkinTone = NewSkinTone;
    UpdateCharacterAppearance();
}

void AChar_PrimitiveHumanCharacter::EquipSpear()
{
    UnequipAllWeapons();
    PrimitiveWeapons.bHasStoneSpear = true;
    if (SpearMeshComponent)
    {
        SpearMeshComponent->SetVisibility(true);
    }
}

void AChar_PrimitiveHumanCharacter::EquipAxe()
{
    UnequipAllWeapons();
    PrimitiveWeapons.bHasStoneAxe = true;
    if (AxeMeshComponent)
    {
        AxeMeshComponent->SetVisibility(true);
    }
}

void AChar_PrimitiveHumanCharacter::EquipKnife()
{
    UnequipAllWeapons();
    PrimitiveWeapons.bHasBoneKnife = true;
    if (KnifeMeshComponent)
    {
        KnifeMeshComponent->SetVisibility(true);
    }
}

void AChar_PrimitiveHumanCharacter::UnequipAllWeapons()
{
    if (SpearMeshComponent) SpearMeshComponent->SetVisibility(false);
    if (AxeMeshComponent) AxeMeshComponent->SetVisibility(false);
    if (KnifeMeshComponent) KnifeMeshComponent->SetVisibility(false);
}

bool AChar_PrimitiveHumanCharacter::IsWearingTribalClothing() const
{
    return TribalClothing.bWearingAnimalHide || TribalClothing.bWearingBoneAccessories;
}

bool AChar_PrimitiveHumanCharacter::HasAnyWeapon() const
{
    return PrimitiveWeapons.bHasStoneSpear || PrimitiveWeapons.bHasStoneAxe || PrimitiveWeapons.bHasBoneKnife;
}

float AChar_PrimitiveHumanCharacter::GetOverallCondition() const
{
    float clothingCondition = 1.0f - TribalClothing.WearLevel;
    float weaponCondition = PrimitiveWeapons.WeaponDurability;
    float skinCondition = 1.0f - SkinWeatheringLevel;
    
    return (clothingCondition + weaponCondition + skinCondition) / 3.0f;
}

void AChar_PrimitiveHumanCharacter::UpdateCharacterAppearance()
{
    // Update character mesh materials based on skin tone and weathering
    if (GetMesh() && GetMesh()->GetMaterial(0))
    {
        UMaterialInterface* BaseMaterial = GetMesh()->GetMaterial(0);
        if (BaseMaterial)
        {
            // Create dynamic material instance for runtime customization
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetVectorParameterValue(TEXT("SkinTone"), SkinTone);
                DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), SkinWeatheringLevel);
                DynamicMaterial->SetVectorParameterValue(TEXT("ClothingColor"), TribalClothing.HideColor);
                DynamicMaterial->SetScalarParameterValue(TEXT("ClothingWear"), TribalClothing.WearLevel);
                
                GetMesh()->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void AChar_PrimitiveHumanCharacter::UpdateWeaponVisibility()
{
    // Update weapon visibility based on current weapon configuration
    if (SpearMeshComponent)
    {
        SpearMeshComponent->SetVisibility(PrimitiveWeapons.bHasStoneSpear);
    }
    
    if (AxeMeshComponent)
    {
        AxeMeshComponent->SetVisibility(PrimitiveWeapons.bHasStoneAxe);
    }
    
    if (KnifeMeshComponent)
    {
        KnifeMeshComponent->SetVisibility(PrimitiveWeapons.bHasBoneKnife);
    }
}