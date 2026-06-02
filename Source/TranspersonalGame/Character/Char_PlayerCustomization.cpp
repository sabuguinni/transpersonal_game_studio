#include "Char_PlayerCustomization.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

UChar_PlayerCustomization::UChar_PlayerCustomization()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default appearance
    CurrentAppearance = FChar_TribalAppearance();
    CurrentClothing = FChar_ClothingSet();
    
    TargetMeshComponent = nullptr;
    DynamicSkinMaterial = nullptr;
    DynamicHairMaterial = nullptr;
}

void UChar_PlayerCustomization::BeginPlay()
{
    Super::BeginPlay();
    
    LoadTribalMeshVariants();
    LoadClothingPresets();
    CreateDynamicMaterials();
    
    // Apply default appearance
    ApplyTribalAppearance(CurrentAppearance);
}

void UChar_PlayerCustomization::ApplyTribalAppearance(const FChar_TribalAppearance& NewAppearance)
{
    CurrentAppearance = NewAppearance;
    
    if (!TargetMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UChar_PlayerCustomization: No target mesh component set"));
        return;
    }
    
    // Apply role-based modifications first
    ApplyRoleBasedModifications(NewAppearance.TribalRole);
    
    // Update material parameters
    UpdateMaterialParameters();
    
    // Apply body build modifications
    ApplyBodyBuildModifications();
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal appearance for role: %d"), (int32)NewAppearance.TribalRole);
}

void UChar_PlayerCustomization::ChangeClothingSet(const FChar_ClothingSet& NewClothing)
{
    if (!ValidateClothingForRole(NewClothing, CurrentAppearance.TribalRole))
    {
        UE_LOG(LogTemp, Warning, TEXT("Clothing set not valid for current tribal role"));
        return;
    }
    
    CurrentClothing = NewClothing;
    
    if (TargetMeshComponent && NewClothing.TorsoMesh.IsValid())
    {
        USkeletalMesh* LoadedMesh = NewClothing.TorsoMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            TargetMeshComponent->SetSkeletalMesh(LoadedMesh);
        }
    }
    
    if (NewClothing.ClothingMaterial.IsValid())
    {
        UMaterialInterface* ClothingMat = NewClothing.ClothingMaterial.LoadSynchronous();
        if (ClothingMat && TargetMeshComponent)
        {
            UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(ClothingMat, this);
            if (DynMat)
            {
                // Apply durability-based weathering
                DynMat->SetScalarParameterValue(TEXT("Durability"), NewClothing.DurabilityLevel);
                DynMat->SetScalarParameterValue(TEXT("Weathering"), 1.0f - NewClothing.DurabilityLevel);
                
                TargetMeshComponent->SetMaterial(0, DynMat);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Changed clothing set with durability: %f"), NewClothing.DurabilityLevel);
}

void UChar_PlayerCustomization::SetTribalRole(EChar_TribalRole NewRole)
{
    CurrentAppearance.TribalRole = NewRole;
    ApplyTribalAppearance(CurrentAppearance);
}

void UChar_PlayerCustomization::ApplyWeathering(float WeatheringAmount)
{
    CurrentAppearance.WeatheringLevel = FMath::Clamp(WeatheringAmount, 0.0f, 1.0f);
    
    if (DynamicSkinMaterial)
    {
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("Weathering"), CurrentAppearance.WeatheringLevel);
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("Dirt"), CurrentAppearance.WeatheringLevel * 0.8f);
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("Scars"), CurrentAppearance.WeatheringLevel * 0.6f);
    }
    
    // Apply weathering to clothing durability
    CurrentClothing.DurabilityLevel = FMath::Max(0.1f, 1.0f - (CurrentAppearance.WeatheringLevel * 0.7f));
    ChangeClothingSet(CurrentClothing);
}

