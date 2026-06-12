#include "Char_PrimitiveWarrior.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

AChar_PrimitiveWarrior::AChar_PrimitiveWarrior()
{
    PrimaryActorTick.bCanEverTick = false;

    // Set default tribal properties
    TribalRank = EChar_TribalRank::Hunter;
    TribalName = TEXT("Unnamed Warrior");

    // Initialize appearance with default values
    TribalAppearance = FChar_TribalAppearance();
    TribalEquipment = FChar_TribalEquipment();

    // Create equipment components
    WeaponComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponComponent"));
    JewelryComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JewelryComponent"));
    ClothingComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClothingComponent"));

    // Attach equipment to mesh
    if (GetMesh())
    {
        WeaponComponent->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));
        JewelryComponent->SetupAttachment(GetMesh(), TEXT("NeckSocket"));
        ClothingComponent->SetupAttachment(GetMesh());
    }

    // Configure character movement for primitive warrior
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 500.0f;
        GetCharacterMovement()->JumpZVelocity = 400.0f;
        GetCharacterMovement()->AirControl = 0.15f;
        GetCharacterMovement()->GroundFriction = 8.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
    }

    // Configure collision for tribal character
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
        GetCapsuleComponent()->SetCapsuleRadius(34.0f);
    }

    // Initialize material instances to nullptr
    SkinMaterialInstance = nullptr;
    ClothingMaterialInstance = nullptr;
    WarPaintMaterialInstance = nullptr;
}

void AChar_PrimitiveWarrior::BeginPlay()
{
    Super::BeginPlay();

    // Setup equipment and materials
    SetupEquipmentComponents();
    SetupMaterialInstances();
    ApplyTribalCustomization();
}

void AChar_PrimitiveWarrior::SetTribalAppearance(const FChar_TribalAppearance& NewAppearance)
{
    TribalAppearance = NewAppearance;
    
    // Update material instances with new appearance
    UpdateSkinMaterial();
    UpdateClothingMaterial();
    UpdateWarPaintMaterial();
}

void AChar_PrimitiveWarrior::EquipWeapon(UStaticMesh* WeaponMesh, EChar_WeaponType WeaponType)
{
    if (WeaponComponent && WeaponMesh)
    {
        WeaponComponent->SetStaticMesh(WeaponMesh);
        TribalEquipment.WeaponMesh = WeaponMesh;
        TribalEquipment.WeaponType = WeaponType;
        
        UE_LOG(LogTemp, Log, TEXT("Equipped weapon: %s"), *WeaponMesh->GetName());
    }
}

void AChar_PrimitiveWarrior::EquipJewelry(UStaticMesh* JewelryMesh)
{
    if (JewelryComponent && JewelryMesh)
    {
        JewelryComponent->SetStaticMesh(JewelryMesh);
        TribalEquipment.JewelryMesh = JewelryMesh;
        
        UE_LOG(LogTemp, Log, TEXT("Equipped jewelry: %s"), *JewelryMesh->GetName());
    }
}

void AChar_PrimitiveWarrior::EquipClothing(UStaticMesh* ClothingMesh)
{
    if (ClothingComponent && ClothingMesh)
    {
        ClothingComponent->SetStaticMesh(ClothingMesh);
        TribalEquipment.ClothingMesh = ClothingMesh;
        
        UE_LOG(LogTemp, Log, TEXT("Equipped clothing: %s"), *ClothingMesh->GetName());
    }
}

void AChar_PrimitiveWarrior::ApplyWarPaint(const FLinearColor& PaintColor, UMaterialInterface* PaintMaterial)
{
    TribalAppearance.WarPaintColor = PaintColor;
    TribalAppearance.WarPaintMaterial = PaintMaterial;
    
    UpdateWarPaintMaterial();
}

void AChar_PrimitiveWarrior::SetTribalRank(EChar_TribalRank NewRank)
{
    TribalRank = NewRank;
    
    // Adjust character stats based on rank
    switch (NewRank)
    {
        case EChar_TribalRank::Hunter:
            TribalAppearance.MuscleMass = 0.8f;
            TribalAppearance.BattleScars = 0.2f;
            break;
        case EChar_TribalRank::Warrior:
            TribalAppearance.MuscleMass = 1.0f;
            TribalAppearance.BattleScars = 0.4f;
            break;
        case EChar_TribalRank::Chieftain:
            TribalAppearance.MuscleMass = 1.2f;
            TribalAppearance.BattleScars = 0.6f;
            break;
        case EChar_TribalRank::Shaman:
            TribalAppearance.MuscleMass = 0.7f;
            TribalAppearance.BattleScars = 0.3f;
            break;
        case EChar_TribalRank::Scout:
            TribalAppearance.MuscleMass = 0.9f;
            TribalAppearance.BattleScars = 0.3f;
            break;
    }
    
    // Update appearance based on new rank
    UpdateSkinMaterial();
}

