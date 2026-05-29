#include "TribalCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"

ATribalCharacter::ATribalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Configure character collision
    GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;

    // Configure mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Create weapon mesh component
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create accessory mesh component
    AccessoryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccessoryMesh"));
    AccessoryMesh->SetupAttachment(GetMesh(), TEXT("head"));
    AccessoryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default values
    TribalRole = EChar_TribalRole::Hunter;
    CharacterName = TEXT("Tribal Member");
    Age = 25;
    ExperienceLevel = 1.0f;
    bCanInteract = true;
    InteractionRange = 300.0f;
    InteractionPrompt = TEXT("Talk to tribal member");
    bIsHostile = false;
    bIsFriendly = true;
    TrustLevel = 0.5f;

    // Initialize appearance
    Appearance.SkinTone = 0.5f;
    Appearance.WarPaintColor = FLinearColor::Red;
    Appearance.bHasTribalMarkings = true;
    Appearance.bHasFeatherHeaddress = false;
    Appearance.bHasBoneAccessories = true;

    // Cache pointers
    DynamicSkinMaterial = nullptr;
    DynamicClothingMaterial = nullptr;
}

void ATribalCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    SetupDefaultEquipment();
    ConfigureAppearanceByRole();
    UpdateMaterialParameters();
}

void ATribalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATribalCharacter::SetTribalRole(EChar_TribalRole NewRole)
{
    TribalRole = NewRole;
    ConfigureAppearanceByRole();
    
    // Update character name based on role
    switch (TribalRole)
    {
        case EChar_TribalRole::Elder:
            CharacterName = TEXT("Tribal Elder");
            ExperienceLevel = FMath::Max(ExperienceLevel, 8.0f);
            Age = FMath::Max(Age, 50);
            break;
        case EChar_TribalRole::Hunter:
            CharacterName = TEXT("Tribal Hunter");
            ExperienceLevel = FMath::Max(ExperienceLevel, 3.0f);
            break;
        case EChar_TribalRole::Gatherer:
            CharacterName = TEXT("Tribal Gatherer");
            ExperienceLevel = FMath::Max(ExperienceLevel, 2.0f);
            break;
        case EChar_TribalRole::Crafter:
            CharacterName = TEXT("Tribal Crafter");
            ExperienceLevel = FMath::Max(ExperienceLevel, 4.0f);
            break;
        case EChar_TribalRole::Scout:
            CharacterName = TEXT("Tribal Scout");
            ExperienceLevel = FMath::Max(ExperienceLevel, 3.0f);
            GetCharacterMovement()->MaxWalkSpeed = 400.0f; // Scouts are faster
            break;
    }
}

void ATribalCharacter::ApplyTribalAppearance(const FChar_TribalAppearance& NewAppearance)
{
    Appearance = NewAppearance;
    UpdateMaterialParameters();
}

void ATribalCharacter::EquipWeapon(UStaticMesh* WeaponMeshAsset)
{
    if (WeaponMesh && WeaponMeshAsset)
    {
        WeaponMesh->SetStaticMesh(WeaponMeshAsset);
        WeaponMesh->SetVisibility(true);
    }
}

void ATribalCharacter::EquipAccessory(UStaticMesh* AccessoryMeshAsset)
{
    if (AccessoryMesh && AccessoryMeshAsset)
    {
        AccessoryMesh->SetStaticMesh(AccessoryMeshAsset);
        AccessoryMesh->SetVisibility(true);
    }
}

bool ATribalCharacter::CanPlayerInteract(APawn* PlayerPawn) const
{
    if (!bCanInteract || !PlayerPawn)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= InteractionRange;
}

