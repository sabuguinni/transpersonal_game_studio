#include "Char_VisualCustomization.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

UChar_VisualCustomization::UChar_VisualCustomization()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default visual profile for Cretaceous period survivor
    VisualProfile = FChar_VisualProfile();
}

void UChar_VisualCustomization::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial visual configuration
    ApplyVisualProfile(VisualProfile);
}

void UChar_VisualCustomization::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_VisualCustomization::ApplyVisualProfile(const FChar_VisualProfile& NewProfile)
{
    VisualProfile = NewProfile;
    
    UpdateCharacterMesh();
    UpdateMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Applied visual profile: SkinTone=%d, BodyType=%d, ClothingStyle=%d"), 
           (int32)VisualProfile.SkinTone, (int32)VisualProfile.BodyType, (int32)VisualProfile.ClothingStyle);
}

void UChar_VisualCustomization::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    VisualProfile.SkinTone = NewSkinTone;
    UpdateMaterials();
}

void UChar_VisualCustomization::SetBodyType(EChar_BodyType NewBodyType)
{
    VisualProfile.BodyType = NewBodyType;
    UpdateCharacterMesh();
}

void UChar_VisualCustomization::SetClothingStyle(EChar_ClothingStyle NewStyle)
{
    VisualProfile.ClothingStyle = NewStyle;
    UpdateMaterials();
}

void UChar_VisualCustomization::AddScar(bool bFacialScar)
{
    if (bFacialScar)
    {
        VisualProfile.bHasFacialScars = true;
    }
    else
    {
        VisualProfile.bHasBodyScars = true;
    }
    
    UpdateMaterials();
    UE_LOG(LogTemp, Log, TEXT("Added scar: Facial=%s"), bFacialScar ? TEXT("true") : TEXT("false"));
}

void UChar_VisualCustomization::SetWeatheringLevel(float WeatheringAmount)
{
    VisualProfile.SkinWeathering = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);
    UpdateMaterials();
}

void UChar_VisualCustomization::SetDirtLevel(float DirtAmount)
{
    VisualProfile.DirtLevel = FMath::Clamp(DirtAmount, 0.0f, 1.0f);
    UpdateMaterials();
}

void UChar_VisualCustomization::UpdateCharacterMesh()
{
    USkeletalMeshComponent* MeshComp = GetOwnerMeshComponent();
    if (!MeshComp)
    {
        return;
    }
    
    // Select appropriate mesh based on body type
    USkeletalMesh* TargetMesh = nullptr;
    
    switch (VisualProfile.BodyType)
    {
        case EChar_BodyType::Lean:
        case EChar_BodyType::Athletic:
            TargetMesh = BaseMaleMesh;
            break;
        case EChar_BodyType::Stocky:
        case EChar_BodyType::Muscular:
            TargetMesh = BaseMaleMesh; // Could use different mesh variants
            break;
        default:
            TargetMesh = BaseMaleMesh;
            break;
    }
    
    if (TargetMesh && MeshComp->GetSkeletalMeshAsset() != TargetMesh)
    {
        MeshComp->SetSkeletalMesh(TargetMesh);
        UE_LOG(LogTemp, Log, TEXT("Updated character mesh for body type: %d"), (int32)VisualProfile.BodyType);
    }
    
    // Apply scale based on height/weight
    float ScaleFactor = VisualProfile.Height / 1.75f; // Normalize to default height
    MeshComp->SetRelativeScale3D(FVector(ScaleFactor));
}

void UChar_VisualCustomization::UpdateMaterials()
{
    USkeletalMeshComponent* MeshComp = GetOwnerMeshComponent();
    if (!MeshComp)
    {
        return;
    }
    
    // Create dynamic material instances for customization
    for (int32 MaterialIndex = 0; MaterialIndex < MeshComp->GetNumMaterials(); ++MaterialIndex)
    {
        UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(MaterialIndex);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                // Apply skin tone adjustments
                float SkinToneValue = (float)VisualProfile.SkinTone / 5.0f; // Convert enum to 0-1 range
                DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), SkinToneValue);
                
                // Apply weathering and dirt
                DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), VisualProfile.SkinWeathering);
                DynamicMaterial->SetScalarParameterValue(TEXT("DirtLevel"), VisualProfile.DirtLevel);
                
                // Apply scars
                DynamicMaterial->SetScalarParameterValue(TEXT("FacialScars"), VisualProfile.bHasFacialScars ? 1.0f : 0.0f);
                DynamicMaterial->SetScalarParameterValue(TEXT("BodyScars"), VisualProfile.bHasBodyScars ? 1.0f : 0.0f);
                
                MeshComp->SetMaterial(MaterialIndex, DynamicMaterial);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated character materials: Weathering=%.2f, Dirt=%.2f"), 
           VisualProfile.SkinWeathering, VisualProfile.DirtLevel);
}

USkeletalMeshComponent* UChar_VisualCustomization::GetOwnerMeshComponent() const
{
    AActor* Owner = GetOwner();
    if (Owner)
    {
        return Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
    return nullptr;
}