void UChar_PlayerCustomization::AddTribalMarkings(const FLinearColor& MarkingColor, const FString& MarkingPattern)
{
    CurrentAppearance.WarPaintColor = MarkingColor;
    CurrentAppearance.EquippedAccessories.AddUnique(MarkingPattern);
    
    if (DynamicSkinMaterial)
    {
        DynamicSkinMaterial->SetVectorParameterValue(TEXT("WarPaintColor"), MarkingColor);
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("WarPaintIntensity"), 1.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Added tribal markings: %s"), *MarkingPattern);
}

void UChar_PlayerCustomization::RandomizeAppearance()
{
    FChar_TribalAppearance RandomAppearance;
    
    // Random tribal role
    int32 RoleIndex = FMath::RandRange(0, (int32)EChar_TribalRole::Elder);
    RandomAppearance.TribalRole = (EChar_TribalRole)RoleIndex;
    
    // Random physical attributes
    RandomAppearance.SkinTone = GenerateRandomSkinTone();
    RandomAppearance.HairColor = GenerateRandomHairColor();
    RandomAppearance.BodyBuild = FMath::FRandRange(0.2f, 0.9f);
    RandomAppearance.WeatheringLevel = FMath::FRandRange(0.1f, 0.8f);
    
    // Random war paint color
    RandomAppearance.WarPaintColor = FLinearColor(
        FMath::FRandRange(0.5f, 1.0f),
        FMath::FRandRange(0.0f, 0.3f),
        FMath::FRandRange(0.0f, 0.3f),
        1.0f
    );
    
    ApplyTribalAppearance(RandomAppearance);
    
    UE_LOG(LogTemp, Log, TEXT("Randomized character appearance"));
}

void UChar_PlayerCustomization::SetTargetMeshComponent(USkeletalMeshComponent* MeshComp)
{
    TargetMeshComponent = MeshComp;
    
    if (TargetMeshComponent)
    {
        CreateDynamicMaterials();
        UE_LOG(LogTemp, Log, TEXT("Set target mesh component for customization"));
    }
}

void UChar_PlayerCustomization::LoadClothingPresets()
{
    AvailableClothingSets.Empty();
    
    // Hunter clothing set
    FChar_ClothingSet HunterSet;
    HunterSet.RequiredRole = EChar_TribalRole::Hunter;
    HunterSet.DurabilityLevel = 0.8f;
    AvailableClothingSets.Add(HunterSet);
    
    // Scout clothing set
    FChar_ClothingSet ScoutSet;
    ScoutSet.RequiredRole = EChar_TribalRole::Scout;
    ScoutSet.DurabilityLevel = 0.9f;
    AvailableClothingSets.Add(ScoutSet);
    
    // Gatherer clothing set
    FChar_ClothingSet GathererSet;
    GathererSet.RequiredRole = EChar_TribalRole::Gatherer;
    GathererSet.DurabilityLevel = 0.7f;
    AvailableClothingSets.Add(GathererSet);
    
    // Shaman clothing set
    FChar_ClothingSet ShamanSet;
    ShamanSet.RequiredRole = EChar_TribalRole::Shaman;
    ShamanSet.DurabilityLevel = 0.6f;
    AvailableClothingSets.Add(ShamanSet);
    
    // Elder clothing set
    FChar_ClothingSet ElderSet;
    ElderSet.RequiredRole = EChar_TribalRole::Elder;
    ElderSet.DurabilityLevel = 0.5f;
    AvailableClothingSets.Add(ElderSet);
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d clothing presets"), AvailableClothingSets.Num());
}

bool UChar_PlayerCustomization::ValidateClothingForRole(const FChar_ClothingSet& Clothing, EChar_TribalRole Role)
{
    // Allow any clothing for any role (flexible system)
    // Could add restrictions later if needed
    return true;
}

void UChar_PlayerCustomization::UpdateMaterialParameters()
{
    if (DynamicSkinMaterial)
    {
        DynamicSkinMaterial->SetVectorParameterValue(TEXT("SkinTone"), CurrentAppearance.SkinTone);
        DynamicSkinMaterial->SetVectorParameterValue(TEXT("WarPaintColor"), CurrentAppearance.WarPaintColor);
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("BodyBuild"), CurrentAppearance.BodyBuild);
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("Weathering"), CurrentAppearance.WeatheringLevel);
    }
    
    if (DynamicHairMaterial)
    {
        DynamicHairMaterial->SetVectorParameterValue(TEXT("HairColor"), CurrentAppearance.HairColor);
        DynamicHairMaterial->SetScalarParameterValue(TEXT("Weathering"), CurrentAppearance.WeatheringLevel * 0.5f);
    }
}

void UChar_PlayerCustomization::ApplyBodyBuildModifications()
{
    if (!TargetMeshComponent)
        return;
    
    // Scale mesh based on body build
    float ScaleModifier = 0.9f + (CurrentAppearance.BodyBuild * 0.2f); // 0.9 to 1.1 scale
    FVector NewScale = FVector(ScaleModifier, ScaleModifier, 1.0f + (CurrentAppearance.BodyBuild * 0.1f));
    
    TargetMeshComponent->SetWorldScale3D(NewScale);
}

