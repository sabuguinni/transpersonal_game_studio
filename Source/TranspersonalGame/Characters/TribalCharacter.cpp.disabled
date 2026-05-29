#include "TribalCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

ATribalCharacter::ATribalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create weapon mesh component
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create tool mesh component
    ToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
    ToolMesh->SetupAttachment(GetMesh(), TEXT("hand_l"));
    ToolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Set default character properties
    CharacterName = TEXT("Tribal Hunter");
    Age = 25;
    bIsMale = true;
    TribalRole = EChar_TribalRole::Hunter;

    // Initialize tribal stats based on role
    TribalStats.Strength = 60.0f;
    TribalStats.Agility = 70.0f;
    TribalStats.Wisdom = 40.0f;
    TribalStats.Survival = 80.0f;
}

void ATribalCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    ApplyTribalMarkings();
    SetupDefaultEquipment();
}

void ATribalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATribalCharacter::SetTribalRole(EChar_TribalRole NewRole)
{
    TribalRole = NewRole;
    
    // Adjust stats based on role
    switch (TribalRole)
    {
        case EChar_TribalRole::Hunter:
            TribalStats.Strength = 60.0f;
            TribalStats.Agility = 70.0f;
            TribalStats.Wisdom = 40.0f;
            TribalStats.Survival = 80.0f;
            break;
            
        case EChar_TribalRole::Gatherer:
            TribalStats.Strength = 40.0f;
            TribalStats.Agility = 60.0f;
            TribalStats.Wisdom = 70.0f;
            TribalStats.Survival = 75.0f;
            break;
            
        case EChar_TribalRole::Shaman:
            TribalStats.Strength = 30.0f;
            TribalStats.Agility = 40.0f;
            TribalStats.Wisdom = 90.0f;
            TribalStats.Survival = 60.0f;
            break;
            
        case EChar_TribalRole::Warrior:
            TribalStats.Strength = 80.0f;
            TribalStats.Agility = 60.0f;
            TribalStats.Wisdom = 30.0f;
            TribalStats.Survival = 70.0f;
            break;
            
        case EChar_TribalRole::Elder:
            TribalStats.Strength = 35.0f;
            TribalStats.Agility = 30.0f;
            TribalStats.Wisdom = 95.0f;
            TribalStats.Survival = 85.0f;
            break;
    }
}

void ATribalCharacter::SetCharacterName(const FString& NewName)
{
    CharacterName = NewName;
}

void ATribalCharacter::ApplyTribalMarkings()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        return;
    }

    // Create dynamic material instance for tribal markings
    UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("SkinTone"), SkinTone);
            DynamicMaterial->SetVectorParameterValue(TEXT("TribalMarkingColor"), TribalMarkingColor);
            MeshComp->SetMaterial(0, DynamicMaterial);
        }
    }
}

void ATribalCharacter::EquipWeapon(UStaticMesh* WeaponMeshAsset)
{
    if (WeaponMesh && WeaponMeshAsset)
    {
        WeaponMesh->SetStaticMesh(WeaponMeshAsset);
        WeaponMesh->SetVisibility(true);
    }
}

void ATribalCharacter::EquipTool(UStaticMesh* ToolMeshAsset)
{
    if (ToolMesh && ToolMeshAsset)
    {
        ToolMesh->SetStaticMesh(ToolMeshAsset);
        ToolMesh->SetVisibility(true);
    }
}

float ATribalCharacter::GetStatValue(const FString& StatName) const
{
    if (StatName == TEXT("Strength"))
    {
        return TribalStats.Strength;
    }
    else if (StatName == TEXT("Agility"))
    {
        return TribalStats.Agility;
    }
    else if (StatName == TEXT("Wisdom"))
    {
        return TribalStats.Wisdom;
    }
    else if (StatName == TEXT("Survival"))
    {
        return TribalStats.Survival;
    }
    
    return 0.0f;
}

void ATribalCharacter::ModifyStatValue(const FString& StatName, float Delta)
{
    if (StatName == TEXT("Strength"))
    {
        TribalStats.Strength = FMath::Clamp(TribalStats.Strength + Delta, 0.0f, 100.0f);
    }
    else if (StatName == TEXT("Agility"))
    {
        TribalStats.Agility = FMath::Clamp(TribalStats.Agility + Delta, 0.0f, 100.0f);
    }
    else if (StatName == TEXT("Wisdom"))
    {
        TribalStats.Wisdom = FMath::Clamp(TribalStats.Wisdom + Delta, 0.0f, 100.0f);
    }
    else if (StatName == TEXT("Survival"))
    {
        TribalStats.Survival = FMath::Clamp(TribalStats.Survival + Delta, 0.0f, 100.0f);
    }
}

void ATribalCharacter::RandomizeAppearance()
{
    // Randomize skin tone
    float Hue = FMath::RandRange(0.05f, 0.15f); // Brown to tan range
    float Saturation = FMath::RandRange(0.3f, 0.7f);
    float Value = FMath::RandRange(0.4f, 0.8f);
    SkinTone = FLinearColor::MakeFromHSV8(Hue * 255, Saturation * 255, Value * 255);

    // Randomize tribal marking color
    TribalMarkingColor = FLinearColor(
        FMath::RandRange(0.0f, 0.3f),
        FMath::RandRange(0.0f, 0.2f),
        FMath::RandRange(0.0f, 0.1f),
        1.0f
    );

    // Randomize gender
    bIsMale = FMath::RandBool();

    // Randomize age
    Age = FMath::RandRange(18, 60);

    ApplyTribalMarkings();
}

void ATribalCharacter::SetupDefaultEquipment()
{
    // This will be called to set up default weapons and tools
    // Implementation depends on available static mesh assets
    if (WeaponMesh)
    {
        WeaponMesh->SetVisibility(false); // Hide until weapon is equipped
    }
    
    if (ToolMesh)
    {
        ToolMesh->SetVisibility(false); // Hide until tool is equipped
    }
}