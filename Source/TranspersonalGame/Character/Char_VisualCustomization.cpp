#include "Char_VisualCustomization.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UChar_VisualCustomization::UChar_VisualCustomization()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default appearance
    CurrentAppearance = FChar_AppearanceData();
    
    // Initialize component references
    OwnerMeshComponent = nullptr;
    BaseSkinMaterial = nullptr;
    BaseHairMaterial = nullptr;
    BaseClothingMaterial = nullptr;
    MetaHumanMesh = nullptr;
    AppearanceParameters = nullptr;
}

void UChar_VisualCustomization::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner's skeletal mesh component
    if (AActor* Owner = GetOwner())
    {
        OwnerMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        
        if (!OwnerMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("UChar_VisualCustomization: No SkeletalMeshComponent found on owner"));
        }
    }
    
    // Apply initial appearance
    ApplyAppearance(CurrentAppearance);
}

void UChar_VisualCustomization::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_VisualCustomization::ApplyAppearance(const FChar_AppearanceData& NewAppearance)
{
    CurrentAppearance = NewAppearance;
    
    if (!OwnerMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UChar_VisualCustomization::ApplyAppearance: No mesh component available"));
        return;
    }
    
    // Apply all appearance settings
    ApplySkinToneMaterial();
    ApplyBodyBuildScale();
    ApplyTribalMarkingsTexture();
    ApplyHairStyleMesh();
    ApplyClothingMesh();
    UpdateMaterialParameters();
    
    UE_LOG(LogTemp, Log, TEXT("Applied character appearance: SkinTone=%d, BodyBuild=%d"), 
           (int32)CurrentAppearance.SkinTone, (int32)CurrentAppearance.BodyBuild);
}

void UChar_VisualCustomization::RandomizeAppearance()
{
    FChar_AppearanceData RandomAppearance;
    
    // Randomize all appearance properties
    RandomAppearance.SkinTone = static_cast<EChar_SkinTone>(FMath::RandRange(0, 5));
    RandomAppearance.BodyBuild = static_cast<EChar_BodyBuild>(FMath::RandRange(0, 4));
    RandomAppearance.TribalMarkings = static_cast<EChar_TribalMarkings>(FMath::RandRange(0, 5));
    RandomAppearance.HairStyle = static_cast<EChar_HairStyle>(FMath::RandRange(0, 5));
    RandomAppearance.ClothingStyle = static_cast<EChar_ClothingStyle>(FMath::RandRange(0, 5));
    
    // Random colors
    TArray<FLinearColor> HairColors = {
        FLinearColor::Black,
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f), // Brown
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f), // Light Brown
        FLinearColor(0.8f, 0.6f, 0.3f, 1.0f), // Blonde
        FLinearColor(0.4f, 0.4f, 0.4f, 1.0f)  // Gray
    };
    
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f), // Brown
        FLinearColor(0.2f, 0.4f, 0.6f, 1.0f), // Blue
        FLinearColor(0.3f, 0.5f, 0.2f, 1.0f), // Green
        FLinearColor(0.4f, 0.3f, 0.2f, 1.0f), // Hazel
        FLinearColor(0.2f, 0.2f, 0.2f, 1.0f)  // Dark
    };
    
    RandomAppearance.HairColor = HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
    RandomAppearance.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
    
    // Random weathering and scars (survival game appropriate)
    RandomAppearance.WeatheringLevel = FMath::RandRange(0.3f, 0.8f);
    RandomAppearance.ScarLevel = FMath::RandRange(0.1f, 0.6f);
    RandomAppearance.BodyScale = FMath::RandRange(0.9f, 1.1f);
    
    ApplyAppearance(RandomAppearance);
    
    UE_LOG(LogTemp, Log, TEXT("Randomized character appearance"));
}

void UChar_VisualCustomization::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CurrentAppearance.SkinTone = NewSkinTone;
    ApplySkinToneMaterial();
    UpdateMaterialParameters();
}

void UChar_VisualCustomization::SetBodyBuild(EChar_BodyBuild NewBodyBuild)
{
    CurrentAppearance.BodyBuild = NewBodyBuild;
    ApplyBodyBuildScale();
}

void UChar_VisualCustomization::SetTribalMarkings(EChar_TribalMarkings NewMarkings)
{
    CurrentAppearance.TribalMarkings = NewMarkings;
    ApplyTribalMarkingsTexture();
    UpdateMaterialParameters();
}

void UChar_VisualCustomization::SetHairStyle(EChar_HairStyle NewHairStyle)
{
    CurrentAppearance.HairStyle = NewHairStyle;
    ApplyHairStyleMesh();
}

void UChar_VisualCustomization::SetClothingStyle(EChar_ClothingStyle NewClothingStyle)
{
    CurrentAppearance.ClothingStyle = NewClothingStyle;
    ApplyClothingMesh();
}