void UChar_PlayerCustomization::LoadTribalMeshVariants()
{
    TribalMeshVariants.Empty();
    
    // Add paths to tribal mesh variants
    // These would be actual asset paths in a real project
    TribalMeshVariants.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Tribal/SK_TribalHunter"))));
    TribalMeshVariants.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Tribal/SK_TribalScout"))));
    TribalMeshVariants.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Tribal/SK_TribalGatherer"))));
    TribalMeshVariants.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Tribal/SK_TribalShaman"))));
    TribalMeshVariants.Add(TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Tribal/SK_TribalElder"))));
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d tribal mesh variants"), TribalMeshVariants.Num());
}

void UChar_PlayerCustomization::CreateDynamicMaterials()
{
    if (!TargetMeshComponent)
        return;
    
    // Create dynamic skin material
    UMaterialInterface* BaseSkinMaterial = TargetMeshComponent->GetMaterial(0);
    if (BaseSkinMaterial)
    {
        DynamicSkinMaterial = UMaterialInstanceDynamic::Create(BaseSkinMaterial, this);
        TargetMeshComponent->SetMaterial(0, DynamicSkinMaterial);
    }
    
    // Create dynamic hair material if available
    if (TargetMeshComponent->GetNumMaterials() > 1)
    {
        UMaterialInterface* BaseHairMaterial = TargetMeshComponent->GetMaterial(1);
        if (BaseHairMaterial)
        {
            DynamicHairMaterial = UMaterialInstanceDynamic::Create(BaseHairMaterial, this);
            TargetMeshComponent->SetMaterial(1, DynamicHairMaterial);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created dynamic materials for character customization"));
}

FLinearColor UChar_PlayerCustomization::GenerateRandomSkinTone()
{
    // Generate realistic skin tone variations
    float BaseR = FMath::FRandRange(0.6f, 0.9f);
    float BaseG = FMath::FRandRange(0.4f, 0.7f);
    float BaseB = FMath::FRandRange(0.3f, 0.6f);
    
    return FLinearColor(BaseR, BaseG, BaseB, 1.0f);
}

FLinearColor UChar_PlayerCustomization::GenerateRandomHairColor()
{
    // Generate natural hair colors
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.1f, 0.05f, 0.02f, 1.0f), // Dark brown
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),  // Brown
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),   // Light brown
        FLinearColor(0.05f, 0.05f, 0.05f, 1.0f), // Black
        FLinearColor(0.4f, 0.4f, 0.4f, 1.0f)    // Gray (elder)
    };
    
    int32 RandomIndex = FMath::RandRange(0, HairColors.Num() - 1);
    return HairColors[RandomIndex];
}

void UChar_PlayerCustomization::ApplyRoleBasedModifications(EChar_TribalRole Role)
{
    switch (Role)
    {
        case EChar_TribalRole::Hunter:
            CurrentAppearance.BodyBuild = FMath::Max(CurrentAppearance.BodyBuild, 0.6f);
            CurrentAppearance.WeatheringLevel = FMath::Max(CurrentAppearance.WeatheringLevel, 0.4f);
            break;
            
        case EChar_TribalRole::Scout:
            CurrentAppearance.BodyBuild = FMath::Clamp(CurrentAppearance.BodyBuild, 0.3f, 0.7f);
            CurrentAppearance.WeatheringLevel = FMath::Max(CurrentAppearance.WeatheringLevel, 0.3f);
            break;
            
        case EChar_TribalRole::Gatherer:
            CurrentAppearance.BodyBuild = FMath::Clamp(CurrentAppearance.BodyBuild, 0.2f, 0.6f);
            CurrentAppearance.WeatheringLevel = FMath::Clamp(CurrentAppearance.WeatheringLevel, 0.1f, 0.5f);
            break;
            
        case EChar_TribalRole::Shaman:
            CurrentAppearance.WeatheringLevel = FMath::Max(CurrentAppearance.WeatheringLevel, 0.5f);
            // Shamans have more elaborate war paint
            CurrentAppearance.WarPaintColor = FLinearColor(0.8f, 0.2f, 0.8f, 1.0f);
            break;
            
        case EChar_TribalRole::Elder:
            CurrentAppearance.WeatheringLevel = FMath::Max(CurrentAppearance.WeatheringLevel, 0.7f);
            CurrentAppearance.HairColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f); // Gray hair
            break;
    }
}