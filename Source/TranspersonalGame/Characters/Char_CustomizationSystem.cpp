#include "Char_CustomizationSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"

AChar_CustomizationSystem::AChar_CustomizationSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create display mesh component
    DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
    RootComponent = DisplayMesh;

    // Set default customization
    DefaultCustomization = FChar_CustomizationData();
    
    LoadDefaultAssets();
}

void AChar_CustomizationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply default customization on start
    ApplyCustomization(DefaultCustomization);
}

void AChar_CustomizationSystem::LoadDefaultAssets()
{
    // Load default clothing meshes (using engine basic shapes as placeholders)
    UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
    UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));

    if (CylinderMesh)
    {
        ClothingMeshes.Add(EChar_ClothingType::AnimalHide_Basic, CylinderMesh);
        ClothingMeshes.Add(EChar_ClothingType::AnimalHide_Decorated, CylinderMesh);
        ClothingMeshes.Add(EChar_ClothingType::PlantFiber_Woven, CylinderMesh);
        ClothingMeshes.Add(EChar_ClothingType::Leather_Crafted, CylinderMesh);
        ClothingMeshes.Add(EChar_ClothingType::Fur_Winter, CylinderMesh);
    }

    // Load default skeletal meshes for body types
    USkeletalMesh* MannequinMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
    if (!MannequinMesh)
    {
        MannequinMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
    }

    if (MannequinMesh)
    {
        BodyMeshes.Add(EChar_BodyType::Lean_Hunter, MannequinMesh);
        BodyMeshes.Add(EChar_BodyType::Strong_Warrior, MannequinMesh);
        BodyMeshes.Add(EChar_BodyType::Wise_Elder, MannequinMesh);
        BodyMeshes.Add(EChar_BodyType::Agile_Scout, MannequinMesh);
        BodyMeshes.Add(EChar_BodyType::Sturdy_Gatherer, MannequinMesh);
    }
}

void AChar_CustomizationSystem::ApplyCustomization(const FChar_CustomizationData& CustomizationData)
{
    if (!ValidateCustomizationData(CustomizationData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid customization data provided"));
        return;
    }

    // Apply clothing mesh
    if (ClothingMeshes.Contains(CustomizationData.ClothingType))
    {
        UStaticMesh* ClothingMesh = ClothingMeshes[CustomizationData.ClothingType];
        if (ClothingMesh && DisplayMesh)
        {
            DisplayMesh->SetStaticMesh(ClothingMesh);
        }
    }

    UpdateDisplayMesh();
}

FChar_CustomizationData AChar_CustomizationSystem::GetRandomCustomization()
{
    FChar_CustomizationData RandomData;

    // Random clothing type
    int32 ClothingIndex = FMath::RandRange(0, 4);
    RandomData.ClothingType = static_cast<EChar_ClothingType>(ClothingIndex);

    // Random body type
    int32 BodyIndex = FMath::RandRange(0, 4);
    RandomData.BodyType = static_cast<EChar_BodyType>(BodyIndex);

    // Random tribal markings
    int32 MarkingIndex = FMath::RandRange(0, 4);
    RandomData.TribalMarkings = static_cast<EChar_TribalMarking>(MarkingIndex);

    // Random skin tone (earth tones)
    RandomData.SkinTone = FLinearColor(
        FMath::RandRange(0.6f, 0.9f),
        FMath::RandRange(0.4f, 0.7f),
        FMath::RandRange(0.2f, 0.5f),
        1.0f
    );

    // Random hair color (natural colors)
    RandomData.HairColor = FLinearColor(
        FMath::RandRange(0.1f, 0.6f),
        FMath::RandRange(0.1f, 0.4f),
        FMath::RandRange(0.05f, 0.2f),
        1.0f
    );

    // Random tools
    TArray<FString> PossibleTools = {
        TEXT("Stone_Spear"),
        TEXT("Flint_Knife"),
        TEXT("Wooden_Club"),
        TEXT("Bone_Needle"),
        TEXT("Stone_Axe")
    };

    int32 NumTools = FMath::RandRange(1, 3);
    for (int32 i = 0; i < NumTools; i++)
    {
        int32 ToolIndex = FMath::RandRange(0, PossibleTools.Num() - 1);
        RandomData.EquippedTools.AddUnique(PossibleTools[ToolIndex]);
    }

    return RandomData;
}

void AChar_CustomizationSystem::SetClothingType(EChar_ClothingType NewClothingType)
{
    DefaultCustomization.ClothingType = NewClothingType;
    ApplyCustomization(DefaultCustomization);
}

void AChar_CustomizationSystem::SetBodyType(EChar_BodyType NewBodyType)
{
    DefaultCustomization.BodyType = NewBodyType;
    ApplyCustomization(DefaultCustomization);
}

void AChar_CustomizationSystem::SetTribalMarkings(EChar_TribalMarking NewMarkings)
{
    DefaultCustomization.TribalMarkings = NewMarkings;
    ApplyCustomization(DefaultCustomization);
}

bool AChar_CustomizationSystem::ValidateCustomizationData(const FChar_CustomizationData& Data)
{
    // Check if clothing type is valid
    if (!ClothingMeshes.Contains(Data.ClothingType))
    {
        return false;
    }

    // Check if body type is valid
    if (!BodyMeshes.Contains(Data.BodyType))
    {
        return false;
    }

    // Check color values are in valid range
    if (Data.SkinTone.R < 0.0f || Data.SkinTone.R > 1.0f ||
        Data.SkinTone.G < 0.0f || Data.SkinTone.G > 1.0f ||
        Data.SkinTone.B < 0.0f || Data.SkinTone.B > 1.0f)
    {
        return false;
    }

    if (Data.HairColor.R < 0.0f || Data.HairColor.R > 1.0f ||
        Data.HairColor.G < 0.0f || Data.HairColor.G > 1.0f ||
        Data.HairColor.B < 0.0f || Data.HairColor.B > 1.0f)
    {
        return false;
    }

    return true;
}

void AChar_CustomizationSystem::UpdateDisplayMesh()
{
    if (DisplayMesh)
    {
        // Apply scale based on body type
        FVector Scale = FVector(1.0f);
        switch (DefaultCustomization.BodyType)
        {
        case EChar_BodyType::Lean_Hunter:
            Scale = FVector(0.9f, 0.9f, 1.0f);
            break;
        case EChar_BodyType::Strong_Warrior:
            Scale = FVector(1.2f, 1.2f, 1.1f);
            break;
        case EChar_BodyType::Wise_Elder:
            Scale = FVector(1.0f, 1.0f, 0.95f);
            break;
        case EChar_BodyType::Agile_Scout:
            Scale = FVector(0.85f, 0.85f, 1.05f);
            break;
        case EChar_BodyType::Sturdy_Gatherer:
            Scale = FVector(1.1f, 1.1f, 1.0f);
            break;
        }
        
        SetActorScale3D(Scale);
    }
}