void UChar_VisualCustomization::SetWeatheringLevel(float NewLevel)
{
    CurrentAppearance.WeatheringLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);
    UpdateMaterialParameters();
}

void UChar_VisualCustomization::SetScarLevel(float NewLevel)
{
    CurrentAppearance.ScarLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);
    UpdateMaterialParameters();
}

void UChar_VisualCustomization::SaveAppearanceToFile(const FString& FileName)
{
    // Implementation for saving appearance data to file
    // This would serialize the CurrentAppearance struct to JSON or binary format
    UE_LOG(LogTemp, Log, TEXT("Saving appearance to file: %s"), *FileName);
}

bool UChar_VisualCustomization::LoadAppearanceFromFile(const FString& FileName)
{
    // Implementation for loading appearance data from file
    // This would deserialize the appearance data and apply it
    UE_LOG(LogTemp, Log, TEXT("Loading appearance from file: %s"), *FileName);
    return true;
}

void UChar_VisualCustomization::ApplySkinToneMaterial()
{
    if (!OwnerMeshComponent || SkinToneMaterials.Num() == 0)
    {
        return;
    }
    
    int32 SkinToneIndex = static_cast<int32>(CurrentAppearance.SkinTone);
    if (SkinToneMaterials.IsValidIndex(SkinToneIndex))
    {
        UMaterialInterface* SkinMaterial = SkinToneMaterials[SkinToneIndex];
        if (SkinMaterial)
        {
            OwnerMeshComponent->SetMaterial(0, SkinMaterial);
            UE_LOG(LogTemp, Log, TEXT("Applied skin tone material: %d"), SkinToneIndex);
        }
    }
}

void UChar_VisualCustomization::ApplyBodyBuildScale()
{
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    FVector BaseScale = FVector(1.0f, 1.0f, 1.0f);
    
    // Adjust scale based on body build
    switch (CurrentAppearance.BodyBuild)
    {
        case EChar_BodyBuild::Lean:
            BaseScale = FVector(0.95f, 0.95f, 1.0f);
            break;
        case EChar_BodyBuild::Athletic:
            BaseScale = FVector(1.0f, 1.0f, 1.0f);
            break;
        case EChar_BodyBuild::Muscular:
            BaseScale = FVector(1.05f, 1.05f, 1.0f);
            break;
        case EChar_BodyBuild::Stocky:
            BaseScale = FVector(1.1f, 1.1f, 0.98f);
            break;
        case EChar_BodyBuild::Weathered:
            BaseScale = FVector(0.98f, 0.98f, 0.96f);
            break;
    }
    
    // Apply body scale modifier
    BaseScale *= CurrentAppearance.BodyScale;
    
    OwnerMeshComponent->SetWorldScale3D(BaseScale);
    UE_LOG(LogTemp, Log, TEXT("Applied body build scale: %s"), *BaseScale.ToString());
}

void UChar_VisualCustomization::ApplyTribalMarkingsTexture()
{
    // Implementation for applying tribal markings as texture overlays
    // This would use material parameters to blend marking textures
    UE_LOG(LogTemp, Log, TEXT("Applied tribal markings: %d"), (int32)CurrentAppearance.TribalMarkings);
}

void UChar_VisualCustomization::ApplyHairStyleMesh()
{
    // Implementation for changing hair mesh based on style
    // This would swap hair mesh components or modify hair material parameters
    UE_LOG(LogTemp, Log, TEXT("Applied hair style: %d"), (int32)CurrentAppearance.HairStyle);
}

void UChar_VisualCustomization::ApplyClothingMesh()
{
    // Implementation for changing clothing meshes based on style
    // This would swap clothing mesh components or modify clothing materials
    UE_LOG(LogTemp, Log, TEXT("Applied clothing style: %d"), (int32)CurrentAppearance.ClothingStyle);
}

void UChar_VisualCustomization::UpdateMaterialParameters()
{
    if (!OwnerMeshComponent)
    {
        return;
    }
    
    // Create or get dynamic material instances
    for (int32 i = 0; i < OwnerMeshComponent->GetNumMaterials(); ++i)
    {
        UMaterialInterface* Material = OwnerMeshComponent->GetMaterial(i);
        if (Material)
        {
            UMaterialInstanceDynamic* DynamicMaterial = OwnerMeshComponent->CreateAndSetMaterialInstanceDynamic(i);
            if (DynamicMaterial)
            {
                // Set material parameters based on appearance
                DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), CurrentAppearance.HairColor);
                DynamicMaterial->SetVectorParameterValue(TEXT("EyeColor"), CurrentAppearance.EyeColor);
                DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), CurrentAppearance.WeatheringLevel);
                DynamicMaterial->SetScalarParameterValue(TEXT("ScarLevel"), CurrentAppearance.ScarLevel);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated material parameters"));
}