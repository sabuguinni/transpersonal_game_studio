#include "Char_TribalWarriorAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"

UChar_TribalWarriorAsset::UChar_TribalWarriorAsset()
{
    // Initialize with default configuration
    WarriorConfig = FChar_TribalWarriorConfig();
}

void UChar_TribalWarriorAsset::ApplyToSkeletalMesh(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UChar_TribalWarriorAsset::ApplyToSkeletalMesh - MeshComponent is null"));
        return;
    }

    // Load and apply the skeletal mesh
    if (WarriorConfig.BodyMesh.IsValid())
    {
        USkeletalMesh* LoadedMesh = WarriorConfig.BodyMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            MeshComponent->SetSkeletalMesh(LoadedMesh);
            UE_LOG(LogTemp, Log, TEXT("Applied tribal warrior body mesh to component"));
        }
    }

    // Apply skin material
    if (WarriorConfig.SkinMaterial.IsValid())
    {
        UMaterialInterface* SkinMat = WarriorConfig.SkinMaterial.LoadSynchronous();
        if (SkinMat)
        {
            MeshComponent->SetMaterial(0, SkinMat);
            UE_LOG(LogTemp, Log, TEXT("Applied skin material to tribal warrior"));
        }
    }

    // Apply clothing material to material slot 1 (if exists)
    if (WarriorConfig.ClothingMaterial.IsValid())
    {
        UMaterialInterface* ClothingMat = WarriorConfig.ClothingMaterial.LoadSynchronous();
        if (ClothingMat)
        {
            MeshComponent->SetMaterial(1, ClothingMat);
            UE_LOG(LogTemp, Log, TEXT("Applied clothing material to tribal warrior"));
        }
    }

    // Apply bone jewelry material to material slot 2 (if exists)
    if (WarriorConfig.BoneJewelryMaterial.IsValid())
    {
        UMaterialInterface* JewelryMat = WarriorConfig.BoneJewelryMaterial.LoadSynchronous();
        if (JewelryMat)
        {
            MeshComponent->SetMaterial(2, JewelryMat);
            UE_LOG(LogTemp, Log, TEXT("Applied bone jewelry material to tribal warrior"));
        }
    }
}

USkeletalMesh* UChar_TribalWarriorAsset::GetBodyMesh()
{
    if (WarriorConfig.BodyMesh.IsValid())
    {
        return WarriorConfig.BodyMesh.LoadSynchronous();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UChar_TribalWarriorAsset::GetBodyMesh - No valid body mesh configured"));
    return nullptr;
}

UStaticMesh* UChar_TribalWarriorAsset::GetSpearMesh()
{
    if (WarriorConfig.SpearMesh.IsValid())
    {
        return WarriorConfig.SpearMesh.LoadSynchronous();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UChar_TribalWarriorAsset::GetSpearMesh - No valid spear mesh configured"));
    return nullptr;
}