void ATribalCharacter::ModifyTrustLevel(float DeltaTrust)
{
    TrustLevel = FMath::Clamp(TrustLevel + DeltaTrust, 0.0f, 1.0f);
    
    // Update behavior based on trust level
    if (TrustLevel < 0.2f)
    {
        bIsFriendly = false;
        bIsHostile = true;
        InteractionPrompt = TEXT("Hostile tribal member");
    }
    else if (TrustLevel > 0.8f)
    {
        bIsFriendly = true;
        bIsHostile = false;
        InteractionPrompt = TEXT("Friendly tribal member");
    }
    else
    {
        bIsFriendly = true;
        bIsHostile = false;
        InteractionPrompt = TEXT("Talk to tribal member");
    }
}

void ATribalCharacter::SetupDefaultEquipment()
{
    // Configure default equipment based on role
    switch (TribalRole)
    {
        case EChar_TribalRole::Elder:
            // Elders have staffs and elaborate headdresses
            Appearance.bHasFeatherHeaddress = true;
            break;
        case EChar_TribalRole::Hunter:
            // Hunters have spears and minimal accessories for mobility
            Appearance.bHasBoneAccessories = true;
            Appearance.bHasFeatherHeaddress = false;
            break;
        case EChar_TribalRole::Gatherer:
            // Gatherers have baskets and simple tools
            Appearance.bHasBoneAccessories = false;
            Appearance.bHasFeatherHeaddress = false;
            break;
        case EChar_TribalRole::Crafter:
            // Crafters have tool belts and work accessories
            Appearance.bHasBoneAccessories = true;
            break;
        case EChar_TribalRole::Scout:
            // Scouts have minimal gear for speed
            Appearance.bHasBoneAccessories = false;
            Appearance.bHasFeatherHeaddress = false;
            break;
    }
}

void ATribalCharacter::ConfigureAppearanceByRole()
{
    // Configure appearance details based on role
    switch (TribalRole)
    {
        case EChar_TribalRole::Elder:
            Appearance.SkinTone = 0.3f; // Weathered skin
            Appearance.WarPaintColor = FLinearColor::Blue; // Wisdom markings
            Appearance.bHasTribalMarkings = true;
            break;
        case EChar_TribalRole::Hunter:
            Appearance.SkinTone = 0.6f; // Tanned from outdoor work
            Appearance.WarPaintColor = FLinearColor::Red; // War paint
            Appearance.bHasTribalMarkings = true;
            break;
        case EChar_TribalRole::Gatherer:
            Appearance.SkinTone = 0.5f; // Normal skin tone
            Appearance.WarPaintColor = FLinearColor::Green; // Nature markings
            Appearance.bHasTribalMarkings = false;
            break;
        case EChar_TribalRole::Crafter:
            Appearance.SkinTone = 0.4f; // Indoor work, less sun
            Appearance.WarPaintColor = FLinearColor::Yellow; // Craft markings
            Appearance.bHasTribalMarkings = true;
            break;
        case EChar_TribalRole::Scout:
            Appearance.SkinTone = 0.7f; // Very tanned from travel
            Appearance.WarPaintColor = FLinearColor::Black; // Camouflage markings
            Appearance.bHasTribalMarkings = true;
            break;
    }
}

void ATribalCharacter::UpdateMaterialParameters()
{
    if (!GetMesh() || !GetMesh()->GetMaterial(0))
    {
        return;
    }

    // Create dynamic material instances if they don't exist
    if (!DynamicSkinMaterial)
    {
        DynamicSkinMaterial = GetMesh()->CreateDynamicMaterialInstance(0);
    }

    if (DynamicSkinMaterial)
    {
        // Apply skin tone
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("SkinTone"), Appearance.SkinTone);
        
        // Apply war paint color
        DynamicSkinMaterial->SetVectorParameterValue(TEXT("WarPaintColor"), Appearance.WarPaintColor);
        
        // Apply tribal markings visibility
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("TribalMarkings"), Appearance.bHasTribalMarkings ? 1.0f : 0.0f);
    }

    // Update accessory visibility
    if (AccessoryMesh)
    {
        AccessoryMesh->SetVisibility(Appearance.bHasFeatherHeaddress || Appearance.bHasBoneAccessories);
    }
}