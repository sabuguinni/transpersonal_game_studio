#include "Char_TribalPlayerVisuals.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"

UChar_TribalPlayerVisuals::UChar_TribalPlayerVisuals()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default tribal appearance settings
    SkinTone = EChar_SkinTone::Medium;
    HairColor = EChar_HairColor::DarkBrown;
    HairStyle = EChar_HairStyle::LongBraided;
    ClothingStyle = EChar_ClothingStyle::AnimalHide;
    WeatheringLevel = 0.7f;
    TribalMarkings = true;
    
    // Set default mesh paths (will be overridden by actual assets)
    DefaultMaleMeshPath = TEXT("/Game/Characters/Meshes/SK_TribalMale");
    DefaultFemaleMeshPath = TEXT("/Game/Characters/Meshes/SK_TribalFemale");
    
    // Initialize material paths
    SkinMaterialPath = TEXT("/Game/Characters/Materials/M_TribalSkin");
    HairMaterialPath = TEXT("/Game/Characters/Materials/M_TribalHair");
    ClothingMaterialPath = TEXT("/Game/Characters/Materials/M_TribalClothing");
}

void UChar_TribalPlayerVisuals::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial visual configuration
    ApplyTribalAppearance();
}

void UChar_TribalPlayerVisuals::ApplyTribalAppearance()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("UChar_TribalPlayerVisuals: No owner found"));
        return;
    }
    
    // Get the skeletal mesh component
    USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("UChar_TribalPlayerVisuals: No skeletal mesh component found"));
        return;
    }
    
    // Apply the appropriate tribal mesh based on gender
    ApplyTribalMesh(MeshComp);
    
    // Apply materials and textures
    ApplyTribalMaterials(MeshComp);
    
    // Apply tribal markings and weathering
    ApplyTribalDetails(MeshComp);
    
    UE_LOG(LogTemp, Log, TEXT("UChar_TribalPlayerVisuals: Applied tribal appearance"));
}

void UChar_TribalPlayerVisuals::ApplyTribalMesh(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Determine which mesh to use based on gender
    FString MeshPath = (bIsFemale) ? DefaultFemaleMeshPath : DefaultMaleMeshPath;
    
    // Try to load the tribal mesh
    USkeletalMesh* TribalMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
    
    if (TribalMesh)
    {
        MeshComponent->SetSkeletalMesh(TribalMesh);
        UE_LOG(LogTemp, Log, TEXT("UChar_TribalPlayerVisuals: Applied tribal mesh: %s"), *MeshPath);
    }
    else
    {
        // Fallback to default UE5 mannequin if tribal mesh not available
        USkeletalMesh* FallbackMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn"));
        if (!FallbackMesh)
        {
            FallbackMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Quinn"));
        }
        
        if (FallbackMesh)
        {
            MeshComponent->SetSkeletalMesh(FallbackMesh);
            UE_LOG(LogTemp, Warning, TEXT("UChar_TribalPlayerVisuals: Using fallback mesh"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UChar_TribalPlayerVisuals: No mesh available"));
        }
    }
}

void UChar_TribalPlayerVisuals::ApplyTribalMaterials(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Load and apply skin material
    UMaterialInterface* SkinMaterial = LoadObject<UMaterialInterface>(nullptr, *SkinMaterialPath);
    if (SkinMaterial)
    {
        MeshComponent->SetMaterial(0, SkinMaterial);
    }
    
    // Load and apply hair material
    UMaterialInterface* HairMaterial = LoadObject<UMaterialInterface>(nullptr, *HairMaterialPath);
    if (HairMaterial)
    {
        MeshComponent->SetMaterial(1, HairMaterial);
    }
    
    // Load and apply clothing material
    UMaterialInterface* ClothingMaterial = LoadObject<UMaterialInterface>(nullptr, *ClothingMaterialPath);
    if (ClothingMaterial)
    {
        MeshComponent->SetMaterial(2, ClothingMaterial);
    }
    
    UE_LOG(LogTemp, Log, TEXT("UChar_TribalPlayerVisuals: Applied tribal materials"));
}

void UChar_TribalPlayerVisuals::ApplyTribalDetails(USkeletalMeshComponent* MeshComponent)
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Apply weathering and tribal markings through material parameters
    if (TribalMarkings)
    {
        // Create dynamic material instances for customization
        for (int32 i = 0; i < MeshComponent->GetNumMaterials(); ++i)
        {
            UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(i);
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                if (DynamicMaterial)
                {
                    // Set weathering level
                    DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), WeatheringLevel);
                    
                    // Set tribal markings visibility
                    DynamicMaterial->SetScalarParameterValue(TEXT("TribalMarkings"), TribalMarkings ? 1.0f : 0.0f);
                    
                    // Set skin tone
                    float SkinToneValue = static_cast<float>(SkinTone) / 4.0f;
                    DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), SkinToneValue);
                    
                    MeshComponent->SetMaterial(i, DynamicMaterial);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("UChar_TribalPlayerVisuals: Applied tribal details"));
}