void AChar_PrimitiveWarrior::UpdateSkinMaterial()
{
    if (GetMesh() && BaseSkinMaterial)
    {
        if (!SkinMaterialInstance)
        {
            SkinMaterialInstance = UMaterialInstanceDynamic::Create(BaseSkinMaterial, this);
        }
        
        if (SkinMaterialInstance)
        {
            // Apply skin tone
            SkinMaterialInstance->SetVectorParameterValue(TEXT("SkinTone"), TribalAppearance.SkinTone);
            
            // Apply muscle mass and battle scars
            SkinMaterialInstance->SetScalarParameterValue(TEXT("MuscleMass"), TribalAppearance.MuscleMass);
            SkinMaterialInstance->SetScalarParameterValue(TEXT("BattleScars"), TribalAppearance.BattleScars);
            
            // Apply to character mesh
            GetMesh()->SetMaterial(0, SkinMaterialInstance);
        }
    }
}

void AChar_PrimitiveWarrior::UpdateClothingMaterial()
{
    if (ClothingComponent && BaseClothingMaterial)
    {
        if (!ClothingMaterialInstance)
        {
            ClothingMaterialInstance = UMaterialInstanceDynamic::Create(BaseClothingMaterial, this);
        }
        
        if (ClothingMaterialInstance)
        {
            // Apply clothing customization
            ClothingComponent->SetMaterial(0, ClothingMaterialInstance);
        }
    }
}

void AChar_PrimitiveWarrior::UpdateWarPaintMaterial()
{
    if (GetMesh() && BaseWarPaintMaterial)
    {
        if (!WarPaintMaterialInstance)
        {
            WarPaintMaterialInstance = UMaterialInstanceDynamic::Create(BaseWarPaintMaterial, this);
        }
        
        if (WarPaintMaterialInstance)
        {
            // Apply war paint color
            WarPaintMaterialInstance->SetVectorParameterValue(TEXT("PaintColor"), TribalAppearance.WarPaintColor);
            
            // Apply as overlay material
            GetMesh()->SetOverlayMaterial(WarPaintMaterialInstance);
        }
    }
}

void AChar_PrimitiveWarrior::SetupEquipmentComponents()
{
    if (!WeaponComponent || !JewelryComponent || !ClothingComponent)
    {
        return;
    }

    // Configure weapon component
    WeaponComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponComponent->SetCastShadow(true);
    
    // Configure jewelry component
    JewelryComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    JewelryComponent->SetCastShadow(false);
    
    // Configure clothing component
    ClothingComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ClothingComponent->SetCastShadow(true);
    
    // Attach to proper sockets
    AttachEquipmentToSockets();
}

void AChar_PrimitiveWarrior::SetupMaterialInstances()
{
    // Create material instances if base materials are available
    if (BaseSkinMaterial)
    {
        SkinMaterialInstance = UMaterialInstanceDynamic::Create(BaseSkinMaterial, this);
    }
    
    if (BaseClothingMaterial)
    {
        ClothingMaterialInstance = UMaterialInstanceDynamic::Create(BaseClothingMaterial, this);
    }
    
    if (BaseWarPaintMaterial)
    {
        WarPaintMaterialInstance = UMaterialInstanceDynamic::Create(BaseWarPaintMaterial, this);
    }
}

void AChar_PrimitiveWarrior::AttachEquipmentToSockets()
{
    if (!GetMesh())
    {
        return;
    }

    // Attach weapon to weapon socket
    if (WeaponComponent && GetMesh()->DoesSocketExist(TribalEquipment.WeaponSocket))
    {
        WeaponComponent->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            TribalEquipment.WeaponSocket
        );
    }
    
    // Attach jewelry to neck socket
    if (JewelryComponent && GetMesh()->DoesSocketExist(TribalEquipment.JewelrySocket))
    {
        JewelryComponent->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            TribalEquipment.JewelrySocket
        );
    }
}

void AChar_PrimitiveWarrior::ApplyTribalCustomization()
{
    // Apply all current tribal appearance settings
    UpdateSkinMaterial();
    UpdateClothingMaterial();
    UpdateWarPaintMaterial();
    
    // Log character setup
    UE_LOG(LogTemp, Log, TEXT("Applied tribal customization for %s (%s)"), 
        *TribalName, 
        *UEnum::GetValueAsString(TribalRank));
}