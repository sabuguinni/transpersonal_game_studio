#include "Char_TribalWarriorAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

UChar_TribalWarriorAsset::UChar_TribalWarriorAsset()
{
    // Initialize default values
    bHasFacePaint = false;
    bHasScars = false;
    bHasTattoos = false;
    WeatheringLevel = 0.5f;
    bIsFemale = false;
}

void UChar_TribalWarriorAsset::ApplyToSkeletalMesh(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UChar_TribalWarriorAsset::ApplyToSkeletalMesh - Invalid MeshComponent"));
        return;
    }

    // Apply the warrior mesh if available
    if (WarriorMesh.IsValid())
    {
        USkeletalMesh* LoadedMesh = WarriorMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            MeshComponent->SetSkeletalMesh(LoadedMesh);
            UE_LOG(LogTemp, Log, TEXT("Applied tribal warrior mesh to character"));
        }
    }

    // Apply random skin material
    UMaterialInterface* SkinMat = GetRandomSkinMaterial();
    if (SkinMat)
    {
        MeshComponent->SetMaterial(0, SkinMat); // Assume slot 0 is skin
        UE_LOG(LogTemp, Log, TEXT("Applied skin material to character"));
    }

    // Apply random clothing material
    UMaterialInterface* ClothingMat = GetRandomClothingMaterial();
    if (ClothingMat)
    {
        MeshComponent->SetMaterial(1, ClothingMat); // Assume slot 1 is clothing
        UE_LOG(LogTemp, Log, TEXT("Applied clothing material to character"));
    }

    // Apply random jewelry material
    UMaterialInterface* JewelryMat = GetRandomJewelryMaterial();
    if (JewelryMat)
    {
        MeshComponent->SetMaterial(2, JewelryMat); // Assume slot 2 is jewelry
        UE_LOG(LogTemp, Log, TEXT("Applied jewelry material to character"));
    }

    // Apply random tool material
    UMaterialInterface* ToolMat = GetRandomToolMaterial();
    if (ToolMat)
    {
        MeshComponent->SetMaterial(3, ToolMat); // Assume slot 3 is tools
        UE_LOG(LogTemp, Log, TEXT("Applied tool material to character"));
    }
}

UMaterialInterface* UChar_TribalWarriorAsset::GetRandomSkinMaterial() const
{
    if (SkinMaterials.Num() == 0)
    {
        return nullptr;
    }

    int32 RandomIndex = FMath::RandRange(0, SkinMaterials.Num() - 1);
    TSoftObjectPtr<UMaterialInterface> SoftMaterial = SkinMaterials[RandomIndex];
    
    if (SoftMaterial.IsValid())
    {
        return SoftMaterial.LoadSynchronous();
    }

    return nullptr;
}

UMaterialInterface* UChar_TribalWarriorAsset::GetRandomClothingMaterial() const
{
    if (ClothingMaterials.Num() == 0)
    {
        return nullptr;
    }

    int32 RandomIndex = FMath::RandRange(0, ClothingMaterials.Num() - 1);
    TSoftObjectPtr<UMaterialInterface> SoftMaterial = ClothingMaterials[RandomIndex];
    
    if (SoftMaterial.IsValid())
    {
        return SoftMaterial.LoadSynchronous();
    }

    return nullptr;
}

UMaterialInterface* UChar_TribalWarriorAsset::GetRandomJewelryMaterial() const
{
    if (JewelryMaterials.Num() == 0)
    {
        return nullptr;
    }

    int32 RandomIndex = FMath::RandRange(0, JewelryMaterials.Num() - 1);
    TSoftObjectPtr<UMaterialInterface> SoftMaterial = JewelryMaterials[RandomIndex];
    
    if (SoftMaterial.IsValid())
    {
        return SoftMaterial.LoadSynchronous();
    }

    return nullptr;
}

UMaterialInterface* UChar_TribalWarriorAsset::GetRandomToolMaterial() const
{
    if (ToolMaterials.Num() == 0)
    {
        return nullptr;
    }

    int32 RandomIndex = FMath::RandRange(0, ToolMaterials.Num() - 1);
    TSoftObjectPtr<UMaterialInterface> SoftMaterial = ToolMaterials[RandomIndex];
    
    if (SoftMaterial.IsValid())
    {
        return SoftMaterial.LoadSynchronous();
    }

    return nullptr;
}