void UChar_TribalPlayerVisuals::SetGender(bool bFemale)
{
    if (bIsFemale != bFemale)
    {
        bIsFemale = bFemale;
        ApplyTribalAppearance();
    }
}

void UChar_TribalPlayerVisuals::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    if (SkinTone != NewSkinTone)
    {
        SkinTone = NewSkinTone;
        
        // Update only the skin material
        AActor* Owner = GetOwner();
        if (Owner)
        {
            USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
            if (MeshComp)
            {
                ApplyTribalDetails(MeshComp);
            }
        }
    }
}

void UChar_TribalPlayerVisuals::SetHairStyle(EChar_HairStyle NewHairStyle)
{
    if (HairStyle != NewHairStyle)
    {
        HairStyle = NewHairStyle;
        ApplyTribalAppearance();
    }
}

void UChar_TribalPlayerVisuals::SetClothingStyle(EChar_ClothingStyle NewClothingStyle)
{
    if (ClothingStyle != NewClothingStyle)
    {
        ClothingStyle = NewClothingStyle;
        
        // Update clothing materials
        AActor* Owner = GetOwner();
        if (Owner)
        {
            USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
            if (MeshComp)
            {
                ApplyTribalMaterials(MeshComp);
            }
        }
    }
}

void UChar_TribalPlayerVisuals::SetWeatheringLevel(float NewWeatheringLevel)
{
    WeatheringLevel = FMath::Clamp(NewWeatheringLevel, 0.0f, 1.0f);
    
    // Update weathering on materials
    AActor* Owner = GetOwner();
    if (Owner)
    {
        USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (MeshComp)
        {
            ApplyTribalDetails(MeshComp);
        }
    }
}

void UChar_TribalPlayerVisuals::SetTribalMarkings(bool bEnabled)
{
    if (TribalMarkings != bEnabled)
    {
        TribalMarkings = bEnabled;
        
        // Update tribal markings
        AActor* Owner = GetOwner();
        if (Owner)
        {
            USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
            if (MeshComp)
            {
                ApplyTribalDetails(MeshComp);
            }
        }
    }
}

FChar_TribalAppearanceData UChar_TribalPlayerVisuals::GetCurrentAppearance() const
{
    FChar_TribalAppearanceData AppearanceData;
    AppearanceData.bIsFemale = bIsFemale;
    AppearanceData.SkinTone = SkinTone;
    AppearanceData.HairColor = HairColor;
    AppearanceData.HairStyle = HairStyle;
    AppearanceData.ClothingStyle = ClothingStyle;
    AppearanceData.WeatheringLevel = WeatheringLevel;
    AppearanceData.bTribalMarkings = TribalMarkings;
    
    return AppearanceData;
}

void UChar_TribalPlayerVisuals::ApplyAppearanceData(const FChar_TribalAppearanceData& AppearanceData)
{
    bIsFemale = AppearanceData.bIsFemale;
    SkinTone = AppearanceData.SkinTone;
    HairColor = AppearanceData.HairColor;
    HairStyle = AppearanceData.HairStyle;
    ClothingStyle = AppearanceData.ClothingStyle;
    WeatheringLevel = AppearanceData.WeatheringLevel;
    TribalMarkings = AppearanceData.bTribalMarkings;
    
    ApplyTribalAppearance();
}