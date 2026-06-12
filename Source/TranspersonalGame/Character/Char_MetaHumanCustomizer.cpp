#include "Char_MetaHumanCustomizer.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UChar_MetaHumanCustomizer::UChar_MetaHumanCustomizer()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CharacterAsset = nullptr;
    BodyMeshComponent = nullptr;
    SpearComponent = nullptr;
    KnifeComponent = nullptr;
    NecklaceComponent = nullptr;
}

void UChar_MetaHumanCustomizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-find components if not manually assigned
    if (!BodyMeshComponent)
    {
        BodyMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    // Apply character asset if assigned
    if (CharacterAsset && CharacterAsset->IsAssetValid())
    {
        ApplyCharacterAsset();
    }
}

void UChar_MetaHumanCustomizer::ApplyCharacterAsset()
{
    if (!CharacterAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("No character asset assigned to MetaHuman Customizer"));
        return;
    }
    
    ApplyBodyCustomization();
    ApplyEquipment();
    ApplyMaterials();
}

void UChar_MetaHumanCustomizer::ApplyBodyCustomization()
{
    if (!BodyMeshComponent || !CharacterAsset)
    {
        return;
    }
    
    // Apply body mesh if available
    if (!CharacterAsset->BodyMesh.IsNull())
    {
        USkeletalMesh* LoadedMesh = CharacterAsset->BodyMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            BodyMeshComponent->SetSkeletalMesh(LoadedMesh);
        }
    }
    
    // Apply height scaling
    SetHeightScale(CharacterAsset->HeightScale);
}

void UChar_MetaHumanCustomizer::ApplyEquipment()
{
    if (!CharacterAsset)
    {
        return;
    }
    
    // Apply spear equipment
    if (SpearComponent && !CharacterAsset->SpearMesh.IsNull())
    {
        UStaticMesh* LoadedSpear = CharacterAsset->SpearMesh.LoadSynchronous();
        if (LoadedSpear)
        {
            SpearComponent->SetStaticMesh(LoadedSpear);
            AttachEquipmentToSocket(SpearComponent, FName("hand_r"));
        }
    }
    
    // Apply knife equipment
    if (KnifeComponent && !CharacterAsset->KnifeMesh.IsNull())
    {
        UStaticMesh* LoadedKnife = CharacterAsset->KnifeMesh.LoadSynchronous();
        if (LoadedKnife)
        {
            KnifeComponent->SetStaticMesh(LoadedKnife);
            AttachEquipmentToSocket(KnifeComponent, FName("thigh_l"));
        }
    }
    
    // Apply necklace equipment
    if (NecklaceComponent && !CharacterAsset->BoneNecklaceMesh.IsNull())
    {
        UStaticMesh* LoadedNecklace = CharacterAsset->BoneNecklaceMesh.LoadSynchronous();
        if (LoadedNecklace)
        {
            NecklaceComponent->SetStaticMesh(LoadedNecklace);
            AttachEquipmentToSocket(NecklaceComponent, FName("neck_01"));
        }
    }
}

void UChar_MetaHumanCustomizer::ApplyMaterials()
{
    if (!BodyMeshComponent || !CharacterAsset)
    {
        return;
    }
    
    // Create dynamic material instances for customization
    int32 MaterialCount = BodyMeshComponent->GetNumMaterials();
    DynamicMaterials.Empty();
    
    for (int32 i = 0; i < MaterialCount; i++)
    {
        UMaterialInterface* BaseMaterial = BodyMeshComponent->GetMaterial(i);
        if (BaseMaterial)
        {
            CreateDynamicMaterialInstance(BaseMaterial, i);
        }
    }
    
    // Apply skin tone and hair color
    SetSkinTone(CharacterAsset->SkinTone);
    SetHairColor(CharacterAsset->HairColor);
}

void UChar_MetaHumanCustomizer::SetHeightScale(float NewScale)
{
    if (BodyMeshComponent)
    {
        FVector CurrentScale = BodyMeshComponent->GetRelativeScale3D();
        BodyMeshComponent->SetRelativeScale3D(FVector(CurrentScale.X, CurrentScale.Y, NewScale));
    }
}

void UChar_MetaHumanCustomizer::SetSkinTone(FLinearColor NewSkinTone)
{
    ApplyColorParameter(FName("SkinTone"), NewSkinTone, 0);
}

void UChar_MetaHumanCustomizer::SetHairColor(FLinearColor NewHairColor)
{
    ApplyColorParameter(FName("HairColor"), NewHairColor, 1);
}

bool UChar_MetaHumanCustomizer::ValidateSetup() const
{
    return BodyMeshComponent != nullptr && CharacterAsset != nullptr && CharacterAsset->IsAssetValid();
}

TArray<FString> UChar_MetaHumanCustomizer::GetSetupErrors() const
{
    TArray<FString> Errors;
    
    if (!BodyMeshComponent)
    {
        Errors.Add(TEXT("No BodyMeshComponent assigned"));
    }
    
    if (!CharacterAsset)
    {
        Errors.Add(TEXT("No CharacterAsset assigned"));
    }
    else if (!CharacterAsset->IsAssetValid())
    {
        TArray<FString> AssetErrors = CharacterAsset->GetMissingAssets();
        for (const FString& AssetError : AssetErrors)
        {
            Errors.Add(FString::Printf(TEXT("Missing asset: %s"), *AssetError));
        }
    }
    
    return Errors;
}

void UChar_MetaHumanCustomizer::AttachEquipmentToSocket(UStaticMeshComponent* Equipment, const FName& SocketName)
{
    if (Equipment && BodyMeshComponent && BodyMeshComponent->DoesSocketExist(SocketName))
    {
        Equipment->AttachToComponent(BodyMeshComponent, 
            FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
    }
}

void UChar_MetaHumanCustomizer::CreateDynamicMaterialInstance(UMaterialInterface* BaseMaterial, int32 MaterialIndex)
{
    if (BaseMaterial && BodyMeshComponent)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            BodyMeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
            DynamicMaterials.Add(DynamicMaterial);
        }
    }
}

void UChar_MetaHumanCustomizer::ApplyColorParameter(const FName& ParameterName, const FLinearColor& Color, int32 MaterialIndex)
{
    if (DynamicMaterials.IsValidIndex(MaterialIndex) && DynamicMaterials[MaterialIndex])
    {
        DynamicMaterials[MaterialIndex]->SetVectorParameterValue(ParameterName, Color);
    }
}