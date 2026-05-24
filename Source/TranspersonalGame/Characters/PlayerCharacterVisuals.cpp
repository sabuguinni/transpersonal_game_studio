#include "PlayerCharacterVisuals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/AssetManager.h"

UPlayerCharacterVisuals::UPlayerCharacterVisuals()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default appearance
    CurrentAppearance.BodyType = EChar_PlayerBodyType::Athletic;
    CurrentAppearance.SkinTone = EChar_PlayerSkinTone::Medium;
    CurrentAppearance.HairStyle = EChar_PlayerHairStyle::Medium;
    CurrentAppearance.HairColor = FLinearColor::Black;
    CurrentAppearance.EyeColor = FLinearColor(0.4f, 0.2f, 0.1f, 1.0f); // Brown
    CurrentAppearance.Height = 1.0f;
    CurrentAppearance.Weight = 1.0f;
}

void UPlayerCharacterVisuals::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply default appearance on start
    ApplyPlayerAppearance(CurrentAppearance);
}

void UPlayerCharacterVisuals::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerCharacterVisuals::ApplyPlayerAppearance(const FChar_PlayerAppearance& Appearance)
{
    CurrentAppearance = Appearance;
    
    // Update mesh based on body type
    UpdateCharacterMesh();
    
    // Update materials based on skin tone and other properties
    UpdateCharacterMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Applied player appearance: BodyType=%d, SkinTone=%d, HairStyle=%d"), 
        (int32)Appearance.BodyType, (int32)Appearance.SkinTone, (int32)Appearance.HairStyle);
}

void UPlayerCharacterVisuals::SetPlayerMesh(USkeletalMesh* NewMesh)
{
    if (!NewMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetPlayerMesh: NewMesh is null"));
        return;
    }
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character && Character->GetMesh())
    {
        Character->GetMesh()->SetSkeletalMesh(NewMesh);
        UE_LOG(LogTemp, Log, TEXT("Set player mesh to: %s"), *NewMesh->GetName());
    }
}

void UPlayerCharacterVisuals::SetPlayerMaterial(UMaterialInterface* NewMaterial)
{
    if (!NewMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetPlayerMaterial: NewMaterial is null"));
        return;
    }
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character && Character->GetMesh())
    {
        Character->GetMesh()->SetMaterial(0, NewMaterial);
        UE_LOG(LogTemp, Log, TEXT("Set player material to: %s"), *NewMaterial->GetName());
    }
}

void UPlayerCharacterVisuals::RandomizePlayerAppearance()
{
    FChar_PlayerAppearance RandomAppearance;
    
    // Randomize body type
    int32 BodyTypeIndex = FMath::RandRange(0, 3);
    RandomAppearance.BodyType = static_cast<EChar_PlayerBodyType>(BodyTypeIndex);
    
    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 4);
    RandomAppearance.SkinTone = static_cast<EChar_PlayerSkinTone>(SkinToneIndex);
    
    // Randomize hair style
    int32 HairStyleIndex = FMath::RandRange(0, 4);
    RandomAppearance.HairStyle = static_cast<EChar_PlayerHairStyle>(HairStyleIndex);
    
    // Randomize hair color
    TArray<FLinearColor> HairColors = {
        FLinearColor::Black,
        FLinearColor(0.4f, 0.2f, 0.1f, 1.0f), // Brown
        FLinearColor(0.8f, 0.6f, 0.2f, 1.0f), // Blonde
        FLinearColor(0.6f, 0.3f, 0.1f, 1.0f), // Auburn
        FLinearColor(0.3f, 0.3f, 0.3f, 1.0f)  // Gray
    };
    RandomAppearance.HairColor = HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
    
    // Randomize eye color
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.4f, 0.2f, 0.1f, 1.0f), // Brown
        FLinearColor(0.2f, 0.4f, 0.8f, 1.0f), // Blue
        FLinearColor(0.2f, 0.6f, 0.2f, 1.0f), // Green
        FLinearColor(0.5f, 0.5f, 0.5f, 1.0f), // Gray
        FLinearColor(0.3f, 0.5f, 0.3f, 1.0f)  // Hazel
    };
    RandomAppearance.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
    
    // Randomize height and weight within reasonable bounds
    RandomAppearance.Height = FMath::RandRange(0.9f, 1.1f);
    RandomAppearance.Weight = FMath::RandRange(0.8f, 1.2f);
    
    ApplyPlayerAppearance(RandomAppearance);
    
    UE_LOG(LogTemp, Log, TEXT("Randomized player appearance"));
}

void UPlayerCharacterVisuals::UpdateCharacterMesh()
{
    USkeletalMesh* TargetMesh = GetMeshForBodyType(CurrentAppearance.BodyType);
    if (TargetMesh)
    {
        SetPlayerMesh(TargetMesh);
    }
    else
    {
        // Fallback to UE5 Mannequin if no custom mesh is available
        USkeletalMesh* MannequinMesh = LoadObject<USkeletalMesh>(nullptr, 
            TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
        if (MannequinMesh)
        {
            SetPlayerMesh(MannequinMesh);
            UE_LOG(LogTemp, Log, TEXT("Using fallback UE5 Mannequin mesh"));
        }
    }
}

void UPlayerCharacterVisuals::UpdateCharacterMaterials()
{
    UMaterialInterface* SkinMaterial = GetSkinMaterialForTone(CurrentAppearance.SkinTone);
    if (SkinMaterial)
    {
        SetPlayerMaterial(SkinMaterial);
    }
    else
    {
        // Create dynamic material instance for skin customization
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        if (Character && Character->GetMesh())
        {
            UMaterialInterface* BaseMaterial = Character->GetMesh()->GetMaterial(0);
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                if (DynamicMaterial)
                {
                    // Set skin tone parameter if material supports it
                    float SkinToneValue = static_cast<float>(CurrentAppearance.SkinTone) / 4.0f;
                    DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), SkinToneValue);
                    
                    // Set hair color parameter if material supports it
                    DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), CurrentAppearance.HairColor);
                    
                    // Set eye color parameter if material supports it
                    DynamicMaterial->SetVectorParameterValue(TEXT("EyeColor"), CurrentAppearance.EyeColor);
                    
                    Character->GetMesh()->SetMaterial(0, DynamicMaterial);
                    UE_LOG(LogTemp, Log, TEXT("Created dynamic material for character customization"));
                }
            }
        }
    }
}

UMaterialInterface* UPlayerCharacterVisuals::GetSkinMaterialForTone(EChar_PlayerSkinTone SkinTone)
{
    if (SkinMaterials.Num() > static_cast<int32>(SkinTone))
    {
        TSoftObjectPtr<UMaterialInterface> MaterialPtr = SkinMaterials[static_cast<int32>(SkinTone)];
        if (MaterialPtr.IsValid())
        {
            return MaterialPtr.Get();
        }
        else if (!MaterialPtr.IsNull())
        {
            // Try to load the material
            return MaterialPtr.LoadSynchronous();
        }
    }
    
    return nullptr;
}

USkeletalMesh* UPlayerCharacterVisuals::GetMeshForBodyType(EChar_PlayerBodyType BodyType)
{
    if (PlayerMeshVariants.Num() > static_cast<int32>(BodyType))
    {
        TSoftObjectPtr<USkeletalMesh> MeshPtr = PlayerMeshVariants[static_cast<int32>(BodyType)];
        if (MeshPtr.IsValid())
        {
            return MeshPtr.Get();
        }
        else if (!MeshPtr.IsNull())
        {
            // Try to load the mesh
            return MeshPtr.LoadSynchronous();
        }
    }
    
    return nullptr;
}